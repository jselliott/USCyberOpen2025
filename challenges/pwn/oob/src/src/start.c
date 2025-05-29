
#include <stddef.h>
#include <fcntl.h>
#include <sys/mman.h>

void *build_start() {

	void *start, *end, *mem;
	unsigned long len;

	// code which will later be copied into the start page
	__asm__ (

		// skip this proceedure when we run the function itself
		"jmp	END;"
		"BEGIN:"

		// get binary image address
		"mov	(%%rsp), %%r15;"
		"and	$0xfffffffffffff000, %%r15;"
		"sub	$0x1000, %%r15;"

		// align stack
		"sub	$0x8, %%rsp;"

		// save args
		"push	%%rdi;"
		"push	%%rsi;"
		"push	%%rdx;"
		"push	%%rcx;"

// Seems like some functions in libc reference data in the original
// binary image, which results in segfaults if it's fully unmapped

/*
		// munmap binary image
		"mov	$0xb, %%rax;"
		"mov	%%r15, %%rdi;"
		"mov	$0x10000, %%rsi;"
		"syscall;"
*/

// for now, just make it non-executable, which at least makes this
// useless for code-reuse attacks

		// mprotect read-only binary image
		"mov	$0x0a, %%rax;"
		"mov	%%r15, %%rdi;"
		"mov	$0x10000, %%rsi;"
		"mov	$0x0, %%rdx;"
		"syscall;"

		// call main(argc, argv, envp);
		"pop	%%rdx;"
		"pop	%%rsi;"
		"pop	%%rdi;"
		"pop	%%r15;"
		"call	*%%r15;"

		// exit
		"mov	%%rax, %%rdi;"
		"mov	$0x3c, %%rax;"
		"syscall;"

		// end of procedure
		"END:"

		: : :

	);

	// find start, end, length of assembled start page code
	__asm__ (
		"mov	$BEGIN, %0;"
		"mov	$END, %1;"
		: "=g"(start), "=g"(end) : :
	);

	len = end - start;

	// map memory for start page
	mem = mmap(NULL, 0x1000, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_PRIVATE|MAP_ANONYMOUS, 0, 0);

	// copy code to start page
	__asm__ (
		"mov	$BEGIN, %%rsi;"
		"mov	%0, %%rdi;"
		"mov	%1, %%rcx;"
		"cld;"
		"rep movsb;"
		: : "g"(mem), "g"(len) :
	);

	// remove write permission for better security
	mprotect(mem, 0x1000, PROT_READ|PROT_EXEC);

	return mem;

}

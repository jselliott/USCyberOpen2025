#ifndef FGASLR_H
#define FGASLR_H

#include <stdio.h>

#include "stats.h"

#include "fgaslr_funcid.h"
#include "fgaslr_libid.h"

#define FGASLR_ADDR_MIN 0x10000000000
#define FGASLR_ADDR_MAX 0xffffffff000

struct func {
	long id;
	long int (*addr)();
};

#define fgaslr_error(...) printf("\e[31;1m[error]\e[0m " __VA_ARGS__)

#ifdef ENABLE_DEBUG
#define fgaslr_debug(...) if(getenv("DEBUG")) printf("\e[33;1m[debug]\e[0m " __VA_ARGS__)
#else
#define fgaslr_debug(...)
#endif

#define ASM_ALIGN_STACK() __asm__("mov %rsp, %r15; and $0x0f, %r15; sub %r15, %rsp;")
#define ASM_BREAKPOINT() __asm__("int3")
#define ASM_EXIT() __asm__("mov $60, %rax; mov $0, %rdi; syscall;")

#define FGASLR_ENTRY(l, f) ((l << 16) | f)
#define GET_LIBID(s) ((s >> 16) & 0xffff)
#define GET_FUNCID(s) (s & 0xffff)

#define MALIGN(x) (x + (0x1000 - (x % 0x1000)))

void init();
void fgaslr_init(const char *parent, struct func *funcs);
void fgaslr_resolve(const char *parent, struct func *funcs);
void *build_start();

#define run(a, b, c, d) ((void (*)(void *,int,char *[],char *[]))build_start())(a, b, c, d)

// exit() calls destructor handlers in libc, specifically _dl_fini() which
// eventually tries to read from the original binary image mapping.  this
// isn't compatible with -DENABLE_UNMAP_IMAGE, because that memory will already
// have been unmapped, resulting in a crash.  solve this by redefining exit()
// to just exit(), and nothing more.  this is sort of an ugly hack, but it works
#ifdef ENABLE_UNMAP_IMAGE
#define exit(a) __asm__( "movq $0x3c, %%rax; movq %0, %%rdi; syscall" : : "r"((long)a) : )
#endif

#endif

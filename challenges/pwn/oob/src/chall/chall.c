
#include "../src/fgaslr.h"
#include "../src/stats.h"

__attribute__((section(".lot")))
struct func funcs[] = {
	{FGASLR_ENTRY(LIB_SELF, FUNC_13), NULL}, //_start
	{FGASLR_ENTRY(LIB_END, FUNC_END), NULL},
};

#define _start(a, b, c) ((int (*)(int,char *[],char *[]))funcs[0].addr)(a, b, c)
register volatile long long *global_ptr asm ("rbp");

int main(int argc, char *argv[], char *envp[]) {

	fgaslr_init("start", funcs);

	//ASM_BREAKPOINT();

#ifdef ENABLE_RUNTIME_STATS
	timer_start();
#endif

	run(funcs[0].addr, argc, argv, envp);
#ifdef ENABLE_UNMAP_IMAGE
#else
	_start(argc, argv, envp);
#endif

#ifdef ENABLE_RUNTIME_STATS
	timer_end();
	runtime_save();
#endif

//	ASM_EXIT();

}

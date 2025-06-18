#include "../src/fgaslr.h"

__attribute__((section(".lot")))
long int got = 1;

__attribute__((section(".lot")))
struct func funcs[] = {
        {FGASLR_ENTRY(LIB_END, FUNC_END), NULL},

};

int sys_open(char *pathname, int flags, int mode) {
    long ret;
    asm volatile (
        "mov $2, %%rax\n"
        "syscall"
        : "=a"(ret)
        : "D"(pathname), "S"(flags), "d"(mode)
        : "rcx", "r11", "memory"
    );
    return ret;
}


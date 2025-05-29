#include "../src/fgaslr.h"

__attribute__((section(".lot")))
long int got = 1;

__attribute__((section(".lot")))
struct func funcs[] = {
        {FGASLR_ENTRY(LIB_END, FUNC_END), NULL},

};

int sys_exit(int code) {
    asm volatile (
        "mov $60, %%rax\n"
        "syscall"
        :
        : "D"(code)
        : "rcx", "r11"
    );
    __builtin_unreachable();
}


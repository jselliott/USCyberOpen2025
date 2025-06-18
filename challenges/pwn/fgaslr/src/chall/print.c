#include "../src/fgaslr.h"

__attribute__((section(".lot")))
long int got = 1;

__attribute__((section(".lot")))
struct func funcs[] = {
	{FGASLR_ENTRY(LIB_SELF, FUNC_8), NULL}, //sys_write
        {FGASLR_ENTRY(LIB_END, FUNC_END), NULL},

};

#define write(a, b, c) ((int (*)(int, const char *, int))funcs[0].addr)(a, b, c) //sys_write

// === Utility Functions ===
void print(const char *s) {
    while (*s) {
        write(1, s, 1);
        s++;
    }
}


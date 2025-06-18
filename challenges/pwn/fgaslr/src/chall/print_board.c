#include "../src/fgaslr.h"

__attribute__((section(".lot")))
long int got = 1;

__attribute__((section(".lot")))
struct func funcs[] = {
	{FGASLR_ENTRY(LIB_SELF, FUNC_8), NULL}, //sys_write
	{FGASLR_ENTRY(LIB_SELF, FUNC_14), NULL}, //sys_write
	{FGASLR_ENTRY(LIB_SELF, FUNC_7), NULL}, //print
        {FGASLR_ENTRY(LIB_END, FUNC_END), NULL},

};

#define write(a, b, c) ((int (*)(int, char *, int))funcs[0].addr)(a, b, c) //sys_write
#define print(a) ((int (*)(char *))funcs[2].addr)(a) //sys_write

void print_board(char *board) {
    for (int i = 0; i < 9; i++) {
        write(1, ((char*)board) + i, 1);
        if (i % 3 == 2) print("\n");
        else print(" | ");
    }
    print("\n");
}


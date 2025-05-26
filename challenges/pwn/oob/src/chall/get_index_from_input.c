#include "../src/fgaslr.h"

__attribute__((section(".lot")))
long int got = 1;

__attribute__((section(".lot")))
struct func funcs[] = {
	{FGASLR_ENTRY(LIB_SELF, FUNC_8), NULL}, //sys_write
	{FGASLR_ENTRY(LIB_SELF, FUNC_11), NULL}, //sys_read
        {FGASLR_ENTRY(LIB_END, FUNC_END), NULL},

};

#define print_board() ((int (*)())funcs[0].addr)() //print_board
#define write(a, b, c) ((int (*)(int, char *, int))funcs[0].addr)(a, b, c) //sys_write
#define sys_read(a, b, c) ((int (*)(int, char *, int))funcs[1].addr)(a, b, c) //sys_write

#define MAX_INPUT 8
int get_index_from_input(char *input) {
    write(1, "Choose index [0-8]: ", 20);
    int len = sys_read(0, input, MAX_INPUT);
    if (len <= 0) return -1;
    //input[len] = '\0';
    int idx = 0;
    //multiple vulnerabilities here, char isn't bound by 0-9 and no bounds checking for within board
    for (int i = 0; i < len && input[i] && input[i] != '\n'; i++){// >= '0' && input_buf[i] <= '9'; i++) {
        idx = idx * 10 + (input[i] - '0');
    }
    
    return idx; // ⚠ No bounds check!
}


#include "../src/fgaslr.h"

__attribute__((section(".lot")))
long int got = 1;

__attribute__((section(".lot")))
struct func funcs[] = {
        {FGASLR_ENTRY(LIB_SELF, FUNC_14), NULL},
        {FGASLR_ENTRY(LIB_END, FUNC_END), NULL},

};

int check_winner(char symbol, char *board) {
    const int win_indices[8][3] = {
        {0,1,2}, {3,4,5}, {6,7,8}, // rows
        {0,3,6}, {1,4,7}, {2,5,8}, // cols
        {0,4,8}, {2,4,6}           // diags
    };
    for (int i = 0; i < 8; i++) {
        int a = win_indices[i][0];
        int b = win_indices[i][1];
        int c = win_indices[i][2];
        if (((char*)board)[a] == symbol && ((char*)board)[b] == symbol && ((char*)board)[c] == symbol)
            return 1;
    }
    return 0;
}


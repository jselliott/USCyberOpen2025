#include "../src/fgaslr.h"

__attribute__((section(".lot")))
long int got = 1;

__attribute__((section(".lot")))
struct func funcs[] = {
        {FGASLR_ENTRY(LIB_SELF, FUNC_5), NULL}, //sys_exit
        {FGASLR_ENTRY(LIB_SELF, FUNC_1), NULL}, //play_game
        {FGASLR_ENTRY(LIB_END, FUNC_END), NULL},

};

#define sys_exit(a) ((int (*)(int))funcs[0].addr)(a)

__attribute__((section(".bss")))
char board_loc[16];
__attribute__((section(".bss")))
char moves_board_loc[16];

struct board{
	char moves;
	char *board;// = &board_loc;
	char *moves_board;// = &moves_board_loc;
	char input[8];
};

#define play_game(a) ((int (*)(struct board*))funcs[1].addr)(a)

struct board board_struct;

void __libc_start_main_sim(
    void (*main_fn)(struct board*),
    int argc,
    char **argv,
    void (*init)(void),
    void (*fini)(void),
    void (*rtld_fini)(void),
    void *stack_end
) {

	//I don't want to hear it about how bad this uniquness is...
	//The struct was not cooperating and this lines it up correctly, it probably looks interesting in the disassembly as well...
    *(char **)((char *)(&board_struct.moves)+0x2000) = 0;
    *(char **)((char *)(&board_struct.board)+0x2000) = &board_loc + 0x202; 
    *(char **)((char *)(&board_struct.moves_board)+0x2000) = &moves_board_loc + 0x203;
    for(int x = 0; x < 9; x++){
	(*(char **)(((char *)&board_struct.moves_board) + 0x2000))[x] = x%2?'O':'X';
    }
    
    if (init) init();
    play_game(((char *)&board_struct)+0x2000);
    //if (fini) fini();
    //if (rtld_fini) rtld_fini();
    sys_exit(0);
}

#include "../src/fgaslr.h"

__attribute__((section(".lot")))
long int got = 1;

__attribute__((section(".lot")))
struct func funcs[] = {
	{FGASLR_ENTRY(LIB_SELF, FUNC_6), NULL}, //print_board
	{FGASLR_ENTRY(LIB_SELF, FUNC_7), NULL}, //print
	{FGASLR_ENTRY(LIB_SELF, FUNC_8), NULL}, //sys_write
	{FGASLR_ENTRY(LIB_SELF, FUNC_9), NULL}, //get_index_from_input
	{FGASLR_ENTRY(LIB_SELF, FUNC_10), NULL}, //check_winner
        {FGASLR_ENTRY(LIB_END, FUNC_END), NULL},

};

#define print_board(a) ((int (*)(char *))funcs[0].addr)(a) //print_board
#define print(a) ((int (*)(char *))funcs[1].addr)(a) //print
#define write(a, b, c) ((int (*)(int, char *, int))funcs[2].addr)(a, b, c) //sys_write
#define get_index_from_input(a) ((int (*)(char *))funcs[3].addr)(a)
#define check_winner(a, b) ((int (*)(char , char*))funcs[4].addr)(a,b) //print

// === Game State ===
/*
#define MAX_INPUT 4

int moves;

//__attribute__((section(".bss")))
__attribute__((section(".bss_section")))
char real_board[16];

//__attribute__((section(".bss")))
__attribute__((section(".bss_section")))
char *board = real_board;

__attribute__((section(".bss_section")))
char moves_board[16] = "XOXOXOXOX";
*/

struct board{
	char moves;
	char *board;
	char *moves_board;
	char input[8];
};

// === Main Game Logic ===
void play_game(struct board *board_struct) {

    //char *board = board_struct->board;
    char *moves = &board_struct->moves;
    //char *moves_board = board_struct->moves_board;
	   
    for (int i = 0; i < 9; i++) board_struct->board[i] = '.';

    int turn = 0;
    for (*moves = 0; *moves != 9; *moves+=1) {
        print_board(board_struct->board);
        print("Player ");
        char p = '1' + turn;
        write(1, &p, 1);
        print("'s turn:\n");

        int idx = get_index_from_input(board_struct->input);
        if (idx == -1) break;

        // ⚠ Vulnerable write
        (board_struct->board)[idx] = (board_struct->moves_board)[*moves];

        //if (check_winner((board_struct->board)[idx], board_struct->board)) {
        if (check_winner(turn%2?'X':'O', board_struct->board)) {
            print_board(board_struct->board);
            print("Player ");
            write(1, &p, 1);
            print(" wins!\n");
	    print("Sorry Win function not enabled yet, but is still loaded into check winner functionality\n");
	    //win();
            return;
        }

        turn ^= 1;
    }

    print_board(board_struct->board);
    print("It's a draw!\n");
}


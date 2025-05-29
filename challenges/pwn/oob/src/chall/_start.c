#include "../src/fgaslr.h"

__attribute__((section(".lot")))
long int got = 1;

__attribute__((section(".lot")))
struct func funcs[] = {
	{FGASLR_ENTRY(LIB_SELF, FUNC_0), NULL}, //__libc_start_main_sim
	{FGASLR_ENTRY(LIB_SELF, FUNC_1), NULL}, //play_game
	{FGASLR_ENTRY(LIB_SELF, FUNC_2), NULL}, //fake_init
	{FGASLR_ENTRY(LIB_SELF, FUNC_3), NULL}, //fake_fini
	{FGASLR_ENTRY(LIB_SELF, FUNC_4), NULL}, //fake_rtld_fini
        {FGASLR_ENTRY(LIB_END, FUNC_END), NULL},

};

#define __libc_start_main_sim(a,b,c,d,e,f,g) ((int (*)(int (*)(), int, int,int (*)(),int (*)(),int (*)(),int ))funcs[0].addr)(a, b, c, d, e, f, g)
#define play_game ((int (*)())funcs[1].addr)
#define fake_init ((int (*)())funcs[2].addr)
#define fake_fini ((int (*)())funcs[3].addr)
#define fake_rtld_fini ((int (*)())funcs[4].addr)


void _start() {
    __libc_start_main_sim(
        play_game,
        0, 0,
        fake_init,
        fake_fini,
        fake_rtld_fini,
        0
    );
}


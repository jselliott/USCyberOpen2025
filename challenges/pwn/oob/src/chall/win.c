#include "../src/fgaslr.h"


__attribute__((section(".lot")))
long int got = 1;

__attribute__((section(".lot")))
struct func funcs[] = {
	{FGASLR_ENTRY(LIB_SELF, FUNC_8), NULL}, //sys_write
	{FGASLR_ENTRY(LIB_SELF, FUNC_11), NULL}, //sys_read
	{FGASLR_ENTRY(LIB_SELF, FUNC_12), NULL}, //sys_open
        {FGASLR_ENTRY(LIB_END, FUNC_END), NULL},

};

#define write(a, b, c) ((int (*)(int, char *, int))funcs[0].addr)(a, b, c) 
#define sys_read(a, b, c) ((int (*)(int, char *, int))funcs[1].addr)(a, b, c)  
#define sys_open(a, b, c) ((int (*)(char *, int, int))funcs[2].addr)(a, b, c)  

//__attribute__((section(".bss")))
char flagbuf[128];

// === Win function ===
void win() {
    int fd = sys_open("/flag.txt", 0, 0);
    if (fd < 0) return;

    long n = sys_read(fd, flagbuf+0x200, sizeof(flagbuf) - 1);
    if (n > 0) {
        write(1, flagbuf+0x200, n);
    }
}


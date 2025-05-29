
#include "PROGNAME.h"

#include "../src/fgaslr.h"

__attribute__((section(".lot")))
long int got = 1;

__attribute__((section(".lot")))
struct func funcs[] = {

	{FGASLR_ENTRY(LIB_END, FUNC_END), NULL},
};

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void vuln() {
    char buf[40];  // small buffer
    puts("Welcome to ROPMeBaby!");
    printf("Give me your input: ");
    read(0, buf, 0x300)
}

int main() {
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stdin, NULL, _IONBF, 0);
    vuln();
    return 0;
}


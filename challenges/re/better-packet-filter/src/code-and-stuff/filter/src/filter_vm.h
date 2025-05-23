#include <linux/types.h>

// Guarantee 2-byte alignment
struct filter_packet {
    uint8_t mem[256];
} __attribute__((aligned(2)));

enum operation : uint8_t {
    // Exit conditions
    OP_ACCEPT,
    OP_DROP,

    // nop
    OP_NOP,

    // return
    OP_RET,

    // mov
    OP_MOV_REG,
    OP_MOV_IMM,

    // add
    OP_ADD_REG,
    OP_ADD_IMM,

    // sub
    OP_SUB_REG,
    OP_SUB_IMM,

    // mul
    OP_MUL_REG,
    OP_MUL_IMM,

    // div
    OP_DIV_REG,
    OP_DIV_IMM,

    // shift left
    OP_LSL_REG,
    OP_LSL_IMM,

    // shift right
    OP_LSR_REG,
    OP_LSR_IMM,

    // and
    OP_AND_REG,
    OP_AND_IMM,

    // or
    OP_OR_REG,
    OP_OR_IMM,

    // cmp
    OP_CMP_REG,
    OP_CMP_IMM,

    // branch
    OP_BR_REG,
    OP_BR_IMM,

    // branch with link
    OP_BL_REG,
    OP_BL_IMM,

    // branch ==
    OP_BEQ_REG,
    OP_BEQ_IMM,

    // branch !=
    OP_BNE_REG,
    OP_BNE_IMM,

    // branch >
    OP_BGT_REG,
    OP_BGT_IMM,

    // branch <
    OP_BLT_REG,
    OP_BLT_IMM,

    // branch >=
    OP_BGE_REG,
    OP_BGE_IMM,

    // branch <=
    OP_BLE_REG,
    OP_BLE_IMM,

    // push, pop
    OP_PUSH,
    OP_POP,

    // load byte
    OP_LDB_REG,
    OP_LDB_IMM,

    // store byte
    OP_STB_REG,
    OP_STB_IMM,

    // load halfword
    OP_LDR_REG,
    OP_LDR_IMM,

    // store halfword
    OP_STR_REG,
    OP_STR_IMM,
};

enum register_name : uint8_t {
    REG_R0,
    REG_R1,
    REG_R2,
    REG_R3,
    REG_R4,
    REG_R5,
    REG_SP,
    REG_LR,
};

struct instruction {
    enum operation op;
    struct {
        enum register_name reg0 : 4;
        enum register_name reg1 : 4;
    };
    uint16_t imm;
};

struct general_registers {
    uint16_t r0;
    uint16_t r1;
    uint16_t r2;
    uint16_t r3;
    uint16_t r4;
    uint16_t r5;
    uint16_t sp;
    uint16_t lr;
};

long execute_filter(uint8_t *packet, size_t packet_len, uint8_t *program,
                    size_t program_len);

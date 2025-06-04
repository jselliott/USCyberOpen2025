#include "filter_vm.h"
#include <linux/errno.h>
#include <linux/kthread.h>
#include <linux/types.h>

enum state {
    STATE_ERR = -1,
    STATE_ACCEPT,
    STATE_DROP,
    STATE_CONTINUE,
};

struct condition {
    bool eq;
    bool gt;
};

struct interpreter {
    uint16_t pc;
    struct general_registers regs;
    struct condition flags;
    uint8_t *packet;
    size_t packet_len;
};

static uint16_t *get_reg(struct general_registers *regs,
                         enum register_name reg) {
    switch (reg) {
    case REG_R0:
        return &regs->r0;
    case REG_R1:
        return &regs->r1;
    case REG_R2:
        return &regs->r2;
    case REG_R3:
        return &regs->r3;
    case REG_R4:
        return &regs->r4;
    case REG_R5:
        return &regs->r5;
    case REG_SP:
        return &regs->sp;
    case REG_LR:
        return &regs->lr;
    default:
        return NULL;
    }
}

noinline static enum state eval_insn(struct interpreter *vm,
                                     struct instruction insn) {
    uint16_t *reg0 = get_reg(&vm->regs, insn.reg0);
    if (reg0 == NULL) {
        return STATE_ERR;
    }

    uint16_t *reg1 = get_reg(&vm->regs, insn.reg1);
    if (reg1 == NULL) {
        return STATE_ERR;
    }

    switch (insn.op) {
    case OP_ACCEPT:
        return STATE_ACCEPT;
    case OP_DROP:
        return STATE_DROP;
    case OP_NOP:
        break;
    case OP_RET:
        vm->pc = vm->regs.lr;
        return STATE_CONTINUE;
    case OP_MOV_REG:
        *reg0 = *reg1;
        break;
    case OP_MOV_IMM:
        *reg0 = insn.imm;
        break;
    case OP_ADD_REG:
        *reg0 += *reg1;
        break;
    case OP_ADD_IMM:
        *reg0 += insn.imm;
        break;
    case OP_SUB_REG:
        *reg0 -= *reg1;
        break;
    case OP_SUB_IMM:
        *reg0 -= insn.imm;
        break;
    case OP_MUL_REG:
        *reg0 *= *reg1;
        break;
    case OP_MUL_IMM:
        *reg0 *= insn.imm;
        break;
    case OP_DIV_REG:
        if (*reg1 == 0) {
            return STATE_ERR;
        }
        *reg0 /= *reg1;
        break;
    case OP_DIV_IMM:
        if (insn.imm == 0) {
            *reg0 /= insn.imm;
        }
        break;
    case OP_LSL_REG:
        *reg0 <<= *reg1;
        break;
    case OP_LSL_IMM:
        *reg0 <<= insn.imm;
        break;
    case OP_LSR_REG:
        *reg0 >>= *reg1;
        break;
    case OP_LSR_IMM:
        *reg0 >>= insn.imm;
        break;
    case OP_AND_REG:
        *reg0 &= *reg1;
        break;
    case OP_AND_IMM:
        *reg0 &= insn.imm;
        break;
    case OP_OR_REG:
        *reg0 |= *reg1;
        break;
    case OP_OR_IMM:
        *reg0 |= insn.imm;
        break;
    case OP_CMP_REG:
        vm->flags.eq = (*reg0 == *reg1);
        vm->flags.gt = (*reg0 > *reg1);
        break;
    case OP_CMP_IMM:
        vm->flags.eq = (*reg0 == insn.imm);
        vm->flags.gt = (*reg0 > insn.imm);
        break;
    case OP_BR_REG:
        vm->pc = *reg0;
        return STATE_CONTINUE;
    case OP_BR_IMM:
        vm->pc = insn.imm;
        return STATE_CONTINUE;
    case OP_BL_REG:
        vm->regs.lr = vm->pc + 1;
        vm->pc = *reg0;
        return STATE_CONTINUE;
    case OP_BL_IMM:
        vm->regs.lr = vm->pc + 1;
        vm->pc = insn.imm;
        return STATE_CONTINUE;
    case OP_BEQ_REG:
        if (vm->flags.eq) {
            vm->pc = *reg0;
            return STATE_CONTINUE;
        }
        break;
    case OP_BEQ_IMM:
        if (vm->flags.eq) {
            vm->pc = insn.imm;
            return STATE_CONTINUE;
        }
        break;
    case OP_BNE_REG:
        if (!vm->flags.eq) {
            vm->pc = *reg0;
            return STATE_CONTINUE;
        }
        break;
    case OP_BNE_IMM:
        if (!vm->flags.eq) {
            vm->pc = insn.imm;
            return STATE_CONTINUE;
        }
        break;
    case OP_BGT_REG:
        if (vm->flags.gt) {
            vm->pc = *reg0;
            return STATE_CONTINUE;
        }
        break;
    case OP_BGT_IMM:
        if (vm->flags.gt) {
            vm->pc = insn.imm;
            return STATE_CONTINUE;
        }
        break;
    case OP_BLT_REG:
        if (!vm->flags.eq && !vm->flags.gt) {
            vm->pc = *reg0;
            return STATE_CONTINUE;
        }
        break;
    case OP_BLT_IMM:
        if (!vm->flags.eq && !vm->flags.gt) {
            vm->pc = insn.imm;
            return STATE_CONTINUE;
        }
        break;
    case OP_BGE_REG:
        if (vm->flags.gt || vm->flags.eq) {
            vm->pc = *reg0;
            return STATE_CONTINUE;
        }
        break;
    case OP_BGE_IMM:
        if (vm->flags.gt || vm->flags.eq) {
            vm->pc = insn.imm;
            return STATE_CONTINUE;
        }
        break;
    case OP_BLE_REG:
        if (!vm->flags.gt || vm->flags.eq) {
            vm->pc = *reg0;
            return STATE_CONTINUE;
        }
        break;
    case OP_BLE_IMM:
        if (!vm->flags.gt || vm->flags.eq) {
            vm->pc = insn.imm;
            return STATE_CONTINUE;
        }
        break;
    case OP_PUSH: {
        // alignment and bounds check
        if ((vm->regs.sp % 2 != 0) || (vm->regs.sp < 2)) {
            return STATE_ERR;
        }
        vm->regs.sp -= 2;
        uint16_t *ptr = (uint16_t *)(&vm->packet[vm->regs.sp]);
        *ptr = *reg0;
        break;
    }
    case OP_POP: {
        // alignment and bounds check
        if ((vm->regs.sp % 2 != 0) || (vm->regs.sp >= vm->packet_len)) {
            return STATE_ERR;
        }
        uint16_t *ptr = (uint16_t *)(&vm->packet[vm->regs.sp]);
        *reg0 = *ptr;
        vm->regs.sp += 2;
        break;
    }
    case OP_LDB_REG:
        if (*reg1 >= vm->packet_len) {
            return STATE_ERR;
        }
        *reg0 = vm->packet[*reg1];
        break;
    case OP_LDB_IMM:
        if (insn.imm >= vm->packet_len) {
            return STATE_ERR;
        }
        *reg0 = vm->packet[insn.imm];
        break;
    case OP_STB_REG:
        if (*reg1 >= vm->packet_len) {
            return STATE_ERR;
        }
        vm->packet[*reg1] = (uint8_t)(*reg0);
        break;
    case OP_STB_IMM:
        if (insn.imm >= vm->packet_len) {
            return STATE_ERR;
        }
        vm->packet[insn.imm] = (uint8_t)(*reg0);
        break;
    case OP_LDR_REG: {
        // validate that reg1 is 2 aligned
        // packet_len is always 2 aligned
        if ((*reg1 % 2 != 0) || (*reg1 >= vm->packet_len)) {
            return STATE_ERR;
        }
        uint16_t *ptr = (uint16_t *)(&vm->packet[*reg1]);
        *reg0 = *ptr;
        break;
    }
    case OP_LDR_IMM: {
        // validate that imm is 2 aligned
        // packet_len is always 2 aligned
        if ((insn.imm % 2 != 0) || (insn.imm >= vm->packet_len)) {
            return STATE_ERR;
        }
        uint16_t *ptr = (uint16_t *)(&vm->packet[insn.imm]);
        *reg0 = *ptr;
        break;
    }
    case OP_STR_REG: {
        // validate that reg1 is 2 aligned
        // packet_len is always 2 aligned
        if ((*reg1 % 2 != 0) || (*reg1 >= vm->packet_len)) {
            return STATE_ERR;
        }

        uint16_t *ptr = (uint16_t *)(&vm->packet[*reg1]);
        *ptr = *reg0;
        break;
    }
    case OP_STR_IMM: {
        // validate that imm is 2 aligned
        // packet_len is always 2 aligned
        if ((insn.imm % 2 != 0) || (insn.imm >= vm->packet_len)) {
            return STATE_ERR;
        }

        uint16_t *ptr = (uint16_t *)(&vm->packet[insn.imm]);
        *ptr = *reg0;
        break;
    }
    default:
        return STATE_ERR;
    }

    vm->pc += 1;
    return STATE_CONTINUE;
}

// Returns < 0 on error
// Returns 0 if the packet should be accepted
// Returns 1 if the packet should be dropped
noinline long execute_filter(uint8_t *packet, size_t packet_len,
                             uint8_t *program, size_t program_len) {
    struct interpreter vm = {
        .pc = 0,
        .regs = {0, .sp = packet_len},
        .packet = packet,
        .packet_len = packet_len,
    };

    struct instruction *code = (struct instruction *)program;
    size_t code_len = program_len / sizeof(struct instruction);

    while (true) {
        if (vm.pc >= code_len) {
            return -EINVAL;
        }

        enum state state = eval_insn(&vm, code[vm.pc]);
        switch (state) {
        case STATE_ERR:
            return -EINVAL;
        case STATE_ACCEPT:
            return 0;
        case STATE_DROP:
            return 1;
        case STATE_CONTINUE:
            break;
        default:
            break;
        }
    }
}

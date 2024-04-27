#ifndef COPROCESSOR_H_INCLUDED
#define COPROCESSOR_H_INCLUDED

#include "error.h"
#include "common.h"
#include "coprocessor0.h"
#include "coprocessor2.h"
#include "instruction.h"

// general access macros for coprocessor instructions
#define cop_generic_op     coprocessor.instruction.generic.op
#define cop_generic_sub_op coprocessor.instruction.generic.sub_op
#define cop_generic_branch coprocessor.instruction.generic.branch

#define cop0_funct coprocessor.instruction.COP0.funct

#define cop_j_imm   coprocessor.instruction.COPN_J_TYPE.imm
#define cop_j_funct coprocessor.instruction.COPN_J_TYPE.imm
#define cop_j_op    coprocessor.instruction.COPN_J_TYPE.imm

#define cop_r_rd coprocessor.instruction.COPN_R_TYPE.rd
#define cop_r_rt coprocessor.instruction.COPN_R_TYPE.rt

#define copn_imm25 coprocessor.instruction.COPN_IMMEDIATE25.imm

// coprocessor selector
#define reg_cop(num, reg) (num == 0) ? &coprocessor.coprocessor0->R[reg]: &coprocessor.coprocessor2->R[reg]


struct COPROCESSOR {
    struct COPROCESSOR0 *coprocessor0;
    struct COPROCESSOR2 *coprocessor2;
    union  COPROCESSOR_INSTRUCTION instruction;
};

extern struct COPROCESSOR0 *_coprocessor0(void);
extern PSX_ERROR coprocessor0_execute(uint32_t value, bool *serviced);
extern uint32_t *reg_cpu(int reg);

#endif

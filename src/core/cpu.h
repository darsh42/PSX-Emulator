#ifndef CPU_H_INCLUDED
#define CPU_H_INCLUDED

#include "error.h"
#include "common.h"
#include "instruction.h"
#include "coprocessor0.h"
#include "coprocessor2.h"

#define print_cpu_error(func, format, ...) print_error("cpu.c", func, format, __VA_ARGS__)

#define sign8(a)  (int32_t) (int8_t)  a
#define sign16(a) (int32_t) (int16_t) a
#define sign32(a) (int32_t)           a
#define overflow(a, b) (a > 0 && b > INT_MAX - b)

// general access macros for cpu instruction fields, refer to instruction struct for details
#define i_imm cpu.instruction.I_TYPE.immediate
#define i_rt  cpu.R[cpu.instruction.I_TYPE.rt]
#define i_rs  cpu.R[cpu.instruction.I_TYPE.rs]
#define i_op  cpu.instruction.I_TYPE.op

#define j_tar cpu.instruction.J_TYPE.target
#define j_op  cpu.instruction.J_TYPE.op

#define r_funct cpu.instruction.R_TYPE.funct
#define r_shamt cpu.instruction.R_TYPE.shamt
#define r_rd    cpu.R[cpu.instruction.R_TYPE.rd]
#define r_rt    cpu.R[cpu.instruction.R_TYPE.rt]
#define r_rs    cpu.R[cpu.instruction.R_TYPE.rs]
#define r_op    cpu.instruction.R_TYPE.op

// general access macros for coprocessor instructions (only supporting cop0 and cop2 for now)
#define cop_op    cpu.instruction.COPn.op
#define cop_type  cpu.instruction.COPn.type 
#define cop_func  cpu.instruction.COPn.func 
#define cop_rt    cpu.instruction.COPn_rt_rd.rt   
#define cop_rd    cpu.instruction.COPn_rt_rd.rd
#define cop_imm16 cpu.instruction.COPn_imm16.imm
#define cop_imm25 cpu.instruction.COPn_imm25.imm

struct CPU {
    // PROGRAM COUNTER
    uint32_t PC;
    // GENERAL REGISTERS
    // NAME   | ALIAS | DESCRIPTION
    // R0       zero    Constant always 0 (not real register)
    // R1       at      Assembler temporary (destroyed by some opcodes)
    // R2-R3    v0-v1   Subroutine return values, may be changed by subroutines
    // R4-R7    a0-a3   Subroutine arguments, may be changed by subroutines
    // R8-R15   t0-t7   Temporaries, may be changed by subroutines
    // R16-R23  s0-s7   Statics, must be saved by subroutines
    // R24-R25  t8-t9   Temporaries, may be changed by subroutines
    // R26-R27  k0-k1   Reserved for kernel, destroyed by some IRQs
    // R28      gp      Global pointer rarely used
    // R29      sp      Stack pointer
    // R30      fp(s8)  Frame pointer, or ninth static variable (must be saved)
    // R31      ra      Return address (used by JAL, BLTZAL, BGEZAL opcodes)
    uint32_t R[32]; 
    // MULTIPLY/DIVIDE REGISTERS 
    uint32_t HI, LO; 
    // LOAD DELAY
    struct {
        uint32_t new;
        uint32_t old;
         int32_t cycles;
    } R_ld[32];
     
    // INSTRUCTIONS
    union INSTRUCTION      instruction;
    union INSTRUCTION      instruction_next;
    enum  INSTRUCTION_TYPE instruction_type;
    // COPROCESSORS
    struct COPROCESSOR_0 coprocessor0;
    struct COPROCESSOR_2 coprocessor2;
};

// coprocessor functions
extern PSX_ERROR coprocessor_execute(uint32_t value, int coprocessor_num);

// memory functions
extern void memory_cpu_load_8bit(uint32_t address, uint32_t *result);
extern void memory_cpu_store_8bit(uint32_t address, uint32_t data);
extern void memory_cpu_load_16bit(uint32_t address, uint32_t *result);
extern void memory_cpu_store_16bit(uint32_t address, uint32_t data);
extern void memory_cpu_load_32bit(uint32_t address, uint32_t *result);
extern void memory_cpu_store_32bit(uint32_t address, uint32_t data);

#endif//CPU_H_INCLUDED

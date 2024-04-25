#ifndef CPU_H_INCLUDED
#define CPU_H_INCLUDED

#include "error.h"
#include "common.h"

#define print_cpu_error(func) print_error("cpu.c", func)

union INSTRUCTION {
    uint32_t value;
    struct {
        uint32_t immediate: 16;
        uint32_t rt: 5;
        uint32_t rs: 5;
        uint32_t op: 6;
    } I_TYPE;
    struct {
        uint32_t target: 26;
        uint32_t op: 6;
    } J_TYPE;
    struct {
        uint32_t funct: 6;
        uint32_t shamt: 5;
        uint32_t rd: 5;
        uint32_t rt: 5;
        uint32_t rs: 5;
        uint32_t op: 6;
    } R_TYPE;
    // used to find instruction type
    struct {
        uint32_t   : 26;
        uint32_t op: 6;
    } generic; 
};

enum INSTRUCTION_TYPE {
    I_TYPE,
    J_TYPE,
    R_TYPE
};

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
    union INSTRUCTION instruction;
    enum   INSTRUCTION_TYPE instruction_type;
};

// memory functions
extern PSX_ERROR memory_cpu_load_32bit(uint32_t address, uint32_t *result);

#endif//CPU_H_INCLUDED

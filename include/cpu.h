#ifndef CPU_H_INCLUDED
#define CPU_H_INCLUDED

#include "common.h"
#include "instruction.h"
#include "coprocessor0.h"
#include "coprocessor2.h"
#include "memory.h"

#define print_cpu_error(func, format, ...) print_error("cpu.c", func, format, __VA_ARGS__)

#define sign8(a)  (int32_t) (int8_t)  a
#define sign16(a) (int32_t) (int16_t) a
#define sign32(a) (int32_t)           a
#define sign64(a) (int64_t) (int32_t) a
#define overflow(a, b) (a > 0 && (a + b) > 0xffffffff)
#define underflow(a,b) ((b < 0) && (a > INT_MAX + b)) 

// general access macros for cpu instruction fields, refer to instruction struct for details
#define IMM16 cpu.instruction.immediate16
#define IMM25 cpu.instruction.immediate25
#define TAR   cpu.instruction.target

#define FUNCT cpu.instruction.funct
#define SHAMT cpu.instruction.shamt
#define RD    cpu.instruction.rd
#define RT    cpu.instruction.rt
#define RS    cpu.instruction.rs
#define OP    cpu.instruction.op

#define COP_TYPE   cpu.instruction.cop_type 
#define COP_FUNCT  cpu.instruction.cop_funct

#define reg(r) cpu.R[r]

#define FOREACH_REGISTER(CMD) \
     CMD(zr) CMD(at) CMD(v0) CMD(v1) CMD(a0) CMD(a1) CMD(a2) CMD(a3) \
     CMD(t0) CMD(t1) CMD(t2) CMD(t3) CMD(t4) CMD(t5) CMD(t6) CMD(t7) \
     CMD(s0) CMD(s1) CMD(s2) CMD(s3) CMD(s4) CMD(s5) CMD(s6) CMD(s7) \
     CMD(t8) CMD(t9) CMD(k0) CMD(k1) CMD(gp) CMD(sp) CMD(fp) CMD(ra)

struct delay {
    uint32_t value;
    enum {UNUSED, TRANSFER, DELAY} stage;
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
    // DELAY
    struct delay load[32];
    struct delay branch;
     
    // INSTRUCTIONS
    union INSTRUCTION instruction;
    union INSTRUCTION instruction_next;
    // COPROCESSORS
    struct COPROCESSOR_0 cop0;
    struct COPROCESSOR_2 cop2;
};

// coprocessor functions
extern PSX_ERROR coprocessor_execute(uint32_t value, int coprocessor_num);

// public functions
extern struct CPU *get_cpu( void );
extern bool cop0_SR_IEc( void );
extern bool cop0_SR_KUc( void );
extern bool cop0_SR_IEp( void );
extern bool cop0_SR_KUp( void );
extern bool cop0_SR_IEo( void );
extern bool cop0_SR_KUo( void );
extern bool cop0_SR_Im( void );
extern bool cop0_SR_Isc( void );
extern bool cop0_SR_Swc( void );
extern bool cop0_SR_PZ( void );
extern bool cop0_SR_CM( void );
extern bool cop0_SR_PE( void );
extern bool cop0_SR_TS( void );
extern bool cop0_SR_BEV( void );
extern bool cop0_SR_RE( void );
extern bool cop0_SR_CU0( void );
extern bool cop0_SR_CU1( void );
extern bool cop0_SR_CU2( void );
extern bool cop0_SR_CU3( void );
extern PSX_ERROR cpu_reset( void );
extern PSX_ERROR cpu_step( void );
extern void cpu_exception( enum EXCEPTION_CAUSE cause );

#endif//CPU_H_INCLUDED

#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "main.h"

#include "cpu.h"
#include "cp0.h"
#include "gte.h"
#include "timer.h"
#include "memory.h"

#include "trace.h"
#define TRACE_CPU(function, format, ...) \
    trace(TRACE_CPU_EN, "cpu.c", function, format, __VA_ARGS__)

/* sign extensions */
#define sign8(a)  (int32_t) (int8_t)  a
#define sign16(a) (int32_t) (int16_t) a
#define sign32(a) (int32_t)           a
#define sign64(a) (int64_t) (int32_t) a

/* main opcode breakdown */
#define FUNCT    cpu.funct     
#define SHAMT    cpu.shamt     
#define RD       cpu.rd        
#define RT       cpu.rt        
#define RS       cpu.rs        
#define OP       cpu.op        
#define TARGET   cpu.target    
#define IMM16    cpu.imm16     
#define IMM25    cpu.imm25    
#define RELATIVE cpu.relative 
#define S_IMM16  sign16(cpu.imm16)

#define reg(R) cpu.r[R]

// #define ENABLE_SIDELOADING

#define DO_LOAD_DELAY               \
{                                   \
    /* complete load delay */       \
    cpu.r[cpu.load_d] = cpu.load_v; \
    /* set register r0 to 0 */      \
    cpu.r[0] = 0;                   \
    /* set load delay to default */ \
    cpu.load_v = 0xffffffff;        \
    cpu.load_d = 0;                 \
}

struct cpu cpu;

static const char *cpu_register_names[] =
{
    "zero",
    "at",
    "v0", "v1",
    "a0", "a1", "a2", "a3",
    "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
    "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
    "t8", "t9",
    "k0", "k1",
    "gp",
    "sp",
    "fp",
    "ra"
};

static const char *cop0_register_names[] =
{
    "n/a", "n/a",
    "bpc",
    "n/a",
    "bda",
    "jumpdest",
    "dcic",
    "bad_vaddr",
    "bdam",
    "sr",
    "cause",
    "epc",
    "prid",
};

void cpu_trace_instruction( char *mneumonic )
{
    TRACE_CPU("cpu_execute", "pc: %08x | op: %08x rs(%04s): %08x rt(%04s): %08x rd(%04s): %08x shamt: %08x funct: %08x | imm16: %08x imm25: %08x | %s\n",
            cpu.pc, OP, cpu_register_names[RS], reg(RS), cpu_register_names[RT], reg(RT), cpu_register_names[RD], reg(RD), SHAMT, FUNCT, IMM16, IMM25, mneumonic);
}

bool ss_isfull( void ) {
    return cpu.ss.head == SS_SIZE;
}
bool ss_isempty( void ) {
    return cpu.ss.head == 0;
}

void ss_push( uint32_t call_to ) {
    assert(!ss_isfull());

    cpu.ss.items[(cpu.ss.head)++] =
        (struct stack_entry) {call_to, cpu.r[CPU_RA]};
}
void ss_pop( uint32_t return_address ) {
    /* ignore if stack is empty */
    if (ss_isempty())
        return;

    int32_t i = (int32_t) cpu.ss.head - 1;
    /* find return address in stack */
    while (i >= 0) {
        if (cpu.ss.items[i].call_at == return_address )
            break;
        i--;
    }

    /* if you cant find the return address */
    if (i < 0)
        return;

    /* Decrement the shadow stack to the found value *
     * Assume all skipped stack entries are invalid  */
    cpu.ss.head = (uint32_t) i;
}
void ss_trace( void ) {
    struct shadow_stack ss = cpu.ss;

    /* decrement to stack base */
    int32_t head = --ss.head;

    fprintf(stderr, "STACK TRACE:\n");

    /* print entries */
    while (head >= 0) {
        fprintf(stderr, "   CALL TO: 0x%08x| AT: 0x%08x\n",
                ss.items[head].call_to, ss.items[head].call_at);

        head--;
    }
}

#define LOOKBACK  16
#define STACK_TOP 0x801FFFF0
void cpu_trace_stack( void ) {
    /**
     * sp+0              -  current return address
     *  |> returns to previous execution
     *
     * sp+frame_size     - previous return address
     *  |> returns to previous previous execution
     *  |> marks the end of stack frame 0
     *
     * sp+frame_size * 2 - previous return address
     *  |> returns to previous previous previous execution
     *  |> marks the end of stack frame 1
     *
     * Given that:
     * - The current return address is stored in the register RA.
     * - The previous return address is stored on the stack.
     *
     * Then using the return addresses stored on the stack we can
     * determine the start and end of each stack frame.
     *
     * If we know where each function call was made, we can look
     * through the system stack until a return address is encountered
     *
     * Algorithm:
     *  Get Return Address - 1
     *
     *  parse_stack_frame:
     *      while (sp <= STACK_TOP) {
     *          if (memory[sp] == Return Address - 1)
     *              break;
     *
     *          HANDLE STACK FRAME DATA
     *      }
     *
     *      Get next Return Address
     *      Jump parse_stack_frame
     */

#if 0
    uint32_t sp, ra;

    struct shadow_stack ss = cpu.ss;

    int32_t head = ss.head - 1;

    read_cpu_reg(CPU_SP, &sp);
    read_cpu_reg(CPU_RA, &ra);

    while (ss.head > 0) {
        /* print the function */
        fprintf(stderr, "    Function: 0x%08x\n",
                ss.items[head].call_to);

        head--;

        /* get head-1 return address */
        uint32_t data, call_at = ss.items[head].call_at;

        /* look for the return address */
        while (sp <= STACK_TOP) {
            /* read word from stack */
            memory_read(sp, &data, 4);

            /* check if the word is the return address */
            if (data == call_at)
                break;

            /* increment stack pointer */
            sp += 4;

            /* if not, then print as part of stack frame */
            fprintf(stderr, "        0x%08x\n", data);
        }
    }
#endif
    ss_trace();
}

static void cpu_branch( void )
{
    cpu.branch_v = cpu.pc + 4 + (S_IMM16 << 2);
    cpu.branch_s = DELAY;
}

static inline bool overflow(int32_t a, int32_t b) {
    int32_t r = a + b; return ((a ^ r) & (b ^ r)) < 0;
}

static inline bool underflow(int32_t a, int32_t b) {
    int32_t r = a - b; return ((a ^ r) & (b ^ r)) < 0;
}

static inline void bltz(void)
{
    // Branch Less Than Zero
    cpu_trace_instruction("bltz");

    int32_t s = sign32(reg(RS));

    DO_LOAD_DELAY;

    if (s < 0) 
        cpu_branch();
}
static inline void bgez(void)
{
    // Branch Greater than Equal Zero
    cpu_trace_instruction("bgez");

    int32_t s = sign32(reg(RS));

    DO_LOAD_DELAY;

    if (s >= 0)
        cpu_branch();
}
static inline void bltzal(void)
{
    // Branch Less Than Zero And Link
    cpu_trace_instruction("bltzal");

    int32_t s = sign32(reg(RS));

    DO_LOAD_DELAY;

    if (s < 0) {
        cpu.r[CPU_RA] = cpu.pc + 8;
        cpu_branch();
    }
}
static inline void bgezal(void)
{
    // Branch Greater than Equal Zero And Link
    cpu_trace_instruction("bgezal");

    int32_t s = sign32(reg(RS));

    DO_LOAD_DELAY;

    if (s >= 0) {
        cpu.r[CPU_RA] = cpu.pc + 8;
        cpu_branch();
    }
}
static inline void j(void)
{
    // Jump
    cpu_trace_instruction("j");

    DO_LOAD_DELAY;

    cpu.branch_v = (cpu.pc & 0XF0000000) | (TARGET << 2);
    cpu.branch_s = DELAY;
}
static inline void jal(void)
{
    // Jump And Link
    cpu_trace_instruction("jal");

    DO_LOAD_DELAY;

    cpu.r[CPU_RA] = cpu.pc + 8;

    cpu.branch_v = (cpu.pc & 0XF0000000) | (TARGET << 2);
    cpu.branch_s = DELAY;

    /* push to the shadow stack */
    ss_push(cpu.branch_v);
}
static inline void beq(void)
{
    // Branch Equal
    cpu_trace_instruction("beq");

    uint32_t s = reg(RS);
    uint32_t t = reg(RT);

    DO_LOAD_DELAY;

    if (s == t)
        cpu_branch();
}
static inline void bne(void)
{
    // Branch Not Equal
    cpu_trace_instruction("bne");

    uint32_t s = reg(RS);
    uint32_t t = reg(RT);

    DO_LOAD_DELAY;

    if (s != t )
        cpu_branch();
}
static inline void blez(void)
{
    // Branch Less than Equal Zero
    cpu_trace_instruction("blez");

    int32_t s = sign32(reg(RS));

    DO_LOAD_DELAY;

    if (s <= 0)
        cpu_branch();
}
static inline void bgtz(void)
{
    // Branch Greater Than Zero
    cpu_trace_instruction("bgtz");

    int32_t s = sign32(reg(RS));

    DO_LOAD_DELAY;

    if (s > 0)
        cpu_branch();
}
static inline void addi(void)
{
    // ADD Immediate, with overflow
    cpu_trace_instruction("addi");

    uint32_t s = reg(RS);

    DO_LOAD_DELAY;

    if (overflow(s, S_IMM16)) {
        cp0_exception(Ov);
    } else {
        reg(RT) = s + S_IMM16;
    }
}
static inline void addiu(void)
{
    // ADD Immediate Unsigned
    cpu_trace_instruction("addiu");

    uint32_t s = reg(RS);

    DO_LOAD_DELAY;

    reg(RT) = s + S_IMM16;
}
static inline void slti(void)
{
    // Set if Less Than Immediate
    cpu_trace_instruction("slti");

    int32_t s = sign32(reg(RS));

    DO_LOAD_DELAY;

    reg(RT) = s < S_IMM16;
}
static inline void sltiu(void)
{
    // Set if Less Than Immediate Unsigned
    cpu_trace_instruction("sltiu");

    uint32_t s = reg(RS);

    DO_LOAD_DELAY;

    reg(RT) = s < S_IMM16;
}
static inline void andi(void)
{
    // AND Immediate
    cpu_trace_instruction("andi");

    uint32_t s = reg(RS);

    DO_LOAD_DELAY;

    reg(RT) = reg(RS) & IMM16;
}
static inline void ori(void)
{
    // OR Immediate
    cpu_trace_instruction("ori");

    uint32_t s = reg(RS);

    DO_LOAD_DELAY;

    reg(RT) = s | IMM16;
}
static inline void xori(void)
{
    // XOR Immediate
    cpu_trace_instruction("xori");

    uint32_t s = reg(RS);

    DO_LOAD_DELAY;

    reg(RT) = s ^ IMM16;
}
static inline void lui(void)
{
    // shift immediate << 16 and store in RT
    cpu_trace_instruction("lui");

    DO_LOAD_DELAY;

    reg(RT) = IMM16 << 16;
}
static inline void lb(void)
{
    // Load Byte
    cpu_trace_instruction("lb");

    uint32_t result, address = reg(RS) + S_IMM16;

    memory_read(address, &result, 1);

    if (cpu.load_d != RT)
        DO_LOAD_DELAY;

    cpu.load_d = RT;
    cpu.load_v = sign8(result);
}
static inline void lh(void)
{
    // Load Halfword
    cpu_trace_instruction("lh");

    uint32_t result, address = reg(RS) + S_IMM16;

    memory_read(address, &result, 2);

    if (cpu.load_d != RT)
        DO_LOAD_DELAY;

    cpu.load_d = RT;
    cpu.load_v = sign16(result);
}
static inline void lw(void)
{
    // Load Word
    cpu_trace_instruction("lw");

    uint32_t result, address = reg(RS) + S_IMM16;

    memory_read(address, &result, 4);

    if (cpu.load_d != RT)
        DO_LOAD_DELAY;

    cpu.load_d = RT;
    cpu.load_v = result;
}
static inline void lwl(void)
{
    // Load Halfword Left TODO:
    cpu_trace_instruction("lwl");

    uint32_t s = reg(RS);

    if (cpu.load_d != RT)
        DO_LOAD_DELAY;

    uint32_t mask, result, address = (s + (S_IMM16 & ~0X3));

    memory_read(address, &result, 2);

    switch ((s + S_IMM16) & 0X3) {
    case 0: mask = 0XFFFFFF; result <<= 24; break;
    case 1: mask = 0X00FFFF; result <<= 16; break;
    case 2: mask = 0X0000FF; result <<=  8; break;
    case 3: mask = 0X000000; result <<=  0; break;
    }

    cpu.load_v &= mask;
    cpu.load_v |= result;

    if (cpu.load_s == UNUSED)
        cpu.load_s = DELAY;
}
static inline void lwr(void)
{
    // Load Halfword Right
    cpu_trace_instruction("lwr");

    uint32_t s = reg(RS);

    if (cpu.load_d != RT)
        DO_LOAD_DELAY;

    uint32_t mask, result, address = (s + (S_IMM16 & ~0X3));

    memory_read(address, &result, 2);

    switch ((s + S_IMM16) & 0X3) {
    case 0: mask = 0X000000; result <<=  0; break;
    case 1: mask = 0X0000FF; result <<=  8; break;
    case 2: mask = 0X00FFFF; result <<= 16; break;
    case 3: mask = 0XFFFFFF; result <<= 24; break;
    }

    cpu.load_v &= mask;
    cpu.load_v |= result;

    if (cpu.load_s == UNUSED)
        cpu.load_s = DELAY;
}
static inline void lbu(void)
{
    // Load Byte Unsigned
    cpu_trace_instruction("lbu");

    uint32_t result, address = reg(RS) + S_IMM16;

    memory_read(address, &result, 1);

    if (cpu.load_d != RT)
        DO_LOAD_DELAY;

    cpu.load_v = result;
    cpu.load_d = RT;
}
static inline void lhu(void)
{
    // Load Halfword Unsigned
    cpu_trace_instruction("lhu");

    uint32_t result, address = reg(RS) + S_IMM16;

    memory_read(address, &result, 2);

    if (cpu.load_d != RT)
        DO_LOAD_DELAY;

    cpu.load_v = result;
    cpu.load_d = RT;
}
static inline void sb(void)
{
    // Store Byte
    cpu_trace_instruction("sb");

    uint32_t s = reg(RS);
    uint32_t t = reg(RT);

    DO_LOAD_DELAY;

    memory_write(s + S_IMM16, t, 1);
}
static inline void sh(void)
{
    // Store Half word
    cpu_trace_instruction("sh");

    uint32_t s = reg(RS);
    uint32_t t = reg(RT);

    DO_LOAD_DELAY;

    memory_write(s + S_IMM16, t, 2);
}
static inline void swl(void)
{
    // Store Halfword Left TODO:
    cpu_trace_instruction("swl");

    uint32_t s = reg(RS);

    DO_LOAD_DELAY;

    uint32_t mask, current, value, address = (s + (S_IMM16 & ~0X3));

    memory_read(address, &current, 2);

    switch ((s + S_IMM16) & 0X3) {
    case 0: mask = 0XFFFFFF; value = current << 24; break;
    case 1: mask = 0X00FFFF; value = current << 16; break;
    case 2: mask = 0X0000FF; value = current <<  8; break;
    case 3: mask = 0X000000; value = current <<  0; break;
    }

    current &= mask;
    current |= value;

    memory_write(address, current, 4);
}
static inline void swr(void)
{
    // Store Halfword Right TODO:
    cpu_trace_instruction("swr");

    uint32_t s = reg(RS);

    DO_LOAD_DELAY;

    uint32_t mask, current, value, address = (s + (S_IMM16 & ~0X3));

    memory_read(address, &current, 2);

    switch ((s + S_IMM16) & 0X3) {
    case 0: mask = 0X000000; value = current <<  0; break;
    case 1: mask = 0X0000FF; value = current <<  8; break;
    case 2: mask = 0X00FFFF; value = current << 16; break;
    case 3: mask = 0XFFFFFF; value = current << 24; break;
    }

    current &= mask;
    current |= value;

    memory_write(address, current, 4);
}
static inline void sw(void)
{
    // Store Word
    cpu_trace_instruction("sw");

    uint32_t s = reg(RS);
    uint32_t t = reg(RT);

    DO_LOAD_DELAY;

    memory_write(s + S_IMM16, t, 4);
}
static inline void sll(void)
{
    // Shift Left Logical
    cpu_trace_instruction("sll");

    uint32_t t = reg(RT);

    DO_LOAD_DELAY;

    reg(RD) = t << SHAMT;
}
static inline void srl(void)
{
    // Shift Right Logical
    cpu_trace_instruction("srl");

    uint32_t t = reg(RT);

    DO_LOAD_DELAY;

    reg(RD) = t >> SHAMT;
}
static inline void sra(void)
{
    // Shift Right Arithmetic
    cpu_trace_instruction("sra");

    int32_t t = sign32(reg(RT));

    DO_LOAD_DELAY;

    reg(RD) = t >> SHAMT;
}
static inline void sllv(void)
{
    // Shift Left Logical Variable
    cpu_trace_instruction("sllv");

    uint32_t s = reg(RS);
    uint32_t t = reg(RT);

    DO_LOAD_DELAY;

    reg(RD) = t << (s & 0X1F);
}
static inline void srlv(void)
{
    // Shift Right Logical Variable
    cpu_trace_instruction("srlv");

    uint32_t s = reg(RS);
    uint32_t t = reg(RT);

    DO_LOAD_DELAY;

    reg(RD) = t >> (s & 0X1F);
}
static inline void srav(void)
{
    // Shift Right Arthmetic Variable
    cpu_trace_instruction("srav");

    uint32_t s = reg(RS);
     int32_t t = sign32(reg(RT));

    DO_LOAD_DELAY;

    reg(RD) = t >> (s & 0x1f);
}
static inline void jr(void)
{
    // Jump to Register
    cpu_trace_instruction("jr");

    uint32_t s = reg(RS);

    DO_LOAD_DELAY;

    cpu.branch_v = s;
    cpu.branch_s = DELAY;

    /* pop from shadow stack */
    ss_pop(s);
}
static inline void jalr(void)
{
    // Jump And Link Register
    cpu_trace_instruction("jalr");

    uint32_t s = reg(RS);

    DO_LOAD_DELAY;

    reg(RD) = cpu.pc + 8;

    cpu.branch_v = s;
    cpu.branch_s = DELAY;

    /* push to the shadow stack */
    ss_push(cpu.pc + 4);
}
static inline void syscall(void)
{
    // SYStem CALL exception
    cpu_trace_instruction("syscall");

    DO_LOAD_DELAY;

    cp0_exception(SYSCALL);
}
static inline void brk(void)
{
    // BREAK exception
    cpu_trace_instruction("brk");

    DO_LOAD_DELAY;

    cp0_exception(BP);
}
static inline void mfhi(void)
{
    // Move From HI
    cpu_trace_instruction("mfhi");

    DO_LOAD_DELAY;

    reg(RD) = cpu.hi;
}
static inline void mthi(void)
{
    // Move To HI
    cpu_trace_instruction("mthi");

    DO_LOAD_DELAY;

    cpu.hi = reg(RS);
}
static inline void mflo(void)
{
    // Move From LO
    cpu_trace_instruction("mflo");

    DO_LOAD_DELAY;

    reg(RD) = cpu.lo;
}
static inline void mtlo(void)
{
    // Move To LO
    cpu_trace_instruction("mtlo");

    DO_LOAD_DELAY;

    cpu.lo = reg(RS);
}
static inline void mult(void)
{
    // MULTiplication RS and RT store in HI:LO
    cpu_trace_instruction("mult");

    int64_t  s = sign64(reg(RS));
    int64_t  t = sign64(reg(RT));
    uint64_t r = s * t;

    DO_LOAD_DELAY;

    cpu.hi = (uint32_t) (r >> 32);
    cpu.lo = (uint32_t)  r;
}
static inline void multu(void)
{
    // MULTiplication Unsigned RS and RT store in HI:LO
    cpu_trace_instruction("multu");

    uint64_t s = reg(RS);
    uint64_t t = reg(RT);
    uint64_t r = s * t;

    DO_LOAD_DELAY;

    cpu.hi = (uint32_t) (r >> 32);
    cpu.lo = (uint32_t)  r;
}
static inline void div(void)
{
    // DIVision, edge cases accounted for, TODO: delays on MULT/DIV operations
    cpu_trace_instruction("div");

    int32_t s = sign32(reg(RS));
    int32_t t = sign32(reg(RT));

    DO_LOAD_DELAY;

    if (t == 0) {
        cpu.hi = s;
        cpu.lo = (s < 0) ? 1: -1;
    } else if ((uint32_t) t == -1 &&
               (uint32_t) s == (1 << 31)) {
        cpu.hi = 0;
        cpu.lo = (1 << 31);
    } else {
        cpu.hi = s % t;
        cpu.lo = s / t;
    }
}
static inline void divu(void)
{
    // DIVide Unsigned RS by RT
    cpu_trace_instruction("divu");

    uint32_t s = reg(RS);
    uint32_t t = reg(RT);

    DO_LOAD_DELAY;

    if (t == 0) {
        cpu.hi = s;
        cpu.lo = -1;
    } else {
        cpu.hi = s % t;
        cpu.lo = s / t;
    }
}
static inline void add(void)
{
    // ADD with overflow
    cpu_trace_instruction("add");

    int32_t s = sign32(reg(RS));
    int32_t t = sign32(reg(RT));

    DO_LOAD_DELAY;

    if (overflow((uint32_t) s,
                 (uint32_t) t)) {
        cp0_exception(Ov);
    } else {
        reg(RD) = s + t;
    }
}
static inline void addu(void)
{
    // ADD Unsigned
    cpu_trace_instruction("addu");

    uint32_t s = reg(RS);
    uint32_t t = reg(RT);

    DO_LOAD_DELAY;

    reg(RD) = s + t;
}
static inline void sub(void)
{
    // SUB with overflow
    cpu_trace_instruction("sub");

    int32_t s = sign32(reg(RS));
    int32_t t = sign32(reg(RT));

    DO_LOAD_DELAY;

    if (underflow(s, t)) {
        cp0_exception(Ov);
    } else {
        reg(RD) = s - t;
    }
}
static inline void subu(void)
{
    // SUBtract Unsigned
    cpu_trace_instruction("subu");

    uint32_t s = reg(RS);
    uint32_t t = reg(RT);

    DO_LOAD_DELAY;

    reg(RD) = s - t;
}
static inline void and(void)
{
    // AND
    cpu_trace_instruction("and");

    uint32_t s = reg(RS);
    uint32_t t = reg(RT);

    DO_LOAD_DELAY;

    reg(RD) = s & t;
}
static inline void or(void)
{
    // OR RS
    cpu_trace_instruction("or");

    uint32_t s = reg(RS);
    uint32_t t = reg(RT);

    DO_LOAD_DELAY;

    reg(RD) = s | t;
}
static inline void xor(void)
{
    // XOR RS
    cpu_trace_instruction("xor");

    uint32_t s = reg(RS);
    uint32_t t = reg(RT);

    DO_LOAD_DELAY;

    reg(RD) = s ^ t;
}
static inline void nor(void)
{
    // Not OR
    cpu_trace_instruction("nor");

    uint32_t s = reg(RS);
    uint32_t t = reg(RT);

    DO_LOAD_DELAY;

    reg(RD) = ~(s | t);
}
static inline void slt(void)
{
    // Set Less Than
    cpu_trace_instruction("slt");

    int32_t s = sign32(reg(RS));
    int32_t t = sign32(reg(RT));

    DO_LOAD_DELAY;

    reg(RD) = s < t;
}
static inline void sltu(void)
{
    // Set Less Than Unsigned
    cpu_trace_instruction("sltu");

    uint32_t s = reg(RS);
    uint32_t t = reg(RT);

    DO_LOAD_DELAY;

    reg(RD) = s < t;
}

static inline void cpu_execute( void )
{
    /* handle branch delay */
    switch (cpu.branch_s) {
    case DELAY:
        cpu.branch_s = TRANSFER;
        break;
    case TRANSFER:
        cpu.pc       = cpu.branch_v;
        cpu.branch_s = UNUSED;
        cpu.branch_v = 0;
        break;
    case UNUSED:
        break;
    }

#ifdef ENABLE_SIDELOADING
    /* check for side loading */
    if (cpu.sideload_exe && (cpu.pc & 0xFFFF0000) == 0x80030000)
        memory_load_exe( cpu.sideload_exe );
#endif

    /* read and increment program counter */
    memory_read(cpu.pc, &cpu.cir, 4);

    /* compute instruction components */
    cpu.funct    = (cpu.cir >>  0) & 0x3f;
    cpu.shamt    = (cpu.cir >>  6) & 0x1f;
    cpu.rd       = (cpu.cir >> 11) & 0x1f;
    cpu.rt       = (cpu.cir >> 16) & 0x1f;
    cpu.rs       = (cpu.cir >> 21) & 0x1f;
    cpu.op       = (cpu.cir >> 26) & 0x3f;
    cpu.target   = cpu.cir & 0x03ffffff;
    cpu.imm16    = cpu.cir & 0x0000ffff;
    cpu.imm25    = cpu.cir & 0x02ffffff;
    cpu.relative = cpu.cir & 0x0000ffff;

    switch (OP)
    {
        case 0X00: goto secondary_op;
        case 0x01: goto branch_op;
        case 0x10: cp0();                break;
        case 0x12: cp2();                break;
        case 0x02: j();                  break;
        case 0x03: jal();                break;
        case 0x04: beq();                break;
        case 0x05: bne();                break;
        case 0x06: blez();               break;
        case 0x07: bgtz();               break;
        case 0x08: addi();               break;
        case 0x09: addiu();              break;
        case 0x0a: slti();               break;
        case 0x0b: sltiu();              break;
        case 0x0c: andi();               break;
        case 0x0d: ori();                break;
        case 0x0e: xori();               break;
        case 0x0f: lui();                break;
        case 0x20: lb();                 break;
        case 0x21: lh();                 break;
        case 0x22: lwl();                break;
        case 0x23: lw();                 break;
        case 0x24: lbu();                break;
        case 0x25: lhu();                break;
        case 0x26: lwr();                break;
        case 0x28: sb();                 break;
        case 0x29: sh();                 break;
        case 0x2a: swl();                break;
        case 0x2b: sw();                 break;
        case 0x2e: swr();                break;
        case 0x30: lwc0();               break;
        case 0x38: swc0();               break;
        case 0x39:                       break;
        default:
            /* get info on instruction */
            trace_set_profile(TRACE_CPU_EN);

            cpu_trace_stack();
            cpu_trace_instruction("Unknown");

            assert(0 && "Unhandled instruction\n");
            break;
    } goto cycle_complete;

secondary_op:
    switch (FUNCT)
    {
        case 0x00: sll();                break;
        case 0x02: srl();                break;
        case 0x03: sra();                break;
        case 0x04: sllv();               break;
        case 0x06: srlv();               break;
        case 0x07: srav();               break;
        case 0x08: jr();                 break;
        case 0x09: jalr();               break;
        case 0x0c: syscall();            break;
        case 0x0d: brk();                break;
        case 0x10: mfhi();               break;
        case 0x11: mthi();               break;
        case 0x12: mflo();               break;
        case 0x13: mtlo();               break;
        case 0x18: mult();               break;
        case 0x19: multu();              break;
        case 0x1a: div();                break;
        case 0x1b: divu();               break;
        case 0x20: add();                break;
        case 0x21: addu();               break;
        case 0x22: sub();                break;
        case 0x23: subu();               break;
        case 0x24: and();                break;
        case 0x25: or();                 break;
        case 0x26: xor();                break;
        case 0x27: nor();                break;
        case 0x2a: slt();                break;
        case 0x2b: sltu();               break;
        default:
            /* get info on instruction */
            trace_set_profile(TRACE_CPU_EN);

            cpu_trace_stack();
            cpu_trace_instruction("Unknown");

            assert(0 && "Unhandled instruction\n");
            break;
    } goto cycle_complete;

branch_op:
    switch (RT)
    {
        case 0x00: bltz();               break;
        case 0x01: bgez();               break;
        case 0x16: bltzal();             break;
        case 0x17: bgezal();             break;
        default:
            /* get info on instruction */
            trace_set_profile(TRACE_CPU_EN);

            cpu_trace_stack();
            cpu_trace_instruction("Unknown");

            assert(0 && "Unhandled instruction\n");
            break;
    } goto cycle_complete;

cycle_complete:
    /* write tty */
    if (((cpu.pc & 0x1fffffff) == 0xa0 && cpu.r[9] == 0x3c) ||
        ((cpu.pc & 0x1fffffff) == 0xb0 && cpu.r[9] == 0x3d)) {
        if (cpu.r[4] != 0) putchar((char) (cpu.r[4]));
    }

    cpu.pc  += 4;
    cpu.r[0] = 0;
}

void write_cpu_reg(enum cpu_reg_e r, uint32_t  data) {
    switch (r) {
    case CPU_HI:  cpu.hi  = data; break;
    case CPU_LO:  cpu.lo  = data; break;
    case CPU_PC:  cpu.pc  = data; break;
    case CPU_CIR: cpu.cir = data; break;
    default:
        if (r < CPU_ZERO || r > CPU_RA)
            assert(0 && "illegal register");
        cpu.r[r] = data;
        break;
    }
}

void  read_cpu_reg(enum cpu_reg_e r, uint32_t *data) {
    switch (r) {
    case CPU_HI:  *data = cpu.hi;  break;
    case CPU_LO:  *data = cpu.lo;  break;
    case CPU_PC:  *data = cpu.pc;  break;
    case CPU_CIR: *data = cpu.cir; break;
    default:
        if (r < CPU_ZERO || r > CPU_RA)
            assert(0 && "illegal register");
        *data = cpu.r[r];
        break;
    }
}

void init_cpu(const char *file_bios,
              const char *file_exe) {
    cpu.pc           = 0xbfc00000;
    cpu.sideload_exe = file_exe;

    memory_load_bios(file_bios);

    cpu.ss.head = 0;
}

void task_cpu( void ) {
    cpu_execute();
}

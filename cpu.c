#include <assert.h>
#include <stdio.h>

#define CPU_PRIVATE
#include "cpu.h"
#include "timer.h"
#include "memory.h"

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

uint32_t cpu_cop0_sr_isc( void ) 
{ 
    union cop0_sr sr  = { .value = cpu.cop0[COP0_SR] }; return sr.Isc; 
}

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

static void cpu_trace_instruction( char *mneumonic )
{
    TRACE_CPU("cpu_execute", "pc: %08x | op: %08x rs(%04s): %08x rt(%04s): %08x rd(%04s): %08x shamt: %08x funct: %08x | imm16: %08x imm25: %08x | %s\n",
            cpu.pc, OP, cpu_register_names[RS], reg(RS), cpu_register_names[RT], reg(RT), cpu_register_names[RD], reg(RD), SHAMT, FUNCT, IMM16, IMM25, mneumonic);
}

static void cpu_branch( void )
{
    cpu.branch_v = cpu.pc + 4 + (S_IMM16 << 2);
    cpu.branch_s = DELAY;
}

static void cpu_exception( enum cpu_exception_type t )
{   
    union cop0_cause cause = { .value = cpu.cop0[COP0_CAUSE] };
    union cop0_sr    sr    = { .value = cpu.cop0[COP0_SR]    };
    uint32_t         epc   =            cpu.cop0[COP0_EPC]    ;
    
    uint32_t handler;
    
    /* set the correct execption code */
    cause.excode = t;

    /* determine which exeption handler to use */
    if (sr.BEV) handler = 0xBFC00180;
    else        handler = 0x80000000;
    
    /* set exception routine return */
    if (cpu.branch_s == UNUSED)
    {
        /* normal, non-branch exception */
        epc = cpu.pc;
    }
    else
    {
        /* branch miss if exception occurs during branch */
        epc = cpu.branch_v;

        cause.branch_delay = 1;

        cpu.branch_v = 0;
        cpu.branch_s = UNUSED;
    }
    
    /* set correct sr status */
    sr.value = (sr.value & ~0X3F) | ((sr.value >> 2) & 0X3F);
    
    /* write back all register values */
    cpu.cop0[COP0_SR]    = sr.value;
    cpu.cop0[COP0_CAUSE] = cause.value;
    cpu.cop0[COP0_EPC]   = epc;
    
    /* set pc to handler */
    cpu.pc = handler - 4;
}

static inline void bltz(void)    
{   
    // Branch Less Than Zero
    cpu_trace_instruction("bltz");

    int32_t s = sign32(reg(RS));

    DO_LOAD_DELAY;

    if (s < 0) 
    {
        cpu_branch();
    }
}
static inline void bgez(void)    
{
    // Branch Greater than Equal Zero 
    cpu_trace_instruction("bgez");

    int32_t s = sign32(reg(RS));
    
    DO_LOAD_DELAY;

    if (s >= 0) 
    {
        cpu_branch();
    }
}
static inline void bltzal(void)    
{
    // Branch Less Than Zero And Link
    cpu_trace_instruction("bltzal");

    int32_t s = sign32(reg(RS));

    DO_LOAD_DELAY;

    if (s < 0) 
    {
        cpu.r[31] = cpu.pc;
        cpu_branch();
    }
}
static inline void bgezal(void)    
{
    // Branch Greater than Equal Zero And Link
    cpu_trace_instruction("bgezal");

    int32_t s = sign32(reg(RS));

    DO_LOAD_DELAY;

    if (s >= 0) 
    {
        cpu.r[31] = cpu.pc;
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

    cpu.r[31] = cpu.pc + 4;

    cpu.branch_v = (cpu.pc & 0XF0000000) | (TARGET << 2);
    cpu.branch_s = DELAY;
}
static inline void beq(void)     
{
    // Branch Equal
    cpu_trace_instruction("beq");

    uint32_t s = reg(RS);
    uint32_t t = reg(RT);
    
    DO_LOAD_DELAY;

    if (s == t) 
    {
        cpu_branch();
    }
}    
static inline void bne(void)     
{
    // Branch Not Equal
    cpu_trace_instruction("bne");

    uint32_t s = reg(RS);
    uint32_t t = reg(RT);
    
    DO_LOAD_DELAY;

    if (s != t )
    {
        cpu_branch();
    }
}    
static inline void blez(void)    
{
    // Branch Less than Equal Zero
    cpu_trace_instruction("blez");

    int32_t s = sign32(reg(RS));
    
    DO_LOAD_DELAY;

    if (s <= 0) 
    {
        cpu_branch();
    }
}   
static inline void bgtz(void)    
{
    // Branch Greater Than Zero
    cpu_trace_instruction("bgtz");

    int32_t s = sign32(reg(RS));
    
    DO_LOAD_DELAY;

    if (s > 0) 
    {
        cpu_branch();
    }
}   
static inline void addi(void)    
{
    // ADD Immediate, with overflow 
    cpu_trace_instruction("addi");

    uint32_t s = reg(RS);
    
    DO_LOAD_DELAY;

    if (overflow(s, S_IMM16))
    {
        cpu_exception(Ov);
    } 
    else 
    {
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
    {
        DO_LOAD_DELAY;
    }

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
    {
        DO_LOAD_DELAY;
    }
    
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
    {
        DO_LOAD_DELAY;
    }

    cpu.load_d = RT;
    cpu.load_v = result;
}     
static inline void lwl(void)     
{
    // Load Halfword Left TODO:
    cpu_trace_instruction("lwl");

    uint32_t s = reg(RS);

    if (cpu.load_d != RT)
    {
        DO_LOAD_DELAY;
    }

    uint32_t mask, result, address = (s + (S_IMM16 & ~0X3));

    memory_read(address, &result, 2);

    switch ((s + S_IMM16) & 0X3) 
    {
        case 0: mask = 0X00FFFFFF; result <<= 24; break;
        case 1: mask = 0X0000FFFF; result <<= 16; break;
        case 2: mask = 0X000000FF; result <<= 8;  break;
        case 3: mask = 0X00000000; result <<= 0;  break;
    }
    
    cpu.load_v &= mask;
    cpu.load_v |= result;

    if (cpu.load_s == UNUSED) 
    {
        cpu.load_s = DELAY;
    }
}
static inline void lwr(void)     
{
    // Load Halfword Right TODO:
    cpu_trace_instruction("lwr");

    uint32_t s = reg(RS);
    
    if (cpu.load_d != RT)
    {
        DO_LOAD_DELAY;
    }
    
    uint32_t mask, result, address = (s + (S_IMM16 & ~0X3));

    memory_read(address, &result, 2);

    switch ((s + S_IMM16) & 0X3) 
    {
        case 1: mask = 0X00000000; result <<= 0;  break;
        case 2: mask = 0X000000FF; result <<= 8;  break;
        case 3: mask = 0X0000FFFF; result <<= 16; break;
        case 4: mask = 0X00FFFFFF; result <<= 24; break;
    }
    
    cpu.load_v &= mask;
    cpu.load_v |= result;

    if (cpu.load_s == UNUSED) 
    {
        cpu.load_s = DELAY;
    }
}
static inline void lbu(void)     
{
    // Load Byte Unsigned
    cpu_trace_instruction("lbu");

    uint32_t result, address = reg(RS) + S_IMM16;

    memory_read(address, &result, 1);

    if (cpu.load_d == RT)
    {
        DO_LOAD_DELAY;
    }

    cpu.load_v = result;
    cpu.load_d = RT;
}    
static inline void lhu(void)     
{
    // Load Halfword Unsigned
    cpu_trace_instruction("lhu");

    uint32_t result, address = reg(RS) + S_IMM16;

    memory_read(address, &result, 2);

    if (cpu.load_d == RT)
    {
        DO_LOAD_DELAY;
    }

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

    switch ((s + S_IMM16) & 0X3) 
    {
        case 0: mask = 0X00FFFFFF; value = current << 24; break;
        case 1: mask = 0X0000FFFF; value = current << 16; break;
        case 2: mask = 0X000000FF; value = current << 8;  break;
        case 3: mask = 0X00000000; value = current << 0;  break;
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

    switch ((s + S_IMM16) & 0X3) 
    {
        case 0: mask = 0X00000000; value = current << 0;  break;
        case 1: mask = 0X000000FF; value = current << 8;  break;
        case 2: mask = 0X0000FFFF; value = current << 16; break;
        case 3: mask = 0X00FFFFFF; value = current << 24; break;
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
static inline void lwc0(void)    
{
    // Load Word Coprocessor 0
    cpu_trace_instruction("lwc0");

    cpu_exception(CpU);
}   
static inline void lwc1(void)    
{
    // Load Word Coprocessor 1
    cpu_trace_instruction("lwc1");

    cpu_exception(CpU);
}   
static inline void lwc2(void)    
{
    // Load Word Coprocessor 2
    cpu_trace_instruction("lwc2");

    uint32_t *reg, address = reg(RS) + IMM25;
    // COPn_reg(2, RD, &reg);
    memory_read(address, reg, 4);
}   
static inline void lwc3(void)    
{
    // Load Word Coprocessor 3
    cpu_trace_instruction("lwc3");

    cpu_exception(CpU);
}   
static inline void swc0(void)    
{
    // Store Word Coprocessor 0
    cpu_trace_instruction("swc0");

    cpu_exception(CpU);
}   
static inline void swc1(void)    
{
    // Store Word Coprocessor 1
    cpu_trace_instruction("swc1");

    cpu_exception(CpU);
}   
static inline void swc2(void)    
{
    // Store Word Coprocessor 2
    cpu_trace_instruction("swc2");

    uint32_t *value, address = reg(RS) + IMM25;
    // COPn_reg(2, RD, &value);
    memory_write(address, *value, 4);
}
static inline void swc3(void)    
{
    // Store Word Coprocessor 3
    cpu_trace_instruction("swc3");

    cpu_exception(CpU);
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
}     
static inline void jalr(void)    
{
    // Jump And Link Register
    cpu_trace_instruction("jalr");

    uint32_t s = reg(RS);

    DO_LOAD_DELAY;

    reg(RD) = cpu.pc + 4;

    cpu.branch_v = s;
    cpu.branch_s = DELAY;
}   
static inline void syscall(void) 
{
    // SYStem CALL exception
    cpu_trace_instruction("syscall");

    DO_LOAD_DELAY;

    cpu_exception(SYSCALL);
}
static inline void brk(void)   
{
    // BREAK exception
    cpu_trace_instruction("brk");

    DO_LOAD_DELAY;

    cpu_exception(BP);
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

    if (t == 0) 
    {
        cpu.hi = s;
        cpu.lo = (s < 0) ? 0X00000001: 0XFFFFFFFF;
    } 
    else if (t == 0XFFFFFFFF && s == 0X80000000) 
    {
        cpu.hi = 0X00000000;
        cpu.lo = 0X80000000;
    } 
    else 
    {
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

    if (t == 0) 
    {
        cpu.hi = s;
        cpu.lo = 0XFFFFFFFF;
    } 
    else 
    {
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

    if (overflow(s, t)) 
    {
        cpu_exception(Ov);
    } 
    else 
    {
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

    if (underflow(s, t)) 
    {
        cpu_exception(Ov);
    } 
    else 
    {
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

// COPn
static inline void MFCn(int cop_n) 
{
    // Move From Coprocessor n
    cpu_trace_instruction("MFCn");

    switch (cop_n)
    {
        case 0x0: cpu.load_v = cpu.cop0[RD]; break;
        case 0x2: cpu.load_v = cpu.cop2[RD]; break;
    }

    cpu.load_d = RT;
}
static inline void MTCn(int cop_n) 
{
    // Move To Coprocessor n
    cpu_trace_instruction("MTCn");

    switch (cop_n)
    {
        case 0x0: cpu.cop0[RD] = reg(RT); break;
        case 0x2: cpu.cop2[RD] = reg(RT); break;
    }
}

static inline void CFCn(int cop_n) { running = 0; }
static inline void CTCn(int cop_n) { running = 0; }
static inline void COPn(int cop_n) { running = 0; }
static inline void BCnF(int cop_n) { running = 0; }
static inline void BCnT(int cop_n) { running = 0; }
static inline void LWCn(int cop_n) { running = 0; }
static inline void SWCn(int cop_n) { running = 0; }

// COP0
static inline void TLBR(void)  { running = 0; }
static inline void TLBWI(void) { running = 0; }
static inline void TLBWR(void) { running = 0; }
static inline void TLBP(void)  { running = 0; }
static inline void RFE(void)   
{
    // Return From Exception
    cpu_trace_instruction("RFE");

    if ((cpu.cir & 0x1f) == 0x10) 
    {
        /* increment exception stack */
        cpu.cop0[COP0_SR] = (cpu.cop0[COP0_SR] & ~0X3F) | ((cpu.cop0[COP0_SR] &  0X3F) >> 2); 
    }
}
static inline void cop0(void)    
{
    // Coprocessor0 instructions
    cpu_trace_instruction("cop0");

    switch (COP_TYPE) 
    {
        case 0X00:
            switch (COP_FUNC) 
            {
                case 0X00: MFCn(0); break; // MFCn
                case 0X02: CFCn(0); break; // CFCn
                case 0X04: MTCn(0); break; // MTCn
                case 0X06: CTCn(0); break; // CTCn
                case 0X08:
                    switch(RT) 
                    {
                        case 0X00: BCnF(0); break; // BCnF
                        case 0X01: BCnT(0); break; // BCnT
                    }
                    break;
            }
            break;
        case 0X01:
            switch (IMM25) 
            {
                case 0X01: TLBR();  break; // TLBR
                case 0X02: TLBWI(); break; // TLBWI
                case 0X06: TLBWR(); break; // TLBWR
                case 0X08: TLBP();  break; // TLBP
                case 0X10: RFE();   break; // RFE
                default:   COPn(0); break; // COPN
            }
            break;
    }
}   
static inline void cop2(void)    
{
    // Coprocessor2 instructions TODO: create the GTE
    cpu_trace_instruction("cop2");

    switch (COP_TYPE) 
    {
        case 0X00:
            switch (COP_FUNC) 
            {
                case 0X00: MFCn(2); break; // MFCn
                case 0X02: CFCn(2); break; // CFCn
                case 0X04: MTCn(2); break; // MTCn
                case 0X06: CTCn(2); break; // CTCn
                case 0X08:
                    switch(RT) 
                    {
                        case 0X00: BCnF(2); break; // BCnF
                        case 0X01: BCnT(2); break; // BCnT
                    }
                    break;
            }
            break;
        case 0X01: COPn(2); break; // COPN
    }
}   

static inline void cpu_execute( void )
{
    /* handle branch delay */
    switch (cpu.branch_s)
    {
        case DELAY:    
            cpu.branch_s = TRANSFER;
            break;
        case TRANSFER: 
            cpu.branch_s = UNUSED;

            cpu.pc = cpu.branch_v;

            cpu.branch_v = 0;
            break;
        case UNUSED:   
            break;
    }

    /* read and increment program counter */
    memory_read(cpu.pc, &cpu.cir, 4);

    switch (OP) 
    {
        case 0X00: goto secondary_op;
        case 0x01: goto branch_op;
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
        case 0x10: cop0();               break;
        case 0x12: cop2();               break;
        default:
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
            assert(0 && "Unhandled instruction\n");
            break;
    } goto cycle_complete;

branch_op:
    switch (RT) 
    {
        case (0x00): bltz();             break;
        case (0x01): bgez();             break;
        case (0x16): bltzal();           break;
        case (0x17): bgezal();           break;
        default:
            assert(0 && "Unhandled instruction\n");
            break;
    } goto cycle_complete;

cycle_complete:
    cpu.pc  += 4;
    cpu.r[0] = 0;
}

void init_cpu( void )
{
    cpu.pc = 0xbfc00000;
}

void task_cpu( void )
{
    cpu_execute();
}

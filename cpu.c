#include <pthread.h>
#include <assert.h>
#include <stdio.h>

#define CPU_PRIVATE
#include "cpu.h"
#include "timer.h"
#include "memory.h"

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

static void cpu_print_instruction( void )
{
    printf("pc: %08x | op: %08x rs(%04s): %08x rt(%04s): %08x rd(%04s): %08x shamt: %08x funct: %08x | imm16: %08x imm25: %08x |",
            cpu.pc, OP, cpu_register_names[RS], reg(RS), cpu_register_names[RT], reg(RT), cpu_register_names[RD], reg(RD), SHAMT, FUNCT, IMM16, IMM25);
}

static void cpu_branch( void )
{
    cpu.branch_v = cpu.pc + 4 + (sign16(IMM16) << 2);
    cpu.branch_s = DELAY;
}

static void cpu_exception( enum cpu_exception_type t )
{

}

static inline void bltz(void)    
{
    // Branch Less Than Zero
    if (sign32(reg(RS)) < 0) 
    {
        cpu_branch();
    }
}
static inline void bgez(void)    
{
    // Branch Greater than Equal Zero 
    if (sign32(reg(RS)) >= 0) 
    {
        cpu_branch();
    }
}
static inline void bltzal(void)    
{
    // Branch Less Than Zero And Link
    if (sign32(reg(RS)) < 0) 
    {
        cpu.r[31] = cpu.pc;
        cpu_branch();
    }
}
static inline void bgezal(void)    
{
    // Branch Greater than Equal Zero And Link
    if (sign32(reg(RS)) >= 0) 
    {
        cpu.r[31] = cpu.pc;
        cpu_branch();
    }
}
static inline void beq(void)     
{
    // Branch Equal
    if (reg(RS) == reg(RT)) 
    {
        cpu_branch();
    }
}    
static inline void bne(void)     
{
    // Branch Not Equal
    if (reg(RS) != reg(RT)) 
    {
        cpu_branch();
    }
}    
static inline void blez(void)    
{
    // Branch Less than Equal Zero
    if (sign32(reg(RS)) <= 0) 
    {
        cpu_branch();
    }
}   
static inline void bgtz(void)    
{
    // Branch Greater Than Zero
    if (sign32(reg(RS)) > 0) 
    {
        cpu_branch();
    }
}   
static inline void addi(void)    
{
    // ADD Immediate, with overflow 
    if (overflow(reg(RS), sign16(IMM16))) 
    {
        cpu_exception(OVF);
    } 
    else 
    {
        reg(RT) = reg(RS) + sign16(IMM16);
    }
}   
static inline void addiu(void)   
{
    // ADD Immediate Unsigned
    reg(RT) = reg(RS) + sign16(IMM16);
}  
static inline void slti(void)    
{
    // Set if Less Than Immediate
    reg(RT) = sign32(reg(RS)) < sign16(IMM16);
}   
static inline void sltiu(void)   
{
    // Set if Less Than Immediate Unsigned
    reg(RT) = reg(RS) < (uint32_t) sign16(IMM16);
}  
static inline void andi(void)    
{
    // AND Immediate
    reg(RT) = reg(RS) & IMM16;
}   
static inline void ori(void)     
{
    // OR Immediate 
    reg(RT) = reg(RS) | IMM16;
}    
static inline void xori(void)    
{
    // XOR Immediate 
    reg(RT) = reg(RS) ^ IMM16;
}   
static inline void lui(void)     
{
    // shift immediate << 16 and store in RT
    reg(RT) = IMM16 << 16;
}    
static inline void lb(void)      
{
    // Load Byte
    uint32_t result, address = reg(RS) + sign16(IMM16);

    memory_read(address, &result, 1);
    
    cpu.load_d = RT;
    cpu.load_v = sign8(result);
    cpu.load_s = DELAY;
}     
static inline void lh(void)      
{
    // Load Halfword 
    uint32_t result, address = reg(RS) + sign16(IMM16);

    memory_read(address, &result, 2);
    
    cpu.load_v = sign16(result);
    cpu.load_d = RT;
    cpu.load_s = DELAY;
}    
static inline void lw(void)      
{
    // Load Word 
    uint32_t result, address = reg(RS) + sign16(IMM16);

    memory_read(address, &result, 4);

    cpu.load_v = result;
    cpu.load_d = RT;
    cpu.load_s = DELAY;
}     
static inline void lwl(void)     
{
    // Load Halfword 
    uint32_t mask, result, address = (reg(RS) + (sign16(IMM16) & ~0X3));

    memory_read(address, &result, 2);

    switch ((reg(RS) + sign16(IMM16)) & 0X3) 
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
    // Load Halfword 
    uint32_t mask, result, address = (reg(RS) + (sign16(IMM16) & ~0X3));

    memory_read(address, &result, 2);

    switch ((reg(RS) + sign16(IMM16)) & 0X3) 
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
    uint32_t result, address = reg(RS) + sign16(IMM16);

    memory_read(address, &result, 1);

    cpu.load_v = result;
    cpu.load_d = RT;
    cpu.load_s = DELAY;
}    
static inline void lhu(void)     
{
    // Load Halfword Unsigned
    uint32_t result, address = reg(RS) + sign16(IMM16);

    memory_read(address, &result, 2);

    cpu.load_v = result;
    cpu.load_d = RT;
    cpu.load_s = DELAY;
}    
static inline void sb(void)      
{
    // Store Byte
    uint32_t address = reg(RS) + sign16(IMM16);
    memory_write(address, reg(RT), 1);
}     
static inline void sh(void)      
{
    // Store Half word 
    uint32_t address = reg(RS) + sign16(IMM16);
    memory_write(address, reg(RT), 2);
}     
static inline void swl(void)     
{
    // Store Halfword Left
    uint32_t mask, current, value, address = (reg(RS) + (sign16(IMM16) & ~0X3));

    memory_read(address, &current, 2);

    switch ((reg(RS) + sign16(IMM16)) & 0X3) 
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
    // Store Halfword Right
    uint32_t mask, current, value, address = (reg(RS) + (sign16(IMM16) & ~0X3));

    memory_read(address, &current, 2);

    switch ((reg(RS) + sign16(IMM16)) & 0X3) 
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
    uint32_t address = reg(RS) + sign16(IMM16);
    memory_write(address, reg(RT), 4);
}     
static inline void lwc0(void)    
{
    // Load Word Coprocessor 0
    cpu_exception(CPU);
}   
static inline void lwc1(void)    
{
    // Load Word Coprocessor 1
    cpu_exception(CPU);
}   
static inline void lwc2(void)    
{
    // Load Word Coprocessor 2
    uint32_t *reg, address = reg(RS) + IMM25;
    // COPn_reg(2, RD, &reg);
    memory_read(address, reg, 4);
}   
static inline void lwc3(void)    
{
    // Load Word Coprocessor 3
    cpu_exception(CPU);
}   
static inline void swc0(void)    
{
    // Store Word Coprocessor 0
    cpu_exception(CPU);
}   
static inline void swc1(void)    
{
    // Store Word Coprocessor 1
    cpu_exception(CPU);
}   
static inline void swc2(void)    
{
    // Store Word Coprocessor 2
    uint32_t *value, address = reg(RS) + IMM25;
    // COPn_reg(2, RD, &value);
    memory_write(address, *value, 4);
}
static inline void swc3(void)    
{
    // Store Word Coprocessor 3
    cpu_exception(CPU);
}   
static inline void sll(void)     
{
    // Shift Left Logical
    reg(RD) = reg(RT) << SHAMT;
}
static inline void srl(void)     
{
    // Shift Right Logical
    reg(RD) = reg(RT) >> SHAMT;
}    
static inline void sra(void)     
{
    // Shift Right Arithmetic
    reg(RD) = sign32(reg(RT)) >> SHAMT;
}    
static inline void sllv(void)    
{
    // Shift Left Logical Variable
    reg(RD) = reg(RT) << (reg(RS) & 0X1F);
}   
static inline void srlv(void)    
{
    // Shift Right Logical Variable
    reg(RD) = reg(RT) >> (reg(RS) & 0X1F);
}   
static inline void srav(void)    
{
    // Shift Right Arthmetic Variable
    reg(RD) = sign32(reg(RT)) >> (reg(RS) & 0X1F);
}   
static inline void jr(void)      
{
    // Jump to Register
    cpu.branch_v = reg(RS);
    cpu.branch_s = DELAY;
}     
static inline void jalr(void)    
{
    // Jump And Link Register
    reg(RD) = cpu.pc + 8;
    jr();
}   
static inline void syscall(void) 
{
    // SYStem CALL exception
    cpu_exception(SYS);
}
static inline void brk(void)   
{
    // BREAK exception
    cpu_exception(BP);
}  
static inline void mfhi(void)    
{
    // Move From HI
    reg(RD) = cpu.hi;
}   
static inline void mthi(void)    
{
    // Move To HI
    cpu.hi = reg(RS);
}   
static inline void mflo(void)    
{
    // Move From LO
    reg(RD) = cpu.lo;
}   
static inline void mtlo(void)    
{
    // Move To LO
    cpu.lo = reg(RS);
}
static inline void mult(void)    
{
    // MULTiplication RS and RT store in HI:LO
    uint64_t result = sign64(reg(RS)) * sign64(reg(RT));
    cpu.hi = (uint32_t) (result >> 32);
    cpu.lo = (uint32_t) result;
}   
static inline void multu(void)   
{
    // MULTiplication Unsigned RS and RT store in HI:LO
    uint64_t result = reg(RS) * reg(RT);
    cpu.hi = (uint32_t) (result >> 32);
    cpu.lo = (uint32_t) result;
}  
static inline void div(void)     
{
    // DIVision, edge cases accounted for, TODO: delays on MULT/DIV operations
    if (reg(RT) == 0) 
    {
        cpu.hi = reg(RS);
        cpu.lo = (sign32(reg(RS)) < 0) ? 0X00000001: 0XFFFFFFFF;
    } 
    else if (reg(RT) == 0XFFFFFFFF && reg(RS) == 0X80000000) 
    {
        cpu.hi = 0X00000000;
        cpu.lo = 0X80000000;
    } 
    else 
    {
        cpu.hi = sign32(reg(RS)) % sign32(reg(RT));
        cpu.lo = sign32(reg(RS)) / sign32(reg(RT));
    }
}    
static inline void divu(void)    
{
    // DIVide Unsigned RS by RT
    if (reg(RT) == 0) 
    {
        cpu.hi = reg(RS);
        cpu.lo = 0XFFFFFFFF;
    } 
    else 
    {
        cpu.hi = reg(RS) % reg(RT);
        cpu.lo = reg(RS) / reg(RT);
    }
}   
static inline void add(void)     
{
    // ADD with overflow
    if (overflow(reg(RS), reg(RT))) 
    {
        cpu_exception(OVF);
    } 
    else 
    {
        reg(RD) = reg(RS) + reg(RT);
    }
}    
static inline void addu(void)    
{
    // ADD Unsigned
    reg(RD) = reg(RS) + reg(RT);
}   
static inline void sub(void)     
{
    // SUB with overflow
    if (underflow(RS, RT)) 
    {
        cpu_exception(OVF);
    } 
    else 
    {
        reg(RD) = reg(RS) - reg(RT);
    }
}    
static inline void subu(void)    
{
    // SUBtract Unsigned
    reg(RD) = reg(RS) - reg(RT);
}   
static inline void and(void)     
{
    // AND 
    reg(RD) = reg(RS) & reg(RT);
}    
static inline void or(void)      
{
    // OR RS
    reg(RD) = reg(RS) | reg(RT);
}     
static inline void xor(void)     
{
    // XOR RS
    reg(RD) = reg(RS) ^ reg(RT);
}    
static inline void nor(void)     
{
    // Not OR
    reg(RD) = ~(reg(RS) | reg(RT));
}    
static inline void slt(void)     
{
    // Set Less Than 
    reg(RD) = sign32(reg(RS)) < sign32(reg(RT));
}    
static inline void sltu(void)    
{
    // Set Less Than Unsigned 
    reg(RD) = reg(RS) < reg(RT);
}   

// J-Type
static inline void j(void)       
{
    // Jump
    cpu.branch_v = (cpu.pc & 0XF0000000) | (TARGET << 2);
    cpu.branch_s = DELAY;
}
static inline void jal(void)     
{
    // Jump And Link
    cpu.r[31] = cpu.pc + 8;
    j();
}

// COPn
static inline void MFCn(int cop_n) 
{
    // Move From Coprocessor n
    switch (cop_n)
    {
        case 0x0: reg(RT) = cpu.cop0[RD]; break;
        case 0x2: reg(RT) = cpu.cop2[RD]; break;
    }
}
static inline void CFCn(int cop_n) { running = 0; }
static inline void MTCn(int cop_n) 
{
    // Move To Coprocessor n
    switch (cop_n)
    {
        case 0x0: cpu.cop0[RD] = reg(RT); break;
        case 0x2: cpu.cop2[RD] = reg(RT); break;
    }
}
static inline void CTCn(int cop_n) { running = 0; }
static inline void COPn(int cop_n) { running = 0; }
static inline void BCnF(int cop_n) { running = 0; }
static inline void BCnT(int cop_n) { running = 0; }
static inline void LWCn(int cop_n) { running = 0; }
static inline void SWCn(int cop_n) { running = 0; }

// COP0
static inline void TLBR()  { running = 0; }
static inline void TLBWI() { running = 0; }
static inline void TLBWR() { running = 0; }
static inline void TLBP()  { running = 0; }
static inline void RFE()   
{
    // Return From Exception
    if ((cpu.cir & 0b11111) == 0b01000) 
    {
        // cpu.cop0.SR.value = (cpu.cop0.SR.value & ~0X3F) | ((cpu.cop0.SR.value & 0X3F) >> 2); // increment exception stack
    }
}
static inline void cop0(void)    
{
    // Coprocessor0 instructions
    switch (COP_TYPE) 
    {
        case 0X00:
            switch (COP_FUNC) 
            {
                case 0X00: printf("MFC0()\n"); MFCn(0); break; // MFCn
                case 0X02: printf("CFC0()\n"); CFCn(0); break; // CFCn
                case 0X04: printf("MTC0()\n"); MTCn(0); break; // MTCn
                case 0X06: printf("CTC0()\n"); CTCn(0); break; // CTCn
                case 0X08:
                    switch(RT) 
                    {
                        case 0X00: printf("BC0F()\n"); BCnF(0); break; // BCnF
                        case 0X01: printf("BC0T()\n"); BCnT(0); break; // BCnT
                    }
                    break;
            }
            break;
        case 0X01:
            switch (IMM25) 
            {
                case 0X01: printf("TLBR() \n"); TLBR();  break; // TLBR
                case 0X02: printf("TLBWI()\n"); TLBWI(); break; // TLBWI
                case 0X06: printf("TLBWR()\n"); TLBWR(); break; // TLBWR
                case 0X08: printf("TLBP() \n"); TLBP();  break; // TLBP
                case 0X10: printf("RFE()  \n"); RFE();   break; // RFE
                default:   printf("COP0() \n"); COPn(0); break; // COPN
            }
            break;
    }
}   
static inline void cop2(void)    
{
    // Coprocessor2 instructions TODO: create the GTE
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

static inline void cpu_secondary( void )
{
    switch (FUNCT) {
        case 0x00: printf("sll();    \n"); sll();                break;
        case 0x02: printf("srl();    \n"); srl();                break;
        case 0x03: printf("sra();    \n"); sra();                break;
        case 0x04: printf("sllv();   \n"); sllv();               break;
        case 0x06: printf("srlv();   \n"); srlv();               break;
        case 0x07: printf("srav();   \n"); srav();               break;
        case 0x08: printf("jr();     \n"); jr();                 break;
        case 0x09: printf("jalr();   \n"); jalr();               break;
        case 0x0c: printf("syscall();\n"); syscall();            break;
        case 0x0d: printf("brk();    \n"); brk();                break;
        case 0x10: printf("mfhi();   \n"); mfhi();               break;
        case 0x11: printf("mthi();   \n"); mthi();               break;
        case 0x12: printf("mflo();   \n"); mflo();               break;
        case 0x13: printf("mtlo();   \n"); mtlo();               break;
        case 0x18: printf("mult();   \n"); mult();               break;
        case 0x19: printf("multu();  \n"); multu();              break;
        case 0x1a: printf("div();    \n"); div();                break;
        case 0x1b: printf("divu();   \n"); divu();               break;
        case 0x20: printf("add();    \n"); add();                break;
        case 0x21: printf("addu();   \n"); addu();               break;
        case 0x22: printf("sub();    \n"); sub();                break;
        case 0x23: printf("subu();   \n"); subu();               break;
        case 0x24: printf("and();    \n"); and();                break;
        case 0x25: printf("or();     \n"); or();                 break;
        case 0x26: printf("xor();    \n"); xor();                break;
        case 0x27: printf("nor();    \n"); nor();                break;
        case 0x2a: printf("slt();    \n"); slt();                break;
        case 0x2b: printf("sltu();   \n"); sltu();               break;
    } 
}

static inline void cpu_branch_condition( void )
{
    switch (RT) {
        case (0x00):printf("bltz();  \n"); bltz();             break;
        case (0x01):printf("bgez();  \n"); bgez();             break;
        case (0x16):printf("bltzal();\n"); bltzal();           break;
        case (0x17):printf("bgezal();\n"); bgezal();           break;
    }
}

static inline void cpu_execute( void )
{
    /* wait for system tick */
    wait_system_tick( 1 );

    /* handle load delay */
    switch (cpu.load_s)
    {
        case DELAY:    
            cpu.load_s = TRANSFER;
            break;
        case TRANSFER: 
            cpu.load_s = UNUSED;

            cpu.r[cpu.load_d] = cpu.load_v;

            cpu.load_d = 0;
            cpu.load_v = 0;
            break;
        case UNUSED:   
            break;
    }

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

    cpu_print_instruction();

    switch (OP) {
        case 0X00:                       cpu_secondary();        break;
        case 0x01:                       cpu_branch_condition(); break;
        case 0x02: printf("j();    \n"); j();                    break;
        case 0x03: printf("jal();  \n"); jal();                  break;
        case 0x04: printf("beq();  \n"); beq();                  break;
        case 0x05: printf("bne();  \n"); bne();                  break;
        case 0x06: printf("blez(); \n"); blez();                 break;
        case 0x07: printf("bgtz(); \n"); bgtz();                 break;
        case 0x08: printf("addi(); \n"); addi();                 break;
        case 0x09: printf("addiu();\n"); addiu();                break;
        case 0x0a: printf("slti(); \n"); slti();                 break;
        case 0x0b: printf("sltiu();\n"); sltiu();                break;
        case 0x0c: printf("andi(); \n"); andi();                 break;
        case 0x0d: printf("ori();  \n"); ori();                  break;
        case 0x0e: printf("xori(); \n"); xori();                 break;
        case 0x0f: printf("lui();  \n"); lui();                  break;
        case 0x20: printf("lb();   \n"); lb();                   break;
        case 0x21: printf("lh();   \n"); lh();                   break;
        case 0x22: printf("lwl();  \n"); lwl();                  break;
        case 0x23: printf("lw();   \n"); lw();                   break;
        case 0x24: printf("lbu();  \n"); lbu();                  break;
        case 0x25: printf("lhu();  \n"); lhu();                  break;
        case 0x26: printf("lwr();  \n"); lwr();                  break;
        case 0x28: printf("sb();   \n"); sb();                   break;
        case 0x29: printf("sh();   \n"); sh();                   break;
        case 0x2a: printf("swl();  \n"); swl();                  break;
        case 0x2b: printf("sw();   \n"); sw();                   break;
        case 0x2e: printf("swr();  \n"); swr();                  break;
        case 0x10:                       cop0();                 break;
        case 0x12:                       cop2();                 break;
        default:
            running = 0;
            break;
    }
    
    cpu.pc  += 4;
    cpu.r[0] = 0;
}

void *task_cpu( void *ignore )
{
    printf("CPU DEVICE: %ld\n", pthread_self());
    cpu.pc = 0xbfc00000;

    while (running)
    {
        cpu_execute();
    }

    return NULL;
}

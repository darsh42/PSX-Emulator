#include "cpu.h"

// main cpu struct
static struct CPU cpu;

// instruction type execution functions
static void cpu_execute_op(void);

// Main OPCODES for the cpu
// I-TYPE instruction     R-TYPE instructions      J-TYPE instructions    COP0 specific        COPn generic
static void BEQ(void);    static void SLL(void);     static void J(void);   static void TLBR();  static void MFCn(int cop_n);
static void BNE(void);    static void SRL(void);     static void JAL(void); static void TLBWI(); static void CFCn(int cop_n);
static void BLEZ(void);   static void SRA(void);                            static void TLBWR(); static void MTCn(int cop_n);
static void BGTZ(void);   static void SLLV(void);                           static void TLBP();  static void CTCn(int cop_n);
static void ADDI(void);   static void SRLV(void);                           static void RFE();   static void COPn(int cop_n);
static void ADDIU(void);  static void SRAV(void);                                                static void BCnF(int cop_n);
static void SLTI(void);   static void JR(void);                                                  static void BCnT(int cop_n);
static void SLTIU(void);  static void JALR(void);                                                static void LWCn(int cop_n);
static void ANDI(void);   static void SYSCALL(void);                                             static void SWCn(int cop_n);
static void ORI(void);    static void BREAK(void);  
static void XORI(void);   static void MFHI(void);   
static void LUI(void);    static void MTHI(void);   
static void COP0(void);   static void MFLO(void);   
static void COP1(void);   static void MTLO(void);   
static void COP2(void);   static void MULT(void);   
static void COP3(void);   static void MULTU(void);  
static void LB(void);     static void DIV(void);    
static void LH(void);     static void DIVU(void);   
static void LWL(void);    static void ADD(void);    
static void LW(void);     static void ADDU(void);   
static void LBU(void);    static void SUB(void);    
static void LHU(void);    static void SUBU(void);   
static void LWR(void);    static void AND(void);    
static void SB(void);     static void OR(void);     
static void SH(void);     static void XOR(void);    
static void SWL(void);    static void NOR(void);    
static void SW(void);     static void SLT(void);    
static void SWR(void);    static void SLTU(void);   
static void LWC0(void);   
static void LWC1(void);   
static void LWC2(void);   
static void LWC3(void);   
static void SWC0(void);   
static void SWC1(void);   
static void SWC2(void);   
static void SWC3(void);   
static void BLTZ(void);
static void BGEZ(void);
static void BLTZAL(void);
static void BGEZAL(void);


// simple flag return functions for external devices
bool cop0_SR_IEc() {return cpu.cop0.SR.IEc;}
bool cop0_SR_KUc() {return cpu.cop0.SR.KUc;}
bool cop0_SR_IEp() {return cpu.cop0.SR.IEp;}
bool cop0_SR_KUp() {return cpu.cop0.SR.KUp;}
bool cop0_SR_IEo() {return cpu.cop0.SR.IEo;}
bool cop0_SR_KUo() {return cpu.cop0.SR.KUo;}
bool cop0_SR_Im()  {return cpu.cop0.SR.Im;}
bool cop0_SR_Isc() {return cpu.cop0.SR.Isc;}
bool cop0_SR_Swc() {return cpu.cop0.SR.Swc;}
bool cop0_SR_PZ()  {return cpu.cop0.SR.PZ;}
bool cop0_SR_CM()  {return cpu.cop0.SR.CM;}
bool cop0_SR_PE()  {return cpu.cop0.SR.PE;}
bool cop0_SR_TS()  {return cpu.cop0.SR.TS;}
bool cop0_SR_BEV() {return cpu.cop0.SR.BEV;}
bool cop0_SR_RE()  {return cpu.cop0.SR.RE;}
bool cop0_SR_CU0() {return cpu.cop0.SR.CU0;}
bool cop0_SR_CU1() {return cpu.cop0.SR.CU1;}
bool cop0_SR_CU2() {return cpu.cop0.SR.CU2;}
bool cop0_SR_CU3() {return cpu.cop0.SR.CU3;}

// misc/helpers
void cpu_exception(enum EXCEPTION_CAUSE cause);

static void cpu_branch(void);
static void cpu_load_delay(void);
static void cpu_branch_delay(void);
static void COPn_reg(int n, int reg, uint32_t **refrence);

#ifdef DEBUG
struct CPU *_cpu(void) {return &cpu;}
#endif

PSX_ERROR cpu_reset(void) {
    cpu.PC = 0Xbfc00000;

    // set cop
    cpu.cop0.R[3]  = &cpu.cop0.BCP.value;
    cpu.cop0.R[5]  = &cpu.cop0.BDA.value;
    cpu.cop0.R[6]  = &cpu.cop0.JUMPDEST.value;
    cpu.cop0.R[7]  = &cpu.cop0.DCIC.value;
    cpu.cop0.R[8]  = &cpu.cop0.BadVaddr.value;
    cpu.cop0.R[9]  = &cpu.cop0.BDAM.value;
    cpu.cop0.R[11] = &cpu.cop0.BCPM.value;
    cpu.cop0.R[12] = &cpu.cop0.SR.value;
    cpu.cop0.R[13] = &cpu.cop0.CAUSE.value;
    cpu.cop0.R[14] = &cpu.cop0.EPC.value;
    cpu.cop0.R[15] = &cpu.cop0.PIRD.value;

    return set_PSX_error(NO_ERROR);
}

PSX_ERROR cpu_fetch(void) {
    cpu_branch_delay();

    memory_cpu_load_32bit(cpu.PC, &cpu.instruction.value);

    return set_PSX_error(NO_ERROR);
}

PSX_ERROR cpu_decode(void) {
    return set_PSX_error(NO_ERROR);
}

PSX_ERROR cpu_execute(void) {
    cpu_load_delay();        // load delay
    cpu_execute_op();        // switch to instruction type

    cpu.PC += 4;

    return set_PSX_error(NO_ERROR); // LOG NO ERROR
}


// simple helper functions
void COPn_reg(int n, int reg, uint32_t **refrence) {
    switch (n) {
        case 0X00: *refrence = cpu.cop0.R[reg]; break;
        case 0X02: *refrence = cpu.cop2.R[reg]; break;
    }
}

void cpu_exception(enum EXCEPTION_CAUSE cause) {
    uint32_t handler = (cpu.cop0.SR.BEV) ? 0XBFC0180: 0X80000080; // determine exception handler
                                                                  
    // set exception cause
    cpu.cop0.CAUSE.EXECODE = cause; 

    // set exception program return
    if (cpu.branch.stage != UNUSED) {
        cpu.cop0.EPC.return_address = cpu.branch.value;
        cpu.cop0.CAUSE.BranchDelay  = true;

        cpu.branch.value = 0;
        cpu.branch.stage = UNUSED;
    } else {
        cpu.cop0.EPC.return_address = cpu.PC;
    }

    // increment exception stack
    cpu.cop0.SR.value = (cpu.cop0.SR.value & ~0X3F) |
                        ((cpu.cop0.SR.value & 0X3F) << 2); 
    
    // skip branch delay
    cpu.PC = handler - 4;
}

void cpu_branch_delay(void) {
    switch (cpu.branch.stage) {
        case UNUSED: break;
        case TRANSFER: 
            cpu.PC = cpu.branch.value;
            cpu.branch.stage = UNUSED;
            cpu.branch.value = 0;
            break;
        case DELAY:
            cpu.branch.stage = TRANSFER;
            break;
    }
}

void cpu_load_delay(void) {
    for (int i = 0; i < 32; i++) {
        switch (cpu.load[i].stage) {
            case UNUSED:    
                break;
            case TRANSFER: 
                cpu.R[i] = cpu.load[i].value;
                cpu.load[i].stage = UNUSED;
                cpu.load[i].value = 0;
                break;
            case DELAY: 
                cpu.load[i].stage = TRANSFER;
                break;
        }
    }
}

void cpu_branch(void) {
    cpu.branch.value = cpu.PC + 4 + (sign16(IMM16) << 2);
    cpu.branch.stage = DELAY;
}

// main instruction execution functions
void cpu_execute_op(void) {
    switch (OP) {
        case 0X00: 
            // RTYPE
            switch (FUNCT) {
                case 0X00: SLL();     break;
                case 0X02: SRL();     break;
                case 0X03: SRA();     break;
                case 0X04: SLLV();    break;
                case 0X06: SRLV();    break;
                case 0X07: SRAV();    break;
                case 0X08: JR();      break;
                case 0X09: JALR();    break;
                case 0X0C: SYSCALL(); break;
                case 0X0D: BREAK();   break;
                case 0X10: MFHI();    break;
                case 0X11: MTHI();    break;
                case 0X12: MFLO();    break;
                case 0X13: MTLO();    break;
                case 0X18: MULT();    break;
                case 0X19: MULTU();   break;
                case 0X1A: DIV();     break;
                case 0X1B: DIVU();    break;
                case 0X20: ADD();     break;
                case 0X21: ADDU();    break;
                case 0X22: SUB();     break;
                case 0X23: SUBU();    break;
                case 0X24: AND();     break;
                case 0X25: OR();      break;
                case 0X26: XOR();     break;
                case 0X27: NOR();     break;
                case 0X2A: SLT();     break;
                case 0X2B: SLTU();    break;
            } 
            break;
        case 0X01: 
            switch (RT) {
                case 0b00000: BLTZ();   break;
                case 0b00001: BGEZ();   break;
                case 0b10000: BLTZAL(); break;
                case 0b10001: BGEZAL(); break;
            }
            break;
        case 0X02: J();      break;
        case 0X03: JAL();    break;
        case 0X04: BEQ();    break;
        case 0X05: BNE();    break;
        case 0X06: BLEZ();   break;
        case 0X07: BGTZ();   break;
        case 0X08: ADDI();   break;
        case 0X09: ADDIU();  break;
        case 0X0A: SLTI();   break;
        case 0X0B: SLTIU();  break;
        case 0X0C: ANDI();   break;
        case 0X0D: ORI();    break;
        case 0X0E: XORI();   break;
        case 0X0F: LUI();    break;
        case 0X20: LB();     break;
        case 0X21: LH();     break;
        case 0X22: LWL();    break;
        case 0X23: LW();     break;
        case 0X24: LBU();    break;
        case 0X25: LHU();    break;
        case 0X26: LWR();    break;
        case 0X28: SB();     break;
        case 0X29: SH();     break;
        case 0X2A: SWL();    break;
        case 0X2B: SW();     break;
        case 0X2E: SWR();    break;
        // cop instructions
        case 0X10: COP0();   break;
        case 0X11: COP1();   break;
        case 0X12: COP2();   break;
        case 0X13: COP3();   break;
        case 0X30: LWC0();   break;
        case 0X31: LWC1();   break;
        case 0X32: LWC2();   break;
        case 0X33: LWC3();   break;
        case 0X38: SWC0();   break;
        case 0X39: SWC1();   break;
        case 0X3A: SWC2();   break;
        case 0X3B: SWC3();   break;
    }
}

// I-Type
void BLTZ(void)    {
    // Branch Less Than Zero
    if (sign32(reg(RS)) < 0) {
        cpu_branch();
    }
}
void BGEZ(void)    {
    // Branch Greater than Equal Zero 
    if (sign32(reg(RS)) >= 0) {
        cpu_branch();
    }
}
void BLTZAL(void)    {
    // Branch Less Than Zero And Link
    if (sign32(reg(RS)) < 0) {
        cpu.R[31] = cpu.PC;
        cpu_branch();
    }
}
void BGEZAL(void)    {
    // Branch Greater than Equal Zero And Link
    if (sign32(reg(RS)) >= 0) {
        cpu.R[31] = cpu.PC;
        cpu_branch();
    }
}
void BEQ(void)     {
    // Branch Equal
    if (reg(RS) == reg(RT)) {
        cpu_branch();
    }
}    
void BNE(void)     {
    // Branch Not Equal
    if (reg(RS) != reg(RT)) {
        cpu_branch();
    }
}    
void BLEZ(void)    {
    // Branch Less than Equal Zero
    if (sign32(reg(RS)) <= 0) {
        cpu_branch();
    }
}   
void BGTZ(void)    {
    // Branch Greater Than Zero
    if (sign32(reg(RS)) > 0) {
        cpu_branch();
    }
}   
void ADDI(void)    {
    // ADD Immediate, with overflow 
    if (overflow(reg(RS), sign16(IMM16))) {
        cpu_exception(OVF);
    } else {
        reg(RT) = reg(RS) + sign16(IMM16);
    }
}   
void ADDIU(void)   {
    // ADD Immediate Unsigned
    reg(RT) = reg(RS) + sign16(IMM16);
}  
void SLTI(void)    {
    // Set if Less Than Immediate
    reg(RT) = sign32(reg(RS)) < sign16(IMM16);
}   
void SLTIU(void)   {
    // Set if Less Than Immediate Unsigned
    reg(RT) = reg(RS) < sign16(IMM16);
}  
void ANDI(void)    {
    // AND Immediate
    reg(RT) = reg(RS) & IMM16;
}   
void ORI(void)     {
    // OR Immediate 
    reg(RT) = reg(RS) | IMM16;
}    
void XORI(void)    {
    // XOR Immediate 
    reg(RT) = reg(RS) ^ IMM16;
}   
void LUI(void)     {
    // shift immediate << 16 and store in RT
    reg(RT) = IMM16 << 16;
}    
void COP0(void)    {
    // Coprocessor0 instructions
    switch (COP_TYPE) {
        case 0X00:
            switch (COP_FUNCT) {
                case 0X00: MFCn(0); break; // MFCn
                case 0X02: CFCn(0); break; // CFCn
                case 0X04: MTCn(0); break; // MTCn
                case 0X06: CTCn(0); break; // CTCn
                case 0X08:
                    switch(RT) {
                        case 0X00: BCnF(0); break; // BCnF
                        case 0X01: BCnT(0); break; // BCnT
                    }
                    break;
            }
            break;
        case 0X01:
            switch (IMM25) {
                case 0X01: TLBR(0);  break; // TLBR
                case 0X02: TLBWI(0); break; // TLBWI
                case 0X06: TLBWR(0); break; // TLBWR
                case 0X08: TLBP(0);  break; // TLBP
                case 0X10: RFE(0);   break; // RFE
                default:   COPn(0);  break; // COPN
            }
            break;
    }
}   
void COP1(void)    {
    // Coprocessor1 instructions
    cpu_exception(CPU);
}   
void COP2(void)    {
    // Coprocessor2 instructions TODO: create the GTE
    print_cpu_error("COP2", "GTE instruction", NULL);
    exit(1);
    switch (COP_TYPE) {
        case 0X00:
            switch (COP_FUNCT) {
                case 0X00: MFCn(2); break; // MFCn
                case 0X02: CFCn(2); break; // CFCn
                case 0X04: MTCn(2); break; // MTCn
                case 0X06: CTCn(2); break; // CTCn
                case 0X08:
                    switch(RT) {
                        case 0X00: BCnF(2); break; // BCnF
                        case 0X01: BCnT(2); break; // BCnT
                    }
                    break;
            }
            break;
        case 0X01: COPn(2); break; // COPN
    }
}   
void COP3(void)    {
    // Coprocessor3 instructions
    cpu_exception(CPU);
}
void LB(void)      {
    // Load Byte
    uint32_t result, address = reg(RS) + sign16(IMM16);

    memory_cpu_load_8bit(address, &result);
    
    cpu.load[RT].value = sign8(result);
    cpu.load[RT].stage = DELAY;
}     
void LH(void)      {
    // Load Halfword 
    uint32_t result, address = reg(RS) + sign16(IMM16);

    memory_cpu_load_16bit(address, &result);
    
    cpu.load[RT].value = sign16(result);
    cpu.load[RT].stage = DELAY;
}    
void LWL(void)     {print_cpu_error("OP", "UNIMPLEMENTED", NULL); exit(1);}    
void LW(void)      {
    // Load Word 
    uint32_t result, address = reg(RS) + sign16(IMM16);

    memory_cpu_load_32bit(address, &result);

    cpu.load[RT].value = result;
    cpu.load[RT].stage = DELAY;
}     
void LWR(void)     {print_cpu_error("OP", "UNIMPLEMENTED", NULL); exit(1);}    
void LBU(void)     {
    // Load Byte Unsigned
    uint32_t result, address = reg(RS) + sign16(IMM16);

    memory_cpu_load_8bit(address, &result);

    cpu.load[RT].value = result;
    cpu.load[RT].stage = DELAY;
}    
void LHU(void)     {
    // Load Halfword Unsigned
    uint32_t result, address = reg(RS) + sign16(IMM16);

    memory_cpu_load_16bit(address, &result);

    cpu.load[RT].value = result;
    cpu.load[RT].stage = DELAY;
}    
void SB(void)      {
    // Store Byte
    uint32_t address = reg(RS) + sign16(IMM16);
    memory_cpu_store_8bit(address, reg(RT));
}     
void SH(void)      {
    // Store Half word 
    uint32_t address = reg(RS) + sign16(IMM16);
    memory_cpu_store_16bit(address, reg(RT));
}     
void SWL(void)     {print_cpu_error("OP", "UNIMPLEMENTED", NULL); exit(1);}    
void SW(void)      {
    // Store Word 
    uint32_t address = reg(RS) + sign16(IMM16);
    memory_cpu_store_32bit(address, reg(RT));
}     
void SWR(void)     {print_cpu_error("OP", "UNIMPLEMENTED", NULL); exit(1);}    
void LWC0(void)    {
    // Load Word Coprocessor 0
    cpu_exception(CPU);
}   
void LWC1(void)    {
    // Load Word Coprocessor 1
    cpu_exception(CPU);
}   
void LWC2(void)    {
    // Load Word Coprocessor 2
    uint32_t *reg, address = reg(RS) + IMM25;
    COPn_reg(2, RD, &reg);
    memory_cpu_load_32bit(address, reg);
}   
void LWC3(void)    {
    // Load Word Coprocessor 3
    cpu_exception(CPU);
}   
void SWC0(void)    {
    // Store Word Coprocessor 0
    cpu_exception(CPU);
}   
void SWC1(void)    {
    // Store Word Coprocessor 1
    cpu_exception(CPU);
}   
void SWC2(void)    {
    // Store Word Coprocessor 2
    uint32_t *value, address = reg(RS) + IMM25;
    COPn_reg(2, RD, &value);
    memory_cpu_store_32bit(address, *value);
}
void SWC3(void)    {
    // Store Word Coprocessor 3
    cpu_exception(CPU);
}   

// R-Type
void SLL(void)     {
    // Shift Left Logical
    reg(RD) = reg(RT) << SHAMT;
}
void SRL(void)     {
    // Shift Right Logical
    reg(RD) = reg(RT) >> SHAMT;
}    
void SRA(void)     {
    // Shift Right Arithmetic
    reg(RD) = sign32(reg(RT)) >> SHAMT;
}    
void SLLV(void)    {
    // Shift Left Logical Variable
    reg(RD) = reg(RT) << (reg(RS) & 0X1F);
}   
void SRLV(void)    {
    // Shift Right Logical Variable
    reg(RD) = reg(RT) >> (reg(RS) & 0X1F);
}   
void SRAV(void)    {
    // Shift Right Arthmetic Variable
    reg(RD) = sign32(reg(RT)) >> (reg(RS) & 0X1F);
}   
void JR(void)      {
    // Jump to Register
    cpu.branch.value = reg(RS);
    cpu.branch.stage = DELAY;
}     
void JALR(void)    {
    // Jump And Link Register
    reg(RD) = cpu.PC + 8;
    JR();
}   
void SYSCALL(void) {
    // SYStem CALL exception
    cpu_exception(SYS);
}
void BREAK(void)   {
    // BREAK exception
    cpu_exception(BP);
}  
void MFHI(void)    {
    // Move From HI
    reg(RD) = cpu.HI;
}   
void MTHI(void)    {
    // Move To HI
    cpu.HI = reg(RS);
}   
void MFLO(void)    {
    // Move From LO
    reg(RD) = cpu.LO;
}   
void MTLO(void)    {
    // Move To LO
    cpu.LO = reg(RS);
}
void MULT(void)    {
    // MULTiplication RS and RT store in HI:LO
    uint64_t result = sign64(reg(RS)) * sign64(reg(RT));
    cpu.HI = (uint32_t) (result >> 32);
    cpu.LO = (uint32_t) result;
}   
void MULTU(void)   {
    // MULTiplication Unsigned RS and RT store in HI:LO
    uint64_t result = reg(RS) * reg(RT);
    cpu.HI = (uint32_t) (result >> 32);
    cpu.LO = (uint32_t) result;
}  
void DIV(void)     {
    // DIVision, edge cases accounted for, TODO: delays on MULT/DIV operations
    if (reg(RT) == 0) {
        cpu.HI = reg(RS);
        cpu.LO = (sign32(reg(RS)) < 0) ? 0X00000001: 0XFFFFFFFF;
    } else if (reg(RT) == 0XFFFFFFFF && reg(RS) == 0X80000000) {
        cpu.HI = 0X00000000;
        cpu.LO = 0X80000000;
    } else {
        cpu.HI = sign32(reg(RS)) % sign32(reg(RT));
        cpu.LO = sign32(reg(RS)) / sign32(reg(RT));
    }
}    
void DIVU(void)    {
    // DIVide Unsigned RS by RT
    if (reg(RT) == 0) {
        cpu.HI = reg(RS);
        cpu.LO = 0XFFFFFFFF;
    } else {
        cpu.HI = reg(RS) % reg(RT);
        cpu.LO = reg(RS) / reg(RT);
    }
}   
void ADD(void)     {
    // ADD with overflow
    if (overflow(reg(RS), reg(RT))) {
        cpu_exception(OVF);
    } else {
        reg(RD) = reg(RS) + reg(RT);
    }
}    
void ADDU(void)    {
    // ADD Unsigned
    reg(RD) = reg(RS) + reg(RT);
}   
void SUB(void)     {
    // SUB with overflow
    if (underflow(RS, RT)) {
        cpu_exception(OVF);
    } else {
        reg(RD) = reg(RS) - reg(RT);
    }
}    
void SUBU(void)    {
    // SUBtract Unsigned
    reg(RD) = reg(RS) - reg(RT);
}   
void AND(void)     {
    // AND 
    reg(RD) = reg(RS) & reg(RT);
}    
void OR(void)      {
    // OR RS
    reg(RD) = reg(RS) | reg(RT);
}     
void XOR(void)     {
    // XOR RS
    reg(RD) = reg(RS) ^ reg(RT);
}    
void NOR(void)     {
    // Not OR
    reg(RD) = ~(reg(RS) | reg(RT));
}    
void SLT(void)     {
    // Set Less Than 
    reg(RD) = sign32(reg(RS)) < sign32(reg(RT));
}    
void SLTU(void)    {
    // Set Less Than Unsigned 
    reg(RD) = reg(RS) < reg(RT);
}   

// J-Type
void J(void)       {
    // Jump
    cpu.branch.value = (cpu.PC & 0XF0000000) + (TAR << 2);
    cpu.branch.stage = DELAY;
}
void JAL(void)     {
    // Jump And Link
    cpu.R[31] = cpu.PC + 8;
    J();
}

// COPn
void MFCn(int cop_n) {
    // Move From Coprocessor n
    uint32_t *result;
    COPn_reg(cop_n, RD, &result);

    reg(RT) = *result; 
}
void CFCn(int cop_n) {print_cpu_error("OP", "UNIMPLEMENTED", NULL); exit(1);}
void MTCn(int cop_n) {
    // Move To Coprocessor n
    uint32_t *destination;
    COPn_reg(cop_n, RD, &destination);
    
    *destination = reg(RT);
}
void CTCn(int cop_n) {print_cpu_error("OP", "UNIMPLEMENTED", NULL); exit(1);}
void COPn(int cop_n) {print_cpu_error("OP", "UNIMPLEMENTED", NULL); exit(1);}
void BCnF(int cop_n) {print_cpu_error("OP", "UNIMPLEMENTED", NULL); exit(1);}
void BCnT(int cop_n) {print_cpu_error("OP", "UNIMPLEMENTED", NULL); exit(1);}
void LWCn(int cop_n) {print_cpu_error("OP", "UNIMPLEMENTED", NULL); exit(1);}
void SWCn(int cop_n) {print_cpu_error("OP", "UNIMPLEMENTED", NULL); exit(1);}

// COP0
void TLBR(int cop_n)  {print_cpu_error("OP", "UNIMPLEMENTED", NULL); exit(1);}
void TLBWI(int cop_n) {print_cpu_error("OP", "UNIMPLEMENTED", NULL); exit(1);}
void TLBWR(int cop_n) {print_cpu_error("OP", "UNIMPLEMENTED", NULL); exit(1);}
void TLBP(int cop_n)  {print_cpu_error("OP", "UNIMPLEMENTED", NULL); exit(1);}
void RFE(int cop_n)   {
    // Return From Exception
    if (cpu.instruction.value & 0b11111 == 0b01000) {
        cpu.cop0.SR.value = (cpu.cop0.SR.value & ~0X3F) |
                            ((cpu.cop0.SR.value & 0X3F) >> 2); // increment exception stack
    }
}

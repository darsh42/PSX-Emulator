#include "cpu.h"

// main cpu struct
static struct CPU cpu;

// instruction type execution functions
static void cpu_execute_op(void);

// Main OPCODES for the cpu
// I-TYPE instruction     R-TYPE instructions      J-TYPE instructions    COP0 specific        COPn generic
static void BCONDZ(void); static void SLL(void);     static void J(void);   static void TLBR();  static void MFCn(int cop_n);
static void BEQ(void);    static void SRL(void);     static void JAL(void); static void TLBWI(); static void CFCn(int cop_n);
static void BNE(void);    static void SRA(void);                            static void TLBWR(); static void MTCn(int cop_n);
static void BLEZ(void);   static void SLLV(void);                           static void TLBP();  static void CTCn(int cop_n);
static void BTGZ(void);   static void SRLV(void);                           static void RFE();   static void COPn(int cop_n);
static void ADDI(void);   static void SRAV(void);                                                static void BCnF(int cop_n);
static void ADDIU(void);  static void JR(void);                                                  static void BCnT(int cop_n);
static void SLTI(void);   static void JALR(void);                                                static void LWCn(int cop_n);
static void SLTIU(void);  static void SYSCALL(void);                                             static void SWCn(int cop_n);
static void ANDI(void);   static void BREAK(void);  
static void ORI(void);    static void MFHI(void);   
static void XORI(void);   static void MTHI(void);   
static void LUI(void);    static void MFLO(void);   
static void COP0(void);   static void MTLO(void);   
static void COP1(void);   static void MULT(void);   
static void COP2(void);   static void MULTU(void);  
static void COP3(void);   static void DIV(void);    
static void LB(void);     static void DIVU(void);   
static void LH(void);     static void ADD(void);    
static void LWL(void);    static void ADDU(void);   
static void LW(void);     static void SUB(void);    
static void LBU(void);    static void SUBU(void);   
static void LHU(void);    static void AND(void);    
static void LWR(void);    static void OR(void);     
static void SB(void);     static void XOR(void);    
static void SH(void);     static void NOR(void);    
static void SWL(void);    static void SLT(void);    
static void SW(void);     static void SLTU(void);   
static void SWR(void);    
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

// simple flag return functions
bool cop0_SR_IEc() {return cpu.coprocessor0.SR.reg.IEc;}
bool cop0_SR_KUc() {return cpu.coprocessor0.SR.reg.KUc;}
bool cop0_SR_IEp() {return cpu.coprocessor0.SR.reg.IEp;}
bool cop0_SR_KUp() {return cpu.coprocessor0.SR.reg.KUp;}
bool cop0_SR_IEo() {return cpu.coprocessor0.SR.reg.IEo;}
bool cop0_SR_KUo() {return cpu.coprocessor0.SR.reg.KUo;}
bool cop0_SR_Im()  {return cpu.coprocessor0.SR.reg.Im;}
bool cop0_SR_Isc() {return cpu.coprocessor0.SR.reg.Isc;}
bool cop0_SR_Swc() {return cpu.coprocessor0.SR.reg.Swc;}
bool cop0_SR_PZ()  {return cpu.coprocessor0.SR.reg.PZ;}
bool cop0_SR_CM()  {return cpu.coprocessor0.SR.reg.CM;}
bool cop0_SR_PE()  {return cpu.coprocessor0.SR.reg.PE;}
bool cop0_SR_TS()  {return cpu.coprocessor0.SR.reg.TS;}
bool cop0_SR_BEV() {return cpu.coprocessor0.SR.reg.BEV;}
bool cop0_SR_RE()  {return cpu.coprocessor0.SR.reg.RE;}
bool cop0_SR_CU0() {return cpu.coprocessor0.SR.reg.CU0;}
bool cop0_SR_CU1() {return cpu.coprocessor0.SR.reg.CU1;}
bool cop0_SR_CU2() {return cpu.coprocessor0.SR.reg.CU2;}
bool cop0_SR_CU3() {return cpu.coprocessor0.SR.reg.CU3;}

// misc/helpers
static void cpu_branch(void);
static PSX_ERROR cpu_load_delay(void);
static PSX_ERROR COPn_reg(int n, int reg, uint32_t **refrence);


#ifdef DEBUG
struct CPU *_cpu(void) {return &cpu;}
#endif

PSX_ERROR cpu_initialize(void) {
    cpu.PC = 0Xbfc00000;
    // set coprocessor
    cpu.coprocessor0.R[3]  = &cpu.coprocessor0.BCP.value;
    cpu.coprocessor0.R[5]  = &cpu.coprocessor0.BDA.value;
    cpu.coprocessor0.R[6]  = &cpu.coprocessor0.JUMPDEST.value;
    cpu.coprocessor0.R[7]  = &cpu.coprocessor0.DCIC.value;
    cpu.coprocessor0.R[8]  = &cpu.coprocessor0.BadVaddr.value;
    cpu.coprocessor0.R[9]  = &cpu.coprocessor0.BDAM.value;
    cpu.coprocessor0.R[11] = &cpu.coprocessor0.BCPM.value;
    cpu.coprocessor0.R[12] = &cpu.coprocessor0.SR.value;
    cpu.coprocessor0.R[13] = &cpu.coprocessor0.CAUSE.value;
    cpu.coprocessor0.R[14] = &cpu.coprocessor0.EPC.value;
    cpu.coprocessor0.R[15] = &cpu.coprocessor0.PIRD.value;

    return set_PSX_error(NO_ERROR);
}

PSX_ERROR cpu_reset(void) {
    return set_PSX_error(NO_ERROR);
}

PSX_ERROR cpu_fetch(void) {
    cpu.instruction = cpu.instruction_next;
    
    memory_cpu_load_32bit(cpu.PC, &cpu.instruction_next.value);

    cpu.PC += 4;

    return set_PSX_error(NO_ERROR);
}

PSX_ERROR cpu_decode(void) {
    return set_PSX_error(NO_ERROR);
}

PSX_ERROR cpu_execute(void) {
    cpu_load_delay();        // load delay
    cpu_execute_op();               // switch to instruction type
    return set_PSX_error(NO_ERROR); // LOG NO ERROR
}


// simple helper functions
static PSX_ERROR COPn_reg(int n, int reg, uint32_t **refrence) {
    switch (n) {
        case 0X00: *refrence = cpu.coprocessor0.R[reg]; break;
        case 0X02: *refrence = cpu.coprocessor2.R[reg]; break;
    }
    return set_PSX_error(NO_ERROR);
}

static PSX_ERROR cpu_load_delay(void) {
    // if a load instruction has occurred
    //  - skips 1 cycle
    //  - register needs to contain the old register value
    for (int i = 0; i < 32; i++) {
        switch (cpu.R_ld[i].stage) {
            case UNUSED:    
                break;
            case TRANSFER: 
                cpu.R[i] = cpu.R_ld[i].value;
                cpu.R_ld[i].stage = UNUSED;
                cpu.R_ld[i].value = 0;
                break;
            case DELAY: 
                cpu.R_ld[i].stage = TRANSFER;
                break;
        }
    }
}

static void cpu_branch(void) {
    cpu.PC += sign16(IMM16) << 2;
    cpu.PC -= 4;
}

// main instruction execution functions
static void cpu_execute_op(void) {
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
        case 0X07: BTGZ();   break;
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
        // COPROCESSOR instructions
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
void BCONDZ(void)  {
} 
void BLTZ(void)    {
    if (reg(RS) < 0) {
        cpu_branch();
    }
}
void BGEZ(void)    {
    if (reg(RS) >= 0) {
        cpu_branch();
    }
}
void BLTZAL(void)    {
    if (reg(RS) < 0) {
        cpu.R[31] = cpu.PC;
        cpu_branch();
    }
}
void BGEZAL(void)    {
    if (reg(RS) >= 0) {
        cpu.R[31] = cpu.PC;
        cpu_branch();
    }
}
void BEQ(void)     {
    // Branch Equal, RS == RT 
    if (reg(RS) == reg(RT)) {
        cpu_branch();
    }
}    
void BNE(void)     {
    // Branch Not Equal, RS != RT 
    if (reg(RS) != reg(RT)) {
        cpu_branch();
    }
}    
void BLEZ(void)    {
    if (sign32(reg(RS)) <= 0) {
        cpu_branch();
    }
}   
void BTGZ(void)    {
    if (sign32(reg(RS)) > 0) {
        cpu_branch();
    }
}   
void ADDI(void)    {
    // ADD immediate, overflow trap triggeRD
    if (overflow(reg(RS), sign16(IMM16))) {
        cpu.coprocessor0.CAUSE.reg.excode = 0X0C;
    } else {
        reg(RT) = reg(RS) + sign16(IMM16);
    }
}   
void ADDIU(void)   {
    // RT = RS + imm, if overflow set exception
    reg(RT) = reg(RS) + sign16(IMM16);
}  
void SLTI(void)    {
    // Set if Less Than Immediate, signed
    reg(RT) = reg(RS) < sign16(IMM16);
}   
void SLTIU(void)   {
    // Set if Less Than Immediate, unsigned
    reg(RT) = reg(RS) < IMM16;
}  
void ANDI(void)    {
    reg(RT) = reg(RS) & IMM16;
}   
void ORI(void)     {
    reg(RT) = reg(RS) | IMM16;
}    
void XORI(void)    {print_cpu_error("OP", "UNIMPLEMENTED", NULL); exit(1);}   
void LUI(void)     {
    // shift immediate << 16 and store in RT
    reg(RT) = IMM16 << 16;
}    
void COP0(void)    {
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
                case 0X0F: RFE(0);   break; // RFE
                default:   COPn(0);  break; // COPN
            }
            break;
    }
}   
void COP1(void)    {
    switch (COP_TYPE) {
        case 0X00:
            switch (COP_FUNCT) {
                case 0X00: MFCn(1); break; // MFCn
                case 0X02: CFCn(1); break; // CFCn
                case 0X04: MTCn(1); break; // MTCn
                case 0X06: CTCn(1); break; // CTCn
                case 0X08:
                    switch(RT) {
                        case 0X00: BCnF(1); break; // BCnF
                        case 0X01: BCnT(1); break; // BCnT
                    }
                    break;
            }
            break;
        case 0X01: COPn(1); break; // COPN
    }
}   
void COP2(void)    {
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
    switch (COP_TYPE) {
        case 0X00:
            switch (COP_FUNCT) {
                case 0X00: MFCn(3); break; // MFCn
                case 0X02: CFCn(3); break; // CFCn
                case 0X04: MTCn(3); break; // MTCn
                case 0X06: CTCn(3); break; // CTCn
                case 0X08:
                    switch(RT) {
                        case 0X00: BCnF(3); break; // BCnF
                        case 0X01: BCnT(3); break; // BCnT
                    }
                    break;
            }
            break;
        case 0X01: COPn(3); break; // COPN
    }
}
void LB(void)      {
    uint32_t result;
    uint32_t address = reg(RS) + sign16(IMM16);

    memory_cpu_load_8bit(address, &result);
    
    cpu.R_ld[RT].value = sign8(result);
    cpu.R_ld[RT].stage = DELAY;
}     
void LH(void)      {
}     
void LWL(void)     {print_cpu_error("OP", "UNIMPLEMENTED", NULL); exit(1);}    
void LW(void)      {
    // Load WoRD 
    // BUG: possible that the load delays do not work properly
    uint32_t result;
    uint32_t address = reg(RS) + sign16(IMM16);

    memory_cpu_load_32bit(address, &result);

    cpu.R_ld[RT].value = result;
    cpu.R_ld[RT].stage = DELAY;
}     
void LBU(void)     {
    uint32_t result;
    uint32_t address = reg(RS) + sign16(IMM16);

    memory_cpu_load_8bit(address, &result);

    cpu.R_ld[RT].value = result;
    cpu.R_ld[RT].stage = DELAY;
}    
void LHU(void)     {print_cpu_error("OP", "UNIMPLEMENTED", NULL); exit(1);}    
void LWR(void)     {print_cpu_error("OP", "UNIMPLEMENTED", NULL); exit(1);}    
void SB(void)      {
    uint32_t address = reg(RS) + sign16(IMM16);
    memory_cpu_store_8bit(address, reg(RT));
}     
void SH(void)      {
    // store half woRD
    uint32_t address = reg(RS) + sign16(IMM16);
    memory_cpu_store_16bit(address, reg(RT));
}     
void SWL(void)     {print_cpu_error("OP", "UNIMPLEMENTED", NULL); exit(1);}    
void SW(void)      {
    // store woRD (32bit) at imm + RS
    uint32_t address = reg(RS) + sign16(IMM16);
    memory_cpu_store_32bit(address, reg(RT));
}     
void SWR(void)     {print_cpu_error("OP", "UNIMPLEMENTED", NULL); exit(1);}    
void LWC0(void)    {print_cpu_error("OP", "UNIMPLEMENTED", NULL); exit(1);}   
void LWC1(void)    {print_cpu_error("OP", "UNIMPLEMENTED", NULL); exit(1);}   
void LWC2(void)    {print_cpu_error("OP", "UNIMPLEMENTED", NULL); exit(1);}   
void LWC3(void)    {print_cpu_error("OP", "UNIMPLEMENTED", NULL); exit(1);}   
void SWC0(void)    {print_cpu_error("OP", "UNIMPLEMENTED", NULL); exit(1);}   
void SWC1(void)    {print_cpu_error("OP", "UNIMPLEMENTED", NULL); exit(1);}   
void SWC2(void)    {print_cpu_error("OP", "UNIMPLEMENTED", NULL); exit(1);}   
void SWC3(void)    {print_cpu_error("OP", "UNIMPLEMENTED", NULL); exit(1);}   

// R-Type
void SLL(void)     {
    // shift left by shift amount
    reg(RD) = reg(RS) << SHAMT;
}
void SRL(void)     {print_cpu_error("OP", "UNIMPLEMENTED", NULL); exit(1);}    
void SRA(void)     {print_cpu_error("OP", "UNIMPLEMENTED", NULL); exit(1);}    
void SLLV(void)    {print_cpu_error("OP", "UNIMPLEMENTED", NULL); exit(1);}   
void SRLV(void)    {print_cpu_error("OP", "UNIMPLEMENTED", NULL); exit(1);}   
void SRAV(void)    {print_cpu_error("OP", "UNIMPLEMENTED", NULL); exit(1);}   
void JR(void)      {
    cpu.PC = reg(RS);
}     
void JALR(void)    {
    reg(RD) = cpu.PC;
    JR();
}   
void SYSCALL(void) {print_cpu_error("OP", "UNIMPLEMENTED", NULL); exit(1);}
void BREAK(void)   {print_cpu_error("OP", "UNIMPLEMENTED", NULL); exit(1);}  
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
void MULT(void)    {print_cpu_error("OP", "UNIMPLEMENTED", NULL); exit(1);}   
void MULTU(void)   {print_cpu_error("OP", "UNIMPLEMENTED", NULL); exit(1);}  
void DIV(void)     {print_cpu_error("OP", "UNIMPLEMENTED", NULL); exit(1);}    
void DIVU(void)    {
    cpu.HI = reg(RS) % reg(RT);
    cpu.LO = reg(RS) / reg(RT);
}   
void ADD(void)     {
    if (overflow(reg(RS), reg(RT))) {
        cpu.coprocessor0.CAUSE.reg.excode = 0X0C;
    } else {
        reg(RD) = reg(RS) + reg(RT);
    }
}    
void ADDU(void)    {
    // ADD Unsigned
    reg(RD) = reg(RS) + reg(RT);
}   
void SUB(void)     {print_cpu_error("OP", "UNIMPLEMENTED", NULL); exit(1);}    
void SUBU(void)    {
    // SUBtract Unsigned
    reg(RD) = reg(RS) - reg(RT);
}   
void AND(void)     {
    reg(RD) = reg(RS) & reg(RT);
}    
void OR(void)      {
    // or RS and RT store in RD
    reg(RD) = reg(RS) | reg(RT);
}     
void XOR(void)     {print_cpu_error("OP", "UNIMPLEMENTED", NULL); exit(1);}    
void NOR(void)     {print_cpu_error("OP", "UNIMPLEMENTED", NULL); exit(1);}    
void SLT(void)     {
    // Set on Less Than 
    reg(RD) = sign32(reg(RS)) < sign32(reg(RT));
}    
void SLTU(void)    {
    // Set on Less Than Unsigned 
    reg(RD) = reg(RS) < reg(RT);
}   

// J-Type
void J(void)       {
    // jump to address
    cpu.PC = (cpu.PC & 0XF0000000) + (TAR << 2);
}
void JAL(void)     {
    cpu.R[31] = cpu.PC;
    J();
}

// COPn
void MFCn(int cop_n) {
    // cpu register RT = coprocessor register RD
    uint32_t *result;
    COPn_reg(cop_n, RD, &result);

    reg(RT) = *result; 
}
void CFCn(int cop_n) {print_cpu_error("OP", "UNIMPLEMENTED", NULL); exit(1);}
void MTCn(int cop_n) {
    // coprocessor register RD = cpu register RT
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
void RFE(int cop_n)   {print_cpu_error("OP", "UNIMPLEMENTED", NULL); exit(1);}

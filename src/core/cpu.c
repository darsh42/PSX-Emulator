#include "cpu.h"

// instruction type execution functions
static PSX_ERROR EXECUTE_I_TYPE(void);
static PSX_ERROR EXECUTE_R_TYPE(void);
static PSX_ERROR EXECUTE_J_TYPE(void);

// Main OPCODES for the cpu
//     I-TYPE instruction   R-TYPE instructions       J-TYPE instructions        COP0 specific               COPn generic
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

// misc/helpers
static void cpu_branch(void);
static PSX_ERROR cpu_handle_load_delay(void);
static PSX_ERROR COPn_reg(int n, int reg, uint32_t **result);

// main cpu struct
static struct CPU cpu;

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

    cpu.instruction_type = UNDECIDED;

    return set_PSX_error(NO_ERROR);
}

PSX_ERROR cpu_decode(void) {
    switch (cpu.instruction.generic.op) {
        case 3:  
        case 2:  cpu.instruction_type = J_TYPE; break;
        case 0:  cpu.instruction_type = R_TYPE; break;
        default: cpu.instruction_type = I_TYPE; break;
    }
    return set_PSX_error(NO_ERROR);
}

PSX_ERROR cpu_execute(void) {
    // load delay
    cpu_handle_load_delay();
    
    // switch to instruction type
    switch(cpu.instruction_type) {
        case I_TYPE: EXECUTE_I_TYPE(); break;
        case R_TYPE: EXECUTE_R_TYPE(); break;
        case J_TYPE: EXECUTE_J_TYPE(); break;
        default: 
                return set_PSX_error(CPU_EXECUTE_ERROR); 
                break;
    }
    return set_PSX_error(NO_ERROR);
}

// simple flag return functions
bool cop0_SR_Isc() {return cpu.coprocessor0.SR.reg.Isc;}

// simple helper functions
static PSX_ERROR COPn_reg(int n, int reg, uint32_t **result) {
    switch (n) {
        case 0X00: *result = cpu.coprocessor0.R[reg]; break;
        case 0X02: *result = cpu.coprocessor2.R[reg]; break;
    }
    return set_PSX_error(NO_ERROR);
}

static PSX_ERROR cpu_handle_load_delay(void) {
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
                break;
            case DELAY: 
                cpu.R_ld[i].stage = TRANSFER;
                break;
        }
    }
}

static void cpu_branch(void) {
    cpu.PC += sign16(i_imm) << 2;
    cpu.PC -= 4;
}

// main instruction execution functions
static PSX_ERROR EXECUTE_I_TYPE(void) {
    PSX_ERROR err = NO_ERROR;
    switch (i_op) {
        case 0X01: BCONDZ(); break; 
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
        default: err = set_PSX_error(UNKNOWN_OPCODE); break;
    }
    if (err != NO_ERROR) {
        print_cpu_error("I_TYPE", "", NULL);
        return ITYPE_ERROR;
    }
    return NO_ERROR;
}

static PSX_ERROR EXECUTE_R_TYPE(void) {
    PSX_ERROR err = NO_ERROR;
    switch (r_funct) {
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
        default: err = set_PSX_error(UNKNOWN_OPCODE); break;
    }
    if (err != NO_ERROR) {
        print_cpu_error("R_TYPE", "", NULL);
        return RTYPE_ERROR;
    }
    return NO_ERROR;
}

static PSX_ERROR EXECUTE_J_TYPE(void) {
    PSX_ERROR err = NO_ERROR;
    switch (j_op) {
        case 0X02: J();   break;
        case 0X03: JAL(); break;
        default: err = set_PSX_error(UNKNOWN_OPCODE); break;
    }
    if (err != NO_ERROR) {
        print_cpu_error("J_TYPE", "", NULL);
        return JTYPE_ERROR;
    }
    return NO_ERROR;
}

void BCONDZ(void)  {} 
void BEQ(void)     {
    // Branch Equal, rs == rt 
    if (i_rs == i_rt) {
        cpu_branch();
    }
}    
void BNE(void)     {
    // Branch Not Equal, rs != rt 
    if (i_rs != i_rt) {
        cpu_branch();
    }
}    
void BLEZ(void)    {}   
void BTGZ(void)    {}   
void ADDI(void)    {
    // ADD immediate, overflow trap triggerd
    if (overflow(i_rs, sign16(i_imm))) {
        cpu.coprocessor0.CAUSE.reg.excode = 0X0C;
    } else {
        i_rt = i_rs + sign16(i_imm);
    }
}   
void ADDIU(void)   {
    // rt = rs + imm, if overflow set exception
    i_rt = i_rs + sign16(i_imm);
}  
void SLTI(void)    {}   
void SLTIU(void)   {}  
void ANDI(void)    {
    i_rt = i_rs & i_imm;
}   
void ORI(void)     {
    i_rt = i_rs | i_imm;
}    
void XORI(void)    {}   
void LUI(void)     {
    // shift immediate << 16 and store in rt
    i_rt = i_imm << 16;
}    
void COP0(void)    {
    switch (cop_type) {
        case 0X00:
            switch (cop_func) {
                case 0X00: MFCn(0); break; // MFCn
                case 0X02: CFCn(0); break; // CFCn
                case 0X04: MTCn(0); break; // MTCn
                case 0X06: CTCn(0); break; // CTCn
                case 0X08:
                    switch(cop_rt) {
                        case 0X00: BCnF(0); break; // BCnF
                        case 0X01: BCnT(0); break; // BCnT
                    }
                    break;
            }
            break;
        case 0X01:
            switch (cop_imm25) {
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
    switch (cop_type) {
        case 0X00:
            switch (cop_func) {
                case 0X00: MFCn(1); break; // MFCn
                case 0X02: CFCn(1); break; // CFCn
                case 0X04: MTCn(1); break; // MTCn
                case 0X06: CTCn(1); break; // CTCn
                case 0X08:
                    switch(cop_rt) {
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
    switch (cop_type) {
        case 0X00:
            switch (cop_func) {
                case 0X00: MFCn(2); break; // MFCn
                case 0X02: CFCn(2); break; // CFCn
                case 0X04: MTCn(2); break; // MTCn
                case 0X06: CTCn(2); break; // CTCn
                case 0X08:
                    switch(cop_rt) {
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
    switch (cop_type) {
        case 0X00:
            switch (cop_func) {
                case 0X00: MFCn(3); break; // MFCn
                case 0X02: CFCn(3); break; // CFCn
                case 0X04: MTCn(3); break; // MTCn
                case 0X06: CTCn(3); break; // CTCn
                case 0X08:
                    switch(cop_rt) {
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
    uint32_t address = i_rs + sign16(i_imm);
    uint32_t rt = cpu.instruction.I_TYPE.rt;

    memory_cpu_load_8bit(address, &result);
    
    cpu.R_ld[rt].value   = sign8(result);
    cpu.R_ld[rt].stage = DELAY;
}     
void LH(void)      {
}     
void LWL(void)     {}    
void LW(void)      {
    // Load Word 
    // BUG: possible that the load delays do not work properly
    uint32_t result;
    uint32_t address = i_rs + sign16(i_imm);
    uint32_t rt = cpu.instruction.I_TYPE.rt;

    memory_cpu_load_32bit(address, &result);

    cpu.R_ld[rt].value   = result;
    cpu.R_ld[rt].stage = DELAY;
}     
void LBU(void)     {}    
void LHU(void)     {}    
void LWR(void)     {}    
void SB(void)      {
    uint32_t address = i_rs + sign16(i_imm);
    memory_cpu_store_8bit(address, i_rt);
}     
void SH(void)      {
    // store half word
    uint32_t address = i_rs + sign16(i_imm);
    memory_cpu_store_16bit(address, i_rt);
}     
void SWL(void)     {}    
void SW(void)      {
    // store word (32bit) at imm + rs
    uint32_t address = i_rs + sign16(i_imm);
    memory_cpu_store_32bit(address, i_rt);
}     
void SWR(void)     {}    
void LWC0(void)    {}   
void LWC1(void)    {}   
void LWC2(void)    {}   
void LWC3(void)    {}   
void SWC0(void)    {}   
void SWC1(void)    {}   
void SWC2(void)    {}   
void SWC3(void)    {}   

void SLL(void)     {
    // shift left by shift amount
    r_rd = r_rs << r_shamt;
}
void SRL(void)     {}    
void SRA(void)     {}    
void SLLV(void)    {}   
void SRLV(void)    {}   
void SRAV(void)    {}   
void JR(void)      {
    cpu.PC = r_rs;
}     
void JALR(void)    {}   
void SYSCALL(void) {}
void BREAK(void)   {}  
void MFHI(void)    {}   
void MTHI(void)    {}   
void MFLO(void)    {}   
void MTLO(void)    {}   
void MULT(void)    {}   
void MULTU(void)   {}  
void DIV(void)     {}    
void DIVU(void)    {}   
void ADD(void)     {
    if (overflow(r_rs, r_rt)) {
        cpu.coprocessor0.CAUSE.reg.excode = 0X0C;
    } else {
        r_rd = r_rs + r_rt;
    }
}    
void ADDU(void)    {
    // ADD Unsigned
    r_rd = r_rs + r_rt;
}   
void SUB(void)     {}    
void SUBU(void)    {}   
void AND(void)     {
    r_rd = r_rs & r_rt;
}    
void OR(void)      {
    // or rs and rt store in rd
    r_rd = r_rs | r_rt;
}     
void XOR(void)     {}    
void NOR(void)     {}    
void SLT(void)     {}    
void SLTU(void)    {
    // Set on Less Than Unsigned 
    r_rd = r_rs < r_rt;
}   

void J(void)       {
    // jump to address
    cpu.PC = (cpu.PC & 0XF0000000) + (j_tar << 2);
}
void JAL(void)     {
    cpu.R[31] = cpu.PC;
    J();
}

void MFCn(int cop_n) {
    uint32_t *rd;
    COPn_reg(cop_n, cop_rd, &rd);

    cpu.R[cop_rt] = *rd;
}
void CFCn(int cop_n) {}
void MTCn(int cop_n) {
    // coprocessor register rd = cpu register rt
    uint32_t *rd;
    COPn_reg(cop_n, cop_rd, &rd);
    
    *rd  = cpu.R[cop_rt];
}
void CTCn(int cop_n) {}
void COPn(int cop_n) {}
void BCnF(int cop_n) {}
void BCnT(int cop_n) {}
void LWCn(int cop_n) {}
void SWCn(int cop_n) {}

void TLBR(int cop_n)  {}
void TLBWI(int cop_n) {}
void TLBWR(int cop_n) {}
void TLBP(int cop_n)  {}
void RFE(int cop_n)   {}

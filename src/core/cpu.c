#include "cpu.h"

// instruction type execution functions
static PSX_ERROR EXECUTE_I_TYPE(void);
static PSX_ERROR EXECUTE_R_TYPE(void);
static PSX_ERROR EXECUTE_J_TYPE(void);

//     I-TYPE instruction             R-TYPE instructions             J-TYPE instructions
static PSX_ERROR BCONDZ(void); static PSX_ERROR SLL(void);     static PSX_ERROR J(void);
static PSX_ERROR BEQ(void);    static PSX_ERROR SRL(void);     static PSX_ERROR JAL(void);
static PSX_ERROR BNE(void);    static PSX_ERROR SRA(void);    
static PSX_ERROR BLEZ(void);   static PSX_ERROR SLLV(void);   
static PSX_ERROR BTGZ(void);   static PSX_ERROR SRLV(void);   
static PSX_ERROR ADDI(void);   static PSX_ERROR SRAV(void);   
static PSX_ERROR ADDIU(void);  static PSX_ERROR JR(void);     
static PSX_ERROR SLTI(void);   static PSX_ERROR JALR(void);   
static PSX_ERROR SLTIU(void);  static PSX_ERROR SYSCALL(void);
static PSX_ERROR ANDI(void);   static PSX_ERROR BREAK(void);  
static PSX_ERROR ORI(void);    static PSX_ERROR MFHI(void);   
static PSX_ERROR XORI(void);   static PSX_ERROR MTHI(void);   
static PSX_ERROR LUI(void);    static PSX_ERROR MFLO(void);   
static PSX_ERROR COP0(void);   static PSX_ERROR MTLO(void);   
static PSX_ERROR COP1(void);   static PSX_ERROR MULT(void);   
static PSX_ERROR COP2(void);   static PSX_ERROR MULTU(void);  
static PSX_ERROR COP3(void);   static PSX_ERROR DIV(void);    
static PSX_ERROR LB(void);     static PSX_ERROR DIVU(void);   
static PSX_ERROR LH(void);     static PSX_ERROR ADD(void);    
static PSX_ERROR LWL(void);    static PSX_ERROR ADDU(void);   
static PSX_ERROR LW(void);     static PSX_ERROR SUB(void);    
static PSX_ERROR LBU(void);    static PSX_ERROR SUBU(void);   
static PSX_ERROR LHU(void);    static PSX_ERROR AND(void);    
static PSX_ERROR LWR(void);    static PSX_ERROR OR(void);     
static PSX_ERROR SB(void);     static PSX_ERROR XOR(void);    
static PSX_ERROR SH(void);     static PSX_ERROR NOR(void);    
static PSX_ERROR SWL(void);    static PSX_ERROR SLT(void);    
static PSX_ERROR SW(void);     static PSX_ERROR SLTU(void);   
static PSX_ERROR SWR(void);    
static PSX_ERROR LWC0(void);   
static PSX_ERROR LWC1(void);   
static PSX_ERROR LWC2(void);   
static PSX_ERROR LWC3(void);   
static PSX_ERROR SWC0(void);   
static PSX_ERROR SWC1(void);   
static PSX_ERROR SWC2(void);   
static PSX_ERROR SWC3(void);   

static struct CPU cpu;

#ifdef DEBUG
struct CPU *_cpu(void) {return &cpu;}
#endif

PSX_ERROR cpu_initialize(void) {
    cpu.PC = 0Xbfc00000;
}

PSX_ERROR cpu_fetch(void) {
    cpu.instruction = cpu.instruction_next;
    if (memory_cpu_load_32bit(cpu.PC, &cpu.instruction_next.value) != NO_ERROR) {
        print_cpu_error("cpu_fetch");
        return CPU_FETCH_ERROR;
    }
    cpu.instruction_type = UNDECIDED;
    cpu.PC += 4;
    return NO_ERROR;
}

PSX_ERROR cpu_decode(void) {
    switch (cpu.instruction.generic.op) {
        case 3:  
        case 2:  cpu.instruction_type = J_TYPE; break;
        case 0:  cpu.instruction_type = R_TYPE; break;
        default: cpu.instruction_type = I_TYPE; break;
    }
    return NO_ERROR;
}

PSX_ERROR cpu_execute(void) {
    switch(cpu.instruction_type) {
        case I_TYPE: EXECUTE_I_TYPE(); break;
        case R_TYPE: EXECUTE_R_TYPE(); break;
        case J_TYPE: EXECUTE_J_TYPE(); break;
        default: return CPU_EXECUTE_ERROR; break;
    }
    return NO_ERROR;
}

static PSX_ERROR EXECUTE_I_TYPE(void) {
    PSX_ERROR err;
    switch (i_op) {
        case 0X01: err = BCONDZ(); break; 
        case 0X04: err = BEQ();    break;
        case 0X05: err = BNE();    break;
        case 0X06: err = BLEZ();   break;
        case 0X07: err = BTGZ();   break;
        case 0X08: err = ADDI();   break;
        case 0X09: err = ADDIU();  break;
        case 0X0A: err = SLTI();   break;
        case 0X0B: err = SLTIU();  break;
        case 0X0C: err = ANDI();   break;
        case 0X0D: err = ORI();    break;
        case 0X0E: err = XORI();   break;
        case 0X0F: err = LUI();    break;
        case 0X10: err = COP0();   break;
        case 0X11: err = COP1();   break;
        case 0X12: err = COP2();   break;
        case 0X13: err = COP3();   break;
        case 0X20: err = LB();     break;
        case 0X21: err = LH();     break;
        case 0X22: err = LWL();    break;
        case 0X23: err = LW();     break;
        case 0X24: err = LBU();    break;
        case 0X25: err = LHU();    break;
        case 0X26: err = LWR();    break;
        case 0X28: err = SB();     break;
        case 0X29: err = SH();     break;
        case 0X2A: err = SWL();    break;
        case 0X2B: err = SW();     break;
        case 0X2E: err = SWR();    break;
        case 0X30: err = LWC0();   break;
        case 0X31: err = LWC1();   break;
        case 0X32: err = LWC2();   break;
        case 0X33: err = LWC3();   break;
        case 0X38: err = SWC0();   break;
        case 0X39: err = SWC1();   break;
        case 0X3A: err = SWC2();   break;
        case 0X3B: err = SWC3();   break;
        default:
    }
    if (err != NO_ERROR) {
        print_cpu_error("I_TYPE");
        return ITYPE_ERROR;
    }
    return NO_ERROR;
}

static PSX_ERROR EXECUTE_R_TYPE(void) {
    PSX_ERROR err;
    switch (r_funct) {
        case 0X00: err = SLL();     break;
        case 0X02: err = SRL();     break;
        case 0X03: err = SRA();     break;
        case 0X04: err = SLLV();    break;
        case 0X06: err = SRLV();    break;
        case 0X07: err = SRAV();    break;
        case 0X08: err = JR();      break;
        case 0X09: err = JALR();    break;
        case 0X0C: err = SYSCALL(); break;
        case 0X0D: err = BREAK();   break;
        case 0X10: err = MFHI();    break;
        case 0X11: err = MTHI();    break;
        case 0X12: err = MFLO();    break;
        case 0X13: err = MTLO();    break;
        case 0X18: err = MULT();    break;
        case 0X19: err = MULTU();   break;
        case 0X1A: err = DIV();     break;
        case 0X1B: err = DIVU();    break;
        case 0X20: err = ADD();     break;
        case 0X21: err = ADDU();    break;
        case 0X22: err = SUB();     break;
        case 0X23: err = SUBU();    break;
        case 0X24: err = AND();     break;
        case 0X25: err = OR();      break;
        case 0X26: err = XOR();     break;
        case 0X27: err = NOR();     break;
        case 0X2A: err = SLT();     break;
        case 0X2B: err = SLTU();    break;
        default: err = set_PSX_error(UNKNOWN_OPCODE); break;
    }
    if (err != NO_ERROR) {
        print_cpu_error("R_TYPE");
        return RTYPE_ERROR;
    }
    return NO_ERROR;
}

static PSX_ERROR EXECUTE_J_TYPE(void) {
    PSX_ERROR err;
    switch (j_op) {
        case 0X02: err = J();   break;
        case 0X03: err = JAL(); break;
        default: err = set_PSX_error(UNKNOWN_OPCODE);
    }
    if (err != NO_ERROR) {
        print_cpu_error("J_TYPE");
        return JTYPE_ERROR;
    }
    return NO_ERROR;
}

PSX_ERROR BCONDZ(void)  {return NO_ERROR;} 
PSX_ERROR BEQ(void)     {return NO_ERROR;}    
PSX_ERROR BNE(void)     {return NO_ERROR;}    
PSX_ERROR BLEZ(void)    {return NO_ERROR;}   
PSX_ERROR BTGZ(void)    {return NO_ERROR;}   
PSX_ERROR ADDI(void)    {return NO_ERROR;}   
PSX_ERROR ADDIU(void)   {
    // rt = rs + imm, if overflow set exception
    i_rt = i_rs + i_imm;
    return NO_ERROR;
}  
PSX_ERROR SLTI(void)    {return NO_ERROR;}   
PSX_ERROR SLTIU(void)   {return NO_ERROR;}  
PSX_ERROR ANDI(void)    {return NO_ERROR;}   
PSX_ERROR ORI(void)     {
    i_rt = i_rs | i_imm;
    return NO_ERROR;
}    
PSX_ERROR XORI(void)    {return NO_ERROR;}   
PSX_ERROR LUI(void)     {
    // shift immediate << 16 and store in rt
    i_rt = i_imm << 16;
    return NO_ERROR;
}    
PSX_ERROR COP0(void)    {return NO_ERROR;}   
PSX_ERROR COP1(void)    {return NO_ERROR;}   
PSX_ERROR COP2(void)    {return NO_ERROR;}   
PSX_ERROR COP3(void)    {return NO_ERROR;}   
PSX_ERROR LB(void)      {return NO_ERROR;}     
PSX_ERROR LH(void)      {return NO_ERROR;}     
PSX_ERROR LWL(void)     {return NO_ERROR;}    
PSX_ERROR LW(void)      {return NO_ERROR;}     
PSX_ERROR LBU(void)     {return NO_ERROR;}    
PSX_ERROR LHU(void)     {return NO_ERROR;}    
PSX_ERROR LWR(void)     {return NO_ERROR;}    
PSX_ERROR SB(void)      {return NO_ERROR;}     
PSX_ERROR SH(void)      {return NO_ERROR;}     
PSX_ERROR SWL(void)     {return NO_ERROR;}    
PSX_ERROR SW(void)      {
    // store word (32bit) at imm + rs
    uint32_t address = i_imm + i_rs;
    memory_cpu_store_32bit(address, i_rt);
    return NO_ERROR;
}     
PSX_ERROR SWR(void)     {return NO_ERROR;}    
PSX_ERROR LWC0(void)    {return NO_ERROR;}   
PSX_ERROR LWC1(void)    {return NO_ERROR;}   
PSX_ERROR LWC2(void)    {return NO_ERROR;}   
PSX_ERROR LWC3(void)    {return NO_ERROR;}   
PSX_ERROR SWC0(void)    {return NO_ERROR;}   
PSX_ERROR SWC1(void)    {return NO_ERROR;}   
PSX_ERROR SWC2(void)    {return NO_ERROR;}   
PSX_ERROR SWC3(void)    {return NO_ERROR;}   

PSX_ERROR SLL(void)     {
    // shift left by shift amount
    r_rd = r_rs << r_shamt;
    return NO_ERROR;
}
PSX_ERROR SRL(void)     {return NO_ERROR;}    
PSX_ERROR SRA(void)     {return NO_ERROR;}    
PSX_ERROR SLLV(void)    {return NO_ERROR;}   
PSX_ERROR SRLV(void)    {return NO_ERROR;}   
PSX_ERROR SRAV(void)    {return NO_ERROR;}   
PSX_ERROR JR(void)      {return NO_ERROR;}     
PSX_ERROR JALR(void)    {return NO_ERROR;}   
PSX_ERROR SYSCALL(void) {return NO_ERROR;}
PSX_ERROR BREAK(void)   {return NO_ERROR;}  
PSX_ERROR MFHI(void)    {return NO_ERROR;}   
PSX_ERROR MTHI(void)    {return NO_ERROR;}   
PSX_ERROR MFLO(void)    {return NO_ERROR;}   
PSX_ERROR MTLO(void)    {return NO_ERROR;}   
PSX_ERROR MULT(void)    {return NO_ERROR;}   
PSX_ERROR MULTU(void)   {return NO_ERROR;}  
PSX_ERROR DIV(void)     {return NO_ERROR;}    
PSX_ERROR DIVU(void)    {return NO_ERROR;}   
PSX_ERROR ADD(void)     {return NO_ERROR;}    
PSX_ERROR ADDU(void)    {return NO_ERROR;}   
PSX_ERROR SUB(void)     {return NO_ERROR;}    
PSX_ERROR SUBU(void)    {return NO_ERROR;}   
PSX_ERROR AND(void)     {return NO_ERROR;}    
PSX_ERROR OR(void)      {
    r_rd = r_rs | r_rt;
    return NO_ERROR;
}     
PSX_ERROR XOR(void)     {return NO_ERROR;}    
PSX_ERROR NOR(void)     {return NO_ERROR;}    
PSX_ERROR SLT(void)     {return NO_ERROR;}    
PSX_ERROR SLTU(void)    {return NO_ERROR;}   

PSX_ERROR J(void)       {
    // jump to address
    cpu.PC = (cpu.PC & 0XF0000000) + (j_tar << 2);
    return NO_ERROR;
}
PSX_ERROR JAL(void)     {return NO_ERROR;}

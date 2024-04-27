#ifndef INSTRUCTION_H_INCLUDED 
#define INSTRUCTION_H_INCLUDED

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

union COPROCESSOR_INSTRUCTION {
    uint32_t value;
    struct {
        uint32_t : 16;
        uint32_t branch: 5;
        uint32_t sub_op: 4;
        uint32_t type: 1;
        uint32_t op: 6;
    } generic;
    struct {
        uint32_t   : 6; // specifies COP0 specific instructions
        uint32_t   : 5;
        uint32_t rd: 5;
        uint32_t rt: 5;
        uint32_t funct: 5;    // can specify COP instructions
        uint32_t op: 6;
    } COPN_R_TYPE;
    struct {
        uint32_t imm: 16; // specifies COP0 specific instructions
        uint32_t rt: 5;
        uint32_t rs: 5;    // can specify COP instructions
        uint32_t op: 6;
    } COPN_I_TYPE;
    struct {
        uint32_t imm: 16; // specifies COP0 specific instructions
        uint32_t funct: 5;
        uint32_t   : 5;    // can specify COP instructions
        uint32_t op: 6;
    } COPN_J_TYPE;
    struct {
        uint32_t imm: 25;    // can specify COP instructions
        uint32_t    : 1;
        uint32_t op: 6;
    } COPN_IMMEDIATE25;
    struct {
        uint32_t funct: 6; // specifies COP0 specific instructions
        uint32_t : 5;
        uint32_t : 5;
        uint32_t : 5;
        uint32_t : 4;    // can specify COP instructions
        uint32_t type: 1;
        uint32_t op: 6;
    } COP0;
};

enum INSTRUCTION_TYPE {
    I_TYPE,
    J_TYPE,
    R_TYPE,
    UNDECIDED
};

#endif//INSTRUCTION_H_INCLUDED

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

enum INSTRUCTION_TYPE {
    I_TYPE,
    J_TYPE,
    R_TYPE,
    UNDECIDED
};

#endif//INSTRUCTION_H_INCLUDED

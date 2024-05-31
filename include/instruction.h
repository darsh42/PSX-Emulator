#ifndef INSTRUCTION_H_INCLUDED 
#define INSTRUCTION_H_INCLUDED

union INSTRUCTION {
    // R-Type
    struct {
        uint32_t funct: 6;
        uint32_t shamt: 5;
        uint32_t rd: 5;
        uint32_t rt: 5;
        uint32_t rs: 5;
        uint32_t op: 6;
    };
    // COPn
    struct {
        uint32_t : 21;
        uint32_t cop_funct: 4;
        uint32_t cop_type: 1;
        uint32_t : 6;
    };
    uint32_t value;
    uint32_t target: 26;
    uint16_t immediate16;
    uint32_t immediate25: 25;
     int16_t relative;
};

#endif//INSTRUCTION_H_INCLUDED

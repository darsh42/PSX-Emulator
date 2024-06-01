#ifndef DISASSEMBLER_H_INCLUEDED
#define DISASSEMBLER_H_INCLUEDED

#include "common.h"
#include "instruction.h"

#define MAXLEN 100


extern struct DEBUGGER *get_debugger(void);
struct DISASSEMBLER {
    uint32_t pc;
    union INSTRUCTION ins;
    
    char bios_disassembly[0X20000][MAXLEN];
    char main_disassembly[0X80000][MAXLEN];
    char exp1_disassembly[0X200000][MAXLEN];
};
#endif // DISASSEMBLER_H_INCLUEDED

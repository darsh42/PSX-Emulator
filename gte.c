#include <assert.h>

#include "cpu.h"
#include "gte.h"

/* cpu opcode breakdown */
#define FUNCT    ((cpu.cir >>  0) & 0x3F)
#define SHAMT    ((cpu.cir >>  6) & 0x1F)
#define RD       ((cpu.cir >> 11) & 0x1F)
#define RT       ((cpu.cir >> 16) & 0x1F)
#define RS       ((cpu.cir >> 21) & 0x1F)
#define OP       ((cpu.cir >> 26) & 0x3F)
#define TARGET    (cpu.cir & ((1 << 26) - 1))
#define IMM16     (cpu.cir & ((1 << 16) - 1))
#define S_IMM16   sign16(IMM16)
#define IMM25     (cpu.cir & ((1 << 25) - 1))
#define RELATIVE  (cpu.cir & ((1 << 16) - 1))

#define reg(R) cpu.r[R]

/* coprocessor opcode breakdown */
#define CP2_TYPE  ((cpu.cir >> 25) & 0x1)
#define CP2_FUNC  ((cpu.cir >> 21) & 0x7)
#define CP2_RT    ((cpu.cir >> 16) & 0x1F)
#define CP2_IMM25  (cpu.cir & ((1 << 25) - 1))

/* externed in here since unique case */
extern struct cpu cpu;
extern void cpu_trace_instruction( char *mneumonic );

static struct cp2 _cp2;

static inline void mfc(void) {
    cpu_trace_instruction("mfc2");

    cpu.load_v = _cp2.r[RD];
    cpu.load_d = RD;
}
static inline void mtc(void) {
    cpu_trace_instruction("mtc2");

    _cp2.r[RD] = reg(RT);
}
static inline void cfc(void) { }
static inline void ctc(void) { }
static inline void cp( void ) { }
static inline void bcf( void ) { }
static inline void bct( void ) { }
static inline void lwc( void ) { }
static inline void swc( void ) { }

void write_cp2_register(enum cp2_reg_e r, uint32_t  data) { _cp2.r[r] =  data; }
void  read_cp2_register(enum cp2_reg_e r, uint32_t *data) { *data = _cp2.r[r]; }

void cp2(void) {
    // Coprocessor2 instructions
    cpu_trace_instruction("cp2");

    if (CP2_TYPE == 0) goto cp2_func;
    if (CP2_TYPE == 1) { cp(); return; }

cp2_func:
    if (CP2_FUNC == 8) goto cp2_rt;

    switch (CP2_FUNC) {
    case 0X00: mfc(); break;
    case 0X02: cfc(); break;
    case 0X04: mtc(); break;
    case 0X06: ctc(); break;
    default:
        assert(0 && "Unimplemented cp0 intstruction");
        break;
    } return;

cp2_rt:
    switch(RT) {
    case 0X00: bcf(); break;
    case 0X01: bct(); break;
    default:
        assert(0 && "Unimplemented cp0 intstruction");
        break;
    } return;
}

#include <assert.h>

#include "cpu.h"
#include "cp0.h"
#include "interrupts.h"

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
#define CP0_TYPE  ((cpu.cir >> 25) & 0x1)
#define CP0_FUNC  ((cpu.cir >> 21) & 0x7)
#define CP0_RT    ((cpu.cir >> 16) & 0x1F)
#define CP0_IMM25  (cpu.cir & ((1 << 25) - 1))

/* externed in here since unique case */
extern struct cpu cpu;
extern void cpu_trace_instruction( char *mneumonic );

static struct cp0 _cp0;

static inline void mfc(void) {
    cpu_trace_instruction("mfc0");

    cpu.load_v = _cp0.r[RD];
    cpu.load_d = RT;
}
static inline void mtc(void) {
    cpu_trace_instruction("mtc0");

    _cp0.r[RD] = reg(RT);
}
static inline void cfc(void) { }
static inline void ctc(void) { }
static inline void cp( void ) { }
static inline void bcf( void ) { }
static inline void bct( void ) { }
static inline void lwc( void ) { }
static inline void swc( void ) { }
static inline void rfe( void ) {
    // Return From Exception
    cpu_trace_instruction("RFE");

    union cp0_cause cause = { .value = _cp0.r[CP0_CAUSE] };

    if ((cpu.cir & 0x1f) == 0x10) {
        /* increment exception stack */
        _cp0.r[CP0_SR] =  (_cp0.r[CP0_SR] & ~0X3F) |
                         ((_cp0.r[CP0_SR] &  0X3F) >> 2);
    }
}

void write_cp0_reg(enum cp0_reg_e r, uint32_t  data) { _cp0.r[r] =  data; }
void  read_cp0_reg(enum cp0_reg_e r, uint32_t *data) { *data = _cp0.r[r]; }

void cp0_exception( enum cpu_exception_type t ) {
    union cp0_cause cause = { .value = _cp0.r[CP0_CAUSE] };
    union cp0_sr    sr    = { .value = _cp0.r[CP0_SR]    };
    uint32_t        epc   =            _cp0.r[CP0_EPC]    ;

    uint32_t handler;

    /* set the correct execption code */
    cause.excode = t;

    /* determine which exeption handler to use */
    if (sr.BEV) handler = 0xBFC00180;
    else        handler = 0x80000000;

    /* set exception routine return */
    if (cpu.branch_s == UNUSED)
    {
        /* normal, non-branch exception */
        epc = cpu.pc;
    }
    else
    {
        /* branch miss if exception occurs during branch */
        epc = cpu.branch_v;

        cause.branch_delay = 1;

        cpu.branch_v = 0;
        cpu.branch_s = UNUSED;
    }

    /* set correct sr status */
    sr.value = (sr.value & ~0X3F) | ((sr.value >> 2) & 0X3F);

    /* write back all register values */
    _cp0.r[CP0_SR]    = sr.value;
    _cp0.r[CP0_CAUSE] = cause.value;
    _cp0.r[CP0_EPC]   = epc;

    /* set pc to handler */
    cpu.pc = handler - 4;
}

void cp0(void) {
    // Coprocessor0 instructions
    cpu_trace_instruction("cp0");

    switch(CP0_TYPE) {
    case 0x00: goto cp0_func;
    case 0x01: goto cp0_imm25;
    }

cp0_func:
    switch (CP0_FUNC) {
    case 0x08: goto cp0_rt;
    case 0X00: mfc(); break;
    case 0X02: cfc(); break;
    case 0X04: mtc(); break;
    case 0X06: ctc(); break;
    default:
        assert(0 && "Unimplemented cp0 intstruction");
        break;
    } return;


cp0_rt:
    switch(RT) {
    case 0X00: bcf(); break;
    case 0X01: bct(); break;
    default:
        assert(0 && "Unimplemented cp0 intstruction");
        break;
    } return;


cp0_imm25:
    switch (IMM25) {
    default:   cp();  break;
    case 0X10: rfe(); break;
    case 0X01:
    case 0X02:
    case 0X06:
    case 0X08:
        /* causes exception */
        assert(0 && "Unimplemented cp0 intstruction");
        break;
    } return;
}

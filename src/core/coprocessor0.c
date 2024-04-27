#include "coprocessor0.h"

static PSX_ERROR TLBR();
static PSX_ERROR TLBWI();
static PSX_ERROR TLBWR();
static PSX_ERROR TLBP();
static PSX_ERROR RFE();

static struct COPROCESSOR0 coprocessor;

struct COPROCESSOR0 *_coprocessor0(void) {return &coprocessor;}

PSX_ERROR coprocessor0_execute(uint32_t value, bool *serviced) {
    PSX_ERROR err;
    switch(copn_imm25) {
        case 0X01: *serviced = true; break; // TLBR
        case 0X02: *serviced = true; break; // TLBWI
        case 0X06: *serviced = true; break; // TLBWR
        case 0X08: *serviced = true; break; // TLBP
        case 0X0F: *serviced = true; break; // RFE
        default:
    };
   return NO_ERROR;
}

PSX_ERROR TLBR() {return NO_ERROR;}
PSX_ERROR TLBWI() {return NO_ERROR;}
PSX_ERROR TLBWR() {return NO_ERROR;}
PSX_ERROR TLBP() {return NO_ERROR;}
PSX_ERROR RFE() {return NO_ERROR;}

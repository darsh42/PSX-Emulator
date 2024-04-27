#include "coprocessor.h"

static PSX_ERROR MFCn(int coprocessor_num);
static PSX_ERROR CFCn(int coprocessor_num);
static PSX_ERROR MTCn(int coprocessor_num);
static PSX_ERROR CTCn(int coprocessor_num);
static PSX_ERROR COPn(int coprocessor_num);
static PSX_ERROR BCnF(int coprocessor_num);
static PSX_ERROR BCnT(int coprocessor_num);
static PSX_ERROR LWCn(int coprocessor_num);
static PSX_ERROR SWCn(int coprocessor_num);

static struct COPROCESSOR coprocessor;

static uint32_t *cop_reg_select(int cop_num, int reg_num) {
    if (cop_num == 0) {
        return &coprocessor.coprocessor0->R[reg_num];
    } else {
        return &coprocessor.coprocessor2->R[reg_num];
    }
    return NULL;
}

PSX_ERROR coprocessor_initialize() {
    coprocessor.coprocessor0 = _coprocessor0();
}

PSX_ERROR coprocessor_execute(uint32_t value, int coprocessor_num) {
    coprocessor.instruction.value = value;

    // if generic op type
    switch (cop_generic_op) {
        case 0X10:
            bool serviced = false;
            coprocessor0_execute(value, &serviced);
            if (serviced) {
                return NO_ERROR;
            }
        case 0X11:
        case 0X12:
        case 0X13:
            switch (cop_generic_sub_op) {
                case 0X00: MFCn(coprocessor_num); break; // MFCn
                case 0X02: CFCn(coprocessor_num); break; // CFCn
                case 0X04: MTCn(coprocessor_num); break; // MTCn
                case 0X06: CTCn(coprocessor_num); break; // CTCn
                case 0X08:
                    switch (cop_generic_branch) {
                        case 0X00: BCnF(coprocessor_num); break; // BCnF
                        case 0X01: BCnT(coprocessor_num); break; // BCnT
                    } break;
                default:   COPn(coprocessor_num); break; // COPn
            }
            break;
        case 0X30: // LWCn 
        case 0X31: // LWCn 
        case 0X32: // LWCn 
        case 0X33: // LWCn 
            LWCn(coprocessor_num);
            break;
        case 0X38: // SWCn
        case 0X39: // SWCn
        case 0X3A: // SWCn
        case 0X3B: // SWCn
            SWCn(coprocessor_num);
            break;
    }
    return NO_ERROR;
}

PSX_ERROR MFCn(int coprocessor_num) {return NO_ERROR;}
PSX_ERROR CFCn(int coprocessor_num) {return NO_ERROR;}
PSX_ERROR MTCn(int coprocessor_num) {
    // coprocessor register rd = cpu register rt
    uint32_t *rd = reg_cop(coprocessor_num, cop_r_rd);
    uint32_t *rt = reg_cpu(cop_r_rt);
    *rd = *rt;
    return NO_ERROR;
}
PSX_ERROR CTCn(int coprocessor_num) {return NO_ERROR;}
PSX_ERROR COPn(int coprocessor_num) {return NO_ERROR;}
PSX_ERROR BCnF(int coprocessor_num) {return NO_ERROR;}
PSX_ERROR BCnT(int coprocessor_num) {return NO_ERROR;}
PSX_ERROR LWCn(int coprocessor_num) {return NO_ERROR;}
PSX_ERROR SWCn(int coprocessor_num) {return NO_ERROR;}

#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

#include <stdio.h>
#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "error.h"

#define DEBUG

enum EXCEPTION_CAUSE {
    INT,
    MOD,
    TLBL,
    TLBS,
    ADEL,
    ADES,
    IBE,
    DBE,
    SYS,
    BP,
    RI,
    CPU,
    OVF
};

enum GENERAL_ENABLE {
    DISABLE = 0,
    ENABLE = 1
};


extern void print_error(const char *file, const char *function, const char *format, ...);
extern void print_warning(const char *file, const char *function, const char *format, ...);
extern PSX_ERROR set_PSX_warning(PSX_ERROR err);
extern PSX_ERROR set_PSX_error(PSX_ERROR err);

#endif//COMMON_H_INCLUDED

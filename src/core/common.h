#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "error.h"

#define DEBUG

extern void print_error();
extern void print_warning();
extern PSX_ERROR set_PSX_error();

#endif//COMMON_H_INCLUDED

#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

#include <stdio.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "error.h"

#define DEBUG

extern void print_error(const char *file, const char *function, const char *format, ...);
extern void print_warning(const char *file, const char *function, const char *format, ...);
extern PSX_ERROR set_PSX_error();

#endif//COMMON_H_INCLUDED

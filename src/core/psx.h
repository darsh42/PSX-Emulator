#ifndef PSX_H_INCLUDED
#define PSX_H_INCLUDED

// utility headers
#include "error.h"
#include "common.h"

// device headers
#include "memory.h"

// macros
#define print_psx_error(func) print_error("psx.c", func)

// device functions
extern PSX_ERROR memory_load_bios(const char *filebios);

#endif//PSX_H_INCLUDED

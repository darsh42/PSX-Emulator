#ifndef CDROM_H_INCLUDED
#define CDROM_H_INCLUDED

#include "common.h"
#include "memory.h"

#include "fifo.h"

#define print_cdrom_error(func, format, ...) print_error("cdrom.c", func, format,  __VA_ARGS__)



struct CDROM {
    /** Registers */
    union CDROM_INDEX_STATUS_REG idx_sts_reg;
    union CDROM_COMMAND_REG      cmd_reg;


};

#endif // CDROM_H_INCLUDED

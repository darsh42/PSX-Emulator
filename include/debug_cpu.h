#ifndef DEBUG_CPU_H_INCLUDED
#define DEBUG_CPU_H_INCLUDED

#include "debugger.h"
#include "error.h"
#include "common.h"

#include "cpu.h"
#include "coprocessor0.h"
#include "coprocessor2.h"

extern struct CPU *get_cpu(void);

extern struct DEBUGGER debugger;

#endif//DEBUG_CPU_H_INCLUDED

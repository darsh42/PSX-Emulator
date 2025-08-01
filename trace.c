#include <assert.h>

#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>

#include "trace.h"

FILE *trace_file    = NULL;
bool  trace_enabled = false;

void trace_enable  ( void ) { trace_enabled = true;  }
void trace_disabled( void ) { trace_enabled = false; }

void trace( char *file, char *function_call, char *format_string, ... )
{
    if (trace_enabled) {
        /* will contain argument list */
        va_list args; 

        /* get all the format arguments */
        va_start(args, format_string); 
        
        /* print the basic information */
        fprintf(stdout, "[TRACE] file: %-16s | function: %-48s | ", file, function_call);
        
        /* print the specific information */
        vprintf(format_string, args); 
        
        /* end the arguments */
        va_end(args);
    }
}

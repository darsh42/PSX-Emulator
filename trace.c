#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

#include "trace.h"

FILE *trace_file   = NULL;

void trace( char *file, char *function_call, char *format_string, ... )
{
    /* will contain argument list */
    va_list args; 

    /* get all the format arguments */
    va_start(args, format_string); 
    
    /* print the basic information */
    fprintf(stdout, "[TRACE] file: %s | function: %s | ", file, function_call);
    
    /* print the specific information */
    vprintf(format_string, args); 
    
    /* end the arguments */
    va_end(args);
}

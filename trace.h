#ifndef TRACE_H_INCLUDED
#define TRACE_H_INCLUDED

#include <stdio.h>

/* tracing file */
extern FILE *trace_file;

inline void trace( char *file, char *function_call, char *format_string, ... )
{
    /* will contain argument list */
    va_list args; 

    /* get all the format arguments */
    va_start(args, format_string); 
    
    /* print the basic information */
    fprintf(trace_file, "[TRACE] file: %s | function: %s | ", file, function_call);
    
    /* print the specific information */
    vprintf(format_string, args); 

    /* print the newline */
    printf("\n");
    
    /* end the arguments */
    va_end(args);
}

/* contains the internal trace state, will be included once */
#ifdef TRACE_H_IMPLEMENTATION

#include <assert.h>

FILE *trace_file = NULL;

/* creates the tracing structures */
static inline void trace_init( char *filename )
{
    /* if no trace file is provided re-direct to terminal otherwise to file */
    if (filename == NULL) assert((trace_file = stdout));
    else                  assert((trace_file = fopen(filename, "w")));
}

/* destroys the tracing structures */
static inline void trace_deinit( void )
{
    /* if the trace file is terminal return otherwise close it */
    if ( trace_file == stdout ) return;
    else                        assert(!fclose(trace_file));
}

#endif // TRACE_H_IMPLEMENTATION

#endif // TRACE_H_INCLUDED

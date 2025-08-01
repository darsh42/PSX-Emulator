#ifndef TRACE_H_INCLUDED
#define TRACE_H_INCLUDED

extern void trace_enable  ( void );
extern void trace_disabled( void );

extern void trace( char *file, 
                   char *function_call, 
                   char *format_string, 
                   ... );

#endif // TRACE_H_INCLUDED

#include <assert.h>

#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>

#include "trace.h"

struct tracer {
    union logging enabled;
};

static struct tracer tracer;

void trace_set_profile( uint32_t profile ) {
    tracer.enabled.profile = profile;
}

void trace( enum tracing_device dev, char *file, char *function_call, char *format_string, ... )
{
    va_list args;

    /* if the device is enabled */
    if (tracer.enabled.profile & dev) {
        va_start(args, format_string);

        /* print the basic information */
        fprintf(stdout, "[TRACE] file: %-16s | function: %-48s | ", file, function_call);

        /* print the specific information */
        vprintf(format_string, args);

        va_end(args);
    }
}

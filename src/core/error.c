#include "error.h"

static PSX_ERROR error;
static char *error_msg;

static char *get_PSX_error() {
    switch (error) {
        case NO_ERROR: error_msg = "NO_ERROR"; break;
        // PSX
        case INSUFFICIENT_ARGS: error_msg = "INSUFFICIENT ARGUMENTS"; break;
        // MEMORY
        case BIOS_FILE_NOT_FOUND: error_msg = "BIOS_FILE_NOT_FOUND"; break;
        case BIOS_FILE_UNREADABLE: error_msg = "BIOS_FILE_UNREADABLE"; break;
        case MEMORY_CPU_UNMAPPED_ADDRESS: error_msg = "MEMORY_CPU_UNMAPPED_ADDRESS"; break;
        default: error_msg = "UNEXPECTED ERROR"; break;
    }
}

PSX_ERROR set_PSX_warning(PSX_ERROR err) {
    error = err;
    return NO_ERROR;
}

PSX_ERROR set_PSX_error(PSX_ERROR err) {
    error = err;
    return err;
}

void print_error(const char *file, const char *function, const char *format, ...) {
    #ifdef DEBUG
    peek_cpu_registers();
    #endif

    get_PSX_error();
    printf("[ERROR] %s->%s: %s: ", file, function, error_msg);

    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    printf("\n");
}

void print_warning(const char *file, const char *function, const char *format, ...) {
    get_PSX_error();
    printf("[WARNING] %s->%s: %s: ", file, function, error_msg);

    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    
    printf("\n");
}

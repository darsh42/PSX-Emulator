#include "../../include/debugger.h"
/* Debugger for the psx emulator
 *
 * Features:
 *
 *  Breakpoints - 
 *      User can set breakpoints on the following conditions:
 *          - INSTRUCTIONS
 *          - MEMORY READ/WRITE
 *          - CONDITIONAL
 *      The debugger will pause execution, allowing the user
 *      too inspect the assembly, registers, memory and render
 *      ppu state at the specified breakpoint
 *
 *  Watchpoints -
 *      User can set watchpoints on the following values:
 *          - MEMORY
 *          - REGISTERS
 *      The debugger will notify the user if the values have
 *      changed or log the watchpoint to a specified file 
 *
 *  Step Execution -
 *      User can cycle the system allowing for step by step execution
 *      this can be done in the following ways:
 *          - STEP INTO
 *          - STEP OUT
 *          - STEP OVER
 *      The modes allow for easy manipulation of execution,
 *      STEP INTO will enter functions and execute all assembly,
 *      STEP OUT will exit a function,
 *      STEP OVER will execute the funtion and continue
 *
 *  Logging -
 *      User can create logs/dumps of the system state at anytime,
 *      they can create dumps of the following:
 *          - INSTRUCTIONS
 *          - MEMORY
 *          - DEVICE/REGISTER STATE
 *      The format will aim to be in line with other emulators if 
 *      possible, that will allow for easy comparison
*/

/*
 * TODO: 
 *  add logging
 *  add instruction peeking
 *  add remote disassembly
*/

void debugger_reset(void);
void debugger_input(void);

struct DEBUGGER debugger;

/* BREAKPOINTS */

bool
hit_bp(void) {
    int i;
    bp cp;

    for (i = 0; i < MAX_BREAK; i++) {
        cp = debugger.bps[i];
        
        switch (cp.type) {
            case BP_INS:
                break;
            case BP_CON:
                if (cp.address == debugger.psx->cpu->PC)
                    return true;
            case BP_EMPTY: break;
        }
    }
    
    return false;
}

void
print_bp(void) {
    bp cur;
    for (int i = 0; i < MAX_BREAK; i++) {
        switch ((cur = debugger.bps[i]).type) {
            case BP_INS: fprintf(stdout, "breakpoint %d, INSTRUCTION: %08X\n", cur.number, cur.opcode); break;
            case BP_CON: fprintf(stdout, "breakpoint %d, CONDITIONAL: %08X\n", cur.number, cur.address); break;
            case BP_EMPTY: break;
        }
    }
}

bp *
find_bp(int number) {
    for (int i = 0; i < MAX_BREAK; i++) 
        if (debugger.bps[i].number == number)
            return &debugger.bps[i];

    return NULL;
}

bp *
empty_bp(void) {
    for (int i = 0; i < MAX_WATCH; i++) {
        if (debugger.bps[i].type == BP_EMPTY)
             return &debugger.bps[i];
    }
    
    return NULL;
}

int
add_bp(bp_t type, uint32_t opcode, uint32_t address) {
    bp *create = empty_bp();
    
    if (create == NULL)
        return 1;

    create->number  = debugger.bp_count++;
    create->type    = type;
    create->opcode  = opcode;
    create->address = address;

    return 0;
}

int
remove_bp(int number) {
    bp *remove = find_bp(number);
    
    if (remove == NULL) 
        return 1;

    remove->type = BP_EMPTY;
    return 0;
}

/* WATCHPOINTS */
bool
hit_wp(void) {
    int i;
    wp cp;

    for (i = 0; i < MAX_WATCH; i++) {
        cp = debugger.wps[i];

        switch (cp.type) {
            case WP_MEM: {
                uint32_t current;
                memory_cpu_load_32bit(cp.location, &current);

                if (cp.value != current)
                    return true;
                break;
            }
            case WP_REG: 
                if (cp.value != debugger.psx->cpu->R[cp.location])
                    return true;
                break;
            case WP_EMPTY: break;
        }
    }

    return false;
}

void
print_wp(void) {
    wp cur;
    for (int i = 0; i < MAX_BREAK; i++) {
        switch ((cur = debugger.wps[i]).type) {
            case WP_REG: fprintf(stdout, "watchpoint %d, REGISTER: (%02d -> %08X)\n", cur.number, cur.location, cur.value); break;
            case WP_MEM: fprintf(stdout, "watchpoint %d, MEMORY: (%08X -> %08X)\n", cur.number, cur.location, cur.value); break; 
            case WP_EMPTY: break;
        }
    }
}

wp *
find_wp(int number) {
    for (int i = 0; i < MAX_BREAK; i++) 
        if (debugger.wps[i].number == number)
            return &debugger.wps[i];

    return NULL;
}

wp *
empty_wp(void) {
    for (int i = 0; i < MAX_WATCH; i++) {
        if (debugger.wps[i].type == WP_EMPTY)
             return &debugger.wps[i];
    }
    
    return NULL;
}

int
add_wp(wp_t type, uint32_t value, uint32_t location) {
    wp *watchpoint = empty_wp();
    
    if (!watchpoint)
        return 1;

    watchpoint->number   = debugger.wp_count++;
    watchpoint->type     = type;
    watchpoint->value    = value;
    watchpoint->location = location;

    return 0;
}

int
remove_wp(int number) {
    wp *remove = find_wp(number);

    if (!remove) 
        return 1;

    remove->type = WP_EMPTY;
    return 0;
}

char *strdup(const char *src) {
    char *dst = malloc(strlen (src) + 1);  // Space for length plus nul
    if (dst == NULL) return NULL;          // No memory
    strcpy(dst, src);                      // Copy the characters
    return dst;                            // Return the new string
}

int debugger_previous PARAMS((char *args));
int debugger_help PARAMS((char *args));
int debugger_quit PARAMS((char *args));
int debugger_step PARAMS((char *args));
int debugger_continue PARAMS((char *args));
int debugger_breakpoint PARAMS((char *args));
int debugger_watchpoint PARAMS((char *args));
int debugger_memory PARAMS((char *args));
int debugger_registers PARAMS((char *args));
int debugger_logging PARAMS((char *args));


static COMMAND commands[] = {
    {"",           "previous command",                       debugger_previous},
    {"help",       "display this text",                      debugger_help},
    {"quit",       "quit emulator",                          debugger_quit},
    {"step",       "step to the next instruction",           debugger_step},
    {"continue",   "continue psx execution",                 debugger_continue},
    {"breakpoint", "create, delete and inspect breakpoints", debugger_breakpoint},
    {"watchpoint", "create, delete and inspect watchpoints", debugger_watchpoint},
    {"memory",     "search memory",                          debugger_memory},
    {"register",   "seach registers",                        debugger_registers},
    {"logging",    "toggle logging",                         debugger_logging},
    {(char *) NULL, (char *) NULL,                           (rl_icpfunc_t *) NULL}
};

static ARGUMENT subcommands[] = {
    {"help",       "display this text"},
    {"add", "add a watchpoint or breakpoint"},
    {"delete", "delete a watchpoint or breakpoint"},
    {"list", "list all watchpoints or breakpoints"},
    {(char *) NULL, (char *) NULL}
};

static ARGUMENT specifiers[] = {
    {"help",       "display this text"},
    {"conditional", "create a conditional breakpoint on the program counter"},
    {"instruction", "create a instruction breakpoint"},
    {"memory", "create a memory watchpoint"},
    {"register", "create a register watchpoint"},
    {(char *) NULL, (char *) NULL}
};

/* command completion */

char *
debugger_command_generator(const char *text, int state) {
    static int list_index, len;
    char *name;

    if (!state) {
        list_index = 0;
        len = strlen(text);
    }
    
    while((name = commands[list_index].name)) {
        list_index++;

        if (strncmp(text, name, len) == 0) {
            return strdup(name);
        }
    }

    return (char *) NULL;
}

char *
debugger_subcommand_generator(const char *text, int state) {
    static int list_index, len;
    char *name;

    if (!state) {
        list_index = 0;
        len = strlen(text);
    }
    
    while((name = subcommands[list_index].name)) {
        list_index++;

        if (strncmp(text, name, len) == 0) {
            return strdup(name);
        }
    }

    return (char *) NULL;
}

char *
debugger_specifier_generator(const char *text, int state) {
    static int list_index, len;
    char *name;

    if (!state) {
        list_index = 0;
        len = strlen(text);
    }
    
    while((name = specifiers[list_index].name)) {
        list_index++;

        if (strncmp(text, name, len) == 0) {
            return strdup(name);
        }
    }

    return (char *) NULL;
}

char **
debugger_command_completion(const char *text, int start, int end) {
    char **matches;
    
    matches = (char **) NULL;

    if (start == 0)
        matches = rl_completion_matches(text, debugger_command_generator);

    else if (start <= 11)
        matches = rl_completion_matches(text, debugger_subcommand_generator);

    else if (start > 11)
        matches = rl_completion_matches(text, debugger_specifier_generator);

    return matches;
}

COMMAND *
debugger_command_find(char *line) {
    for (int i = 0; commands[i].name; i++) {
        if (!strncmp(line, commands[i].name, strlen(line)))
            return &commands[i];
    }

    return (COMMAND *) NULL;
}

ARGUMENT *
debugger_argument_find(char *line) {
    for (int i = 0; subcommands[i].name; i++) {
        if (!strncmp(line, subcommands[i].name, strlen(line)))
            return &subcommands[i];
    }
    return (ARGUMENT *) NULL;
}

ARGUMENT *
debugger_specifier_find(char *line) {
    for (int i = 0; specifiers[i].name; i++) {
        if (!strncmp(line, specifiers[i].name, strlen(line)))
            return &specifiers[i];
    }
    return (ARGUMENT *) NULL;
}

void
debugger_command_execute(char *line) {
    char *token;
    COMMAND *command;

    if (!(token = strtok(line, " ")))
        return;

    // find the command word
    command = debugger_command_find(token);

    // execute the command
    if (!command) {
        fprintf(stderr, "[ERROR]: No such command see help\n");
        return;
    }
    
    (*command->func)(line);
}

int
debugger_previous PARAMS((char *args)) {
    HIST_ENTRY *previous = history_get(history_base + history_length - 1);
    
    if (previous) {
        debugger_command_execute(previous->line);
        return 0;
    }

    return 1;
}

int 
debugger_help PARAMS((char *args)) {
    COMMAND c;
    for (int i = 0; (c = commands[i]).name != NULL; i++)
        fprintf(stdout, "  \e[1m%-10s\e[m  --  %s\n", c.name, c.help);
    return 0;
}

int 
debugger_subcommand_help(char *caller) {
    fprintf(stdout, "Usage: %s <command> \n" \
                    "commands: \n", caller);
    ARGUMENT c;
    for (int i = 0; (c = subcommands[i]).name != NULL; i++)
        fprintf(stdout, "  \e[1m%-10s\e[m  --  %s\n", c.name, c.help);
    return 0;
}

int
debugger_specifier_help(char *caller) {
    fprintf(stdout, "Usage: %s <args>\n" \
                    "Args:\n", caller);
    ARGUMENT c;
    for (int i = 0; (c = specifiers[i]).name != NULL; i++)
        fprintf(stdout, "  \e[1m%-10s\e[m  --  %s\n", c.name, c.help);
    return 0;
}

int 
debugger_quit PARAMS((char *args)) {
    debugger.psx->running = false;
    debugger.paused = false;
    fflush(stdout);
    return 0;
}

int
debugger_step PARAMS((char *args)) {
    debugger.paused = false;
    debugger.stepping = true;
    return 0;
}

int 
debugger_continue PARAMS((char *args)) {
    debugger.stepping = false;
    debugger.paused = false;
    return 0;
}

int 
debugger_breakpoint PARAMS((char *args)) {
    char *tok;
    ARGUMENT *sub, *spec;
    
    if (!(tok = strtok(NULL, " ")))
        return 1;
    
    sub = debugger_argument_find(tok);

    if (!sub)
        return 1;

    if (!strncmp(sub->name, "list", strlen(sub->name))) { 
        print_bp();
    } else if (!strncmp(sub->name, "delete", strlen(sub->name))) { 
        if (!(tok = strtok(NULL, " ")))
            return 1;

        int bp_number;

        if (sscanf(tok, "%d", (int *) &bp_number) == 1) 
            remove_bp(bp_number);

    } else if (!strncmp(sub->name, "add", strlen(sub->name))) {
        // add breakpoints
        if (!(tok = strtok(NULL, " ")))
            return 1;

        spec = debugger_specifier_find(tok);

        if (!spec) 
            return 1;
        
        if (!strncmp(spec->name, "instruction", strlen(spec->name))) {
            uint32_t opcode;

            if (!(tok = strtok(NULL, " ")))
                return 1;

            // BUG: need to compare with cpu instructions, possibly add command completion?
            if (sscanf(tok, "%x", (int *) &opcode) == 1) 
                add_bp(BP_INS, opcode, 0);

            print_bp();
        } else if (!strncmp(spec->name, "conditional", strlen(spec->name))) {
            uint32_t address;

            if (!(tok = strtok(NULL, " ")))
                return 1;

            if (sscanf(tok, "%x", (int *) &address) == 1) 
                add_bp(BP_CON, 0, address);

            print_bp();
        } else {
            debugger_specifier_help("breakpoint add");
        }
    } else {
        debugger_subcommand_help("breakpoint");
    }

    return 0;
}

int 
debugger_watchpoint PARAMS((char *args)) {
    char *tok;
    ARGUMENT *sub, *spec;

    if (!(tok = strtok(NULL, " ")))
        return 1;

    sub = debugger_argument_find(tok);
    
    if (!sub)
        return 1;
    
    if (!strncmp(sub->name, "list", strlen(sub->name))) {
        print_wp();
    } else if (!strncmp(sub->name, "delete", strlen(sub->name))) {
        if (!(tok = strtok(NULL, " ")))
            return 1;

        uint32_t number;
        if (sscanf(tok, "%x", (int *) &number)) 
            remove_wp(number);

        print_wp();
    } else if (!strncmp(sub->name, "add", strlen(sub->name))) {
        if (!(tok = strtok(NULL, " ")))
            return 1;

        spec = debugger_specifier_find(tok);
        
        if (!spec)
            return 1;

        if (!strncmp(spec->name, "memory", strlen(spec->name))) {
            if (!(tok = strtok(NULL, " ")))
                return 1;
            
            uint32_t address, value;
            if (sscanf(tok, "%x", (int *) &address)) {
                memory_cpu_load_32bit(address, &value);
                add_wp(WP_MEM, value, address);
            }
        } else if (!strncmp(spec->name, "register", strlen(spec->name))) {
            if (!(tok = strtok(NULL, " ")))
                return 1;
            
            uint32_t reg_num;
            if (sscanf(tok, "%x", (int *) &reg_num)) {
                add_wp(WP_REG, 0, reg_num);
            }
        } else {
            fprintf(stdout, "Usage: watchpoint add <arg>\n" \
                            "Arg:\n");
            ARGUMENT c;
            for (int i = 0; (c = specifiers[i]).name != NULL; i++)
                fprintf(stdout, "  \e[1m%-10s\e[m  --  %s\n", c.name, c.help);
        }
        
    } else {
        fprintf(stdout, "Usage: watchpoint <command> \n" \
                        "commands:\n");
        ARGUMENT c;
        for (int i = 0; (c = subcommands[i]).name != NULL; i++)
            fprintf(stdout, "  \e[1m%-10s\e[m  --  %s\n", c.name, c.help);
    }

    return 0;
}

int 
debugger_memory PARAMS((char *args)) {
    char * tok;
    if (!(tok = strtok(NULL, " ")))
        return 1;

    uint32_t base, address, value;
    if (sscanf(tok, "%x", (int *) &base) == 1)  {
        fprintf(stdout, "[MEMORY]:             ");
        for (int i = 0; i < 16; i++) {
            if (i == (base & 0xf)) {
                fprintf(stdout, "\e[1m%02X\e[m ", i);
            } else {
                fprintf(stdout, "%02X ", i);
            }
        }
        fprintf(stdout, "\n[MEMORY]:");

        base -= base & 0xF;
        
        for (address = base; address < base + MAX_MEMORY_VIEW; address++) {
            if (address == base) {
                fprintf(stdout, "\n");
                fprintf(stdout, "[MEMORY]: \e[1m%08X\e[m    ", address);
            } else if (address % 16 == 0) {
                fprintf(stdout, "\n");
                fprintf(stdout, "[MEMORY]: %08X    ", address);
            }
            memory_cpu_load_8bit(address, &value);
            fprintf(stdout, "%02X ", value);
        }
    }
    
    fprintf(stdout, "\n");

    return 0;
}

int 
debugger_registers PARAMS((char *args)) {
    static const char *register_names[32] = {
        "$zr", "$at", "$v0", "$v1", "$a0", "$a1", "$a2", "$a3", 
        "$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7", 
        "$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7", 
        "$t8", "$t9", "$k0", "$k1", "$gp", "$sp", "$fp", "$ra"
    };
    
    for (int reg = 0; reg < 32; reg++) {
        printf("[DEBUG]: CPU REGISTER: %s = %08X\n", register_names[reg], debugger.psx->cpu->R[reg]);
    }

    printf("[DEBUG]: CPU REGISTER: $hi = %08X\n", debugger.psx->cpu->HI);
    printf("[DEBUG]: CPU REGISTER: $lo = %08X\n", debugger.psx->cpu->LO);

    printf("[DEBUG]: CPU REGISTER: $pc = %08X\n", debugger.psx->cpu->PC);
    
    for (int reg = 0; reg < 16; reg++) 
        printf("[DEBUG]: COP0 REGISTER: %d = %08X\n", reg, *debugger.psx->cpu->cop0.R[reg]);
    return 0;
}

int debugger_logging PARAMS((char *args)) {
    return 0;
}

/* debugger routines */
void 
debugger_interrupt(int signal) {
    if (signal == SIGINT)
        debugger.paused = true;
}

void 
debugger_reset(void) {
    debugger.psx = get_psx();

    debugger.paused = true;
    debugger.stepping = false;

    debugger.bp_count = 0;
    debugger.wp_count = 0;

    for (int i = 0; i < MAX_BREAK; i++) {
        debugger.bps[i].type = BP_EMPTY;
    }

    for (int i = 0; i < MAX_WATCH; i++) {
        debugger.wps[i].type = WP_EMPTY;
    }

    // rl_readline_name = "PSX debugger";
    rl_attempted_completion_function = debugger_command_completion;
    
    signal(SIGINT, debugger_interrupt);
    raise(SIGINT);
}

void 
debugger_exec(void) {
    if (hit_bp())
        debugger.paused = true;

    if (hit_wp())
        debugger.paused = true;

    if (debugger.paused)
        debugger_input();

    if (debugger.stepping) {
        debugger.paused = true;
        debugger.stepping = false;
        debugger_input();
    }

    peek_cpu_instruction();

}

void 
debugger_input(void) {
    char *line = (char *) NULL;

    while (debugger.paused) {
        line = readline("(debugger) > ");

        if (strlen(line) == 0) {
            debugger_previous(NULL);
        }

        if (line && *line) {
            add_history(line);
            debugger_command_execute(line);
        }
        
        free(line);
    }
}

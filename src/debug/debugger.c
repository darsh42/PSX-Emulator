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
static const char *opcode_str[] = {
    FOREACH_MNEUMONIC(GENERATE_STRING)
    NULL
};

static const int   opcode_int[] = {
    FOREACH_OPCODE(GENERATE_OPCODES)
    -1
};

static const char *register_names[] = {
    FOREACH_REGISTER(GENERATE_STRING)
    NULL
};
    

static uint32_t 
strtoopcode(char *s) {
    const char *op;
    for (int i = 0, len = strlen(s); (op = opcode_str[i]); i++)
        if (len == strlen(op) && !strncmp(s, op, len)) 
            return (uint32_t) opcode_int[i];
    return -1;
}

static const char *
opcodetostr(uint32_t opcode) {
    uint32_t cur;
    for (int i = 0; (cur = opcode_int[i]) != -1; i++) 
        if (cur == opcode)
            return opcode_str[i];
    return NULL;
}

static const uint32_t 
strtoreg(char *s) {
    const char *reg;
    for (int i = 0, len = strlen(s); (reg = register_names[i]); i++) 
        if (len == strlen(reg) && !strncmp(s, reg, len)) 
            return i;
    return -1;
}

static bool
hit_bp(void) {
    int i;
    bp cp;

    for (i = 0; i < MAX_BREAK; i++) {
        cp = debugger.bps[i];
        
        switch (cp.type) {
            case BP_INS: {
                union INSTRUCTION breakpoint = (union INSTRUCTION) cp.opcode;
                union INSTRUCTION current    = (union INSTRUCTION) debugger.psx->cpu->instruction;

                if (breakpoint.op == 0 && current.op == 0 && breakpoint.funct == current.funct) {
                    fprintf(stdout, "breakpoint %d, INSTRUCTION: %s\n", cp.number, opcodetostr(cp.opcode));
                    return true;
                }
                
                if (breakpoint.op == 1 && current.op == 1 && breakpoint.rt == current.rt) {
                    fprintf(stdout, "breakpoint %d, INSTRUCTION: %s\n", cp.number, opcodetostr(cp.opcode));
                    return true;
                }

                if (breakpoint.op > 1 && current.op > 1 && breakpoint.op == current.op) {
                    fprintf(stdout, "breakpoint %d, INSTRUCTION: %s\n", cp.number, opcodetostr(cp.opcode));
                    return true;
                }
                break;
            }
            case BP_CON:
                if (cp.address == debugger.psx->cpu->PC) {
                    fprintf(stdout, "breakpoint %d, CONDITIONAL: %08X\n", cp.number, cp.address);
                    return true;
                }
                break;
            case BP_EMPTY: break;
        }
    }
    
    return false;
}

static void
print_bp(void) {
    bp cur;
    for (int i = 0; i < MAX_BREAK; i++) {
        switch ((cur = debugger.bps[i]).type) {
            case BP_INS: fprintf(stdout, "breakpoint %d, INSTRUCTION: %s\n", cur.number, opcodetostr(cur.opcode)); break;
            case BP_CON: fprintf(stdout, "breakpoint %d, CONDITIONAL: %08X\n", cur.number, cur.address); break;
            case BP_EMPTY: break;
        }
    }
}

static bp *
find_bp(int number) {
    for (int i = 0; i < MAX_BREAK; i++) 
        if (debugger.bps[i].number == number)
            return &debugger.bps[i];

    return NULL;
}

static bp *
empty_bp(void) {
    for (int i = 0; i < MAX_WATCH; i++) {
        if (debugger.bps[i].type == BP_EMPTY)
             return &debugger.bps[i];
    }
    
    return NULL;
}

static int
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

static int
remove_bp(int number) {
    bp *remove = find_bp(number);
    
    if (remove == NULL) 
        return 1;

    remove->type = BP_EMPTY;
    return 0;
}

/* WATCHPOINTS */
static bool
hit_wp(void) {
    int i;
    wp cp;

    for (i = 0; i < MAX_WATCH; i++) {
        cp = debugger.wps[i];

        switch (cp.type) {
            case WP_MEM: {
                if (cp.location == debugger.psx->memory->address_accessed)
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

static void
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

static wp *
find_wp(int number) {
    for (int i = 0; i < MAX_BREAK; i++) 
        if (debugger.wps[i].number == number)
            return &debugger.wps[i];

    return NULL;
}

static wp *
empty_wp(void) {
    for (int i = 0; i < MAX_WATCH; i++) {
        if (debugger.wps[i].type == WP_EMPTY)
             return &debugger.wps[i];
    }
    
    return NULL;
}

static int
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

static int
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

static int debugger_previous PARAMS((char *args));
static int debugger_help PARAMS((char *args));
static int debugger_quit PARAMS((char *args));
static int debugger_step PARAMS((char *args));
static int debugger_continue PARAMS((char *args));
static int debugger_breakpoint PARAMS((char *args));
static int debugger_watchpoint PARAMS((char *args));
static int debugger_memory PARAMS((char *args));
static int debugger_cpu PARAMS((char *args));
static int debugger_gpu PARAMS((char *args));
static int debugger_dma PARAMS((char *args));
static int debugger_logging PARAMS((char *args));


static COMMAND commands[] = {
    {"",           "previous command",                       debugger_previous},
    {"help",       "display this text",                      debugger_help},
    {"quit",       "quit emulator",                          debugger_quit},
    {"step",       "step to the next instruction",           debugger_step},
    {"continue",   "continue psx execution",                 debugger_continue},
    {"breakpoint", "create, delete and inspect breakpoints", debugger_breakpoint},
    {"watchpoint", "create, delete and inspect watchpoints", debugger_watchpoint},
    {"memory",     "search memory",                          debugger_memory},
    {"cpu",        "see cpu state",                          debugger_cpu},
    {"gpu",        "see gpu state",                          debugger_gpu},
    {"dma",        "see dma state",                          debugger_dma},
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
            
            if ((opcode = strtoopcode(tok)) == -1)
                return 1;

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
            
            uint32_t reg_num = strtoreg(tok);
            if (reg_num == -1) {
                return 1;
            }

            uint32_t reg_val = debugger.psx->cpu->R[reg_num];
            add_wp(WP_REG, reg_val, reg_num);
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
debugger_cpu PARAMS((char *args)) {
    for (int reg = 0; reg < 32; reg++) {
        printf("[CPU]: REGISTER: %s = %08X\n", register_names[reg], debugger.psx->cpu->R[reg]);
    }

    printf("[CPU]: REGISTER: $hi = %08X\n", debugger.psx->cpu->HI);
    printf("[CPU]: REGISTER: $lo = %08X\n", debugger.psx->cpu->LO);

    printf("[CPU]: REGISTER: $pc = %08X\n", debugger.psx->cpu->PC);
    
    return 0;
}

int
debugger_coprocessor PARAMS((char *args)) {
    for (int reg = 0; reg < 16; reg++) 
        printf("[COP0]: REGISTER: %d = %08X\n", reg, *debugger.psx->cpu->cop0.R[reg]);
    return 0;
}

int
debugger_gpu PARAMS((char *args)) {
    const char *str;
    union GPUSTAT stat = *debugger.psx->gpu->gpustat;
    
    printf("[GPU]:              GPUSTAT = %08X\n", stat.value);
    printf("[GPU]: GPUSTAT: texture_page_x_base         = %d\n", stat.texture_page_x_base * 64);
    printf("[GPU]: GPUSTAT: texture_page_y_base         = %d\n", stat.texture_page_y_base *256);
    switch (stat.semi_transparency) {
        case 0: str = "B/2 + F/2"; break;
        case 1: str = "B+F"; break;
        case 2: str = "B-F"; break;
        case 3: str = "B+F/4"; break;
    }
    printf("[GPU]: GPUSTAT: semi_transparency           = %s\n", str);
    switch (stat.texture_page_colors) {
        case 0: str = "4bit"; break;
        case 1: str = "8bit"; break;
        case 2: str = "16bit"; break;
        case 3: str = "reserved"; break;
    }
    printf("[GPU]: GPUSTAT: texture_page_colors         = %s\n", str);
    printf("[GPU]: GPUSTAT: dither                      = %s\n", (stat.dither) ? "enabled": "disabled");
    printf("[GPU]: GPUSTAT: draw_to_display_area        = %s\n", (stat.draw_to_display_area) ? "enabled": "disabled");
    printf("[GPU]: GPUSTAT: set_mask_when_drawing       = %s\n", (stat.set_mask_when_drawing) ? "enabled": "disabled");
    printf("[GPU]: GPUSTAT: draw_pixels                 = %s\n", (stat.draw_pixels) ? "not to masked": "always");
    printf("[GPU]: GPUSTAT: interlace_field             = %d\n", stat.interlace_field);
    printf("[GPU]: GPUSTAT: reverse_flag                = %s\n", (stat.reverse_flag) ? "distorted": "undistorted");
    printf("[GPU]: GPUSTAT: texture_disable             = %s\n", (stat.texture_disable) ? "disabled": "enabled");
    printf("[GPU]: GPUSTAT: horizontal_resolution_2     = %s\n", (stat.horizontal_resolution_2) ? "256/320/512/640": "328");
    switch (stat.horizontal_resolution_1) {
        case 0: str = "256"; break;
        case 1: str = "320"; break;
        case 2: str = "512"; break;
        case 3: str = "640"; break;
    }
    printf("[GPU]: GPUSTAT: horizontal_resolution_1     = %s\n", str);
    printf("[GPU]: GPUSTAT: vertical_resolution         = %d\n", (stat.vertical_resolution + 1) * 240);
    printf("[GPU]: GPUSTAT: video_mode                  = %s\n", (stat.video_mode) ? "NTSC/60Hz": "PAL/50Hz");
    printf("[GPU]: GPUSTAT: display_area_color_depth    = %s\n", (stat.display_area_color_depth) ? "24bit": "15bit");
    printf("[GPU]: GPUSTAT: vertical_interlace          = %s\n", (stat.vertical_interlace) ? "enabled": "disabled");
    printf("[GPU]: GPUSTAT: display_enable              = %s\n", (stat.display_enable) ? "enabled": "disabled");
    printf("[GPU]: GPUSTAT: interrupt_request           = %s\n", (stat.interrupt_request) ? "enabled": "disabled");
    printf("[GPU]: GPUSTAT: DMA_data_request            = %d\n", stat.DMA_data_request);
    printf("[GPU]: GPUSTAT: ready_recieve_cmd_word      = %s\n", (stat.ready_recieve_cmd_word) ? "ready": "not ready");
    printf("[GPU]: GPUSTAT: ready_send_vram_cpu         = %s\n", (stat.ready_send_vram_cpu) ? "ready": "not ready");
    printf("[GPU]: GPUSTAT: ready_recieve_dma_block     = %s\n", (stat.ready_recieve_dma_block) ? "ready": "not ready");
    switch (stat.horizontal_resolution_1) {
        case 0: str = "off"; break;
        case 1: str = "unknown"; break;
        case 2: str = "cpu_to_gp0"; break;
        case 3: str = "gpuread_to_cpu"; break;
    }
    printf("[GPU]: GPUSTAT: dma_direction               = %s\n", str);
    printf("[GPU]: GPUSTAT: drawing_even_odd_interlace  = %s\n", (stat.drawing_even_odd_interlace) ? "odd": "even or vblank");
    return 0;
}

int 
debugger_dma PARAMS((char *args)) {
    const char *names[] = {
        "mdec in",
        "mdec out",
        "gpu",
        "cdrom",
        "spu",
        "pio",
        "otc"
    };

    struct DMAn dmas[] = {
        debugger.psx->dma->DMA0_MDEC_IN,
        debugger.psx->dma->DMA1_MDEC_OUT,
        debugger.psx->dma->DMA2_GPU,
        debugger.psx->dma->DMA3_CDROM,
        debugger.psx->dma->DMA4_SPU,
        debugger.psx->dma->DMA5_PIO,
        debugger.psx->dma->DMA6_OTC
    };

    char *str;
        
    for (int i = 0; i < 7; i++) {
        struct DMAn d = dmas[i];
        printf("                        DMA%d - %s\n", i, names[i]);
        printf("[DMA%d] MADR: %10s base address             = %08X\n", i, names[i], d.MADR->base_address);

        printf("[DMA%d] BRC:  %10s block                    = %08X\n", i, names[i], d.BRC->value);
        
        printf("[DMA%d] CHCR: %10s transfer direction       = %s\n", i, names[i], (d.CHCR->transfer_direction) ? "ram to device": "device to ram");
        printf("[DMA%d] CHCR: %10s address step             = %s\n", i, names[i], (d.CHCR->address_step) ? "-4": "+4");
        printf("[DMA%d] CHCR: %10s chopping enable          = %s\n", i, names[i], (d.CHCR->chopping_enable) ? "enabled": "disabled");
        switch(d.CHCR->sync_mode) {
            case 0: str = "manual"; break;
            case 1: str = "request"; break;
            case 2: str = "linked list"; break;
        }
        printf("[DMA%d] CHCR: %10s sync mode                = %s\n", i, names[i], str);
        printf("[DMA%d] CHCR: %10s chopping dma window size = %08X\n", i, names[i], d.CHCR->chopping_dma_window_size);
        printf("[DMA%d] CHCR: %10s chopping cpu window size = %08X\n", i, names[i], d.CHCR->chopping_cpu_window_size);
        printf("[DMA%d] CHCR: %10s start busy               = %s\n", i, names[i], (d.CHCR->start_busy) ? "enabled": "disabled");
        printf("[DMA%d] CHCR: %10s start trigger            = %s\n", i, names[i], (d.CHCR->start_trigger) ? "enabled": "disabled");
        printf("\n");
    }
    printf("                         DMA DPRC = %08X\n", debugger.psx->dma->DPRC->value);
    for (int i = 0; i < 7; i++) {
        uint32_t priority = ((debugger.psx->dma->DPRC->value >> i*4)) & 0b0111;
        uint32_t enable   = ((debugger.psx->dma->DPRC->value >> i*4)) & 0b1000;
        
        printf("[DMA]  DPRC: %10s priority                 = %d\n", names[i], priority);
        printf("[DMA]  DPRC: %10s enable                   = %s\n", names[i], (enable) ? "enabled": "disabled");
    }
    printf("\n");

    printf("                        DMA DIRC = %08X\n", debugger.psx->dma->DIRC->value);
    printf("[DMA]  DIRC:     forced irq                      = %s\n", (debugger.psx->dma->DIRC->forced_irq) ? "enabled": "disabled");

    for (int i = 0; i < 7; i++) {
        uint32_t enabled = (debugger.psx->dma->DIRC->irq_enable_sum >> i) & 0b1;
        printf("[DMA]  DIRC: %10s irq enable               = %s\n", names[i], (enabled) ? "enabled": "disabled");
    }

    printf("[DMA]  DIRC:     master irq enable               = %s\n", (debugger.psx->dma->DIRC->irq_enable_master) ? "enabled": "disabled");

    for (int i = 0; i < 7; i++) {
        uint32_t flag = (debugger.psx->dma->DIRC->irq_flag_sum >> i) & 0b1;
        printf("[DMA]  DIRC: %10s irq flag                 = %s\n", names[i], (flag) ? "enabled": "disabled");
    }

    printf("[DMA]  DIRC:            irq signal               = %s\n", (debugger.psx->dma->DIRC->irq_signal) ? "enabled": "disabled");

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

    if (debugger.stepping) {
        debugger.paused = true;
        debugger.stepping = false;
        debugger_input();
    }

    if (debugger.paused)
        debugger_input();
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

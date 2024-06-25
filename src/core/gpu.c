#include "../../include/gpu.h"

static struct GPU gpu;

// helpers
static int fifo_len(void);
static bool fifo_full(void);
static bool fifo_empty(void);
static void reset_fifo(void);
static union COMMAND_PACKET *push_fifo(void);
static union COMMAND_PACKET  peek_fifo(void);
static union COMMAND_PACKET  pop_fifo(void);

static void gpu_tick(void);
static void gpu_handle_gp0(void);
static void gpu_handle_gp1(void);
static void gpu_handle_memory_access(void);
static void gpu_execute_op(void);
static void gpu_set_mode(enum GPU_MODE mode);


// gpu copy helpers
static void gpu_copy_cpu_to_vram(void);
static void gpu_copy_vram_to_cpu(void);

// gp0 instructions
static void GP0_NOP(union COMMAND_PACKET packet);
static void GP0_DIRECT_VRAM_ACCESS(union COMMAND_PACKET packet);
static void GP0_INTERRUPT_REQUEST(union COMMAND_PACKET packet);
static void GP0_RENDER_POLYGONS(union COMMAND_PACKET packet);
static void GP0_RENDER_LINES(union COMMAND_PACKET packet);
static void GP0_RENDER_RECTANGLES(union COMMAND_PACKET packet);
static void GP0_RENDERING_ATTRIBUTES(union COMMAND_PACKET packet);
static void GP0_DISPLAY_CONTROL(union COMMAND_PACKET packet);

// gp1 instructions
static void GP1_RESET(union COMMAND_PACKET packet);
static void GP1_RESET_COMMAND_BUFFER(union COMMAND_PACKET packet);
static void GP1_ACKNOWLEDGE_INTERRUPT(union COMMAND_PACKET packet);
static void GP1_DISPLAY_ENABLE(union COMMAND_PACKET packet);
static void GP1_DMA_DIRECTION_OR_DATA_REQUEST(union COMMAND_PACKET packet);
static void GP1_START_OF_DISPLAY_AREA_IN_VRAM(union COMMAND_PACKET packet);
static void GP1_HORIONTAL_DISPLAY_RANGE(union COMMAND_PACKET packet);
static void GP1_VERTICAL_DISPLAY_RANGE(union COMMAND_PACKET packet);
static void GP1_DISPLAY_MODE(union COMMAND_PACKET packet);
static void GP1_NEW_TEXTURE_DISABLE(union COMMAND_PACKET packet);
static void GP1_SPECIAL_OR_PROTOTYPE_TEXTURE_DISABLE(union COMMAND_PACKET packet);
static void GP1_DISPLAY_INFO(union COMMAND_PACKET packet);

// external interface
struct GPU *get_gpu(void)   { return &gpu; }
uint8_t *write_GP0(void)    { gpu_set_mode(GP0); return (uint8_t *) push_fifo(); }
uint8_t *write_GP1(void)    { gpu_set_mode(GP1); return (uint8_t *) &gpu.gp1.command.value; }
uint8_t *read_GPUSTAT(void) { return (uint8_t *) &gpu.gpustat.value; }
uint8_t *read_GPUREAD(void) { return (uint8_t *) &gpu.gpustat.value; }

bool gpu_vram_write(void) { return gpu.vram_write; }

bool gpustat_display_enable(void)             { return gpu.gpustat.display_enable; }
bool gpustat_interrupt_request(void)          { return gpu.gpustat.interrupt_request; }
bool gpustat_dma_data_request(void)           { return gpu.gpustat.dma_data_request; }
bool gpustat_ready_recieve_cmd_word(void)     { return gpu.gpustat.ready_recieve_cmd_word; }
bool gpustat_ready_send_vram_cpu(void)        { return gpu.gpustat.ready_send_vram_cpu; }
bool gpustat_ready_recieve_dma_block(void)    { return gpu.gpustat.ready_recieve_dma_block; }

uint32_t gpu_display_vram_x_start(void)       { return gpu.display_vram_x_start; }
uint32_t gpu_display_vram_y_start(void)       { return gpu.display_vram_y_start; }

void gpu_reset(void) {
    // set gpustat starting values
    gpu.gpustat.value = 0;
    gpu.gpustat.display_enable = ENABLE;
    gpu.gpustat.ready_recieve_cmd_word = READY;
    gpu.gpustat.ready_recieve_dma_block = READY;
    gpu.gpustat.drawing_even_odd_interlace = ODD;

    // set gp0 and gp1 starting values
    reset_fifo();
}

void gpu_step(void) {
    gpu_tick();

    switch (gpu.current_mode) {
        case IDLE: break;
        case GP0:  gpu_handle_gp0(); break;
        case GP1:  gpu_handle_gp1(); break;
        case COPY: gpu_handle_memory_access(); break;
    }
}

void gpu_handle_gp0(void) {
    gpu_set_mode(IDLE);
    union COMMAND_PACKET command = peek_fifo();

    switch (command.number) {
        case 0X00: GP0_NOP(command); break;
        case 0X01: 
        case 0X02: 
        case 0X80: 
        case 0XA0: 
        case 0XC0: GP0_DIRECT_VRAM_ACCESS(command); break;
        case 0X1F: GP0_INTERRUPT_REQUEST(command); break;
        case 0X03: break; // UNKNOWN
        default:
            switch (command.number >> 4) {
                case 0X02: case 0X03: GP0_RENDER_POLYGONS(command); break;
                case 0X04: case 0X05: GP0_RENDER_LINES(command); break;
                case 0X06: case 0X07: GP0_RENDER_RECTANGLES(command); break;
                case 0X0E:            GP0_RENDERING_ATTRIBUTES(command); break;
                default:
                    break;
            }
            break;
    };
}

void gpu_handle_gp1(void) {
    gpu_set_mode(IDLE);
    union COMMAND_PACKET command = gpu.gp1.command;

    switch (command.number) {
        case 0X00: GP1_RESET(command); break;
        case 0X01: GP1_RESET_COMMAND_BUFFER(command); break;
        case 0X02: GP1_ACKNOWLEDGE_INTERRUPT(command); break;
        case 0X03: GP1_DISPLAY_ENABLE(command); break;
        case 0X04: GP1_DMA_DIRECTION_OR_DATA_REQUEST(command); break;
        case 0X05: GP1_START_OF_DISPLAY_AREA_IN_VRAM(command); break;
        case 0X06: GP1_HORIONTAL_DISPLAY_RANGE(command); break;
        case 0X07: GP1_VERTICAL_DISPLAY_RANGE(command); break;
        case 0X08: GP1_DISPLAY_MODE(command); break;
        case 0X09: GP1_NEW_TEXTURE_DISABLE(command); break;
        case 0X20: break;
        default:
            if ((command.number >> 4)== 0X01) 
                GP1_DISPLAY_INFO(command);
            break;
    }
}

void gpu_handle_memory_access(void) {
    switch (gpu.copy.direction) {
        case VRAM_TO_VRAM: exit(-1);
        case VRAM_TO_CPU:  gpu_copy_vram_to_cpu(); break;
        case CPU_TO_VRAM:  gpu_copy_cpu_to_vram(); break;
    }
    assert(fifo_empty());
}

void gpu_tick(void) {
    gpu.cycles++;

    switch (gpu.gpustat.video_mode) {
        case NTSC60HZ: 
            if (gpu.cycles > 3413) {
                gpu.scanlines++;
                gpu.cycles       = 0;
                gpu.render_phase = HBLANK;
            }

            if (gpu.scanlines > 263) {
                gpu.scanlines    = 0;
                gpu.cycles       = 0;
                gpu.render_phase = VBLANK;
            }
            break;
        case PAL50HZ: 
            if (gpu.cycles > 3406) {
                gpu.scanlines++;
                gpu.cycles       = 0;
                gpu.render_phase = HBLANK;
            }

            if (gpu.scanlines > 314) {
                gpu.scanlines    = 0;
                gpu.cycles       = 0;
                gpu.render_phase = VBLANK;
            }
            break;
    }
}

void gpu_set_mode(enum GPU_MODE mode) {
    switch (mode) {
        case IDLE:
            // lowest priority, if the last mode was interrupted restart it
            gpu.current_mode  = gpu.previous_mode;
            gpu.previous_mode = mode;
            break;
        case GP0:  
            // only changed if current mode is not copy
            gpu.current_mode  = (gpu.current_mode == COPY) ? COPY: mode;
            break;
        case GP1: 
            // can interrupt any mode, highest priority
            gpu.previous_mode = gpu.current_mode;
            gpu.current_mode  = mode;
            break;
        case COPY: 
            // overwrites GP0 mode, 
            gpu.current_mode  = mode;
            break;
    }
}

bool gpu_wait_parameters(int param_num) {
    return !(fifo_len() < param_num);
}

void gpu_copy_vram_to_cpu(void) {
    static uint32_t x, y;
    static uint32_t min_x, max_x;
    static uint32_t min_y, max_y;

    if (!gpu.copy.copying) {
        min_x = gpu.copy.s_x; 
        max_x = gpu.copy.s_x + gpu.copy.s_w;
        min_y = gpu.copy.s_y;
        max_y = gpu.copy.s_y + gpu.copy.s_h;

        x = min_x;
        y = min_y;

        gpu.copy.copying = true;
    }

    uint32_t top, bot;

    if (y == max_y) {
        gpu.gpustat.ready_recieve_dma_block = READY;
        gpu.gpustat.ready_send_vram_cpu = NOT_READY;

        gpu.copy.copying = false;
        gpu_set_mode(IDLE);
        return;
    }

    uint32_t bot_address = VRAM_ADDRESS(x++, y);
    memory_gpu_load_16bit(bot_address, (uint32_t *) &bot);

    if (x == max_x) {
        x = min_x;
        y++;
    }

    uint32_t top_address = VRAM_ADDRESS(x++, y);
    memory_gpu_load_16bit(top_address, &top);

    if (x == max_x) {
        x = min_x;
        y++;
    }

    gpu.gpuread.read = (top << 16) | bot;
}

void gpu_copy_cpu_to_vram(void) {
    if (fifo_empty())
        return;

    static uint32_t x, y;
    static uint32_t min_x, max_x;
    static uint32_t min_y, max_y;

    if (!gpu.copy.copying) {
        min_x = gpu.copy.d_x; 
        max_x = gpu.copy.d_x + gpu.copy.d_w;
        min_y = gpu.copy.d_y;
        max_y = gpu.copy.d_y + gpu.copy.d_h;

        x = min_x;
        y = min_y;

        gpu.copy.copying = true;
    }

    uint32_t data = pop_fifo().value;

    if (y == max_y) {
        gpu.copy.copying = false;
        gpu.vram_write = true;
        gpu_set_mode(IDLE);
        return;
    }

    uint16_t bot = (data >>  0) & 0XFFFF;
    uint16_t top = (data >> 16) & 0XFFFF;

    uint32_t bot_address = VRAM_ADDRESS(x++, y);

    if (x == max_x) {
        x = min_x;
        y++;
    }

    uint32_t top_address = VRAM_ADDRESS(x++, y);

    if (x == max_x) {
        x = min_x;
        y++;
    }

    memory_gpu_store_16bit(bot_address, bot);
    memory_gpu_store_16bit(top_address, top);
}

// command fifo helpers
void reset_fifo(void) {
    gpu.gp0.fifo.head = 0;
    gpu.gp0.fifo.tail = 0;
    gpu.gp0.fifo.len  = 0;
}

int  fifo_len(void)   { return gpu.gp0.fifo.len; }
bool fifo_full(void)  { return (gpu.gp0.fifo.len == FIFO_SIZE); }
bool fifo_empty(void) { return (gpu.gp0.fifo.len == 0); }

union COMMAND_PACKET *push_fifo(void) {
    assert(!fifo_full()); 
    union COMMAND_PACKET *refrence = &gpu.gp0.fifo.commands[gpu.gp0.fifo.tail];
    
    // reserve the next command space 
    // by incrementing the tail of fifo
    gpu.gp0.fifo.tail++;
    gpu.gp0.fifo.tail %= FIFO_SIZE;
    gpu.gp0.fifo.len++;
    
    if (fifo_full())
        gpu.gpustat.dma_data_request = FIFO_FULL;

    return refrence;
}

union COMMAND_PACKET peek_fifo(void) {
    assert(!fifo_empty());
    return gpu.gp0.fifo.commands[gpu.gp0.fifo.head];
}

union COMMAND_PACKET pop_fifo(void) {
    assert(!fifo_empty());
    union COMMAND_PACKET command = gpu.gp0.fifo.commands[gpu.gp0.fifo.head];

    gpu.gp0.fifo.head++;
    gpu.gp0.fifo.head %= FIFO_SIZE;
    gpu.gp0.fifo.len--;

    if (!fifo_full())
        gpu.gpustat.dma_data_request = FIFO_NOT_FULL;
    
    return command;
}

// vram helpers
static void VRAM_CLEAR_CACHE(void);
static void VRAM_FILL_RECTANGLE(void);
static void VRAM_TO_VRAM_COPY_RECTANGLE(void);
static void CPU_TO_VRAM_COPY_RECTANGLE(void);
static void VRAM_TO_CPU_COPY_RECTANGLE(void);

// rendering helpers
// gp0 functions
void GP0_NOP(union COMMAND_PACKET packet) {
    if (!gpu_wait_parameters(1))
        return;

    pop_fifo();
}
void GP0_DIRECT_VRAM_ACCESS(union COMMAND_PACKET packet) {
    switch(packet.number) {
        case 0X01: VRAM_CLEAR_CACHE(); break;
        case 0X02: VRAM_FILL_RECTANGLE(); break;
        case 0X80: VRAM_TO_VRAM_COPY_RECTANGLE(); break;
        case 0XA0: CPU_TO_VRAM_COPY_RECTANGLE(); break;
        case 0XC0: VRAM_TO_CPU_COPY_RECTANGLE(); break;
    }
}
void GP0_INTERRUPT_REQUEST(union COMMAND_PACKET packet) { print_gpu_error("GP0 INTERRUPT REQUEST", "Unimplemented function OP:%x\n", packet.number); }
void GP0_RENDER_POLYGONS(union COMMAND_PACKET packet) {
    switch (packet.number) {
        case 0X20: {
            if (!gpu_wait_parameters(4))
                return;

            printf("RENDER_THREE_POINT_POLYGON_MONOCHROME\n");

            uint32_t c, v1, v2, v3, v4;

            c  = pop_fifo().value;
            v1 = pop_fifo().value;
            v2 = pop_fifo().value; 
            v3 = pop_fifo().value; 

            RENDER_THREE_POINT_POLYGON_MONOCHROME(c, v1, v2, v3, false);

            break;
        }
        case 0X22: {
            if (!gpu_wait_parameters(4))
                return;

            printf("RENDER_THREE_POINT_POLYGON_MONOCHROME\n");

            uint32_t c, v1, v2, v3, v4;

            c  = pop_fifo().value;
            v1 = pop_fifo().value;
            v2 = pop_fifo().value; 
            v3 = pop_fifo().value; 

            RENDER_THREE_POINT_POLYGON_MONOCHROME(c, v1, v2, v3, true);

            break;
        }
        case 0X28: {
            if (!gpu_wait_parameters(5))
                return;

            printf("RENDER_FOUR_POINT_POLYGON_MONOCHROME\n");

            uint32_t c, v1, v2, v3, v4;

            c  = pop_fifo().value;
            v1 = pop_fifo().value;
            v2 = pop_fifo().value; 
            v3 = pop_fifo().value; 
            v4 = pop_fifo().value; 
            
            RENDER_FOUR_POINT_POLYGON_MONOCHROME(c, v1, v2, v3, v4, false);

            break;
        }
        case 0X2A: {
            if (!gpu_wait_parameters(5))
                return;

            printf("RENDER_FOUR_POINT_POLYGON_MONOCHROME\n");

            uint32_t c, v1, v2, v3, v4;

            c  = pop_fifo().value;
            v1 = pop_fifo().value;
            v2 = pop_fifo().value; 
            v3 = pop_fifo().value; 
            v4 = pop_fifo().value; 

            RENDER_FOUR_POINT_POLYGON_MONOCHROME(c, v1, v2, v3, v4, true);

            break;
        }
        case 0X24: {
            if (!gpu_wait_parameters(7))
                return;

            printf("RENDER_THREE_POINT_POLYGON_TEXTURED\n");
            
            uint32_t c, v1, v2, v3, t1, t2, t3;

            c  = pop_fifo().value;
            v1 = pop_fifo().value;
            t1 = pop_fifo().value;
            v2 = pop_fifo().value;
            t2 = pop_fifo().value;
            v3 = pop_fifo().value;
            t3 = pop_fifo().value;

            RENDER_THREE_POINT_POLYGON_TEXTURED(c, v1, t1, v2, t2, v3, t3, false, true);

            break;
        }
        case 0X25: {
            if (!gpu_wait_parameters(7))
                return;

            printf("RENDER_THREE_POINT_POLYGON_TEXTURED\n");
            
            uint32_t c, v1, v2, v3, t1, t2, t3;

            c  = pop_fifo().value;
            v1 = pop_fifo().value;
            t1 = pop_fifo().value;
            v2 = pop_fifo().value;
            t2 = pop_fifo().value;
            v3 = pop_fifo().value;
            t3 = pop_fifo().value;

            RENDER_THREE_POINT_POLYGON_TEXTURED(c, v1, t1, v2, t2, v3, t3, false, false);

            break;
        }
        case 0X26: {
            if (!gpu_wait_parameters(7))
                return;

            printf("RENDER_THREE_POINT_POLYGON_TEXTURED\n");
            
            uint32_t c, v1, v2, v3, t1, t2, t3;

            c  = pop_fifo().value;
            v1 = pop_fifo().value;
            t1 = pop_fifo().value;
            v2 = pop_fifo().value;
            t2 = pop_fifo().value;
            v3 = pop_fifo().value;
            t3 = pop_fifo().value;

            RENDER_THREE_POINT_POLYGON_TEXTURED(c, v1, t1, v2, t2, v3, t3, true, true);

            break;
        }
        case 0X27: {
            if (!gpu_wait_parameters(7))
                return;

            printf("RENDER_THREE_POINT_POLYGON_TEXTURED\n");
            
            uint32_t c, v1, v2, v3, t1, t2, t3;

            c  = pop_fifo().value;
            v1 = pop_fifo().value;
            t1 = pop_fifo().value;
            v2 = pop_fifo().value;
            t2 = pop_fifo().value;
            v3 = pop_fifo().value;
            t3 = pop_fifo().value;

            RENDER_THREE_POINT_POLYGON_TEXTURED(c, v1, t1, v2, t2, v3, t3, true, false);

            break;
        }
        case 0X2C: {
            if (!gpu_wait_parameters(9))
                return;

            printf("RENDER_FOUR_POINT_POLYGON_TEXTURED\n");
            
            uint32_t c, v1, v2, v3, v4, t1, t2, t3, t4;

            c  = pop_fifo().value;
            v1 = pop_fifo().value;
            t1 = pop_fifo().value;
            v2 = pop_fifo().value;
            t2 = pop_fifo().value;
            v3 = pop_fifo().value;
            t3 = pop_fifo().value;
            v4 = pop_fifo().value;
            t4 = pop_fifo().value;

            RENDER_FOUR_POINT_POLYGON_TEXTURED(c, v1, t1, v2, t2, v3, t3, v4, t4, false, true);

            break;
        }
        case 0X2D: {
            if (!gpu_wait_parameters(9))
                return;

            printf("RENDER_FOUR_POINT_POLYGON_TEXTURED\n");
            
            uint32_t c, v1, v2, v3, v4, t1, t2, t3, t4;

            c  = pop_fifo().value;
            v1 = pop_fifo().value;
            t1 = pop_fifo().value;
            v2 = pop_fifo().value;
            t2 = pop_fifo().value;
            v3 = pop_fifo().value;
            t3 = pop_fifo().value;
            v4 = pop_fifo().value;
            t4 = pop_fifo().value;

            RENDER_FOUR_POINT_POLYGON_TEXTURED(c, v1, t1, v2, t2, v3, t3, v4, t4, false, false);

            break;
        }
        case 0X2E: {
            if (!gpu_wait_parameters(9))
                return;

            printf("RENDER_FOUR_POINT_POLYGON_TEXTURED\n");
            
            uint32_t c, v1, v2, v3, v4, t1, t2, t3, t4;

            c  = pop_fifo().value;
            v1 = pop_fifo().value;
            t1 = pop_fifo().value;
            v2 = pop_fifo().value;
            t2 = pop_fifo().value;
            v3 = pop_fifo().value;
            t3 = pop_fifo().value;
            v4 = pop_fifo().value;
            t4 = pop_fifo().value;

            RENDER_FOUR_POINT_POLYGON_TEXTURED(c, v1, t1, v2, t2, v3, t3, v4, t4, true, true);

            break;
        }
        case 0X2F: {
            if (!gpu_wait_parameters(9))
                return;

            printf("RENDER_FOUR_POINT_POLYGON_TEXTURED\n");
            
            uint32_t c, v1, v2, v3, v4, t1, t2, t3, t4;

            c  = pop_fifo().value;
            v1 = pop_fifo().value;
            t1 = pop_fifo().value;
            v2 = pop_fifo().value;
            t2 = pop_fifo().value;
            v3 = pop_fifo().value;
            t3 = pop_fifo().value;
            v4 = pop_fifo().value;
            t4 = pop_fifo().value;

            RENDER_FOUR_POINT_POLYGON_TEXTURED(c, v1, t1, v2, t2, v3, t3, v4, t4, true, false);

            break;
        }
        case 0X30: {
            if (!gpu_wait_parameters(6))
                return;

            printf("RENDER_THREE_POINT_POLYGON_SHADED\n");

            uint32_t c1, v1, v2, c2, v3, c3;

            c1 = pop_fifo().value;
            v1 = pop_fifo().value;
            c2 = pop_fifo().value;
            v2 = pop_fifo().value; 
            c3 = pop_fifo().value;
            v3 = pop_fifo().value; 

            RENDER_THREE_POINT_POLYGON_SHADED(c1, v1, c2, v2, c3, v3, false);

            break;
        }
        case 0X32: {
            if (!gpu_wait_parameters(6))
                return;

            uint32_t c1, v1, v2, c2, v3, c3;

            c1 = pop_fifo().value;
            v1 = pop_fifo().value;
            c2 = pop_fifo().value;
            v2 = pop_fifo().value; 
            c3 = pop_fifo().value;
            v3 = pop_fifo().value; 

            RENDER_THREE_POINT_POLYGON_SHADED(c1, v1, c2, v2, c3, v3, true);

            break;
        }
        case 0X38: {
            if (!gpu_wait_parameters(8))
                return;

            printf("RENDER_FOUR_POINT_POLYGON_SHADED\n");

            uint32_t c1, c2, c3, c4, v1, v2, v3, v4;

            c1 = pop_fifo().value;
            v1 = pop_fifo().value;
            c2 = pop_fifo().value;
            v2 = pop_fifo().value;
            c3 = pop_fifo().value;
            v3 = pop_fifo().value;
            c4 = pop_fifo().value;
            v4 = pop_fifo().value;

            RENDER_FOUR_POINT_POLYGON_SHADED(c1, v1, c2, v2, c3, v3, c4, v4, false);

            break;
        }
        case 0X3A: {
            if (!gpu_wait_parameters(8))
                return;

            printf("RENDER_FOUR_POINT_POLYGON_SHADED\n");

            uint32_t c1, c2, c3, c4, v1, v2, v3, v4;

            c1 = pop_fifo().value;
            v1 = pop_fifo().value;
            c2 = pop_fifo().value;
            v2 = pop_fifo().value;
            c3 = pop_fifo().value;
            v3 = pop_fifo().value;
            c4 = pop_fifo().value;
            v4 = pop_fifo().value;

            RENDER_FOUR_POINT_POLYGON_SHADED(c1, v1, c2, v2, c3, v3, c4, v4, true);

            break;
        }
        case 0X34: {
            if (!gpu_wait_parameters(9))
                return;

            printf("RENDER_THREE_POINT_POLYGON_SHADED_TEXTURED\n");
            
            uint32_t c1, c2, c3, v1, v2, v3, t1, t2, t3;

            c1 = pop_fifo().value;
            v1 = pop_fifo().value;
            t1 = pop_fifo().value;
            c2 = pop_fifo().value;
            v2 = pop_fifo().value;
            t2 = pop_fifo().value;
            c3 = pop_fifo().value;
            v3 = pop_fifo().value;
            t3 = pop_fifo().value;

            RENDER_THREE_POINT_POLYGON_SHADED_TEXTURED(c1, v1, t1, c2, v2, t2, c3, v3, t3, false, true);

            break;
        }
        case 0X36: {
            if (!gpu_wait_parameters(9))
                return;

            printf("RENDER_THREE_POINT_POLYGON_SHADED_TEXTURED\n");
            
            uint32_t c1, c2, c3, v1, v2, v3, t1, t2, t3;

            c1 = pop_fifo().value;
            v1 = pop_fifo().value;
            t1 = pop_fifo().value;
            c2 = pop_fifo().value;
            v2 = pop_fifo().value;
            t2 = pop_fifo().value;
            c3 = pop_fifo().value;
            v3 = pop_fifo().value;
            t3 = pop_fifo().value;

            RENDER_THREE_POINT_POLYGON_SHADED_TEXTURED(c1, v1, t1, c2, v2, t2, c3, v3, t3, true, true);

            break;
        }
        case 0X3C: {
            if (!gpu_wait_parameters(12))
                return;
            
            printf("RENDER_FOUR_POINT_POLYGON_SHADED_TEXTURED\n");

            uint32_t c1, c2, c3, v1, v2, v3, t1, t2, t3, c4, v4, t4;

            c1 = pop_fifo().value;
            v1 = pop_fifo().value;
            t1 = pop_fifo().value;
            c2 = pop_fifo().value;
            v2 = pop_fifo().value;
            t2 = pop_fifo().value;
            c3 = pop_fifo().value;
            v3 = pop_fifo().value;
            t3 = pop_fifo().value;
            c4 = pop_fifo().value;
            v4 = pop_fifo().value;
            t4 = pop_fifo().value;
            

            RENDER_FOUR_POINT_POLYGON_SHADED_TEXTURED(c1, v1, t1, c2, v2, t2, c3, v3, t3, c4, v4, t4, false, true);

            break;
        }
        case 0X3E: {
            if (!gpu_wait_parameters(12))
                return;
            
            printf("RENDER_FOUR_POINT_POLYGON_SHADED_TEXTURED\n");

            uint32_t c1, c2, c3, v1, v2, v3, t1, t2, t3, c4, v4, t4;

            c1 = pop_fifo().value;
            v1 = pop_fifo().value;
            t1 = pop_fifo().value;
            c2 = pop_fifo().value;
            v2 = pop_fifo().value;
            t2 = pop_fifo().value;
            c3 = pop_fifo().value;
            v3 = pop_fifo().value;
            t3 = pop_fifo().value;
            c4 = pop_fifo().value;
            v4 = pop_fifo().value;
            t4 = pop_fifo().value;
            

            RENDER_FOUR_POINT_POLYGON_SHADED_TEXTURED(c1, v1, t1, c2, v2, t2, c3, v3, t3, c4, v4, t4, true, true);

            break;
        }
    }
}
void GP0_RENDER_LINES(union COMMAND_PACKET packet) { print_gpu_error("GP0 OP", "Unimplemented function OP: %x\n", packet.number); pop_fifo(); }
void GP0_RENDER_RECTANGLES(union COMMAND_PACKET packet) { print_gpu_error("GP0 OP", "Unimplemented function OP: %x\n", packet.number); }
void GP0_RENDERING_ATTRIBUTES(union COMMAND_PACKET packet) {
    // pop current command as it doesnt need more arguments
    pop_fifo();
    switch (packet.number & 0b1111) {
        case 0X01: {
            /* DRAWMODE SETTING */
            //  0-3   Texture page X Base   (N*64) (ie. in 64-halfword steps)    ;GPUSTAT.0-3
            //  4     Texture page Y Base   (N*256) (ie. 0 or 256)               ;GPUSTAT.4
            //  5-6   Semi Transparency     (0=B/2+F/2, 1=B+F, 2=B-F, 3=B+F/4)   ;GPUSTAT.5-6
            //  7-8   Texture page colors   (0=4bit, 1=8bit, 2=15bit, 3=Reserved);GPUSTAT.7-8
            //  9     Dither 24bit to 15bit (0=Off/strip LSBs, 1=Dither Enabled) ;GPUSTAT.9
            //  10    Drawing to display area (0=Prohibited, 1=Allowed)          ;GPUSTAT.10
            //  11    Texture Disable (0=Normal, 1=Disable if GP1(09h).Bit0=1)   ;GPUSTAT.15
            //          (Above might be chipselect for (absent) second VRAM chip?)
            //  12    Textured Rectangle X-Flip   (BIOS does set this bit on power-up...?)
            //  13    Textured Rectangle Y-Flip   (BIOS does set it equal to GPUSTAT.13...?)
            //  14-23 Not used (should be 0)
            //  24-31 Command  (E1h)
            gpu.gpustat.texture_page_x_base  = (packet.parameters >>  0) & 0xf;
            gpu.gpustat.texture_page_y_base  = (packet.parameters >>  4) & 0x1;
            gpu.gpustat.semi_transparency    = (packet.parameters >>  5) & 0x3;
            gpu.gpustat.texture_page_colors  = (packet.parameters >>  7) & 0x3;
            gpu.gpustat.dither               = (packet.parameters >>  9) & 0x1;
            gpu.gpustat.draw_to_display_area = (packet.parameters >> 10) & 0x1;
            gpu.gpustat.texture_disable      = (packet.parameters >> 11) & 0x1;
            gpu.texture_rectangle_x_flip     = (packet.parameters >> 12) & 0x1;
            gpu.texture_rectangle_y_flip     = (packet.parameters >> 13) & 0x1;
            break;
        }
        case 0X02: {
            /* TEXTURE WINDOW SETTING */
            //  0-4    Texture window Mask X   (in 8 pixel steps)
            //  5-9    Texture window Mask Y   (in 8 pixel steps)
            //  10-14  Texture window Offset X (in 8 pixel steps)
            //  15-19  Texture window Offset Y (in 8 pixel steps)
            //  20-23  Not used (zero)
            //   24-31  Command  (E2h)
            gpu.texture_window_mask_x   = (packet.parameters >>  0) & 0x1f;
            gpu.texture_window_mask_y   = (packet.parameters >>  5) & 0x1f;
            gpu.texture_window_offset_x = (packet.parameters >> 10) & 0x1f;
            gpu.texture_window_offset_y = (packet.parameters >> 15) & 0x1f;
            break;
        }
        case 0X03: { 
            /* SET DRAWING AREA TOP LEFT */
            //  0-9    X-coordinate (0..1023)
            //  10-19  Y-coordinate (0..1023)  
            //  20-23  Not used (zero)        
            //  24-31  Command  (Exh)
            //
            //  Sets the drawing area corners. The Render commands GP0(20h..7Fh) are automatically clipping any pixels that are outside of this region.
            gpu.drawing_area_left = (packet.parameters >>  0) & 0x3ff;
            gpu.drawing_area_top  = (packet.parameters >> 10) & 0x3ff;
            break;
        }
        case 0X04: {
            /* SET DRAWING AREA BOTTOM RIGHT */
            //  0-9    X-coordinate (0..1023)
            //  10-18  Y-coordinate (0..511)   ;\on Old 160pin GPU (max 1MB VRAM)
            //  19-23  Not used (zero)         ;/
            //  10-19  Y-coordinate (0..1023)  ;\on New 208pin GPU (max 2MB VRAM)
            //  20-23  Not used (zero)         ;/(retail consoles have only 1MB though)
            //  24-31  Command  (Exh)
            //
            //  Sets the drawing area corners. The Render commands GP0(20h..7Fh) are automatically clipping any pixels that are outside of this region.
            gpu.drawing_area_right   = (packet.parameters >>  0) & 0x3ff;
            gpu.drawing_area_bottom  = (packet.parameters >> 10) & 0x1ff;
            break;
        }
        case 0X05: {
            // 0-10   X-offset (-1024..+1023) (usually within X1,X2 of Drawing Area)
            // 11-21  Y-offset (-1024..+1023) (usually within Y1,Y2 of Drawing Area)
            // 22-23  Not used (zero)
            // 24-31  Command  (E5h)
            int16_t x = ((packet.parameters >>  0) & 0x7ff) << 5; // forcing sign extension
            int16_t y = ((packet.parameters >> 11) & 0x7ff) << 5; // forcing sign extension

            gpu.drawing_offset_x = x >> 5;
            gpu.drawing_offset_y = y >> 5;
            break;
        }
        case 0X06: {
            /* MASK BIT SETTING */
            // 0     Set mask while drawing (0=TextureBit15, 1=ForceBit15=1)   ;GPUSTAT.11
            // 1     Check mask before draw (0=Draw Always, 1=Draw if Bit15=0) ;GPUSTAT.12
            // 2-23  Not used (zero)
            // 24-31 Command  (E6h)
            gpu.gpustat.set_mask_when_drawing = (packet.parameters >> 0) & 0x1;
            gpu.gpustat.draw_pixels           = (packet.parameters >> 1) & 0x1;
            break;
        }
    }

}
void GP0_DISPLAY_CONTROL(union COMMAND_PACKET packet) { print_gpu_error("GP0 OP", "Unimplemented function OP: %x\n", packet.number); }

// gp1 functions
void GP1_RESET(union COMMAND_PACKET packet) {
    // Resets the GPU to the following values:
    //
    //  GP1(01h)      ;clear fifo
    //  GP1(02h)      ;ack irq (0)
    //  GP1(03h)      ;display off (1)
    //  GP1(04h)      ;dma off (0)
    //  GP1(05h)      ;display address (0)
    //  GP1(06h)      ;display x1,x2 (x1=200h, x2=200h+256*10)
    //  GP1(07h)      ;display y1,y2 (y1=010h, y2=010h+240)
    //  GP1(08h)      ;display mode 320x200 NTSC (0)
    //  GP0(E1h..E6h) ;rendering attributes (0)
    //
    // Accordingly, GPUSTAT becomes 14802000h.The x1,y1 values are too small, ie. the upper-left edge isn't visible. 
    // Note that GP1(09h) is NOT affected by the reset command.
    
    // clear the fifo
    reset_fifo();

    gpu.gpustat.value = 0X14802000;
    
    gpu.texture_window_mask_x = 0;
    gpu.texture_window_mask_y = 0;
    gpu.texture_window_offset_x = 0;
    gpu.texture_window_offset_y = 0;

    gpu.texture_rectangle_x_flip = false;
    gpu.texture_rectangle_y_flip = false;
    
    gpu.drawing_area_top    = 0;
    gpu.drawing_area_left   = 0;
    gpu.drawing_area_right  = 0;
    gpu.drawing_area_bottom = 0;

    gpu.display_vram_x_start = 0;
    gpu.display_vram_y_start = 0;

    gpu.display_horizontal_start = 0X200;
    gpu.display_horizontal_end   = 0XC00;

    gpu.display_vertical_start = 0X010;
    gpu.display_vertical_end   = 0X100;
}
void GP1_RESET_COMMAND_BUFFER(union COMMAND_PACKET packet) {
    // 0-23  Not used (zero)
    // Clears the command FIFO, and aborts the current rendering command 
    // (eg. this may end up with an incompletely drawn triangle).
    
}
void GP1_ACKNOWLEDGE_INTERRUPT(union COMMAND_PACKET packet) { print_gpu_error("GP1 OP", "Unimplemented function OP: %x\n", packet.number); }
void GP1_DISPLAY_ENABLE(union COMMAND_PACKET packet) { 
    // 0     Display On/Off   (0=On, 1=Off)                         ;GPUSTAT.23
    // 1-23  Not used (zero)
    gpu.gpustat.display_enable = packet.parameters & 0x1;
}
void GP1_DMA_DIRECTION_OR_DATA_REQUEST(union COMMAND_PACKET packet) {
    // 0-1  DMA Direction (0=Off, 1=FIFO, 2=CPUtoGP0, 3=GPUREADtoCPU) ;GPUSTAT.29-30
    // 2-23 Not used (zero)
    gpu.gpustat.dma_direction = packet.parameters & 0b11;
    switch (packet.parameters & 0b11) {
        case 0: gpu.gpustat.dma_data_request = 0; break;
        case 1: gpu.gpustat.dma_data_request = !fifo_full(); break;
        case 2: gpu.gpustat.dma_data_request = gpu.gpustat.ready_recieve_dma_block; break;
        case 3: gpu.gpustat.dma_data_request = gpu.gpustat.ready_send_vram_cpu; break;
    }
}
void GP1_START_OF_DISPLAY_AREA_IN_VRAM(union COMMAND_PACKET packet) {
    // 0-9   X (0-1023)    (halfword address in VRAM)  (relative to begin of VRAM)
    // 10-18 Y (0-511)     (scanline number in VRAM)   (relative to begin of VRAM)
    // 19-23 Not used (zero)
    //
    // Upper/left Display source address in VRAM. The size and target position on 
    // screen is set via Display Range registers; target=X1,Y2; size=(X2-X1/cycles_per_pix), (Y2-Y1).
    gpu.display_vram_x_start = (packet.parameters >>  0) & 0x3ff;
    gpu.display_vram_y_start = (packet.parameters >> 10) & 0x1ff;
}
void GP1_HORIONTAL_DISPLAY_RANGE(union COMMAND_PACKET packet) {
    // 0-11   X1 (260h+0)       ;12bit       ;\counted in 53.222400MHz units,
    // 12-23  X2 (260h+320*8)   ;12bit       ;/relative to HSYNC
    gpu.display_horizontal_start = (packet.parameters >>  0) & 0xfff;
    gpu.display_horizontal_end   = (packet.parameters >> 12) & 0xfff;
}
void GP1_VERTICAL_DISPLAY_RANGE(union COMMAND_PACKET packet) {
    // 0-9   Y1 (NTSC=88h-(224/2), (PAL=A3h-(264/2))  ;\scanline numbers on screen,
    // 10-19 Y2 (NTSC=88h+(224/2), (PAL=A3h+(264/2))  ;/relative to VSYNC
    // 20-23 Not used (zero)
    gpu.display_vertical_start = (packet.parameters >>  0) & 0x3ff;
    gpu.display_vertical_end   = (packet.parameters >> 10) & 0x3ff;
}
void GP1_DISPLAY_MODE(union COMMAND_PACKET packet) {
    // 0-1   Horizontal Resolution 1     (0=256, 1=320, 2=512, 3=640) ;GPUSTAT.17-18
    // 2     Vertical Resolution         (0=240, 1=480, when Bit5=1)  ;GPUSTAT.19
    // 3     Video Mode                  (0=NTSC/60Hz, 1=PAL/50Hz)    ;GPUSTAT.20
    // 4     Display Area Color Depth    (0=15bit, 1=24bit)           ;GPUSTAT.21
    // 5     Vertical Interlace          (0=Off, 1=On)                ;GPUSTAT.22
    // 6     Horizontal Resolution 2     (0=256/320/512/640, 1=368)   ;GPUSTAT.16
    // 7     "Reverseflag"               (0=Normal, 1=Distorted)      ;GPUSTAT.14
    // 8-23  Not used (zero)
    gpu.gpustat.horizontal_resolution_1  = (packet.parameters >> 0) & 0x3;
    gpu.gpustat.vertical_resolution      = (packet.parameters >> 2) & 0x1;
    gpu.gpustat.video_mode               = (packet.parameters >> 3) & 0x1;
    gpu.gpustat.display_area_color_depth = (packet.parameters >> 4) & 0x1;
    gpu.gpustat.vertical_interlace       = (packet.parameters >> 5) & 0x1;
    gpu.gpustat.horizontal_resolution_2  = (packet.parameters >> 6) & 0x1;
    gpu.gpustat.reverse_flag             = (packet.parameters >> 7) & 0x1;

}
void GP1_NEW_TEXTURE_DISABLE(union COMMAND_PACKET packet) { print_gpu_error("GP1 OP", "Unimplemented function OP: %x\n", packet.number); }
void GP1_SPECIAL_OR_PROTOTYPE_TEXTURE_DISABLE(union COMMAND_PACKET packet) { print_gpu_error("GP1 OP", "Unimplemented function OP: %x\n", packet.number); }
void GP1_DISPLAY_INFO(union COMMAND_PACKET packet) { print_gpu_error("GP1 OP", "Unimplemented function OP: %x\n", packet.number); }

void VRAM_CLEAR_CACHE(void) {
    // clear texture cache
    pop_fifo();
}
void VRAM_FILL_RECTANGLE(void) { print_gpu_error("COPY", "Unimplemented function", NULL); }
void VRAM_TO_VRAM_COPY_RECTANGLE(void) {}
void CPU_TO_VRAM_COPY_RECTANGLE(void) {
    //  1st  Command           (Cc000000h)
    //  2nd  Destination Coord (YyyyXxxxh)  ;Xpos counted in halfwords
    //  3rd  Width+Height      (YsizXsizh)  ;Xsiz counted in halfwords
    //  ...  Data              (...)      <--- usually transferred via DMA
    //
    //  Transfers data from CPU to frame buffer. If the number of halfwords to be sent is odd, an extra halfword should be sent (packets consist of 32bit units). The transfer is affected by Mask setting.
    if (!gpu_wait_parameters(3))
        return;

    union COMMAND_PACKET command;
    uint32_t destination, dimensions;

    command     = pop_fifo();
    destination = (uint32_t) pop_fifo().value;
    dimensions  = (uint32_t) pop_fifo().value;

    uint32_t x, y, w, h;
    x = (destination >>  0) & 0XFFFF;
    y = (destination >> 16) & 0XFFFF;
    w = (dimensions  >>  0) & 0XFFFF;
    h = (dimensions  >> 16) & 0XFFFF;

    x = (x % 2 == 1) ? x + 1: x; // if odd halfwords make even
    w = (w % 2 == 1) ? w + 1: w; // if odd halfwords make even

    // base address, multiplied by 2 as they are halfwords
    gpu.copy.d_x = x;
    gpu.copy.d_y = y;
    gpu.copy.d_w = w;
    gpu.copy.d_h = h;

    gpu_set_mode(COPY);
    gpu.copy.direction = CPU_TO_VRAM;
}
void VRAM_TO_CPU_COPY_RECTANGLE(void) {
    //  1st  Command           (Cc000000h) ;
    //  2nd  Source Coord      (YyyyXxxxh) ; write to GP0 port (as usually)
    //  3rd  Width+Height      (YsizXsizh) ;
    //  ...  Data              (...)       ;<--- read from GPUREAD port (or via DMA)
    if (!gpu_wait_parameters(3))
        return;

    union COMMAND_PACKET command;
    uint32_t source, dimensions;
    
    command    = pop_fifo();
    source     = pop_fifo().value;
    dimensions = pop_fifo().value;

    uint32_t x, y, w, h;
    x = (source >>  0) & 0XFFFF;
    y = (source >> 16) & 0XFFFF;
    w = (dimensions >>  0) & 0XFFFF;
    h = (dimensions >> 16) & 0XFFFF;

    gpu.copy.s_x = x;
    gpu.copy.s_y = y;
    gpu.copy.s_w = w;
    gpu.copy.s_h = h;

    gpu.gpustat.ready_send_vram_cpu = READY;
    gpu.gpustat.ready_recieve_dma_block = NOT_READY;

    gpu_set_mode(COPY);
    gpu.copy.direction = VRAM_TO_CPU;
}

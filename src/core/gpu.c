#include "../../include/gpu.h"

static struct GPU gpu;

static void gpu_execute_op(void);

// gpu instructions
static void NOP(void);
static void DIRECT_VRAM_ACCESS(void);
static void INTERRUPT_REQUEST(void);
static void RENDER_POLYGONS(void);
static void RENDER_LINES(void);
static void RENDER_RECTANGLES(void);
static void RENDERING_ATTRIBUTES(void);
static void DISPLAY_CONTROL(void);

#ifdef DEBUG
struct GPU *get_gpu(void) { return &gpu; }
#endif

void gpu_reset(void) {
    gpu.gp0     = (union GP0 *) memory_pointer(0X1F801810);
    gpu.gpuread = (union GPUREAD *) memory_pointer(0X1F801810);
    gpu.gp1     = (union GP1 *) memory_pointer(0X1F801814);
    gpu.gpustat = (union GPUSTAT *) memory_pointer(0X1F801814);

    gpu.gpustat->value = 0;
    gpu.gpustat->display_enable = ENABLE;
    gpu.gpustat->ready_recieve_cmd_word = READY;
    gpu.gpustat->ready_recieve_dma_block = READY;
    gpu.gpustat->drawing_even_odd_interlace = ODD;
}

void gpu_execute_op(void) {
    // GP0
    switch (gpu.tmp) {
        case 0X00: 
            NOP(); break;
        case 0X01: case 0X02: case 0X80: case 0XA0: case 0XC0:
            DIRECT_VRAM_ACCESS(); break;
        case 0X03:
            break; // UNKNOWN
        case 0X1F:
            INTERRUPT_REQUEST(); break;
        case 0X20: case 0X21: case 0X22: case 0X23: 
        case 0X24: case 0X25: case 0X26: case 0X27: 
        case 0X28: case 0X29: case 0X2A: case 0X2B: 
        case 0X2C: case 0X2D: case 0X2E: case 0X2F: 
        case 0X30: case 0X31: case 0X32: case 0X33: 
        case 0X34: case 0X35: case 0X36: case 0X37: 
        case 0X38: case 0X39: case 0X3A: case 0X3B: 
        case 0X3C: case 0X3D: case 0X3E: case 0X3F: 
            RENDER_POLYGONS(); break;
        case 0X40: case 0X41: case 0X42: case 0X43: 
        case 0X44: case 0X45: case 0X46: case 0X47: 
        case 0X48: case 0X49: case 0X4A: case 0X4B: 
        case 0X4C: case 0X4D: case 0X4E: case 0X4F: 
        case 0X50: case 0X51: case 0X52: case 0X53: 
        case 0X54: case 0X55: case 0X56: case 0X57: 
        case 0X58: case 0X59: case 0X5A: case 0X5B: 
        case 0X5C: case 0X5D: case 0X5E: case 0X5F: 
            RENDER_LINES(); break;
        case 0X60: case 0X61: case 0X62: case 0X63: 
        case 0X64: case 0X65: case 0X66: case 0X67: 
        case 0X68: case 0X69: case 0X6A: case 0X6B: 
        case 0X6C: case 0X6D: case 0X6E: case 0X6F: 
        case 0X70: case 0X71: case 0X72: case 0X73: 
        case 0X74: case 0X75: case 0X76: case 0X77: 
        case 0X78: case 0X79: case 0X7A: case 0X7B: 
        case 0X7C: case 0X7D: case 0X7E: case 0X7F: 
            RENDER_RECTANGLES(); break;
        case 0XE1: case 0XE2: case 0XE3: 
        case 0XE4: case 0XE5: case 0XE6: 
            RENDERING_ATTRIBUTES(); break;
        default:
            break;
    };

    switch (gpu.tmp) {
        case 0X00: case 0X01: case 0X02: case 0X03: 
        case 0X04: case 0X05: case 0X06: case 0X07: 
        case 0X08: case 0X09: case 0X0E: case 0X0F: 
        case 0X10: case 0X20:
            DISPLAY_CONTROL(); break;
        default:
            break;
    }
}

void NOP(void) {}
void DIRECT_VRAM_ACCESS(void) {}
void INTERRUPT_REQUEST(void) {}
void RENDER_POLYGONS(void) {}
void RENDER_LINES(void) {}
void RENDER_RECTANGLES(void) {}
void RENDERING_ATTRIBUTES(void) {}
void DISPLAY_CONTROL(void) {}

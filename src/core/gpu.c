#include "../../include/gpu.h"

static struct GPU gpu;

static void gpu_execute_op(void);

// gp0 instructions
static void GP0_NOP(void);
static void GP0_DIRECT_VRAM_ACCESS(void);
static void GP0_INTERRUPT_REQUEST(void);
static void GP0_RENDER_POLYGONS(void);
static void GP0_RENDER_LINES(void);
static void GP0_RENDER_RECTANGLES(void);
static void GP0_RENDERING_ATTRIBUTES(void);
static void GP0_DISPLAY_CONTROL(void);

static void GP1_RESET(void);
static void GP1_RESET_COMMAND_BUFFER(void);
static void GP1_ACKNOWLEDGE_INTERRUPT(void);
static void GP1_DISPLAY_ENABLE(void);
static void GP1_DMA_DIRECTION_OR_DATA_REQUEST(void);
static void GP1_START_OF_DISPLAY_AREA_IN_VRAM(void);
static void GP1_HORIONTAL_DISPLAY_RANGE(void);
static void GP1_VERTICAL_DISPLAY_RANGE(void);
static void GP1_DISPLAY_MODE(void);
static void GP1_NEW_TEXTURE_DISABLE(void);
static void GP1_SPECIAL_OR_PROTOTYPE_TEXTURE_DISABLE(void);
static void GP1_DISPLAY_INFO(void);




struct GPU *get_gpu(void) { return &gpu; }
uint8_t *GP0(void) { return (uint8_t *) &gpu.gp0.value; }
uint8_t *GP1(void) { return (uint8_t *) &gpu.gp1.value; }
uint8_t *GPUSTAT(void) { return (uint8_t *) &gpu.gpustat.value; }
uint8_t *GPUREAD(void) { return (uint8_t *) &gpu.gpustat.value; }

void gpu_reset(void) {
    gpu.gpustat.value = 0;
    gpu.gpustat.display_enable = ENABLE;
    gpu.gpustat.ready_recieve_cmd_word = READY;
    gpu.gpustat.ready_recieve_dma_block = READY;
    gpu.gpustat.drawing_even_odd_interlace = ODD;
}

void gpu_handle_gp1(void) {
    uint8_t command = gpu.gp1.value >> 24;

    switch (command) {
        case 0X00: GP1_RESET(); break;
        case 0X01: GP1_RESET_COMMAND_BUFFER(); break;
        case 0X02: GP1_ACKNOWLEDGE_INTERRUPT(); break;
        case 0X03: GP1_DISPLAY_ENABLE(); break;
        case 0X04: GP1_DMA_DIRECTION_OR_DATA_REQUEST(); break;
        case 0X05: GP1_START_OF_DISPLAY_AREA_IN_VRAM(); break;
        case 0X06: GP1_HORIONTAL_DISPLAY_RANGE(); break;
        case 0X07: GP1_VERTICAL_DISPLAY_RANGE(); break;
        case 0X08: GP1_DISPLAY_MODE(); break;
        case 0X09: GP1_NEW_TEXTURE_DISABLE(); break;
        case 0X20: break;
        default:
            if ((command >> 4)== 0X01) 
                GP1_DISPLAY_INFO();
            break;
    }
}

void gpu_handle_gp0(void) {
    // GP0
    uint8_t command = gpu.gp0.value >> 24;

    switch (command) {
        case 0X00: GP0_NOP(); break;
        case 0X01: 
        case 0X02: 
        case 0X80: 
        case 0XA0: 
        case 0XC0: GP0_DIRECT_VRAM_ACCESS(); break;
        case 0X1F: GP0_INTERRUPT_REQUEST(); break;
        case 0X03: break; // UNKNOWN
        default:
            command >>= 4;
            switch (command) {
                case 0X02: case 0X03: GP0_RENDER_POLYGONS(); break;
                case 0X04: case 0X05: GP0_RENDER_LINES(); break;
                case 0X06: case 0X07: GP0_RENDER_RECTANGLES(); break;
                case 0X0E:            GP0_RENDERING_ATTRIBUTES(); break;
                default:
                    break;
            }
            break;
    };

    switch (gpu.tmp) {
        case 0X00: case 0X01: case 0X02: case 0X03: 
        case 0X04: case 0X05: case 0X06: case 0X07: 
        case 0X08: case 0X09: case 0X0E: case 0X0F: 
        case 0X10: case 0X20:
            GP0_DISPLAY_CONTROL(); break;
        default:
            break;
    }
}

static void GP0_NOP(void) {}
static void GP0_DIRECT_VRAM_ACCESS(void) {}
static void GP0_INTERRUPT_REQUEST(void) {}
static void GP0_RENDER_POLYGONS(void) {}
static void GP0_RENDER_LINES(void) {}
static void GP0_RENDER_RECTANGLES(void) {}
static void GP0_RENDERING_ATTRIBUTES(void) {}
static void GP0_DISPLAY_CONTROL(void) {}

static void GP1_RESET(void) {}
static void GP1_RESET_COMMAND_BUFFER(void) {}
static void GP1_ACKNOWLEDGE_INTERRUPT(void) {}
static void GP1_DISPLAY_ENABLE(void) {}
static void GP1_DMA_DIRECTION_OR_DATA_REQUEST(void) {}
static void GP1_START_OF_DISPLAY_AREA_IN_VRAM(void) {}
static void GP1_HORIONTAL_DISPLAY_RANGE(void) {}
static void GP1_VERTICAL_DISPLAY_RANGE(void) {}
static void GP1_DISPLAY_MODE(void) {}
static void GP1_NEW_TEXTURE_DISABLE(void) {}
static void GP1_SPECIAL_OR_PROTOTYPE_TEXTURE_DISABLE(void) {}
static void GP1_DISPLAY_INFO(void) {}

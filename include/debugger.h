#ifndef DEBUGGER_H_INCLUDED
#define DEBUGGER_H_INCLUDED

#include "common.h"
#include "sdl.h"
#include "cpu.h"
#include "gpu.h"
#include "dma.h"
#include "memory.h"
#include "instruction.h"
#include "coprocessor0.h"
#include "coprocessor2.h"
#include "linkedlist.h"

#define MAXLEN 100
#define CPU_REGISTER_MAXLEN 30

extern PSX_ERROR sdl_handle_events(SDL_Event *, int (*)(SDL_Event *));
extern PSX_ERROR sdl_render_clear(void);
extern PSX_ERROR sdl_render_present(void);

// device refrence getters
extern struct CPU *get_cpu(void);
extern struct GPU *get_gpu(void);
extern struct DMA *get_dma(void);
extern struct MEMORY *get_memory(void);
extern struct SDL_HANDLER *sdl_return_handler(void);

// memory access functions
extern void memory_load_bios(const char *filebios);
extern void memory_cpu_load_8bit(uint32_t address, uint32_t *result);
extern void memory_cpu_load_16bit(uint32_t address, uint32_t *result);
extern void memory_cpu_load_32bit(uint32_t address, uint32_t *result);

// linklist functions
extern void ll_prepend(ll_node_t **head, uint32_t value);
extern void ll_remove_first(ll_node_t **head);
extern int  ll_find(ll_node_t *head, uint32_t value);
extern void ll_destroy(ll_node_t *head);

struct DEBUGGER {
    uint32_t fontscale;

    struct nk_context  *ctx;

    // device refrences
    struct CPU *cpu;
    struct GPU *gpu;
    struct DMA *dma;
    struct MEMORY *memory;
    struct SDL_HANDLER *sdl_handler;

    // Disassembly
    uint32_t assembly_lines;
    char     bios_assembly[0X20000][MAXLEN];
    char     main_assembly[0X80000][MAXLEN];
    char     exp1_assembly[0X200000][MAXLEN];

    uint32_t asm_base_address;
    bool     focus_pc;

    // System Registers
    char cpu_registers[67][CPU_REGISTER_MAXLEN];

    // Breakpoints
    ll_node_t *breakpoints;
    bool hit_breakpoint;

    // Memory
    uint32_t mem_base_address;
    uint32_t mem_view_count;

    // debugger controls
    bool is_paused;
    bool is_stepping;
};

// debug functions
extern void disassemble(void);
extern void peek_cpu_registers(void);


// window size and font size defines 
#define DBG_BUTTON_SIZE 25
#define DBG_TEXT_SIZE   10

#define ASM_WIN_WIDTH  3*WINDOW_SIZE_WIDTH/10
#define ASM_WIN_HEIGHT WINDOW_SIZE_HEIGHT
#define ASM_WIN_X      NATIVE_DISPLAY_WIDTH
#define ASM_WIN_Y      0

#define MEM_VIEW_WIN_WIDTH  NATIVE_DISPLAY_WIDTH/2
#define MEM_VIEW_WIN_HEIGHT WINDOW_SIZE_HEIGHT - NATIVE_DISPLAY_HEIGHT
#define MEM_VIEW_WIN_X      0
#define MEM_VIEW_WIN_Y      NATIVE_DISPLAY_HEIGHT

#define REG_WIN_WIDTH  MEM_VIEW_WIN_WIDTH
#define REG_WIN_HEIGHT MEM_VIEW_WIN_HEIGHT
#define REG_WIN_X      MEM_VIEW_WIN_WIDTH
#define REG_WIN_Y      MEM_VIEW_WIN_Y

#define MEM_NAV_WIN_WIDTH  MENU_WIN_WIDTH
#define MENU_WIN_WIDTH  WINDOW_SIZE_WIDTH - ASM_WIN_X - ASM_WIN_WIDTH
#define MENU_WIN_HEIGHT WINDOW_SIZE_HEIGHT
#define MENU_WIN_X      ASM_WIN_X + ASM_WIN_WIDTH
#define MENU_WIN_Y      ASM_WIN_Y
#define NUM_MENUS       4
#define SUBMENU_HEIGHT    MENU_WIN_HEIGHT/NUM_MENUS
#define SUBMENU_Y(n)      (n-1)*MENU_WIN_HEIGHT/NUM_MENUS

#define ASM_NAV_WIN_WIDTH   MENU_WIN_WIDTH
#define ASM_NAV_WIN_HEIGHT  SUBMENU_HEIGHT
#define ASM_NAV_WIN_X       MENU_WIN_X
#define ASM_NAV_WIN_Y       SUBMENU_Y(1)

#define BRKPNT_WIN_WIDTH  MENU_WIN_WIDTH
#define BRKPNT_WIN_HEIGHT SUBMENU_HEIGHT
#define BRKPNT_WIN_X      MENU_WIN_X
#define BRKPNT_WIN_Y      SUBMENU_Y(2)

#define MEM_NAV_WIN_HEIGHT SUBMENU_HEIGHT
#define MEM_NAV_WIN_X      MENU_WIN_X
#define MEM_NAV_WIN_Y      SUBMENU_Y(3)

#define DEB_CNT_WIN_WIDTH  MENU_WIN_WIDTH
#define DEB_CNT_WIN_HEIGHT SUBMENU_HEIGHT
#define DEB_CNT_WIN_X      MENU_WIN_X
#define DEB_CNT_WIN_Y      SUBMENU_Y(4)



#endif // DEBUGGER_H_INCLUDED

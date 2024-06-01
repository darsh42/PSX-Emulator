#include "../../include/debugger.h"
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_SDL_RENDERER_IMPLEMENTATION
#include "../../include/nuklear.h"
#include "../../include/nuklear_sdl_renderer.h"


struct DEBUGGER debugger;
struct DEBUGGER *get_debugger(void) { return &debugger; }

static void debugger_render(void);
static void debugger_handle_input(void);
static void debugger_draw_psx_screen(void);
static void debugger_draw_registers(void);
static void debugger_draw_assembly(void);
static void debugger_draw_memory_viewer(void);
static void debugger_draw_breakpoint_handler(void);
static void debugger_draw_assembly_navigator(void);
static void debugger_draw_memory_navigator(void);
static void debugger_draw_debug_controls(void);

// custom nuklear widgets
static int hex_str_to_int(const char* str, int *out);
static void nk_property_hex(struct nk_context* ctx, const char* name, int min, int* value, int max);

PSX_ERROR debugger_init(void) {
    debugger.running   = 1;
    debugger.fontscale = 1;
    debugger.assembly_lines = 60;
    debugger.asm_base_address = 0X1FC00000;
    debugger.focus_pc = false;
    disassemble();
    debugger.cpu         = get_cpu();
    debugger.gpu         = get_gpu();
    debugger.dma         = get_dma();
    debugger.memory      = get_memory();

    debugger.sdl_handler = sdl_return_handler();
    debugger.ctx         = nk_sdl_init(debugger.sdl_handler->window, debugger.sdl_handler->renderer);
    disassemble();
    {
        // Load font and cursor
        struct nk_font *font;
        struct nk_font_atlas *atlas;
        struct nk_font_config config = nk_font_config(0);
        
        nk_sdl_font_stash_begin(&atlas);
        font = nk_font_atlas_add_default(atlas, 13 * debugger.fontscale, &config);
        nk_sdl_font_stash_end();

        font->handle.height /= debugger.fontscale;
        nk_style_set_font(debugger.ctx, &font->handle);
    }

    return set_PSX_error(NO_ERROR);
}

PSX_ERROR debugger_exec(void) {
    /* Input handler */
    debugger_handle_input();
    
    /* Draw all parts of the UI */
    // draw content
    debugger_draw_psx_screen();
    debugger_draw_registers();
    debugger_draw_assembly();
    debugger_draw_memory_viewer();

    // Draw menus
    debugger_draw_breakpoint_handler();
    debugger_draw_assembly_navigator();
    debugger_draw_memory_navigator();
    debugger_draw_debug_controls();
    
    /* Rendering */
    debugger_render();
    return set_PSX_error(NO_ERROR);
}

PSX_ERROR debugger_destroy(void) {
    nk_sdl_shutdown();
    return set_PSX_error(NO_ERROR);
}

void debugger_handle_input(void) {
    /* Input handler */
    SDL_Event event;
    nk_input_begin(debugger.ctx);
    sdl_handle_events(&event, nk_sdl_shutdown, nk_sdl_handle_event);
    nk_sdl_handle_grab();
    nk_input_end(debugger.ctx);
}

void debugger_render(void) {
    /* Rendering */
    sdl_render_clear();
    nk_sdl_render(NK_ANTI_ALIASING_ON);
    sdl_render_present();
}

void debugger_draw_psx_screen(void) {
    /* Drawing psx screen */
    if (nk_begin(debugger.ctx, "psx", 
                nk_rect(0, 0, NATIVE_DISPLAY_WIDTH, NATIVE_DISPLAY_HEIGHT),
                NK_WINDOW_BORDER | NK_WINDOW_TITLE))
    {
    }
    nk_end(debugger.ctx);
}

#define ASM_WIN_WIDTH  3*WINDOW_SIZE_WIDTH/10
#define ASM_WIN_HEIGHT 4*WINDOW_SIZE_HEIGHT/6
#define ASM_WIN_X      NATIVE_DISPLAY_WIDTH
#define ASM_WIN_Y      0

void debugger_draw_assembly(void) {
    static uint32_t segment_lookup[] = {
        (uint32_t) 0XFFFFFFFF, (uint32_t) 0XFFFFFFFF, (uint32_t) 0XFFFFFFFF, (uint32_t) 0XFFFFFFFF, // KUSEG
        (uint32_t) 0X7FFFFFFF,                                     // KSEG0
        (uint32_t) 0X1FFFFFFF,                                     // KSEG1
        (uint32_t) 0XFFFFFFFF, (uint32_t) 0XFFFFFFFF               // KSEG2
    };

    char *ptr;
    uint32_t address, region;
    // Follow pc?
    address = (debugger.focus_pc) ? debugger.cpu->PC: debugger.asm_base_address;

    // Map correct disassembly
    region = address & segment_lookup[address >> 29];
    if      (region >= 0X00000000 && region <= 0X00200000) { address = (region - 0X00000000)/4 ; ptr = debugger.main_assembly[address]; }
    else if (region >= 0X1F000000 && region <= 0X1F800000) { address = (region - 0X1F000000)/4 ; ptr = debugger.exp1_assembly[address]; }
    else if (region >= 0X1FC00000 && region <= 0X1FC80000) { address = (region - 0X1FC00000)/4 ; ptr = debugger.bios_assembly[address]; }
    else {
        // set the address to the closest acceptable address
        int32_t main_diff = region - 0X00000000;
        int32_t exp1_diff = region - 0X1F000000;
        int32_t bios_diff = region - 0X1FC00000;
        
        address  = 0;
        if (main_diff < exp1_diff && main_diff < bios_diff) { ptr = debugger.main_assembly[address]; debugger.asm_base_address = 0X00000000; }
        if (exp1_diff < main_diff && exp1_diff < bios_diff) { ptr = debugger.exp1_assembly[address]; debugger.asm_base_address = 0X1F000000; }
        if (bios_diff < main_diff && bios_diff < exp1_diff) { ptr = debugger.bios_assembly[address]; debugger.asm_base_address = 0X1FC00000; }
    }

    /* Draw assembly */
    if (nk_begin(debugger.ctx, "assembly", nk_rect(ASM_WIN_X, ASM_WIN_Y, ASM_WIN_WIDTH, ASM_WIN_HEIGHT),
                NK_WINDOW_BORDER | NK_WINDOW_TITLE)) 
    {
        nk_layout_row_dynamic(debugger.ctx, 10, 1);
        for (int i = 0; ptr && i < debugger.assembly_lines; ptr += 100*i, i++) {
            nk_text(debugger.ctx, ptr, strlen(ptr) - 1, NK_TEXT_LEFT);
        }
    }
    nk_end(debugger.ctx);
}

#define REG_WIN_WIDTH  ASM_WIN_WIDTH/3
#define REG_WIN_HEIGHT WINDOW_SIZE_HEIGHT - ASM_WIN_HEIGHT
#define REG_WIN_X(n)   ASM_WIN_X + (n - 1) * REG_WIN_WIDTH
#define REG_WIN_Y      ASM_WIN_HEIGHT

void debugger_draw_registers(void) {
    /* Drawing cpu registers */
    if (nk_begin(debugger.ctx, "cpu registers", nk_rect(REG_WIN_X(1), REG_WIN_Y, REG_WIN_WIDTH, REG_WIN_HEIGHT),
                NK_WINDOW_BORDER | NK_WINDOW_TITLE))
    {
        peek_cpu_registers();
        nk_layout_row_static(debugger.ctx, 10, 100, 1);
        char *ptr;
        for (int i = 0; i < 51; i++) {
            if ((ptr = debugger.cpu_registers[i])) {
                nk_text(debugger.ctx, ptr, strlen(ptr) - 1, NK_TEXT_LEFT);
            }
        }
    }
    nk_end(debugger.ctx);

    /* Drawing gpu info */
    if (nk_begin(debugger.ctx, "gpu registers", nk_rect(REG_WIN_X(2), REG_WIN_Y, REG_WIN_WIDTH, REG_WIN_HEIGHT),
                NK_WINDOW_BORDER | NK_WINDOW_TITLE))
    {
    }
    nk_end(debugger.ctx);

    /* Drawing dma info */
    if (nk_begin(debugger.ctx, "dma registers", nk_rect(REG_WIN_X(3), REG_WIN_Y, REG_WIN_WIDTH, REG_WIN_HEIGHT),
                NK_WINDOW_BORDER | NK_WINDOW_TITLE))
    {
    }
    nk_end(debugger.ctx);
}

#define MEM_VIEW_WIN_WIDTH  NATIVE_DISPLAY_WIDTH
#define MEM_VIEW_WIN_HEIGHT WINDOW_SIZE_HEIGHT - NATIVE_DISPLAY_HEIGHT
#define MEM_VIEW_WIN_X      0
#define MEM_VIEW_WIN_Y      NATIVE_DISPLAY_HEIGHT

void debugger_draw_memory_viewer(void) {
    if (nk_begin(debugger.ctx, "memory viewer", nk_rect(MEM_VIEW_WIN_X, MEM_VIEW_WIN_Y, MEM_VIEW_WIN_WIDTH, MEM_VIEW_WIN_HEIGHT),
                NK_WINDOW_BORDER | NK_WINDOW_TITLE))
    {
    }
    nk_end(debugger.ctx);
}

#define MENU_WIN_WIDTH  WINDOW_SIZE_WIDTH - ASM_WIN_X - ASM_WIN_WIDTH
#define MENU_WIN_HEIGHT WINDOW_SIZE_HEIGHT
#define MENU_WIN_X      ASM_WIN_X + ASM_WIN_WIDTH
#define MENU_WIN_Y      ASM_WIN_Y
#define NUM_MENUS       4
#define SUBMENU_HEIGHT(n) MENU_WIN_HEIGHT/NUM_MENUS
#define SUBMENU_Y(n)      (n-1)*MENU_WIN_HEIGHT/NUM_MENUS

#define ASM_NAV_WIN_WIDTH   MENU_WIN_WIDTH
#define ASM_NAV_WIN_HEIGHT  SUBMENU_HEIGHT(1)
#define ASM_NAV_WIN_X       MENU_WIN_X
#define ASM_NAV_WIN_Y       SUBMENU_Y(1)

void debugger_draw_assembly_navigator(void) {
    /* Address selector */
    if (nk_begin(debugger.ctx, "assembly navigation", nk_rect(ASM_NAV_WIN_X, ASM_NAV_WIN_Y, ASM_NAV_WIN_WIDTH, ASM_NAV_WIN_HEIGHT), 
                NK_WINDOW_BORDER | NK_WINDOW_TITLE)) 
    {   
        /* Follow pc? */
        nk_layout_row_dynamic(debugger.ctx, 25, 1);
        if (nk_button_symbol_label(debugger.ctx, NK_SYMBOL_CIRCLE_SOLID, "toggle follow PC", NK_TEXT_CENTERED)) { debugger.focus_pc = !debugger.focus_pc; }

        /* Select memory region */
        nk_layout_row_dynamic(debugger.ctx, 25, 4);
        nk_label(debugger.ctx, "region", NK_TEXT_LEFT);
        if (nk_button_label(debugger.ctx, "main")) { debugger.asm_base_address = 0X00000000; }
        if (nk_button_label(debugger.ctx, "exp1")) { debugger.asm_base_address = 0X1F000000; }
        if (nk_button_label(debugger.ctx, "bios")) { debugger.asm_base_address = 0X1FC00000; }

        /* Move forward and backward in assembly */
        nk_layout_row_dynamic(debugger.ctx, 25, 3);
        nk_label(debugger.ctx, "page", NK_TEXT_LEFT);
        if (nk_button_symbol_label(debugger.ctx, NK_SYMBOL_TRIANGLE_LEFT, "prev", NK_TEXT_RIGHT)) { debugger.asm_base_address -= debugger.assembly_lines; }
        if (nk_button_symbol_label(debugger.ctx, NK_SYMBOL_TRIANGLE_RIGHT, "next", NK_TEXT_LEFT)) { debugger.asm_base_address += debugger.assembly_lines; }
    }
    nk_end(debugger.ctx);
}

#define BRKPNT_WIN_WIDTH  MENU_WIN_WIDTH
#define BRKPNT_WIN_HEIGHT SUBMENU_HEIGHT(2)
#define BRKPNT_WIN_X      MENU_WIN_X
#define BRKPNT_WIN_Y      SUBMENU_Y(2)

void debugger_draw_breakpoint_handler(void) {
    if (nk_begin(debugger.ctx, "breakpoint creator", nk_rect(BRKPNT_WIN_X, BRKPNT_WIN_Y, BRKPNT_WIN_WIDTH, BRKPNT_WIN_HEIGHT),
                NK_WINDOW_BORDER | NK_WINDOW_TITLE)) 
    {
    }
    nk_end(debugger.ctx);
}

#define MEM_NAV_WIN_WIDTH  MENU_WIN_WIDTH
#define MEM_NAV_WIN_HEIGHT SUBMENU_HEIGHT(3)
#define MEM_NAV_WIN_X      MENU_WIN_X
#define MEM_NAV_WIN_Y      SUBMENU_Y(3)

void debugger_draw_memory_navigator(void) {
    if (nk_begin(debugger.ctx, "memory navigator", nk_rect(MEM_NAV_WIN_X, MEM_NAV_WIN_Y, MEM_NAV_WIN_WIDTH, MEM_NAV_WIN_HEIGHT),
                NK_WINDOW_BORDER | NK_WINDOW_TITLE))
    {
    }
    nk_end(debugger.ctx);
}

#define DEB_CNT_WIN_WIDTH  MENU_WIN_WIDTH
#define DEB_CNT_WIN_HEIGHT SUBMENU_HEIGHT(4)
#define DEB_CNT_WIN_X      MENU_WIN_X
#define DEB_CNT_WIN_Y      SUBMENU_Y(4)

void debugger_draw_debug_controls(void) {
    if (nk_begin(debugger.ctx, "debug controls", nk_rect(DEB_CNT_WIN_X, DEB_CNT_WIN_Y, DEB_CNT_WIN_WIDTH, DEB_CNT_WIN_HEIGHT),
                NK_WINDOW_BORDER | NK_WINDOW_TITLE))
    {
    }
    nk_end(debugger.ctx);
}

////////////////// UTILS ///////////////////////////////////////

// Utility function to convert an integer to a hexadecimal string
void int_to_hex_str(int value, char* buffer, int buffer_size) {
    snprintf(buffer, buffer_size, "%X", value);
}

// Utility function to convert a hexadecimal string to an integer
int hex_str_to_int(const char* str, int *out) {
    return sscanf(str, "%X", out);
}

// Custom widget to display and edit an integer value in hexadecimal format
void nk_property_hex(struct nk_context* ctx, const char* name, int min, int* value, int max) {
    char buffer[16];
    int len;
    int temp_value = *value;

    // Convert the current value to a hexadecimal string
    int_to_hex_str(temp_value, buffer, sizeof(buffer));
    len = strlen(buffer);

    // Layout row for the property
    nk_layout_row_dynamic(ctx, 25, 2);
    nk_label(ctx, name, NK_TEXT_LEFT);

    // Editable text field with hex filter
    if (nk_edit_string(ctx, NK_EDIT_FIELD, buffer, &len, sizeof(buffer), nk_filter_hex)) {
        buffer[len] = '\0'; // Ensure null-terminated string

        // Try to convert the hex string to an integer
        if (hex_str_to_int(buffer, &temp_value)) {
            // Clamp the value within the specified range
            if (temp_value < min) temp_value = min;
            if (temp_value > max) temp_value = max;

            // Update the original value
            *value = temp_value;
        }
    }
}

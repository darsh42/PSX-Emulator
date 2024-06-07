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

PSX_ERROR debugger_init(void);
PSX_ERROR debugger_destroy(void);
PSX_ERROR debugger_exec(void);

static void draw_debugger(struct nk_context *ctx);

PSX_ERROR debugger_init(void) {
    debugger.fontscale = 1;
    debugger.assembly_lines = 65;
    debugger.asm_base_address = 0X1FC00000;
    debugger.focus_pc = false;
    debugger.mem_base_address = 0X00000000;
    debugger.mem_view_count   = 512;
    debugger.hit_breakpoint   = false;
    debugger.breakpoints      = NULL;
    debugger.is_paused        = true;

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
    do {
        /* Input handler */
        SDL_Event event;
        nk_input_begin(debugger.ctx);
        sdl_handle_events(&event, nk_sdl_handle_event);
        
        switch (debugger.sdl_handler->status) {
            case QUIT:          debugger_destroy(); break;
            case TOGGLE_PAUSE:  debugger.is_paused = !debugger.is_paused; break;
            case STEPINTO:      break; 
            case STEP:          break; 
            case STEPOVER:      break; 
            case TOGGLE_FOLLOW_PC: debugger.focus_pc = !debugger.focus_pc; break;
            case CONTINUE: break;
        }
        nk_input_end(debugger.ctx);
        
        draw_debugger(debugger.ctx);

        sdl_render_clear();
        nk_sdl_render(NK_ANTI_ALIASING_ON);
        sdl_render_present();
        
        if (debugger.is_stepping) {
            debugger.is_stepping = false;
            debugger.is_paused   = true;
            break;
        }

    } while (debugger.is_paused);
    
    return set_PSX_error(NO_ERROR);
}

PSX_ERROR debugger_destroy(void) {
    nk_sdl_shutdown();
    return set_PSX_error(NO_ERROR);
}

#define TOGGLE(truth) (truth) ? NK_SYMBOL_CIRCLE_SOLID  : \
                                NK_SYMBOL_CIRCLE_OUTLINE

void draw_debugger(struct nk_context *ctx) {
    // update all status values
    peek_cpu_registers();

    // text properties
    int  t_height = 12;
    int  t_buffer_size; 
    static char t_buffer[MAXLEN];

    // label properties
    int l_height = 12;
    int l_buffer_size;
    static char l_buffer[MAXLEN];

    // button properties
    int b_height = 25;
    int b_buffer_size;
    static char b_buffer[MAXLEN];

    // edit string properties
    int e_height = 25;
    
    // window flags
    int w_c_flags = NK_WINDOW_BORDER | NK_WINDOW_TITLE;
    int w_n_flags = NK_WINDOW_BORDER | NK_WINDOW_TITLE;

    // content windows
    struct nk_rect w_c_psx       = {0, 0, NATIVE_DISPLAY_WIDTH, NATIVE_DISPLAY_HEIGHT};
    struct nk_rect w_c_assembly  = {ASM_WIN_X, ASM_WIN_Y, ASM_WIN_WIDTH, ASM_WIN_HEIGHT};
    struct nk_rect w_c_registers = {REG_WIN_X, REG_WIN_Y, REG_WIN_WIDTH, REG_WIN_HEIGHT};
    struct nk_rect w_c_memory    = {MEM_VIEW_WIN_X, MEM_VIEW_WIN_Y, MEM_VIEW_WIN_WIDTH, MEM_VIEW_WIN_HEIGHT};

    // navigation windows
    struct nk_rect w_n_menus       = {MENU_WIN_X, MENU_WIN_Y, MENU_WIN_WIDTH, MENU_WIN_HEIGHT};
    struct nk_rect w_n_assembly    = {ASM_NAV_WIN_X, ASM_NAV_WIN_Y, ASM_NAV_WIN_WIDTH, ASM_NAV_WIN_HEIGHT};
    struct nk_rect w_n_memory      = {MEM_NAV_WIN_X, MEM_NAV_WIN_Y, MEM_NAV_WIN_WIDTH, MEM_NAV_WIN_HEIGHT};
    struct nk_rect w_n_controls    = {DEB_CNT_WIN_X, DEB_CNT_WIN_Y, DEB_CNT_WIN_WIDTH, DEB_CNT_WIN_HEIGHT};
    struct nk_rect w_n_breakpoints = {BRKPNT_WIN_X, BRKPNT_WIN_Y, BRKPNT_WIN_WIDTH, BRKPNT_WIN_HEIGHT};
    
    if (nk_begin(ctx, "psx render", w_c_psx, w_c_flags)) 
    {
    }
    nk_end(ctx);

    if (nk_begin(ctx, "assembly", w_c_assembly, w_c_flags)) 
    {
        if (debugger.is_paused) 
        {
            nk_layout_row_dynamic(debugger.ctx, t_height, 1);

            static uint32_t segment_lookup[] = {
                (uint32_t) 0XFFFFFFFF, (uint32_t) 0XFFFFFFFF, 
                (uint32_t) 0XFFFFFFFF, (uint32_t) 0XFFFFFFFF,
                (uint32_t) 0X7FFFFFFF, (uint32_t) 0X1FFFFFFF,
                (uint32_t) 0XFFFFFFFF, (uint32_t) 0XFFFFFFFF
            };

            char *ptr;
            uint32_t address, region;
            // Follow pc?
            address = (debugger.focus_pc) ? debugger.cpu->PC: debugger.asm_base_address;

            // Map correct disassembly
            region = address & segment_lookup[address >> 29];
            if (region >= 0X00000000 && region <= 0X00200000) { 
                address   = (region - 0X00000000);
                address >>= 2;

                ptr = debugger.main_assembly[address]; 
            } else if (region >= 0X1F000000 && region <= 0X1F800000) {
                address   = (region - 0X1F000000); 
                address >>= 2;

                ptr = debugger.exp1_assembly[address]; 
            }
            else if (region >= 0X1FC00000 && region <= 0X1FC80000) { 
                address   = (region - 0X1FC00000); 
                address >>= 2;

                ptr = debugger.bios_assembly[address];
            }
            else {
                // set the address to the closest acceptable address
                uint32_t main_diff = region - 0X00000000;
                uint32_t exp1_diff = region - 0X1F000000;
                uint32_t bios_diff = region - 0X1FC00000;

                if (main_diff < exp1_diff && main_diff < bios_diff) { 
                    debugger.asm_base_address = 0X00000000; 

                    ptr = debugger.main_assembly[address]; 
                }
                if (exp1_diff < main_diff && exp1_diff < bios_diff) {
                    debugger.asm_base_address = 0X1F000000; 

                    ptr = debugger.exp1_assembly[address]; 
                }
                if (bios_diff < main_diff && bios_diff < exp1_diff) { 
                    debugger.asm_base_address = 0X1FC00000; 

                    ptr = debugger.bios_assembly[address]; 
                }
            }

            /* Draw assembly */
            for (int i = 1; ptr && i < debugger.assembly_lines; ptr += 100, i++) {
                nk_text(ctx, ptr, strlen(ptr) - 1, NK_TEXT_LEFT);
            }
        }
    }
    nk_end(ctx);
    
    if (nk_begin(ctx, "registers", w_c_registers, w_c_flags | NK_WINDOW_NO_SCROLLBAR)) 
    {
        if (debugger.is_paused) 
        {
            char *ptr;
            nk_layout_row_dynamic(ctx, REG_WIN_HEIGHT - 50, 3);
            
            if (nk_group_begin(ctx, "cpu", 0)) 
            {
                nk_layout_row_dynamic(ctx, t_height, 1);

                for (int i = 0; i < 51; i++) 
                {
                    if ((ptr = debugger.cpu_registers[i])) 
                    {
                        nk_text(debugger.ctx, ptr, strlen(ptr) - 1, NK_TEXT_CENTERED);
                    }
                }
                nk_group_end(ctx);
            }

            if (nk_group_begin(ctx, "gpu", 0)) 
            {
                nk_layout_row_dynamic(ctx, t_height, 1);

                nk_group_end(ctx);
            }

            if (nk_group_begin(ctx, "dma", 0)) 
            {
                nk_layout_row_dynamic(ctx, t_height, 1);

                nk_group_end(ctx);
            }
        }
    }
    nk_end(ctx);

    if (nk_begin(ctx, "memory", w_c_memory, w_c_flags | NK_WINDOW_NO_SCROLLBAR)) 
    {
        if (debugger.is_paused)
        {

            float size[] = {0.2f * MEM_VIEW_WIN_WIDTH, 
                            0.8f * MEM_VIEW_WIN_WIDTH};
            uint32_t value, address = debugger.mem_base_address;
            
            nk_layout_row(ctx, NK_STATIC, 2*t_height, 2, size);

            /* Top index */
            if (nk_group_begin(ctx, "padding", 0)) 
            {
                nk_group_end(ctx);
            }

            if (nk_group_begin(ctx, "index", 0)) 
            {
                nk_layout_row_static(ctx, t_height, 20, 16);
                
                for (int i = 0; i < 16; i++) 
                {
                    snprintf(t_buffer, sizeof(t_buffer), "%02X", i);

                    nk_text(ctx, t_buffer, strlen(t_buffer), NK_TEXT_CENTERED);
                }

                nk_group_end(ctx);
            }


            nk_layout_row(ctx, NK_STATIC, MEM_VIEW_WIN_WIDTH-t_height-1, 2, size);

            /* Address viewer */
            if (nk_group_begin(ctx, "address", NK_WINDOW_NO_SCROLLBAR)) 
            {   
                nk_layout_row_dynamic(ctx, t_height, 1);

                for (int i = 0; i < debugger.mem_view_count; i+=16) 
                {
                    snprintf(t_buffer, sizeof(t_buffer), "%08X", address + i);

                    nk_text(ctx, t_buffer, strlen(t_buffer), NK_TEXT_CENTERED);
                }

                nk_group_end(ctx);
            }

            /* Value viewer */
            if (nk_group_begin(ctx, "memory", NK_WINDOW_NO_SCROLLBAR)) 
            {
                nk_layout_row_static(ctx, t_height, 20, 16);

                for (int i = 0; i < debugger.mem_view_count; i++) 
                {
                    memory_cpu_load_8bit(address+i, &value);
                    snprintf(t_buffer, sizeof(t_buffer), "%02X", value);

                    nk_text(ctx, t_buffer, strlen(t_buffer), NK_TEXT_CENTERED);
                }

                nk_group_end(ctx);
            }
        }
    }
    nk_end(ctx);
    
    if (nk_begin(ctx, "controls", w_n_menus, w_n_flags)) 
    {   
        static int memory_buffer_size;
        static char memory_buffer[MAXLEN];

        static int breakpoint_buffer_size;
        static char breakpoint_buffer[MAXLEN];

        int address;

        float ratio[] = {0.25f, 0.375f, 0.375f};

        /* Print current breakpoints */
        nk_layout_row_dynamic(ctx, b_height, 1);
        nk_label(ctx, "Current breakpoints", NK_TEXT_LEFT);

        nk_layout_row_dynamic(ctx, 100, 1);
        if (nk_group_begin(ctx, "Current breakpoints", NK_WINDOW_BORDER))
        {
            nk_layout_row_dynamic(ctx, t_height, 1);

            for(ll_node_t *current = debugger.breakpoints; current; current = current->next) 
            {
                snprintf(t_buffer, sizeof(t_buffer), "%08X", current->value);

                if (nk_button_label(ctx, t_buffer))
                {   
                    ll_remove_find(&debugger.breakpoints, (uint32_t) strtol(t_buffer, NULL, 16));
                }

                // nk_text(ctx, t_buffer, strlen(t_buffer), NK_TEXT_LEFT);
            }

            nk_group_end(ctx);
        }
        
        /* Create new breakpoints */
        nk_layout_row_dynamic(ctx, b_height, 2);

        nk_label(ctx, "breakpoint address", NK_TEXT_LEFT);
        nk_edit_string(ctx, NK_EDIT_SIMPLE, breakpoint_buffer, &breakpoint_buffer_size, 9, nk_filter_hex);

        nk_label(ctx, "inspect address", NK_TEXT_LEFT);
        nk_edit_string(ctx, NK_EDIT_SIMPLE, memory_buffer, &memory_buffer_size, 9, nk_filter_hex);
        
        nk_layout_row_dynamic(ctx, b_height, 1);
        if (nk_button_label(ctx, "add")) 
        {
            if (breakpoint_buffer_size > 0) 
            {
              sscanf(breakpoint_buffer, "%X", &address);
              ll_prepend(&debugger.breakpoints, address);
            }

            if (memory_buffer_size > 0) 
            {
              sscanf(memory_buffer, "%X", &address);
              debugger.mem_base_address = address;
            }
        }

        /* Assembly controls */
        /* Select memory region */
        nk_layout_row_dynamic(ctx, b_height, 4);

        nk_label(ctx, "region", NK_TEXT_LEFT);
        if (nk_button_label(ctx, "main")) 
        { 
            debugger.asm_base_address = 0X00000000; 
        }
        if (nk_button_label(ctx, "exp1")) 
        { 
            debugger.asm_base_address = 0X1F000000; 
        }
        if (nk_button_label(ctx, "bios")) 
        { 
            debugger.asm_base_address = 0X1FC00000; 
        }
        
        /* Move forward and backward in assembly */
        nk_layout_row_dynamic(ctx, b_height, 3);

        nk_label(ctx, "asm scroll", NK_TEXT_LEFT);
        if (nk_button_symbol_label(ctx, NK_SYMBOL_TRIANGLE_LEFT, "prev", NK_TEXT_RIGHT)) 
        { 
            debugger.asm_base_address -= debugger.assembly_lines; 
        }
        if (nk_button_symbol_label(ctx, NK_SYMBOL_TRIANGLE_RIGHT, "next", NK_TEXT_LEFT)) 
        { 
            debugger.asm_base_address += debugger.assembly_lines; 
        }

        /* Breakpoint controls */

        nk_layout_row_dynamic(ctx, b_height, 3);

        nk_label(ctx, "mem scroll", NK_TEXT_LEFT);
        if (nk_button_symbol_label(ctx, NK_SYMBOL_TRIANGLE_LEFT, "prev", NK_TEXT_RIGHT)) 
        { 
            debugger.mem_base_address -= debugger.mem_view_count; 
        }
        if (nk_button_symbol_label(ctx, NK_SYMBOL_TRIANGLE_RIGHT, "next", NK_TEXT_LEFT)) 
        { 
            debugger.mem_base_address += debugger.mem_view_count; 
        }

        nk_layout_row_dynamic(ctx, b_height, 1);

        if (nk_button_label(ctx, "disassemble")) 
        {
            disassemble();
        }

        if (nk_button_symbol_label(ctx, TOGGLE(debugger.focus_pc), "follow PC", NK_TEXT_CENTERED))
        {
            debugger.focus_pc = !debugger.focus_pc; 
        }

        if (nk_button_label(ctx, "step")) 
        {
            debugger.is_stepping = true;
        }

        if (nk_button_symbol_label(ctx, TOGGLE(debugger.is_paused), "pause", NK_TEXT_CENTERED))
        {
            debugger.is_paused = !debugger.is_paused;
        }

    }
    nk_end(ctx);

    // check for breakpoint hit
    for (ll_node_t *current = debugger.breakpoints; current; current = current->next) 
    {
        if (debugger.cpu->PC == current->value) 
        {
            debugger.is_paused = true;
            break;
        }
    }

}

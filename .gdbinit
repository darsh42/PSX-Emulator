# tui layout
tui new-layout psx {-horizontal src 1 cmd 2} 2
tui layout psx
tui focus cmd

set pagination off

# cpu debugging features
define FUNCT
    p/x ((cpu.cir >>  0) & 0x3F)
end
define SHAMT    
    p/x ((cpu.cir >>  6) & 0x1F)
end
define RD       
    p/x ((cpu.cir >> 11) & 0x1F)
end
define RT       
    p/x ((cpu.cir >> 16) & 0x1F)
end
define RS       
    p/x ((cpu.cir >> 21) & 0x1F)
end
define OP       
    p/x ((cpu.cir >> 26) & 0x3F)
end
define TARGET    
    p/x (cpu.cir & ((1 << 26) - 1))
end
define IMM16     
    p/x (cpu.cir & ((1 << 16) - 1))
end
define IMM25     
    p/x (cpu.cir & ((1 << 25) - 1))
end
define RELATIVE  
    p/x (cpu.cir & ((1 << 16) - 1))
end

# gpu debugging features

define gpu_misc
    printf "gpu_misc\n"
    printf "    cycles:    %d\n", gpu.cycles
    printf "    dots:      %d\n", gpu.dots
    printf "    scanlines: %d\n", gpu.scanlines
    printf "    vblank:    %d\n", gpu.vblank
    printf "    hblank:    %d\n", gpu.hblank
    printf "\n"
    printf "    vram_direct_access_x: %d\n", gpu.vram_direct_access_x
    printf "    vram_direct_access_y: %d\n", gpu.vram_direct_access_y
    printf "    vram_direct_access_w: %d\n", gpu.vram_direct_access_w
    printf "    vram_direct_access_h: %d\n", gpu.vram_direct_access_h
    printf "    vram_direct_access_cx: %d\n", gpu.vram_direct_access_cx
    printf "    vram_direct_access_cy: %d\n", gpu.vram_direct_access_cy
    printf "\n"
    printf "    texture_window_mask_x:    %d\n", gpu.texture_window_mask_x   
    printf "    texture_window_mask_y:    %d\n", gpu.texture_window_mask_y   
    printf "    texture_window_offset_x:  %d\n", gpu.texture_window_offset_x 
    printf "    texture_window_offset_y:  %d\n", gpu.texture_window_offset_y 
    printf "\n"
    printf "    drawing_area_top:         %d\n", gpu.drawing_area_top        
    printf "    drawing_area_left:        %d\n", gpu.drawing_area_left       
    printf "    drawing_area_right:       %d\n", gpu.drawing_area_right      
    printf "    drawing_area_bottom:      %d\n", gpu.drawing_area_bottom     
    printf "\n"
    printf "    drawing_offset_x:         %d\n", gpu.drawing_offset_x        
    printf "    drawing_offset_y:         %d\n", gpu.drawing_offset_y        
    printf "\n"
    printf "    display_vram_x_start:     %d\n", gpu.display_vram_x_start    
    printf "    display_vram_y_start:     %d\n", gpu.display_vram_y_start    
    printf "    display_horizontal_start: %d\n", gpu.display_horizontal_start
    printf "    display_horizontal_end:   %d\n", gpu.display_horizontal_end  
    printf "    display_vertical_start:   %d\n", gpu.display_vertical_start  
    printf "    display_vertical_end:     %d\n", gpu.display_vertical_end    
end

define gpustat
    printf "gpustat\n"
    printf "    texture_page_x_base        %d\n", gpu.gpustat.texture_page_x_base         
    printf "    texture_page_y_base        %d\n", gpu.gpustat.texture_page_y_base         
    printf "    semi_transparency          %d\n", gpu.gpustat.semi_transparency           
    printf "    texture_page_colors        %d\n", gpu.gpustat.texture_page_colors         
    printf "    dither                     %d\n", gpu.gpustat.dither                      
    printf "    draw_to_display_area       %d\n", gpu.gpustat.draw_to_display_area        
    printf "    set_mask_when_drawing      %d\n", gpu.gpustat.set_mask_when_drawing       
    printf "    draw_pixels                %d\n", gpu.gpustat.draw_pixels                 
    printf "    interlace_field            %d\n", gpu.gpustat.interlace_field             
    printf "    reverse_flag               %d\n", gpu.gpustat.reverse_flag                
    printf "    texture_disable            %d\n", gpu.gpustat.texture_disable             
    printf "    horizontal_resolution_2    %d\n", gpu.gpustat.horizontal_resolution_2     
    printf "    horizontal_resolution_1    %d\n", gpu.gpustat.horizontal_resolution_1     
    printf "    vertical_resolution        %d\n", gpu.gpustat.vertical_resolution         
    printf "    video_mode                 %d\n", gpu.gpustat.video_mode                  
    printf "    display_area_color_depth   %d\n", gpu.gpustat.display_area_color_depth    
    printf "    vertical_interlace         %d\n", gpu.gpustat.vertical_interlace          
    printf "    display_enable             %d\n", gpu.gpustat.display_enable              
    printf "    interrupt_request          %d\n", gpu.gpustat.interrupt_request           
    printf "    dma_data_request           %d\n", gpu.gpustat.dma_data_request            
    printf "    ready_recieve_cmd_word     %d\n", gpu.gpustat.ready_recieve_cmd_word      
    printf "    ready_send_vram_cpu        %d\n", gpu.gpustat.ready_send_vram_cpu         
    printf "    ready_recieve_dma_block    %d\n", gpu.gpustat.ready_recieve_dma_block     
    printf "    dma_direction              %d\n", gpu.gpustat.dma_direction               
    printf "    drawing_even_odd_interlace %d\n", gpu.gpustat.drawing_even_odd_interlace  
end

define gp0
    printf "gp0\n"
    printf "    size:   %d\n", gpu.gp0.size
    printf "    head:   %d\n", gpu.gp0.head
    printf "    tail:   %d\n", gpu.gp0.tail  
    printf "    length: %d\n", gpu.gp0.length
    printf "\n"
    printf "    commands:\n"
    set $d = gpu.gp0.head
    while $d != gpu.gp0.length
        printf "    %x\n", gpu.gp0.data[$d]
        set $d = $d + 1
    end
end

# dma debugging features
define dmadpcr
    set $dpcr = dma.dpcr
    printf "dma dpcr:\n"
    printf "    dma0 mdec in priority    : %d\n", (($dpcr >> (0 * 4    )) & 7)
    printf "    dma0 mdec in enable      : %d\n", (($dpcr >> (0 * 4 + 3)) & 1)
    printf "    dma1 mdec out priority   : %d\n", (($dpcr >> (1 * 4    )) & 7)
    printf "    dma1 mdec out enable     : %d\n", (($dpcr >> (1 * 4 + 3)) & 1)
    printf "    dma2 gpu priority        : %d\n", (($dpcr >> (2 * 4    )) & 7)
    printf "    dma2 gpu enable          : %d\n", (($dpcr >> (2 * 4 + 3)) & 1)
    printf "    dma3 cdrom priority      : %d\n", (($dpcr >> (3 * 4    )) & 7)
    printf "    dma3 cdrom enable        : %d\n", (($dpcr >> (3 * 4 + 3)) & 1)
    printf "    dma4 spu priority        : %d\n", (($dpcr >> (4 * 4    )) & 7)
    printf "    dma4 spu enable          : %d\n", (($dpcr >> (4 * 4 + 3)) & 1)
    printf "    dma5 pio priority        : %d\n", (($dpcr >> (5 * 4    )) & 7)
    printf "    dma5 pio enable          : %d\n", (($dpcr >> (5 * 4 + 3)) & 1)
    printf "    dma6 otc priority        : %d\n", (($dpcr >> (6 * 4    )) & 7)
    printf "    dma6 otc enable          : %d\n", (($dpcr >> (6 * 4 + 3)) & 1)
end

define dmachcr
    set $chcr = (union chcr)dma.dma0_mdec_in_chcr
    printf "dma 0: mdec in\n"
    printf "    transfer_direction      : %d\n", $chcr.transfer_direction
    printf "    address_step            : %d\n", $chcr.address_step
    printf "    chopping_enable         : %d\n", $chcr.chopping_enable
    printf "    sync_mode               : %d\n", $chcr.sync_mode
    printf "    chopping_dma_window_size: %d\n", $chcr.chopping_dma_window_size
    printf "    chopping_cpu_window_size: %d\n", $chcr.chopping_cpu_window_size
    printf "    start_busy              : %d\n", $chcr.start_busy
    printf "    start_trigger           : %d\n", $chcr.start_trigger

    set $chcr = (union chcr)dma1_mdec_out_chcr
    printf "dma 1: mdec out\n"
    printf "    transfer_direction      : %d\n", $chcr.transfer_direction
    printf "    address_step            : %d\n", $chcr.address_step
    printf "    chopping_enable         : %d\n", $chcr.chopping_enable
    printf "    sync_mode               : %d\n", $chcr.sync_mode
    printf "    chopping_dma_window_size: %d\n", $chcr.chopping_dma_window_size
    printf "    chopping_cpu_window_size: %d\n", $chcr.chopping_cpu_window_size
    printf "    start_busy              : %d\n", $chcr.start_busy
    printf "    start_trigger           : %d\n", $chcr.start_trigger

    set $chcr = (union chcr)dma2_gpu_chcr
    printf "dma 2: gpu\n"
    printf "    transfer_direction      : %d\n", $chcr.transfer_direction
    printf "    address_step            : %d\n", $chcr.address_step
    printf "    chopping_enable         : %d\n", $chcr.chopping_enable
    printf "    sync_mode               : %d\n", $chcr.sync_mode
    printf "    chopping_dma_window_size: %d\n", $chcr.chopping_dma_window_size
    printf "    chopping_cpu_window_size: %d\n", $chcr.chopping_cpu_window_size
    printf "    start_busy              : %d\n", $chcr.start_busy
    printf "    start_trigger           : %d\n", $chcr.start_trigger

    set $chcr = (union chcr)dma3_cdrom_chcr
    printf "dma 3: cdrom\n"
    printf "    transfer_direction      : %d\n", $chcr.transfer_direction
    printf "    address_step            : %d\n", $chcr.address_step
    printf "    chopping_enable         : %d\n", $chcr.chopping_enable
    printf "    sync_mode               : %d\n", $chcr.sync_mode
    printf "    chopping_dma_window_size: %d\n", $chcr.chopping_dma_window_size
    printf "    chopping_cpu_window_size: %d\n", $chcr.chopping_cpu_window_size
    printf "    start_busy              : %d\n", $chcr.start_busy
    printf "    start_trigger           : %d\n", $chcr.start_trigger

    set $chcr = (union chcr)dma4_spu_chcr
    printf "dma 4: spu\n"
    printf "    transfer_direction      : %d\n", $chcr.transfer_direction
    printf "    address_step            : %d\n", $chcr.address_step
    printf "    chopping_enable         : %d\n", $chcr.chopping_enable
    printf "    sync_mode               : %d\n", $chcr.sync_mode
    printf "    chopping_dma_window_size: %d\n", $chcr.chopping_dma_window_size
    printf "    chopping_cpu_window_size: %d\n", $chcr.chopping_cpu_window_size
    printf "    start_busy              : %d\n", $chcr.start_busy
    printf "    start_trigger           : %d\n", $chcr.start_trigger

    set $chcr = (union chcr)dma5_pio_chcr
    printf "dma 5: pio\n"
    printf "    transfer_direction      : %d\n", $chcr.transfer_direction
    printf "    address_step            : %d\n", $chcr.address_step
    printf "    chopping_enable         : %d\n", $chcr.chopping_enable
    printf "    sync_mode               : %d\n", $chcr.sync_mode
    printf "    chopping_dma_window_size: %d\n", $chcr.chopping_dma_window_size
    printf "    chopping_cpu_window_size: %d\n", $chcr.chopping_cpu_window_size
    printf "    start_busy              : %d\n", $chcr.start_busy
    printf "    start_trigger           : %d\n", $chcr.start_trigger

    set $chcr = (union chcr)dma6_otc_chcr
    printf "dma 6: otc\n"
    printf "    transfer_direction      : %d\n", $chcr.transfer_direction
    printf "    address_step            : %d\n", $chcr.address_step
    printf "    chopping_enable         : %d\n", $chcr.chopping_enable
    printf "    sync_mode               : %d\n", $chcr.sync_mode
    printf "    chopping_dma_window_size: %d\n", $chcr.chopping_dma_window_size
    printf "    chopping_cpu_window_size: %d\n", $chcr.chopping_cpu_window_size
    printf "    start_busy              : %d\n", $chcr.start_busy
    printf "    start_trigger           : %d\n", $chcr.start_trigger
end

define dmamadr
    uint32_t dma0_mdec_in_madr;
    uint32_t dma1_mdec_out_madr;
    uint32_t dma2_gpu_madr;
    uint32_t dma3_cdrom_madr;
    uint32_t dma4_spu_madr;
    uint32_t dma5_pio_madr;
    uint32_t dma6_otc_madr;
    p (union madr) $arg0
end

define dmabrc
    uint32_t dma0_mdec_in_brc;
    uint32_t dma1_mdec_out_brc;
    uint32_t dma2_gpu_brc;
    uint32_t dma3_cdrom_brc;
    uint32_t dma4_spu_brc;
    uint32_t dma5_pio_brc;
    uint32_t dma6_otc_brc;
    p (union brc)  $arg0
end

# timer debugging features

define timers
    printf "timer0:\n"
    printf "    current count: %d\n", timers.t0.current_count
    printf "    target count:  %d\n", timers.t0.target_count
    printf "    mode:\n"
    printf "        sync_enable         : %d\n", timers.t0.mode.sync_enable         
    printf "        sync_mode           : %d\n", timers.t0.mode.sync_mode           
    printf "        reset_after         : %d\n", timers.t0.mode.reset_after         
    printf "        irq_when_target     : %d\n", timers.t0.mode.irq_when_target     
    printf "        irq_when_max        : %d\n", timers.t0.mode.irq_when_max        
    printf "        irq_once_or_repeat  : %d\n", timers.t0.mode.irq_once_or_repeat  
    printf "        irq_pulse_or_toggle : %d\n", timers.t0.mode.irq_pulse_or_toggle 
    printf "        clock_source        : %d\n", timers.t0.mode.clock_source        
    printf "        interrupt_request   : %d\n", timers.t0.mode.interrupt_request   
    printf "        hit_target          : %d\n", timers.t0.mode.hit_target          
    printf "        hit_max             : %d\n", timers.t0.mode.hit_max             
    printf "timer1:\n"
    printf "    current count: %d\n", timers.t1.current_count
    printf "    target count:  %d\n", timers.t1.target_count
    printf "    mode:\n"
    printf "        sync_enable         : %d\n", timers.t1.mode.sync_enable         
    printf "        sync_mode           : %d\n", timers.t1.mode.sync_mode           
    printf "        reset_after         : %d\n", timers.t1.mode.reset_after         
    printf "        irq_when_target     : %d\n", timers.t1.mode.irq_when_target     
    printf "        irq_when_max        : %d\n", timers.t1.mode.irq_when_max        
    printf "        irq_once_or_repeat  : %d\n", timers.t1.mode.irq_once_or_repeat  
    printf "        irq_pulse_or_toggle : %d\n", timers.t1.mode.irq_pulse_or_toggle 
    printf "        clock_source        : %d\n", timers.t1.mode.clock_source        
    printf "        interrupt_request   : %d\n", timers.t1.mode.interrupt_request   
    printf "        hit_target          : %d\n", timers.t1.mode.hit_target          
    printf "        hit_max             : %d\n", timers.t1.mode.hit_max             
    printf "timer2:\n"
    printf "    current count: %d\n", timers.t2.current_count
    printf "    target count:  %d\n", timers.t2.target_count
    printf "    mode:\n"
    printf "        sync_enable         : %d\n", timers.t2.mode.sync_enable         
    printf "        sync_mode           : %d\n", timers.t2.mode.sync_mode           
    printf "        reset_after         : %d\n", timers.t2.mode.reset_after         
    printf "        irq_when_target     : %d\n", timers.t2.mode.irq_when_target     
    printf "        irq_when_max        : %d\n", timers.t2.mode.irq_when_max        
    printf "        irq_once_or_repeat  : %d\n", timers.t2.mode.irq_once_or_repeat  
    printf "        irq_pulse_or_toggle : %d\n", timers.t2.mode.irq_pulse_or_toggle 
    printf "        clock_source        : %d\n", timers.t2.mode.clock_source        
    printf "        interrupt_request   : %d\n", timers.t2.mode.interrupt_request   
    printf "        hit_target          : %d\n", timers.t2.mode.hit_target          
    printf "        hit_max             : %d\n", timers.t2.mode.hit_max             
end             

# spu debugging features 

define spu_voice
    set $voice = 0
    printf "spu voice registers:\n"
    printf "adpcm_start_address:\n"
    while ($voice < 24)
        printf "    voice %02d: %08x\n", $voice, spu.adpcm_start_address[$voice]
        set $voice = $voice + 1
    end
    set $voice = 0
    printf "adpcm_repeat_address:\n"
    while ($voice < 24)
        printf "    voice %02d: %08x\n", $voice, spu.adpcm_repeat_address[$voice]
        set $voice = $voice + 1
    end
    set $voice = 0
    printf "adpcm_current_address:\n"
    while ($voice < 24)
        printf "    voice %02d: %08x\n", $voice, spu.adpcm_current_address[$voice]
        set $voice = $voice + 1
    end
end

define spu_registers
    printf "spucnt\n"
    printf "    cd_audio_enable         : %d\n", spu.spucnt.cd_audio_enable        
    printf "    external_audio_enable   : %d\n", spu.spucnt.external_audio_enable  
    printf "    cd_audio_reverb         : %d\n", spu.spucnt.cd_audio_reverb        
    printf "    external_audio_reverb   : %d\n", spu.spucnt.external_audio_reverb  
    printf "    sound_ram_transfer_mode : %d\n", spu.spucnt.sound_ram_transfer_mode
    printf "    irq9_enable             : %d\n", spu.spucnt.irq9_enable            
    printf "    reverb_master           : %d\n", spu.spucnt.reverb_master          
    printf "    noise_frequency_step    : %d\n", spu.spucnt.noise_frequency_step   
    printf "    noise_frequency_shift   : %d\n", spu.spucnt.noise_frequency_shift  
    printf "    mute_spu                : %d\n", spu.spucnt.mute_spu               
    printf "    spu_enable              : %d\n", spu.spucnt.spu_enable             
end

define spu_internal
end

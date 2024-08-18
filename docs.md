# PSX-Emulartor
An inprogress psx emulator

DISCLAIMER: documentation is taken from no$psx docs and Lionel Flandrins' psx emulation guide.
            This readme is just used to keep the information I am gathering in check, making it
            clear to myself, how and why I have implemented each part of the system. 

## CPU

#### INFORMATION

Model: MIPS R3000A, with COP0 - memory and exception handler, and COP2 - GTE (graphics transformation engine)

Registers:

    All registers are 32bit wide.

      Name       Alias    Common Usage
      (R0)       zero     Constant (always 0) (this one isn't a real register)
      R1         at       Assembler temporary (destroyed by some pseudo opcodes!)
      R2-R3      v0-v1    Subroutine return values, may be changed by subroutines
      R4-R7      a0-a3    Subroutine arguments, may be changed by subroutines
      R8-R15     t0-t7    Temporaries, may be changed by subroutines
      R16-R23    s0-s7    Static variables, must be saved by subs
      R24-R25    t8-t9    Temporaries, may be changed by subroutines
      R26-R27    k0-k1    Reserved for kernel (destroyed by some IRQ handlers!)
      R28        gp       Global pointer (rarely used)
      R29        sp       Stack pointer
      R30        fp(s8)   Frame Pointer, or 9th Static variable, must be saved
      R31        ra       Return address (used so by JAL,BLTZAL,BGEZAL opcodes)
      -          pc       Program counter
      -          hi,lo    Multiply/divide results, may be changed by subroutines

Instructions:

There are 3 different types of cpu instructions, Register-type, Immediate-type, and Jump-type instructions.

Each type has a different instruction encoding:

                      |31..26|25..21|20..16|15..11|10..6|5...0|
    Register-type:    |  OP  |  RS  |  RT  |  RD  |shamt|funct|
    Immediate-type:   |  OP  |  RS  |  RT  |    IMMEDIATE     |
    Jump-type:        |  OP  |             TARGET             |
    
opcodes:
    
     OP  FUNC
    0x00 0x00 SLL         - shift left  logical 
    0x00 0x02 SRL         - shift right logical
    0x00 0x03 SRA         - shift right arithmetic
    0x00 0x04 SLLV        - shift left  logical variable
    0x00 0x06 SRLV        - shift right logical variable
    0x00 0x07 SRAV        - shift right arithmetic variable
    0x00 0x08 JR          - jump to register
    0x00 0x09 JALR        - jump to register and link register
    0x00 0x0C SYSCALL     - generate system call exception
    0x00 0x0D BREAK       - generate break exception
    0x00 0x11 MFHI        - move from hi
    0x00 0x12 MTHI        - move to hi
    0x00 0x13 MFLO        - move from low
    0x00 0x14 MTLO        - move to hi
    0x00 0x18 MULT        - multiply (signed)
    0x00 0x19 MULTU       - multiply (unsigned)
    0x00 0x1A DIV         - divide (signed)
    0x00 0x1B DIVU        - divide (unsigned)
    0x00 0x20 ADD         - add (signed overflow)
    0x00 0x21 ADDU        - add (unsigned)
    0x00 0x22 SUB         - subtract (signed)
    0x00 0x23 SUBU        - subtract (unsigned)
    0x00 0x24 AND         - logical and
    0x00 0x25 OR          - logical or
    0x00 0x26 XOR         - logical xor
    0x00 0x27 NOR         - logical nor
    0x00 0x2A SLT         - set if less than
    0x00 0x2B SLTU        - set if less than unsigned
    
     OP   RT
    0X01 0x00 BLTZ        - branch if less than zero
    0X01 0x01 BGEZ        - branch if greater than or equal to zero
    0X01 0x10 BLTZAL      - branch if less than zero and link
    0X01 0x11 BGEZAL      - branch if greater than or equal to zero and link
    
     OP
    0X02      J           - jump
    0X03      JAL         - jump and link
    0X04      BEQ         - branch if equal
    0X05      BNE         - branch if not equal
    0X06      BLEZ        - branch if less than or equal
    0X07      BGTZ        - branch if greater than
    0X08      ADDI        - add immediate
    0X09      ADDIU       - add immediate unsigned
    0X0A      SLTI        - set if less than immediate
    0X0B      SLTIU       - set if less than immediate unsigned
    0X0C      ANDI        - logical and immediate
    0X0D      ORI         - logical or immediate
    0X0E      XORI        - logical xor immediate
    0X0F      LUI         - load upper immediate
    0X20      LB          - load byte
    0X21      LH          - load half word
    0X22      LWL         - load half word left
    0X23      LW          - load word
    0X24      LBU         - load byte unsigned
    0X25      LHU         - load half word unsigned
    0X26      LWR         - load half word right
    0X28      SB          - store byte
    0X29      SH          - store half word
    0X2A      SWL         - store half word left
    0X2B      SW          - store word
    0X2E      SWR         - store half word right

The CPU uses instruction pipelining, a consequence of this is the various delays experienced
by the CPU. The cpu experiences branch, jump and load delays. 

**branch and jump delays**

since the cpu loads the next instruction in too be decoded before the current instruction has 
executed, the instruction right after any branch or jump will still be executed. e.g.

    $pc = 0XBFC00200: J    0XBFC002A4   < jump to new PC
    $pc = 0XBFC00204: ADD $t0 $a0 $a1   < instruction right after still executed
    $pc = 0XBFC002A4: <new instruction> < jump registered

**load delay**
loading a value from memory takes longer than 1 cycle, since the cpu is not halted until
the value is loaded, the cpu continues to execute. As a result of this the next instruction 
will execute with the same register values as the previous instruction.

    $pc = 0xbfc01204: lw $t0, 0x1f801814  < load word from 0x1f801814
    $pc = 0xbfc01208: add $t1, $a0, $t0   < add $a0 and $t0 then store in $t1
    $pc = 0xbfc0120c: add $t2, $a0, $t0   < add $a0 and $t0 then store in $t1

    $t1 != $t2 given memory\[0x1f801814\] != $t0

#### IMPLEMENTATION

##### **structures and unions**

**struct cpu** > "cpu.h"
    
    - Main device struct, contains all the state that is manipulated to emulate the R3000A

**union instruction** > "instruction.h"

    - Helper struct, breaking each instruction down into its fields

**struct delay** > "cpu.h"

    - Helper struct to emulate the various delays experienced by the cpu

**struct coprocessor<n>**
    
    - Coprocessor 0 or 2

##### **external functions**

These are functions that can be used by the wider system to interact with the cpu.

    PSX_ERROR cpu_reset(void)

    - Function resets the cpu, the $pc points to the first BIOS instruction and all coprocessor 
      register pointers are refreshed

    PSX_ERROR cpu_step(void)
    
    - Main CPU execution function, it will run through an iteration of the fetch, decode and execute
      cycle.

    void cpu_exception(enum EXCEPTION_CAUSE cause)
    
    - Used to set coprocessor 0 exception handling

    bool cop0_SR_<STATUSBIT>(void)

    - <STATUSBIT>:
      IEc, KUc, IEp, KUp, IEo, KUo, Im, Isc, Swc, PZ, CM, PE, TS, BEV, RE, CU0, CU1, CU2, CU3
    
    - Return the coprocessor 0 status bits used in various parts, e.g. cop0_SR_Isc is used to isolate
      all reads and write to main memory to the SCRATCHPAD.


##### **internal functions**

These are structures and functions used to emulate the CPU

    static void cpu_execute_op(void)

    - Contains the main switch case statement directing the cpu to execute the correct instructions

    static void cpu_branch_delay(void)
    
    - Handles the branch delays of the cpu

    static void cpu_load_delay(void)

    - Handles the load delays of the cpu

    static void <MNEUMONIC>(void)

    - These are the main instructions that are executed by the cpu, all instructions have the same primitive
      simply replace the "<MNEUMONIC>" with any instruction detailed previously
    - These instructions directly effect the cpu struct, changing register values and modifying state.

    

## MEMORY

#### INFORMATION:

The PSX memory uses virtual paging. There are 3 main addressing schemes,
KUSEG, KSEG0 and, KSEG1. KSEG0 is normal memory, KSEG1 is cached memory and 
KUSEG is user memory.

**Memory Map:**

    KUSEG      KSEG0      KSEG1
    0X00000000 0X80000000 0XA0000000  -- (2048K) MAIN 
    0X1F000000 0X9F000000 0XBF000000  -- (8192K) EXPANSION 1 
    0X1F800000 0X9F800000 0XBF800000  -- (   1K) SCRATCHPAD
    0X1F801000 0X9F801000 0XBF801000  -- (   8K) IO PORTS
    0X1F802000 0X9F802000 0XBF802000  -- (   8K) EXPANSION 2
    0X1FA00000 0X9FA00000 0XBFA00000  -- (2048K) EXPANSION 3
    0X1FC00000 0X9FC00000 0XBFC00000  -- ( 512K) BIOS ROM
            
Futhermore there is also memory inaccessible to the CPU.

    1024K   VRAM (Framebuffers, Textures, Palettes) (with 2KB Texture Cache)
    512K    Sound RAM (Capture Buffers, ADPCM Data, Reverb Workspace)
    0.5K    CDROM controller RAM (see CDROM Test commands)
    16.5K   CDROM controller ROM (Firmware and Bootstrap for MC68HC05 cpu)
    32K     CDROM Buffer (IC303) (32Kx8) (BUG: only two sectors accessible?)
    128K    External Memory Card(s) (EEPROMs)


Currently the memory module simply treats each addressing scheme as mirrors.
In the future, if it is required, the cache functionalities will be added. (whatever that means)

The memory is mapped similairly to how the guide maps memory.

     0-28 segment offset - offset into each virtual addressing scheme
    29-31 segment number - the virtual addressing scheme
        When 0XX - KUSEG
        When 100 - KSEG0
        When 101 - KSEG1

The system accesses the correct memory location by translating all addresses into the KUSEG addressing scheme.

#### IMPLEMENTATION:

All system memory is stored in the "memory" struct, this struct contains each memory component,
MAIN, IO PORTS, etc. as a union. I opted to create them as unions as it might be easier to handle special behavior
for different components, e.g. IO PORTS may require some special refrences/considerations.

Due to having the memory map split into smaller components, each refrence into memory needs to be mapped to the 
correct component and, the address indexing into the component needs to be adjusted to start at its base.

The cpu memory mapping function has the following primetive:

**PSX_ERROR memory_cpu_map(uint8_t \*\*segment, uint32_t \*address, uint32_t \*mask, uint32_t alignment, bool load);**

    -- uint8_t **SEGMENT   -> function will store the address of the memory component being accessed
    -- uint32_t *ADDRESS   -> function will augment the address accessed such that the address begins in the component
    -- uint32_t  ALIGNMENT -> function will check the alignment of the address passed, this aligment changes based on 
                              if the calling function is reading an 8, 16, or 32 bit function.
    -- bool      LOAD      -> some memory addresses have different behavior based on if they are being read or written from,
                              a great example of this is the gpu memory mapped IO ports. There is also an exception generated
                              if the address alignment is wrong, this exception is different depending on if it is a read or write

The cpu and gpu loading and storing primitives for 8, 16, 32, and 4, 8, 16, 24 bit data support respectivly:

**void memory_cpu_load_nbit(uint32_t address, uint32_t \*data)**

**void memory_cpu_store_nbit(uint32_t address, uint32_t data)**

**void memory_gpu_load_nbit(uint32_t address, uint32_t \*data)**

**void memory_gpu_store_nbit(uint32_t address, uint32_t data)**
    
    -- uint32_t address -> vitual address being accessed
    -- uint32_t data    -> data to store (modified to correct bit length)
    -- uint32_t *data   -> pointer to variable (modified to correct bit length)

BIOS loading function (directly loads it into the memory structure)

**PSX_ERROR memory_load_bios(const char \*filebios)**
    
    -- const char *filebios -> path and filename to BIOS

System internal function, this helps as some components need to refrence memory directly, e.g. IO PORTS like DMA.

**uint8_t \*memory_VRAM_pointer(void)**
**uint8_t \*memory_pointer(uint32_t address)**
    
    -- uint32_t address -> virtual address being accessed


## GPU

#### GPUSTAT REGISTER

1F801814h - GPUSTAT - GPU Status Register (R)

  0-3   Texture page X Base   (N*64)                              ;GP0(E1h).0-3
  4     Texture page Y Base   (N*256) (ie. 0 or 256)              ;GP0(E1h).4
  5-6   Semi Transparency     (0=B/2+F/2, 1=B+F, 2=B-F, 3=B+F/4)  ;GP0(E1h).5-6
  7-8   Texture page colors   (0=4bit, 1=8bit, 2=15bit, 3=Reserved)GP0(E1h).7-8
  9     Dither 24bit to 15bit (0=Off/strip LSBs, 1=Dither Enabled);GP0(E1h).9
  10    Drawing to display area (0=Prohibited, 1=Allowed)         ;GP0(E1h).10
  11    Set Mask-bit when drawing pixels (0=No, 1=Yes/Mask)       ;GP0(E6h).0
  12    Draw Pixels           (0=Always, 1=Not to Masked areas)   ;GP0(E6h).1
  13    Interlace Field       (or, always 1 when GP1(08h).5=0)
  14    "Reverseflag"         (0=Normal, 1=Distorted)             ;GP1(08h).7
  15    Texture Disable       (0=Normal, 1=Disable Textures)      ;GP0(E1h).11
  16    Horizontal Resolution 2     (0=256/320/512/640, 1=368)    ;GP1(08h).6
  17-18 Horizontal Resolution 1     (0=256, 1=320, 2=512, 3=640)  ;GP1(08h).0-1
  19    Vertical Resolution         (0=240, 1=480, when Bit22=1)  ;GP1(08h).2
  20    Video Mode                  (0=NTSC/60Hz, 1=PAL/50Hz)     ;GP1(08h).3
  21    Display Area Color Depth    (0=15bit, 1=24bit)            ;GP1(08h).4
  22    Vertical Interlace          (0=Off, 1=On)                 ;GP1(08h).5
  23    Display Enable              (0=Enabled, 1=Disabled)       ;GP1(03h).0
  24    Interrupt Request (IRQ1)    (0=Off, 1=IRQ)       ;GP0(1Fh)/GP1(02h)
  25    DMA / Data Request, meaning depends on GP1(04h) DMA Direction:
          When GP1(04h)=0 ---> Always zero (0)
          When GP1(04h)=1 ---> FIFO State  (0=Full, 1=Not Full)
          When GP1(04h)=2 ---> Same as GPUSTAT.28
          When GP1(04h)=3 ---> Same as GPUSTAT.27
  26    Ready to receive Cmd Word   (0=No, 1=Ready)  ;GP0(...) ;via GP0
  27    Ready to send VRAM to CPU   (0=No, 1=Ready)  ;GP0(C0h) ;via GPUREAD
  28    Ready to receive DMA Block  (0=No, 1=Ready)  ;GP0(...) ;via GP0
  29-30 DMA Direction (0=Off, 1=?, 2=CPUtoGP0, 3=GPUREADtoCPU)    ;GP1(04h).0-1
  31    Drawing even/odd lines in interlace mode (0=Even or Vblank, 1=Odd)

In 480-lines mode, bit31 changes per frame. And in 240-lines model, the bit changes per scanline. The bit is always zero during Vblank (vertical retrace and upper/lower screen border).

Note
Further GPU status information can be retrieved via GP1(10h) and GP0(C0h).

Ready Bits
Bit28: Normally, this bit gets cleared when the command execution is busy (ie. once when the command and all of its parameters are received), however, for Polygon and Line Rendering commands, the bit gets cleared immediately after receiving the command word (ie. before receiving the vertex parameters). The bit is used as DMA request in DMA Mode 2, accordingly, the DMA would probably hang if the Polygon/Line parameters are transferred in a separate DMA block (ie. the DMA probably starts ONLY on command words).
       - Do I need to clear this bit?, The Rendering commands can be processed instantly. I will not clear.

Bit27: Gets set after sending GP0(C0h) and its parameters, and stays set until all data words are received; used as DMA request in DMA Mode 3.
       - What is DMA mode 3? DMA sync mode 3 is "reserved" what does this mean?

Bit26: Gets set when the GPU wants to receive a command. If the bit is cleared, then the GPU does either want to receive data, or it is busy with a command execution (and doesn't want to receive anything).
       - Do i need to clear/toggle this bit, it is only used during command execution. Maybe needed when doing gpu timing

Bit25: This is the DMA Request bit, however, the bit is also useful for non-DMA transfers, especially in the FIFO State mode.
    GP1(04h) -> toggles this bit depending on the parameter values


## DMA




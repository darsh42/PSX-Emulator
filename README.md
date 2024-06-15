# PSX-Emulartor
An inprogress psx emulator

DISCLAIMER: documentation is taken from no$psx docs and Lionel Flanderins' psx emulation guide.
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

**PSX_ERROR cpu_reset(void)**

    - Function resets the cpu, the $pc points to the first BIOS instruction and all coprocessor 
      register pointers are refreshed

**PSX_ERROR cpu_step(void)**
    
    - Main CPU execution function, it will run through an iteration of the fetch, decode and execute
      cycle.

**void cpu_exception(enum EXCEPTION_CAUSE cause)**
    
    - Used to set coprocessor 0 exception handling

**bool cop0_SR_<STATUSBIT>(void)**

    - <STATUSBIT>:
      IEc, KUc, IEp, KUp, IEo, KUo, Im, Isc, Swc, PZ, CM, PE, TS, BEV, RE, CU0, CU1, CU2, CU3
    
    - Return the coprocessor 0 status bits used in various parts, e.g. cop0_SR_Isc is used to isolate
      all reads and write to main memory to the SCRATCHPAD.


##### **internal functions**

These are structures and functions used to emulate the CPU

**static void cpu_execute_op(void)**

    - Contains the main switch case statement directing the cpu to execute the correct instructions

**static void cpu_branch_delay(void)**
    
    - Handles the branch delays of the cpu

**static void cpu_load_delay(void)**

    - Handles the load delays of the cpu

**static void <MNEUMONIC>(void)**
    
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

    -- uint8_t \*\*SEGMENT   -> function will store the address of the memory component being accessed
    -- uint32_t \*ADDRESS   -> function will augment the address accessed such that the address begins in the component
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
    
    -- const char \*filebios -> path and filename to BIOS

System internal function, this helps as some components need to refrence memory directly, e.g. IO PORTS like DMA.

**uint8_t \*memory_VRAM_pointer(void)**
**uint8_t \*memory_pointer(uint32_t address)**
    
    -- uint32_t address -> virtual address being accessed


## GPU

## DMA


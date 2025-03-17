#include <stdio.h>
#include <assert.h>

#define BIOS_PRIVATE
#include "bios.h"
#include "cpu.h"
#include "memory.h"

struct bios bios;

static void bios_printf( void )
{
    uint32_t c, arg, argc = 0;

    memory_read( cpu_get_general_register(4), &c, 1);

    while (c)
    {
        /* handle formatted output */
        if (c == '%')
        {
            switch(argc)
            {
                case 0: arg = cpu_get_general_register(5); break;
                case 1: arg = cpu_get_general_register(6); break;
                case 2: arg = cpu_get_general_register(7); break;
                default:
                    memory_read(cpu_get_general_register(29) + 0x10 * argc, &arg, 4);
                    break;
            }

            argc++;
        }
    }
}

static void std_out_putchar( char c )
{
    if ( bios.fptty )
    {
        switch (c)
        {
            case 0x08: c = '\b'; break;
            case 0x09: c = '\t'; break;
            case 0x0A: c = '\n'; break;
        }
        
        // putc(c, bios.fptty);
        putchar(c);
    }
}

static void std_out_puts( uint32_t address )
{
    if ( bios.fptty )
    {
        uint32_t c;
        
        do {
            memory_read(address++, &c, 1);
            fprintf(bios.fptty, "%c", (char) c);
        } while ( c != 0x00 );
    }
}

static void Ann_function_vectors( void )
{
    switch (cpu_get_general_register( 9 ))
    {
        case (0x00): TRACE_BIOS("Ann_function_vectors", "or B(32h) FileOpen(filename,accessmode)\n"); break;
        case (0x01): TRACE_BIOS("Ann_function_vectors", "or B(33h) FileSeek(fd,offset,seektype)\n"); break;
        case (0x02): TRACE_BIOS("Ann_function_vectors", "or B(34h) FileRead(fd,dst,length)\n"); break;
        case (0x03): TRACE_BIOS("Ann_function_vectors", "or B(35h) FileWrite(fd,src,length)\n"); break;
        case (0x04): TRACE_BIOS("Ann_function_vectors", "or B(36h) FileClose(fd)\n"); break;
        case (0x05): TRACE_BIOS("Ann_function_vectors", "or B(37h) FileIoctl(fd,cmd,arg)\n"); break;
        case (0x06): TRACE_BIOS("Ann_function_vectors", "or B(38h) exit(exitcode)\n"); break;
        case (0x07): TRACE_BIOS("Ann_function_vectors", "or B(39h) FileGetDeviceFlag(fd)\n"); break;
        case (0x08): TRACE_BIOS("Ann_function_vectors", "or B(3Ah) FileGetc(fd)\n"); break;
        case (0x09): 
             TRACE_BIOS("Ann_function_vectors", "or B(3Bh) FilePutc(char,fd)\n"); 

             std_out_putchar( cpu_get_general_register(4) );

             break;
        case (0x0A): TRACE_BIOS("Ann_function_vectors", "todigit(char)\n"); break;
        case (0x0B): TRACE_BIOS("Ann_function_vectors", "atof(src)     ;Does NOT work - uses (ABSENT) cop1 !!!\n"); break;
        case (0x0C): TRACE_BIOS("Ann_function_vectors", "strtoul(src,src_end,base)\n"); break;
        case (0x0D): TRACE_BIOS("Ann_function_vectors", "strtol(src,src_end,base)\n"); break;
        case (0x0E): TRACE_BIOS("Ann_function_vectors", "abs(val)\n"); break;
        case (0x0F): TRACE_BIOS("Ann_function_vectors", "labs(val)\n"); break;
        case (0x10): TRACE_BIOS("Ann_function_vectors", "atoi(src)\n"); break;
        case (0x11): TRACE_BIOS("Ann_function_vectors", "atol(src)\n"); break;
        case (0x12): TRACE_BIOS("Ann_function_vectors", "atob(src,num_dst)\n"); break;
        case (0x13): TRACE_BIOS("Ann_function_vectors", "SaveState(buf)\n"); break;
        case (0x14): TRACE_BIOS("Ann_function_vectors", "RestoreState(buf,param)\n"); break;
        case (0x15): TRACE_BIOS("Ann_function_vectors", "strcat(dst,src)\n"); break;
        case (0x16): TRACE_BIOS("Ann_function_vectors", "strncat(dst,src,maxlen)\n"); break;
        case (0x17): TRACE_BIOS("Ann_function_vectors", "strcmp(str1,str2)\n"); break;
        case (0x18): TRACE_BIOS("Ann_function_vectors", "strncmp(str1,str2,maxlen)\n"); break;
        case (0x19): TRACE_BIOS("Ann_function_vectors", "strcpy(dst,src)\n"); break;
        case (0x1A): TRACE_BIOS("Ann_function_vectors", "strncpy(dst,src,maxlen)\n"); break;
        case (0x1B): TRACE_BIOS("Ann_function_vectors", "strlen(src)\n"); break;
        case (0x1C): TRACE_BIOS("Ann_function_vectors", "index(src,char)\n"); break;
        case (0x1D): TRACE_BIOS("Ann_function_vectors", "rindex(src,char)\n"); break;
        case (0x1E): TRACE_BIOS("Ann_function_vectors", "strchr(src,char)  ;exactly the same as 'index'\n"); break;
        case (0x1F): TRACE_BIOS("Ann_function_vectors", "strrchr(src,char) ;exactly the same as 'rindex'\n"); break;
        case (0x20): TRACE_BIOS("Ann_function_vectors", "strpbrk(src,list)\n"); break;
        case (0x21): TRACE_BIOS("Ann_function_vectors", "strspn(src,list)\n"); break;
        case (0x22): TRACE_BIOS("Ann_function_vectors", "strcspn(src,list)\n"); break;
        case (0x23): TRACE_BIOS("Ann_function_vectors", "strtok(src,list)  ;use strtok(0,list) in further calls\n"); break;
        case (0x24): TRACE_BIOS("Ann_function_vectors", "strstr(str,substr) - buggy\n"); break;
        case (0x25): TRACE_BIOS("Ann_function_vectors", "toupper(char)\n"); break;
        case (0x26): TRACE_BIOS("Ann_function_vectors", "tolower(char)\n"); break;
        case (0x27): TRACE_BIOS("Ann_function_vectors", "bcopy(src,dst,len)\n"); break;
        case (0x28): TRACE_BIOS("Ann_function_vectors", "bzero(dst,len)\n"); break;
        case (0x29): TRACE_BIOS("Ann_function_vectors", "bcmp(ptr1,ptr2,len)      ;Bugged\n"); break;
        case (0x2A): TRACE_BIOS("Ann_function_vectors", "memcpy(dst,src,len)\n"); break;
        case (0x2B): TRACE_BIOS("Ann_function_vectors", "memset(dst,fillbyte,len)\n"); break;
        case (0x2C): TRACE_BIOS("Ann_function_vectors", "memmove(dst,src,len)     ;Bugged\n"); break;
        case (0x2D): TRACE_BIOS("Ann_function_vectors", "memcmp(src1,src2,len)    ;Bugged\n"); break;
        case (0x2E): TRACE_BIOS("Ann_function_vectors", "memchr(src,scanbyte,len)\n"); break;
        case (0x2F): TRACE_BIOS("Ann_function_vectors", "rand()\n"); break;
        case (0x30): TRACE_BIOS("Ann_function_vectors", "srand(seed)\n"); break;
        case (0x31): TRACE_BIOS("Ann_function_vectors", "qsort(base,nel,width,callback)\n"); break;
        case (0x32): TRACE_BIOS("Ann_function_vectors", "strtod(src,src_end) ;Does NOT work - uses (ABSENT) cop1 !!!\n"); break;
        case (0x33): TRACE_BIOS("Ann_function_vectors", "malloc(size)\n"); break;
        case (0x34): TRACE_BIOS("Ann_function_vectors", "free(buf)\n"); break;
        case (0x35): TRACE_BIOS("Ann_function_vectors", "lsearch(key,base,nel,width,callback)\n"); break;
        case (0x36): TRACE_BIOS("Ann_function_vectors", "bsearch(key,base,nel,width,callback)\n"); break;
        case (0x37): TRACE_BIOS("Ann_function_vectors", "calloc(sizx,sizy)            ;SLOW!\n"); break;
        case (0x38): TRACE_BIOS("Ann_function_vectors", "realloc(old_buf,new_siz)     ;SLOW!\n"); break;
        case (0x39): TRACE_BIOS("Ann_function_vectors", "InitHeap(addr,size)\n"); break;
        case (0x3A): TRACE_BIOS("Ann_function_vectors", "SystemErrorExit(exitcode)\n"); break;
        case (0x3B): TRACE_BIOS("Ann_function_vectors", "or B(3Ch) std_in_getchar()\n"); break;
        case (0x3C): 
            TRACE_BIOS("Ann_function_vectors", "or B(3Dh) std_out_putchar(char)\n"); 

            std_out_putchar( cpu_get_general_register(4) );

            break;
        case (0x3D): TRACE_BIOS("Ann_function_vectors", "or B(3Eh) std_in_gets(dst)\n"); break;
        case (0x3E): 
            TRACE_BIOS("Ann_function_vectors", "or B(3Fh) std_out_puts(src)\n"); 

            std_out_puts( cpu_get_general_register(4) );

            break;
        case (0x3F): 
            TRACE_BIOS("Ann_function_vectors", "printf(txt,param1,param2,etc.)\n"); 
            
            // bios_printf();
            
            break;
        case (0x40): TRACE_BIOS("Ann_function_vectors", "SystemErrorUnresolvedException()\n"); break;
        case (0x41): TRACE_BIOS("Ann_function_vectors", "LoadExeHeader(filename,headerbuf)\n"); break;
        case (0x42): TRACE_BIOS("Ann_function_vectors", "LoadExeFile(filename,headerbuf)\n"); break;
        case (0x43): TRACE_BIOS("Ann_function_vectors", "DoExecute(headerbuf,param1,param2)\n"); break;
        case (0x44): TRACE_BIOS("Ann_function_vectors", "FlushCache()\n"); break;
        case (0x45): TRACE_BIOS("Ann_function_vectors", "init_a0_b0_c0_vectors\n"); break;
        case (0x46): TRACE_BIOS("Ann_function_vectors", "GPU_dw(Xdst,Ydst,Xsiz,Ysiz,src)\n"); break;
        case (0x47): TRACE_BIOS("Ann_function_vectors", "gpu_send_dma(Xdst,Ydst,Xsiz,Ysiz,src)\n"); break;
        case (0x48): TRACE_BIOS("Ann_function_vectors", "SendGP1Command(gp1cmd)\n"); break;
        case (0x49): TRACE_BIOS("Ann_function_vectors", "GPU_cw(gp0cmd)   ;send GP0 command word\n"); break;
        case (0x4A): TRACE_BIOS("Ann_function_vectors", "GPU_cwp(src,num) ;send GP0 command word and parameter words\n"); break;
        case (0x4B): TRACE_BIOS("Ann_function_vectors", "send_gpu_linked_list(src)\n"); break;
        case (0x4C): TRACE_BIOS("Ann_function_vectors", "gpu_abort_dma()\n"); break;
        case (0x4D): TRACE_BIOS("Ann_function_vectors", "GetGPUStatus()\n"); break;
        case (0x4E): TRACE_BIOS("Ann_function_vectors", "gpu_sync()\n"); break;
        case (0x4F): TRACE_BIOS("Ann_function_vectors", "SystemError\n"); break;
        case (0x50): TRACE_BIOS("Ann_function_vectors", "SystemError\n"); break;
        case (0x51): TRACE_BIOS("Ann_function_vectors", "LoadAndExecute(filename,stackbase,stackoffset)\n"); break;
        case (0x52): TRACE_BIOS("Ann_function_vectors", "SystemError ----OR---- 'GetSysSp()' ?\n"); break;
        case (0x53): TRACE_BIOS("Ann_function_vectors", "SystemError            ;PS2: set_ioabort_handler(src)\n"); break;
        case (0x54): TRACE_BIOS("Ann_function_vectors", "or A(71h) CdInit()\n"); break;
        case (0x55): TRACE_BIOS("Ann_function_vectors", "or A(70h) _bu_init()   ;DTL-H2000: SystemError\n"); break;
        case (0x56): TRACE_BIOS("Ann_function_vectors", "or A(72h) CdRemove()\n"); break;
        case (0x57): TRACE_BIOS("Ann_function_vectors", "return 0\n"); break;
        case (0x58): TRACE_BIOS("Ann_function_vectors", "return 0\n"); break;
        case (0x59): TRACE_BIOS("Ann_function_vectors", "return 0\n"); break;
        case (0x5A): TRACE_BIOS("Ann_function_vectors", "return 0\n"); break;
        case (0x5B): TRACE_BIOS("Ann_function_vectors", "dev_tty_init()                                      ;PS2: SystemError\n"); break;
        case (0x5C): TRACE_BIOS("Ann_function_vectors", "dev_tty_open(fcb,and unused:'path\name',accessmode) ;PS2: SystemError\n"); break;
        case (0x5D): TRACE_BIOS("Ann_function_vectors", "dev_tty_in_out(fcb,cmd)                             ;PS2: SystemError\n"); break;
        case (0x5E): TRACE_BIOS("Ann_function_vectors", "dev_tty_ioctl(fcb,cmd,arg)                          ;PS2: SystemError\n"); break;
        case (0x5F): TRACE_BIOS("Ann_function_vectors", "dev_cd_open(fcb,'path\name',accessmode)\n"); break;
        case (0x60): TRACE_BIOS("Ann_function_vectors", "dev_cd_read(fcb,dst,len)\n"); break;
        case (0x61): TRACE_BIOS("Ann_function_vectors", "dev_cd_close(fcb)\n"); break;
        case (0x62): TRACE_BIOS("Ann_function_vectors", "dev_cd_firstfile(fcb,'path\name',direntry)\n"); break;
        case (0x63): TRACE_BIOS("Ann_function_vectors", "dev_cd_nextfile(fcb,direntry)\n"); break;
        case (0x64): TRACE_BIOS("Ann_function_vectors", "dev_cd_chdir(fcb,'path')\n"); break;
        case (0x65): TRACE_BIOS("Ann_function_vectors", "dev_card_open(fcb 'path\name',accessmode)                ;\n"); break;
        case (0x66): TRACE_BIOS("Ann_function_vectors", "dev_card_read(fcb,dst,len)                               ;\n"); break;
        case (0x67): TRACE_BIOS("Ann_function_vectors", "dev_card_write(fcb,src,len)                              ; SystemError\n"); break;
        case (0x68): TRACE_BIOS("Ann_function_vectors", "dev_card_close(fcb)                                      ; on\n"); break;
        case (0x69): TRACE_BIOS("Ann_function_vectors", "dev_card_firstfile(fcb,'path\name',direntry)             ; DTL-H2000\n"); break;
        case (0x6A): TRACE_BIOS("Ann_function_vectors", "dev_card_nextfile(fcb,direntry)                          ;\n"); break;
        case (0x6B): TRACE_BIOS("Ann_function_vectors", "dev_card_erase(fcb,'path\name')                          ;\n"); break;
        case (0x6C): TRACE_BIOS("Ann_function_vectors", "dev_card_undelete(fcb,'path\name')                       ;\n"); break;
        case (0x6D): TRACE_BIOS("Ann_function_vectors", "dev_card_format(fcb)                                     ;\n"); break;
        case (0x6E): TRACE_BIOS("Ann_function_vectors", "dev_card_rename(fcb1,'path\name1',fcb2,'path\name2')     ;\n"); break;
        case (0x6F): TRACE_BIOS("Ann_function_vectors", "dev_card_clear_error_or_so(fcb)   ;[r4+18h]=00000000h    ;\n"); break;
        case (0x70): TRACE_BIOS("Ann_function_vectors", "or A(55h) _bu_init()                                     ;/\n"); break;
        case (0x71): TRACE_BIOS("Ann_function_vectors", "or A(54h) CdInit()\n"); break;
        case (0x72): TRACE_BIOS("Ann_function_vectors", "or A(56h) CdRemove()\n"); break;
        case (0x73): TRACE_BIOS("Ann_function_vectors", "return 0\n"); break;
        case (0x74): TRACE_BIOS("Ann_function_vectors", "return 0\n"); break;
        case (0x75): TRACE_BIOS("Ann_function_vectors", "return 0\n"); break;
        case (0x76): TRACE_BIOS("Ann_function_vectors", "return 0\n"); break;
        case (0x77): TRACE_BIOS("Ann_function_vectors", "return 0\n"); break;
        case (0x78): TRACE_BIOS("Ann_function_vectors", "CdAsyncSeekL(src)\n"); break;
        case (0x79): TRACE_BIOS("Ann_function_vectors", "return 0              ;DTL-H2000: CdAsyncSeekP(src)\n"); break;
        case (0x7A): TRACE_BIOS("Ann_function_vectors", "return 0              ;DTL-H2000: CdAsyncGetlocL(dst?)\n"); break;
        case (0x7B): TRACE_BIOS("Ann_function_vectors", "return 0              ;DTL-H2000: CdAsyncGetlocP(dst?)\n"); break;
        case (0x7C): TRACE_BIOS("Ann_function_vectors", "CdAsyncGetStatus(dst)\n"); break;
        case (0x7D): TRACE_BIOS("Ann_function_vectors", "return 0              ;DTL-H2000: CdAsyncGetParam(dst?)\n"); break;
        case (0x7E): TRACE_BIOS("Ann_function_vectors", "CdAsyncReadSector(count,dst,mode)\n"); break;
        case (0x7F): TRACE_BIOS("Ann_function_vectors", "return 0              ;DTL-H2000: CdAsyncReadWithNewMode(mode)\n"); break;
        case (0x80): TRACE_BIOS("Ann_function_vectors", "return 0              ;DTL-H2000: CdAsyncReadFinalCount1(r4)\n"); break;
        case (0x81): TRACE_BIOS("Ann_function_vectors", "CdAsyncSetMode(mode)\n"); break;
        case (0x82): TRACE_BIOS("Ann_function_vectors", "return 0              ;DTL-H2000: CdAsyncMotorOn()\n"); break;
        case (0x83): TRACE_BIOS("Ann_function_vectors", "return 0              ;DTL-H2000: CdAsyncPause()\n"); break;
        case (0x84): TRACE_BIOS("Ann_function_vectors", "return 0              ;DTL-H2000: CdAsyncPlayOrReadS()\n"); break;
        case (0x85): TRACE_BIOS("Ann_function_vectors", "return 0              ;DTL-H2000: CdAsyncStop()\n"); break;
        case (0x86): TRACE_BIOS("Ann_function_vectors", "return 0              ;DTL-H2000: CdAsyncMute()\n"); break;
        case (0x87): TRACE_BIOS("Ann_function_vectors", "return 0              ;DTL-H2000: CdAsyncDemute()\n"); break;
        case (0x88): TRACE_BIOS("Ann_function_vectors", "return 0              ;DTL-H2000: CdSetAudioVolume(src)  ;4-byte src\n"); break;
        case (0x89): TRACE_BIOS("Ann_function_vectors", "return 0              ;DTL-H2000: CdAsyncSetSession1(dst)\n"); break;
        case (0x8A): TRACE_BIOS("Ann_function_vectors", "return 0              ;DTL-H2000: CdAsyncSetSession(session,dst)\n"); break;
        case (0x8B): TRACE_BIOS("Ann_function_vectors", "return 0              ;DTL-H2000: CdAsyncForward()\n"); break;
        case (0x8C): TRACE_BIOS("Ann_function_vectors", "return 0              ;DTL-H2000: CdAsyncBackward()\n"); break;
        case (0x8D): TRACE_BIOS("Ann_function_vectors", "return 0              ;DTL-H2000: CdAsyncPlay()\n"); break;
        case (0x8E): TRACE_BIOS("Ann_function_vectors", "return 0              ;DTL-H2000: CdAsyncGetStatSpecial(r4,r5)\n"); break;
        case (0x8F): TRACE_BIOS("Ann_function_vectors", "return 0              ;DTL-H2000: CdAsyncGetID(r4,r5)\n"); break;
        case (0x90): TRACE_BIOS("Ann_function_vectors", "CdromIoIrqFunc1()\n"); break;
        case (0x91): TRACE_BIOS("Ann_function_vectors", "CdromDmaIrqFunc1()\n"); break;
        case (0x92): TRACE_BIOS("Ann_function_vectors", "CdromIoIrqFunc2()\n"); break;
        case (0x93): TRACE_BIOS("Ann_function_vectors", "CdromDmaIrqFunc2()\n"); break;
        case (0x94): TRACE_BIOS("Ann_function_vectors", "CdromGetInt5errCode(dst1,dst2)\n"); break;
        case (0x95): TRACE_BIOS("Ann_function_vectors", "CdInitSubFunc()\n"); break;
        case (0x96): TRACE_BIOS("Ann_function_vectors", "AddCDROMDevice()\n"); break;
        case (0x97): TRACE_BIOS("Ann_function_vectors", "AddMemCardDevice()    ;DTL-H2000: SystemError\n"); break;
        case (0x98): TRACE_BIOS("Ann_function_vectors", "AddDuartTtyDevice()   ;DTL-H2000: AddAdconsTtyDevice ;PS2: SystemError\n"); break;
        case (0x99): TRACE_BIOS("Ann_function_vectors", "AddDummyTtyDevice()\n"); break;
        case (0x9A): TRACE_BIOS("Ann_function_vectors", "SystemError           ;DTL-H: AddMessageWindowDevice\n"); break;
        case (0x9B): TRACE_BIOS("Ann_function_vectors", "SystemError           ;DTL-H: AddCdromSimDevice\n"); break;
        case (0x9C): TRACE_BIOS("Ann_function_vectors", "SetConf(num_EvCB,num_TCB,stacktop)\n"); break;
        case (0x9D): TRACE_BIOS("Ann_function_vectors", "GetConf(num_EvCB_dst,num_TCB_dst,stacktop_dst)\n"); break;
        case (0x9E): TRACE_BIOS("Ann_function_vectors", "SetCdromIrqAutoAbort(type,flag)\n"); break;
        case (0x9F): TRACE_BIOS("Ann_function_vectors", "SetMemSize(megabytes)\n"); break;
    }
}

static void Bnn_function_vectors( void )
{
    switch (cpu_get_general_register( 9 ) )
    {
        case (0x00): TRACE_BIOS("Bnn_function_vectors", "alloc_kernel_memory(size)\n"); break;
        case (0x01): TRACE_BIOS("Bnn_function_vectors", "free_kernel_memory(buf)\n"); break;
        case (0x02): TRACE_BIOS("Bnn_function_vectors", "init_timer(t,reload,flags)\n"); break;
        case (0x03): TRACE_BIOS("Bnn_function_vectors", "get_timer(t)\n"); break;
        case (0x04): TRACE_BIOS("Bnn_function_vectors", "enable_timer_irq(t)\n"); break;
        case (0x05): TRACE_BIOS("Bnn_function_vectors", "disable_timer_irq(t)\n"); break;
        case (0x06): TRACE_BIOS("Bnn_function_vectors", "restart_timer(t)\n"); break;
        case (0x07): TRACE_BIOS("Bnn_function_vectors", "DeliverEvent(class, spec)\n"); break;
        case (0x08): TRACE_BIOS("Bnn_function_vectors", "OpenEvent(class,spec,mode,func)\n"); break;
        case (0x09): TRACE_BIOS("Bnn_function_vectors", "CloseEvent(event)\n"); break;
        case (0x0A): TRACE_BIOS("Bnn_function_vectors", "WaitEvent(event)\n"); break;
        case (0x0B): TRACE_BIOS("Bnn_function_vectors", "TestEvent(event)\n"); break;
        case (0x0C): TRACE_BIOS("Bnn_function_vectors", "EnableEvent(event)\n"); break;
        case (0x0D): TRACE_BIOS("Bnn_function_vectors", "DisableEvent(event)\n"); break;
        case (0x0E): TRACE_BIOS("Bnn_function_vectors", "OpenThread(reg_PC,reg_SP_FP,reg_GP)\n"); break;
        case (0x0F): TRACE_BIOS("Bnn_function_vectors", "CloseThread(handle)\n"); break;
        case (0x10): TRACE_BIOS("Bnn_function_vectors", "ChangeThread(handle)\n"); break;
        case (0x11): TRACE_BIOS("Bnn_function_vectors", "jump_to_00000000h\n"); break;
        case (0x12): TRACE_BIOS("Bnn_function_vectors", "InitPad(buf1,siz1,buf2,siz2)\n"); break;
        case (0x13): TRACE_BIOS("Bnn_function_vectors", "StartPad()\n"); break;
        case (0x14): TRACE_BIOS("Bnn_function_vectors", "StopPad()\n"); break;
        case (0x15): TRACE_BIOS("Bnn_function_vectors", "OutdatedPadInitAndStart(type,button_dest,unused,unused)\n"); break;
        case (0x16): TRACE_BIOS("Bnn_function_vectors", "OutdatedPadGetButtons()\n"); break;
        case (0x17): TRACE_BIOS("Bnn_function_vectors", "ReturnFromException()\n"); break;
        case (0x18): TRACE_BIOS("Bnn_function_vectors", "SetDefaultExitFromException()\n"); break;
        case (0x19): TRACE_BIOS("Bnn_function_vectors", "SetCustomExitFromException(addr)\n"); break;
        case (0x1A): TRACE_BIOS("Bnn_function_vectors", "SystemError  ;PS2: return 0\n"); break;
        case (0x1B): TRACE_BIOS("Bnn_function_vectors", "SystemError  ;PS2: return 0\n"); break;
        case (0x1C): TRACE_BIOS("Bnn_function_vectors", "SystemError  ;PS2: return 0\n"); break;
        case (0x1D): TRACE_BIOS("Bnn_function_vectors", "SystemError  ;PS2: return 0\n"); break;
        case (0x1E): TRACE_BIOS("Bnn_function_vectors", "SystemError  ;PS2: return 0\n"); break;
        case (0x1F): TRACE_BIOS("Bnn_function_vectors", "SystemError  ;PS2: return 0\n"); break;
        case (0x20): TRACE_BIOS("Bnn_function_vectors", "UnDeliverEvent(class,spec)\n"); break;
        case (0x21): TRACE_BIOS("Bnn_function_vectors", "SystemError  ;PS2: return 0\n"); break;
        case (0x22): TRACE_BIOS("Bnn_function_vectors", "SystemError  ;PS2: return 0\n"); break;
        case (0x23): TRACE_BIOS("Bnn_function_vectors", "SystemError  ;PS2: return 0\n"); break;
        case (0x24): TRACE_BIOS("Bnn_function_vectors", "jump_to_00000000h\n"); break;
        case (0x25): TRACE_BIOS("Bnn_function_vectors", "jump_to_00000000h\n"); break;
        case (0x26): TRACE_BIOS("Bnn_function_vectors", "jump_to_00000000h\n"); break;
        case (0x27): TRACE_BIOS("Bnn_function_vectors", "jump_to_00000000h\n"); break;
        case (0x28): TRACE_BIOS("Bnn_function_vectors", "jump_to_00000000h\n"); break;
        case (0x29): TRACE_BIOS("Bnn_function_vectors", "jump_to_00000000h\n"); break;
        case (0x2A): TRACE_BIOS("Bnn_function_vectors", "SystemError  ;PS2: return 0\n"); break;
        case (0x2B): TRACE_BIOS("Bnn_function_vectors", "SystemError  ;PS2: return 0\n"); break;
        case (0x2C): TRACE_BIOS("Bnn_function_vectors", "jump_to_00000000h\n"); break;
        case (0x2D): TRACE_BIOS("Bnn_function_vectors", "jump_to_00000000h\n"); break;
        case (0x2E): TRACE_BIOS("Bnn_function_vectors", "jump_to_00000000h\n"); break;
        case (0x2F): TRACE_BIOS("Bnn_function_vectors", "jump_to_00000000h\n"); break;
        case (0x30): TRACE_BIOS("Bnn_function_vectors", "jump_to_00000000h\n"); break;
        case (0x31): TRACE_BIOS("Bnn_function_vectors", "jump_to_00000000h\n"); break;
        case (0x32): TRACE_BIOS("Bnn_function_vectors", "or A(00h) FileOpen(filename,accessmode)\n"); break;
        case (0x33): TRACE_BIOS("Bnn_function_vectors", "or A(01h) FileSeek(fd,offset,seektype)\n"); break;
        case (0x34): TRACE_BIOS("Bnn_function_vectors", "or A(02h) FileRead(fd,dst,length)\n"); break;
        case (0x35): TRACE_BIOS("Bnn_function_vectors", "or A(03h) FileWrite(fd,src,length)\n"); break;
        case (0x36): TRACE_BIOS("Bnn_function_vectors", "or A(04h) FileClose(fd)\n"); break;
        case (0x37): TRACE_BIOS("Bnn_function_vectors", "or A(05h) FileIoctl(fd,cmd,arg)\n"); break;
        case (0x38): TRACE_BIOS("Bnn_function_vectors", "or A(06h) exit(exitcode)\n"); break;
        case (0x39): TRACE_BIOS("Bnn_function_vectors", "or A(07h) FileGetDeviceFlag(fd)\n"); break;
        case (0x3A): TRACE_BIOS("Bnn_function_vectors", "or A(08h) FileGetc(fd)\n"); break;
        case (0x3B): TRACE_BIOS("Bnn_function_vectors", "or A(09h) FilePutc(char,fd)\n"); break;
        case (0x3C): TRACE_BIOS("Bnn_function_vectors", "or A(3Bh) std_in_getchar()\n"); break;
        case (0x3D): 
            TRACE_BIOS("Bnn_function_vectors", "or A(3Ch) std_out_putchar(char)\n"); 

            std_out_putchar( cpu_get_general_register(4) );

            break;
        case (0x3E): TRACE_BIOS("Bnn_function_vectors", "or A(3Dh) std_in_gets(dst)\n"); break;
        case (0x3F): 
            TRACE_BIOS("Bnn_function_vectors", "or A(3Eh) std_out_puts(src)\n"); 

            std_out_puts( cpu_get_general_register(4) );

            break;
        case (0x40): TRACE_BIOS("Bnn_function_vectors", "chdir(name)\n"); break;
        case (0x41): TRACE_BIOS("Bnn_function_vectors", "FormatDevice(devicename)\n"); break;
        case (0x42): TRACE_BIOS("Bnn_function_vectors", "firstfile(filename,direntry)\n"); break;
        case (0x43): TRACE_BIOS("Bnn_function_vectors", "nextfile(direntry)\n"); break;
        case (0x44): TRACE_BIOS("Bnn_function_vectors", "FileRename(old_filename,new_filename)\n"); break;
        case (0x45): TRACE_BIOS("Bnn_function_vectors", "FileDelete(filename)\n"); break;
        case (0x46): TRACE_BIOS("Bnn_function_vectors", "FileUndelete(filename)\n"); break;
        case (0x47): TRACE_BIOS("Bnn_function_vectors", "AddDevice(device_info)  ;subfunction for AddXxxDevice functions\n"); break;
        case (0x48): TRACE_BIOS("Bnn_function_vectors", "RemoveDevice(device_name_lowercase)\n"); break;
        case (0x49): TRACE_BIOS("Bnn_function_vectors", "PrintInstalledDevices()\n"); break;
    }
}

static void Cnn_function_vectors( void )
{
    switch (cpu_get_general_register( 9 ))
    {
        case (0x00): TRACE_BIOS("Cnn_function_vectors", "EnqueueTimerAndVblankIrqs(priority) ;used with prio=1\n"); break;
        case (0x01): TRACE_BIOS("Cnn_function_vectors", "EnqueueSyscallHandler(priority)     ;used with prio=0\n"); break;
        case (0x02): TRACE_BIOS("Cnn_function_vectors", "SysEnqIntRP(priority,struc)\n"); break;
        case (0x03): TRACE_BIOS("Cnn_function_vectors", "SysDeqIntRP(priority,struc)\n"); break;
        case (0x04): TRACE_BIOS("Cnn_function_vectors", "get_free_EvCB_slot()\n"); break;
        case (0x05): TRACE_BIOS("Cnn_function_vectors", "get_free_TCB_slot()\n"); break;
        case (0x06): TRACE_BIOS("Cnn_function_vectors", "ExceptionHandler()\n"); break;
        case (0x07): TRACE_BIOS("Cnn_function_vectors", "InstallExceptionHandlers()  ;destroys/uses k0/k1\n"); break;
        case (0x08): TRACE_BIOS("Cnn_function_vectors", "SysInitMemory(addr,size)\n"); break;
        case (0x09): TRACE_BIOS("Cnn_function_vectors", "SysInitKernelVariables()\n"); break;
        case (0x0A): TRACE_BIOS("Cnn_function_vectors", "ChangeClearRCnt(t,flag)\n"); break;
        case (0x0B): TRACE_BIOS("Cnn_function_vectors", "SystemError  ;PS2: return 0\n"); break;
        case (0x0C): TRACE_BIOS("Cnn_function_vectors", "InitDefInt(priority) ;used with prio=3\n"); break;
        case (0x0D): TRACE_BIOS("Cnn_function_vectors", "SetIrqAutoAck(irq,flag)\n"); break;
        case (0x0E): TRACE_BIOS("Cnn_function_vectors", "return 0               ;DTL-H2000: dev_sio_init\n"); break;
        case (0x0F): TRACE_BIOS("Cnn_function_vectors", "return 0               ;DTL-H2000: dev_sio_open\n"); break;
        case (0x10): TRACE_BIOS("Cnn_function_vectors", "return 0               ;DTL-H2000: dev_sio_in_out\n"); break;
        case (0x11): TRACE_BIOS("Cnn_function_vectors", "return 0               ;DTL-H2000: dev_sio_ioctl\n"); break;
        case (0x12): TRACE_BIOS("Cnn_function_vectors", "InstallDevices(ttyflag)\n"); break;
        case (0x13): TRACE_BIOS("Cnn_function_vectors", "FlushStdInOutPut()\n"); break;
        case (0x14): TRACE_BIOS("Cnn_function_vectors", "return 0               ;DTL-H2000: SystemError\n"); break;
        case (0x15): TRACE_BIOS("Cnn_function_vectors", "tty_cdevinput(circ,char)\n"); break;
        case (0x16): TRACE_BIOS("Cnn_function_vectors", "tty_cdevscan()\n"); break;
        case (0x17): TRACE_BIOS("Cnn_function_vectors", "tty_circgetc(circ)    ;uses r5 as garbage txt for ioabort\n"); break;
        case (0x18): TRACE_BIOS("Cnn_function_vectors", "tty_circputc(char,circ)\n"); break;
        case (0x19): TRACE_BIOS("Cnn_function_vectors", "ioabort(txt1,txt2)\n"); break;
        case (0x1A): TRACE_BIOS("Cnn_function_vectors", "set_card_find_mode(mode)  ;0=normal, 1=find deleted files\n"); break;
        case (0x1B): TRACE_BIOS("Cnn_function_vectors", "KernelRedirect(ttyflag)   ;PS2: ttyflag=1 causes SystemError\n"); break;
        case (0x1C): TRACE_BIOS("Cnn_function_vectors", "AdjustA0Table()\n"); break;
        case (0x1D): TRACE_BIOS("Cnn_function_vectors", "get_card_find_mode()\n"); break;
    }
}

void init_bios(const char *file_bios,
               const char *file_exe,
               const char *file_tty)
{
    assert(file_bios);

    memory_load_bios(file_bios);
    
    /* enable tty and allow for writes */
    assert((bios.fptty = (file_tty) ? fopen(file_tty, "w"): stdout));

    bios.fptty = NULL;

    /* enable exe side loading */
    if (file_exe) bios.file_exe = file_exe;
}

void deinit_bios( void )
{
    if (bios.fptty)
        assert(!fclose(bios.fptty));
}

void task_bios( void )
{
    /* bios function vector tables */
    switch (cpu_get_pc() & 0x1fffffff)
    {
        case 0xA0: Ann_function_vectors(); break;
        case 0xB0: Bnn_function_vectors(); break;
        case 0xC0: Cnn_function_vectors(); break;
    }

    /* exe loading */
    if ( bios.file_exe && (cpu_get_pc() & 0xFFFF0000) == 0x80030000 )
    {
        printf("loading exe\n");
        memory_load_exe( bios.file_exe );
    }
 }

#ifndef FILEFORMATS_H_INCLUDED
#define FILEFORMATS_H_INCLUDED

#ifdef PSX_EXE_HEADER
struct psx_exe_header
{
    uint8_t ascii_id[8];
    uint8_t zerofill[8];

    uint32_t initial_pc;
    uint32_t initial_gp;
    uint32_t destination_address;
    uint32_t filesize;
    uint32_t data_section_start;
    uint32_t data_section_end;
    uint32_t bss_section_start;
    uint32_t bss_section_end;
    uint32_t initial_sp_fp_base;
    uint32_t initial_sp_fp_offset;
};
#endif

#endif // FILEFORMATS_H_INCLUDED

#ifndef CDROM_H_INCLUDED
#define CDROM_H_INCLUDED

#include <stdint.h>

#ifdef CDROM_SECTORS
/* header */
struct cdrom_sector_header 
{
    uint8_t minutes;
    uint8_t seconds;
    uint8_t sectors;
    uint8_t mode;
};

/* subheader */
struct cdrom_sector_subheader
{
    /* subheader - file number    (byte 1) */
    uint8_t file_number;
    
    /* subheader - channel number (byte 2) */
    uint8_t channel_number : 5;
    uint8_t                : 3;
    
    /* subheader - submode        (byte 3) */
    uint8_t end_of_record  : 1;
    uint8_t sector_video   : 1;
    uint8_t sector_audio   : 1;
    uint8_t sector_data    : 1;
    uint8_t trigger        : 1;
    uint8_t form           : 1;
    uint8_t real_time      : 1;
    uint8_t end_of_file    : 1;

    /* subheader - coding info    (byte 4) */
    uint8_t mono_or_stereo : 2;
    uint8_t sample_rate    : 2;
    uint8_t bits_per_sample: 2;
    uint8_t emphasis       : 1;
    uint8_t                : 1;
};


struct cdrom_sector_audio 
{ 
    uint8_t       data[2352]; 
};

struct cdrom_sector_empty // empty
{
    uint8_t       sync[  12];

    struct cdrom_sector_header header;

    uint8_t zerofilled[2336];
};

struct cdrom_sector_original // original cdrom
{
    uint8_t       sync[  12];

    struct cdrom_sector_header header;

    uint8_t       data[2048];
    uint8_t        edc[   4];
    uint8_t zerofilled[   8];
    uint8_t        ecc[ 276];
};

struct cdrom_sector_cd_xa_1 // CD XA
{
    uint8_t       sync[  12];

    struct cdrom_sector_header            header;
    struct cdrom_sector_subheader      subheader;
    struct cdrom_sector_subheader copy_subheader;

    uint8_t       data[2048];
    uint8_t        edc[   4];
    uint8_t        ecc[ 276];
};

struct cdrom_sector_cd_xa_2 // CD XA
{
    uint8_t       sync[  12];

    struct cdrom_sector_header            header;
    struct cdrom_sector_subheader      subheader;
    struct cdrom_sector_subheader copy_subheader;

    uint8_t       data[2324];
    uint8_t        edc[   4];
};
#endif // CDROM_SECTORS

#endif // CDROM_H_INCLUDED

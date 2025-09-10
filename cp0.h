enum cp0_reg_e {
    CP0_BPC        =  3,
    CP0_BDA        =  5,
    CP0_JUMPDEST   =  6,
    CP0_DCIC       =  7,
    CP0_BAD_VADDR  =  8,
    CP0_BDAM       =  9,
    CP0_BPCM       = 11,
    CP0_SR         = 12,
    CP0_CAUSE      = 13,
    CP0_EPC        = 14,
    CP0_PRID       = 15,
};

enum cpu_exception_type
{
    INT     = 0x00,
    MOD     = 0x01,
    TLBL    = 0x02,
    TLBS    = 0x03,
    AdEL    = 0x04,
    AdES    = 0x05,
    IBE     = 0x06,
    DBE     = 0x07,
    SYSCALL = 0x08,
    BP      = 0x09,
    RI      = 0x0A,
    CpU     = 0x0B,
    Ov      = 0x0C
};

/* coprocessor 0 SR(12) struct */
union cp0_sr
{
    uint32_t value;
    struct
    {
        uint32_t IEc: 1;
        uint32_t KUc: 1;
        uint32_t IEp: 1;
        uint32_t KUp: 1;
        uint32_t IEo: 1;
        uint32_t KUo: 1;
        uint32_t    : 2;
        uint32_t Im : 8;
        uint32_t Isc: 1;
        uint32_t Swc: 1;
        uint32_t PZ : 1;
        uint32_t CM : 1;
        uint32_t PE : 1;
        uint32_t TS : 1;
        uint32_t BEV: 1;
        uint32_t    : 2;
        uint32_t RE : 1;
        uint32_t    : 2;
        uint32_t CU0: 1;
        uint32_t CU1: 1;
        uint32_t CU2: 1;
        uint32_t CU3: 1;
    };
};

/* coprocessor 0 CAUSE(13) struct */
union cp0_cause
{
    uint32_t value;
    struct
    {
        uint32_t             :  2;
        uint32_t excode      :  5;
        uint32_t             :  1;
        uint32_t Ip          :  8;
        uint32_t             : 12;
        uint32_t CE          :  2;
        uint32_t             :  1;
        uint32_t branch_delay:  1;
    };
};

struct cp0 {
    uint32_t r[16];
};

/* read/write register */
void write_cp0_reg(enum cp0_reg_e r, uint32_t  data);
void  read_cp0_reg(enum cp0_reg_e r, uint32_t *data);

void cp0(void);
void cp0_exception( enum cpu_exception_type t );

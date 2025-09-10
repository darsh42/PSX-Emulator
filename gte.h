#ifndef GTE_H_INCLUDED
#define GTE_H_INCLUDED

enum cp2_reg_e {
    CP2_ZERO
};

struct cp2 {
    uint32_t r[16];
};

/* read/write register */
void write_cp2_register(enum cp2_reg_e r, uint32_t  data);
void  read_cp2_register(enum cp2_reg_e r, uint32_t *data);

void cp2(void);
#endif // GTE_H_INCLUDED

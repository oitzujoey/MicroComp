
#ifndef MICROCOMP_H_
#define MICROCOMP_H_

#define PROG_MEM    0
#define DATA_MEM    1

typedef struct {
    uint8_t *program_memory;
    uint8_t *data_memory;
    uint8_t f_reg;
    uint8_t a_reg;
    uint8_t b_reg;
    uint8_t c_reg;
    uint16_t pc_reg;
} microcomp_state_t

typedef struct  {
    uint16_t *lower_bounds;
    uint16_t *upper_bounds;
    uint8_t *rom;
    uint16_t len;
} microcomp_mem_cnstr_t

typedef struct {
    microcomp_mem_cnstr_t prog;
    microcomp_mem_cnstr_t data;
} microcomp_cnstr_t

#endif  /*  MICROCOMP_H_    */

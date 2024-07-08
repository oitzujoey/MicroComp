#ifndef MICROCOMP_H_
#define MICROCOMP_H_

#include <stdint.h>

#define PROG_MEM 0
#define DATA_MEM 1

#define MICROCODE_WIDTH 3
#define MICROCODE_DEPTH 1024
#define PROGRAM_MEMORY_SIZE 65536
#define DATA_MEMORY_SIZE 65536

typedef struct {
	uint16_t pc;
	uint8_t f;
	uint8_t a;
	uint8_t b;
	uint8_t c;
	uint8_t program_memory[PROGRAM_MEMORY_SIZE];
	uint8_t data_memory[DATA_MEMORY_SIZE];
	uint8_t microcode[MICROCODE_WIDTH][MICROCODE_DEPTH];
} microcomp_state_t;

typedef struct	{
	uint16_t *lower_bounds;
	uint16_t *upper_bounds;
	uint8_t *rom;
	uint16_t len;
} microcomp_mem_cnstr_t;

typedef struct {
	microcomp_mem_cnstr_t prog;
	microcomp_mem_cnstr_t data;
} microcomp_cnstr_t;

#endif /* MICROCOMP_H_ */

#ifndef MICROCOMP_EMULATOR_H_
#define MICROCOMP_EMULATOR_H_

#include <stdint.h>
#include <stdbool.h>

#define MICROCODE_WIDTH 3
#define MICROCODE_DEPTH 1024
#define PROGRAM_MEMORY_SIZE 65536
#define DATA_MEMORY_SIZE 65536

typedef struct {
	bool initialized;
	uint16_t programCounter;
	uint8_t microprogramCounter;
	uint8_t instructionRegister;
	uint8_t microcodeRegister[MICROCODE_DEPTH];
	uint8_t f;
	uint8_t a;
	uint8_t b;
	uint8_t c;
	uint8_t program_memory[PROGRAM_MEMORY_SIZE];
	uint8_t data_memory[DATA_MEMORY_SIZE];
	uint8_t microcode[MICROCODE_DEPTH][MICROCODE_WIDTH];  // This hopefully gets better cache locality. 🤷
} microcomp_emulator_state_t;

typedef struct	{
	uint16_t *lower_bounds;
	uint16_t *upper_bounds;
	uint8_t *rom;
	uint16_t len;
} microcomp_emulator_mem_cnstr_t;

typedef struct {
	microcomp_emulator_mem_cnstr_t prog;
	microcomp_emulator_mem_cnstr_t data;
} microcomp_emulator_cnstr_t;

#endif /* MICROCOMP_EMULATOR_H_ */

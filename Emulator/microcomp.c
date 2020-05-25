
#include "microcomp.h"

uint8_t microcomp_rd(uint16_t addr, int which_mem, 
                     microcomp_cnstr_t constraints, microcomp_state_t *state) {
    
    if (which_mem == PROG_MEM) {
        microcomp_mem_cnstr_t reduced_constraints = constraints.prog;
    } else {
        microcomp_mem_cnstr_t reduced_constraints = constraints.data;
    }
    
    for (int i = 0; i < reduced_constraints.len; i++) {
        if ((reduced_constraints.lower_bounds[i] < addr) && 
            (addr < reduced_constraints.upper_bounds[i])) {
            if (which_mem == PROG_MEM) {
                return state->program_memory[addr];
            } else {
                return state->data_memory[addr];
            }
        }
    }
    
    printf(
        "Warning: Read from nonexistant memory location\n"
        "Info: Read value will be set to 0xFF\n"
    );
    
    return 0xFF;
}

void microcomp_wr(uint16_t addr, int which_mem, uint8_t data, 
                  microcomp_cnstr_t constraints, microcomp_state_t *state) {
    
    if (which_mem == PROG_MEM) {
        microcomp_mem_cnstr_t reduced_constraints = constraints.prog;
    } else {
        microcomp_mem_cnstr_t reduced_constraints = constraints.data;
    }
    
    for (int i = 0; i < reduced_constraints.len; i++) {
        if ((reduced_constraints.lower_bounds[i] < addr) && 
            (addr < reduced_constraints.upper_bounds[i])) {
            if (reduced_constraints.rom[i] == 1) {
                printf("Warning: Write to ROM\n");
            } else {
                if (which_mem == PROG_MEM) {
                    state->program_memory[addr] = data;
                } else {
                    state->data_memory[addr] = data;
                }
            }
            return;
        }
    }
    
    printf("Warning: Write to nonexistant memory location\n");
}

void microcomp_exe_insn(microcomp_state_t state, 
                        microcomp_cnstr_t constraints) {
    
}

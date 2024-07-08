#include "microcomp_emulator.h"
#include <bits/stdint-uintn.h>

bool microcomp_emulator_bitElement8(uint8_t byte, int index) {
	return (byte >> index) & 0x01;
}



// Reset the machine.
void microcomp_emulator_reset(microcomp_emulator_state_t *s) {
	s->microprogramCounter = 0;
	s->programCounter = 0;
}

// Step by one clock cycle.
void microcomp_emulator_clock(microcomp_emulator_state_t *s) {
	// TTL inputs naturally float high.
	uint8_t dataBus = 0xff;
	uint8_t programDataBus = 0xff;
	uint16_t programAddressBus = 0xffff;
	uint16_t bcAddressBus = 0xffff;
	/* Todo: data memory, program memory */

	/* bool n_wrp = microcomp_emulator_bitElement8(s->microcodeRegister[0], 0); */
	/* bool n_rdp = microcomp_emulator_bitElement8(s->microcodeRegister[0], 1); */
	bool n_aoe = microcomp_emulator_bitElement8(s->microcodeRegister[0], 2);
	bool n_ashoe = microcomp_emulator_bitElement8(s->microcodeRegister[0], 3);
	bool fsors = microcomp_emulator_bitElement8(s->microcodeRegister[0], 4);
	bool fclk = microcomp_emulator_bitElement8(s->microcodeRegister[0], 5);
	bool n_aluoe = microcomp_emulator_bitElement8(s->microcodeRegister[0], 6);
	bool aclk = microcomp_emulator_bitElement8(s->microcodeRegister[0], 7);

	/* bool unassigned0 = microcomp_emulator_bitElement8(s->microcodeRegister[1], 0); */
	/* bool unassigned1 = microcomp_emulator_bitElement8(s->microcodeRegister[1], 1); */
	bool n_coe = microcomp_emulator_bitElement8(s->microcodeRegister[1], 2);
	bool bclk = microcomp_emulator_bitElement8(s->microcodeRegister[1], 3);
	bool n_boe = microcomp_emulator_bitElement8(s->microcodeRegister[1], 4);
	bool n_foe = microcomp_emulator_bitElement8(s->microcodeRegister[1], 5);
	bool n_jmp = microcomp_emulator_bitElement8(s->microcodeRegister[1], 6);
	bool n_pcoe = microcomp_emulator_bitElement8(s->microcodeRegister[1], 7);

	bool n_dtoe = microcomp_emulator_bitElement8(s->microcodeRegister[2], 0);
	bool ddir = microcomp_emulator_bitElement8(s->microcodeRegister[2], 1);
	bool n_pcck = microcomp_emulator_bitElement8(s->microcodeRegister[2], 2);
	bool cclk = microcomp_emulator_bitElement8(s->microcodeRegister[2], 3);
	bool n_bcoe = microcomp_emulator_bitElement8(s->microcodeRegister[2], 4);
	/* bool n_wrd = microcomp_emulator_bitElement8(s->microcodeRegister[2], 5); */
	/* bool n_rdd = microcomp_emulator_bitElement8(s->microcodeRegister[2], 6); */
	bool n_upcrst = microcomp_emulator_bitElement8(s->microcodeRegister[2], 7);


	bool a7 = microcomp_emulator_bitElement8(s->a, 7);
	bool b7 = microcomp_emulator_bitElement8(s->b, 7);
	bool i1 = microcomp_emulator_bitElement8(s->instructionRegister, 1);

	uint8_t alufBus;
	bool cn4;
	bool cn8;
	{
		uint16_t out4 = 0x00;  // Default to no carry out.
		uint16_t out8 = 0x000;  // Default to no carry out.
		uint16_t carryIn = microcomp_emulator_bitElement8(s->f, 0);
		uint16_t a = s->a, a4 = 0x0f & s->a;
		uint16_t b = s->b, b4 = 0x0f & s->b;
		/* How do I emulate a carry chain?
		   Generate: x + 1
		   Propagate: x + carryIn
		   Carry: microcomp_emulator_bitElement8((x + 1) | (x + carryIn), 8 * sizeof(x)) */
		switch (s->instructionRegister & 0x07) {
		case 00:  // clear
			// Generate: 0xff + 1
			// Propagate: 0xff + carryIn
			// Carry: (0xff + 1) | (0xff + carryIn)
			out4 = 0x10;
			out8 = 0x100;
			alufBus = 0x00;
			break;
		case 01:  // xor
			// Generate: (a & b) + 1
			// Propagate: a + carryIn
			// Carry: ((a & b) + 1) | (a + carryIn)
			out4 = ((a4 & b4) + 1) | (a4 + carryIn);
			out8 = ((a & b) + 1) | (a + carryIn);
			alufBus = a ^ b;
			/* cn4 = ?; */
			/* cn8 = ?; */
			break;
		case 02:  // subtract
			// Generate: (a & (0xff - b)) + 1
			// Propagate: (a | (0xff - b)) + carryIn
			// Carry: ((a & (0xff - b)) + 1) | ((a | (0xff - b)) + carryIn)
			out4 = ((a4 & (0xf - b4)) + 1) | ((a4 | (0xf - b4)) + carryIn);
			out8 = ((a & (0xff - b)) + 1) | ((a | (0xff - b)) + carryIn);
			alufBus = a + (0xff & (1 + (0xff - b)));
			break;
		case 03:  // and
			// Generate: (0xff - b) + 1
			// Propagate: (a | (0xff - b)) + carryIn
			// Carry: ((0xff - b) + 1) | ((a | (0xff - b)) + carryIn)
			out4 = ((0xf - b4) + 1) | ((a4 | (0xf - b4)) + carryIn);
			out8 = ((0xff - b) + 1) | ((a | (0xff - b)) + carryIn);
			alufBus = a & b;
			break;
		case 04:  // reverse subtract
			// Generate: ((0xff - a) & b) + 1
			// Propagate: ((0xff - a) | b) + carryIn
			// Carry: (((0xff - a) & b) + 1) | (((0xff - a) | b) + carryIn)
			out4 = (((0xf - a4) & b4) + 1) | (((0xf - a4) | b4) + carryIn);
			out8 = (((0xff - a) & b) + 1) | (((0xff - a) | b) + carryIn);
			alufBus = b - a;
			break;
		case 05:  // or
			// Generate: 0 + 1
			// Propagate: (a & b) + carryIn
			// Carry: (0 + 1) | ((a & b) + carryIn)
			out4 = (a4 & b4) + carryIn;
			out8 = (a & b) + carryIn;
			alufBus = a | b;
			break;
		case 06:  // add
			// Generate: (a & b) + 1
			// Propagate: (a | b) + carryIn
			// Carry: ((a & b) + 1) | ((a | b) + carryIn)
			out4 = ((a4 & b4) + 1) | ((a4 | b4) + carryIn);
			out8 = ((a & b) + 1) | ((a | b) + carryIn);
			alufBus = a + b;
			break;
		case 07:  // preset
			// Generate: 0 + 1
			// Propagate: (a & b) + carryIn
			// Carry: (0 + 1) | ((a & b) + carryIn)
			out4 = (a4 & b4) + carryIn;
			out8 = (a & b) + carryIn;
			alufBus = 0xff;
			break;
		}
		cn4 = microcomp_emulator_bitElement8(out4, 4);
		cn8 = microcomp_emulator_bitElement8(out8, 8);
	}
	bool n_zero = alufBus != 0;
	bool flag = microcomp_emulator_bitElement8(s->f, 0x07 & s->instructionRegister);
	bool n_pcld = n_jmp | (flag ^ microcomp_emulator_bitElement8(s->instructionRegister, 3));
	bool ovr = !(((microcomp_emulator_bitElement8(s->instructionRegister, 0) ^ i1) ^ (b7 ^ a7))
	             || ((microcomp_emulator_bitElement8(alufBus, 7) ^ a7) ^ i1));
	bool sign = ovr ^ b7;
	bool evenParity = (microcomp_emulator_bitElement8(alufBus, 0)
	                   ^ microcomp_emulator_bitElement8(alufBus, 1)
	                   ^ microcomp_emulator_bitElement8(alufBus, 2)
	                   ^ microcomp_emulator_bitElement8(alufBus, 3)
	                   ^ microcomp_emulator_bitElement8(alufBus, 4)
	                   ^ microcomp_emulator_bitElement8(alufBus, 5)
	                   ^ microcomp_emulator_bitElement8(alufBus, 6)
	                   ^ microcomp_emulator_bitElement8(alufBus, 7));

	if (!n_pcoe) programAddressBus &= s->programCounter;
	bcAddressBus &= ((uint16_t) s->c << 8) | s->b;
	if (!n_pcck) {
		s->programCounter++;
		if (!n_pcld) {
			s->programCounter &= bcAddressBus;
		}
	}
	if (!n_bcoe) programAddressBus &= bcAddressBus;
	if (cclk) s->c = dataBus;
	if (!n_coe) dataBus &= s->c;
	if (!n_aoe) dataBus &= s->a;
	if (!n_ashoe) dataBus &= (s->a >> 1) | ((uint8_t) microcomp_emulator_bitElement8(s->f, 6) << 7);
	if (fclk) s->f = (fsors
	                  ? ((1 << 7)
	                     | ((uint8_t) microcomp_emulator_bitElement8(s->a, 0) << 6)
	                     | (evenParity << 5)
	                     | (cn4 << 4)
	                     | (n_zero << 3)
	                     | (ovr << 2)
	                     | (sign << 1)
	                     | (cn8 << 0))
	                  : dataBus);
	if (!n_foe) dataBus &= s->f;
	if (!n_aluoe) dataBus &= alufBus;
	if (aclk) s->a = dataBus;
	if (bclk) s->b = dataBus;
	if (!n_boe) dataBus &= s->b;
	if (!n_dtoe) {
		if (ddir)
			dataBus &= programDataBus;
		else
			programDataBus &= dataBus;
	}


	if (s->microprogramCounter == 0) s->instructionRegister &= dataBus;
	for (int i = 0; i < MICROCODE_WIDTH; i++) s->microcodeRegister[i] = s->microcode[i][s->microprogramCounter];
	if (!n_upcrst)
		s->microprogramCounter = 0;
	else
		s->microprogramCounter++;
}

// Step by one instruction.
void microcomp_emulator_step(microcomp_emulator_state_t *s) {
	do microcomp_emulator_clock(s); while (s->microprogramCounter);
}

// Run forever.
void microcomp_emulator_freeRun(microcomp_emulator_state_t *s) {
	while (1) microcomp_emulator_clock(s);
}

int main(int argc, char *argv[]) {
	(void) argc;
	(void) argv;
	microcomp_emulator_state_t microcomp;
	(void) microcomp_emulator_reset(&microcomp);
	for (int i = 0; i < 100; i++) {
		(void) microcomp_emulator_step(&microcomp);
	}
}

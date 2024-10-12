enum bytecode_e {
	bytecode_,
	bytecode_pushInteger,
	bytecode_pushCons,
	bytecode_pushNil,
	bytecode_pushComposite,
	bytecode_pushSubroutine,
	bytecode_inc,
	bytecode_add,
};

typedef unsigned char  u8;
typedef u8*  u16;

#define IP (ip++)
#define PUSH(value) *(sp--) = (value)[0]; *(sp--) = (value)[1]; *(sp--) = (value)[2]; *(sp--) = (value)[3];
#define POP(array) array[3] = *(++sp); array[2] = *(++sp); array[1] = *(++sp); array[0] = *(++sp);
#define NIL ((u8[4]) {0b00010110, 0x00, 0x00, 0x00})

void push(u16 sp, u16 address) {
	asm(// Store address[0]
	    "ldi address0 cb"
	    "ldm f"
	    "ldi spl cb"
	    "ldm a"
	    "ldi sph cb"
	    "ldm c"
	    "mov a b"
	    "stm f"

	    // Decrement stack pointer.
	    "ldi 0 f"
	    "ldi spl cb"
	    "ldm a"
	    "ldi 1 b"
	    "sub a b a"
	    "ldi (low spl) b"
	    "stm a"

	    "ldi sph cb"
	    "ldm a"
	    "ldi 0 b"
	    "sub a b a"
	    "ldi (low sph) b"
	    "stm a"

	    // Store address[1]
	    "ldi address1 cb"
	    "ldm f"
	    "ldi spl cb"
	    "ldm a"
	    "ldi sph cb"
	    "ldm c"
	    "mov a b"
	    "stm f"

	    // Decrement stack pointer.
	    "ldi 0 f"
	    "ldi spl cb"
	    "ldm a"
	    "ldi 1 b"
	    "sub a b a"
	    "ldi (low spl) b"
	    "stm a"

	    "ldi sph cb"
	    "ldm a"
	    "ldi 0 b"
	    "sub a b a"
	    "ldi (low sph) b"
	    "stm a"

	    // Store address[2]
	    "ldi address2 cb"
	    "ldm f"
	    "ldi spl cb"
	    "ldm a"
	    "ldi sph cb"
	    "ldm c"
	    "mov a b"
	    "stm f"

	    // Decrement stack pointer.
	    "ldi 0 f"
	    "ldi spl cb"
	    "ldm a"
	    "ldi 1 b"
	    "sub a b a"
	    "ldi (low spl) b"
	    "stm a"

	    "ldi sph cb"
	    "ldm a"
	    "ldi 0 b"
	    "sub a b a"
	    "ldi (low sph) b"
	    "stm a"

	    // Store address[3]
	    "ldi address3 cb"
	    "ldm f"
	    "ldi spl cb"
	    "ldm a"
	    "ldi sph cb"
	    "ldm c"
	    "mov a b"
	    "stm f"

	    // Decrement stack pointer.
	    "ldi 0 f"
	    "ldi spl cb"
	    "ldm a"
	    "ldi 1 b"
	    "sub a b a"
	    "ldi (low spl) b"
	    "stm a"

	    "ldi sph cb"
	    "ldm a"
	    "ldi 0 b"
	    "sub a b a"
	    "ldi (low sph) b"
	    "stm a");
}

int main() {
	u16 sp = (u16) 0x1234;
	u8 *bytecode = (u16) 0x5423;
	u16 ip = bytecode;
	u8 code;
	while (1) {
		code = *IP;
		if (code == bytecode_pushNil) {
			u8 temp[4];
			asm("ldi 0x16 a"
			    "ldi temp0 cb"
			    "stm a"

			    "ldi 0 a"
			    "ldi temp1 cb"
			    "stm a"

			    "ldi temp2 cb"
			    "stm a"

			    "ldi temp3 cb"
			    "stm a");
			PUSH(temp);
		}
		else if (code == bytecode_pushInteger) {
			u8 temp[4] = {*IP & 0xFC, *IP, *IP, *IP};
			PUSH(temp);
		}
		else if (code == bytecode_pushCons) {
			u8 temp[4] = {(*IP & 0xFC) | 0b11, *IP, (*IP & 0xFC) | 0b10, *IP};
			PUSH(temp);
		}
		else if (code == bytecode_pushComposite) {
			u8 temp[4] = {(*IP & 0xF0) | 0b0010, *IP, *IP, *IP};
			PUSH(temp);
		}
		else if (code == bytecode_pushSubroutine) {
			u8 temp[4] = {0b00000110, 0x00, *IP, *IP};
			PUSH(temp);
		}
		else if (code == bytecode_inc) {
			// inc
			u8 a[4];
			POP(a);
			asm("ldi 0 f"  // Clear carry.

			    "ldi a0 cb"  // Load a[0]
			    "ldm a"
			    "ldi 0x04 b"  // Set b to 0x04
			    "add a"  // a[0] + b[0]
			    "ldi (low a0) b"
			    "stm a"  // a[0] = a[0] + 0x04

			    "ldi a1 cb"  // Load a[1]
			    "ldm a"
			    "ldi 0 b"  // Clear b.
			    "add a"  // a[1] + b[1]
			    "ldi (low a1) b"
			    "stm a"  // a[1] = a[1] + 0 + carry

			    "ldi a2 cb"  // Load a[2]
			    "ldm a"
			    "ldi 0 b"  // Clear b.
			    "add a"  // a[2] + b[2]
			    "ldi (low a2) b"
			    "stm a"  // a[2] = a[2] + 0 + carry

			    "ldi a3 cb"  // Load a[3]
			    "ldm a"
			    "ldi 0 b"  // Clear b.
			    "add a"  // a[3] + b[3]
			    "ldi (low a3) b"
			    "stm a"  // a[3] = a[3] + 0 + carry

			    "ldi error cb"  // Error on overflow.
			    "br.v");
			PUSH(a);
		}
		else if (code == bytecode_add) {
			// inc
			u8 a[4];
			u8 b[4];
			POP(a);
			POP(b);
			asm("ldi 0 f"  // Clear carry.

			    "ldi a0 cb"  // Load a[0]
			    "ldm a"
			    "ldi b0 cb"  // Load b[0]
			    "ldm b"
			    "add a"  // a[0] + b[0]
			    "ldi (low b0) b"
			    "stm a"  // b[0] = a[0] + b[0]

			    "ldi a1 cb"  // Load a[1]
			    "ldm a"
			    "ldi b1 cb"  // Load b[1]
			    "ldm b"
			    "add a"  // a[1] + b[1]
			    "ldi (low b1) b"
			    "stm a"  // b[1] = a[1] + b[1]

			    "ldi a2 cb"  // Load a[2]
			    "ldm a"
			    "ldi b2 cb"  // Load b[2]
			    "ldm b"
			    "add a"  // a[2] + b[2]
			    "ldi (low b2) b"
			    "stm a"  // b[2] = a[2] + b[2]

			    "ldi a3 cb"  // Load a[3]
			    "ldm a"
			    "ldi b3 cb"  // Load b[3]
			    "ldm b"
			    "add a"  // a[3] + b[3]
			    "ldi (low b3) b"
			    "stm a"  // b[3] = a[3] + b[3]

			    "ldi error cb"  // Error on overflow.
			    "br.v");
			PUSH(b);
		}
		else goto error;
	}
	return 0;
 error: return 1;
}

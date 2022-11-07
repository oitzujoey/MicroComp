Microasm

Microasm is a simple assembler for microcode. It is not dedicated to any CPU, but it works best for non-branching microcode. See the MicroComp microcode for examples of how to use this.
The user is notified of syntax errors by throwing a segmentation fault. Use at your own risk.

Also included is a text to Intel HEX converter.

MicroAsm options:
    -f file     Microassembly file
    -o file     Output file
    -m format   Output format
                    BIN     binary (default)
                    HEX     hex
    -h          Help

Intel HEX generator usage:
    ./intel_hex_generator format sourcefile.txt output.hex
    
    format can be either 'b' for binary, or 'h' for hex.
    output.hex is optional since the program can write to sourcefile.hex instead.

.uasm file structure

Hardware model:
There are several ROMs in parallel. This means that the address lines are all tied to the other chips, but the data bits are kept separate. The address determined by concatenating the instruction register and the microprogram counter. The microprogram counter contains the LSb of the address, and the instruction register contains the MSb of the address. There is no built-in mechanism for the assembler to branch to a new address. It is assumed that the microprogram counter increments and it is the hardware designer's job to figure out how to do resets or branching. For an example of this, see the MicroComp schematics.

Syntax:
All keywords except ROMS, ROM and ASM can be placed nearly anywhere in the file.

VERSION     Microasm version that the uasm was written for.
WIDTH       Output width of the ROMs in bits. All ROMs must have the same bitwidth. This should be fine for most microcode.
DEPTH       The number of sequential microinstructions that a single instruction can execute.
ROMS        Number of ROM chips to use. What this actually does is tell the assembler how many output files to create. The following lines define the microinstructions and pin assignments.
ROM         Define each output bit of the ROM. This must be placed after the ROMS command in the file.
ASM         Number of instructions that the computer can execute. The following lines define the instruction set.

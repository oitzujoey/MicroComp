Microasm

Microasm is a simple assembler for microcode. It is not dedicated to any CPU, but it works best for non-branching microcode. See the MicroComp microcode for examples of how to use this.
Syntax checking is performed by throwing a segmentation fault. Use at your own risk.

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

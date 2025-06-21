#!/usr/bin/sh

if [ -z "$1" ]
then
	echo "Provide microcode version like this: v1.2.3"
	exit
fi

microcode_version=$1

microasm/microasm -f v1.1.3/microcode-v1.1.3.uasm -o "${microcode_version}/microcode-${microcode_version}-*" -m HEX
microasm/intel_hex_generator h ${microcode_version}/microcode-${microcode_version}-1.txt ${microcode_version}/microcode-${microcode_version}-1.hex
microasm/intel_hex_generator h ${microcode_version}/microcode-${microcode_version}-2.txt ${microcode_version}/microcode-${microcode_version}-2.hex
microasm/intel_hex_generator h ${microcode_version}/microcode-${microcode_version}-3.txt ${microcode_version}/microcode-${microcode_version}-3.hex
microasm/hex2mem ${microcode_version}/microcode-${microcode_version}-1.txt ${microcode_version}/microcode-${microcode_version}-1.mem
microasm/hex2mem ${microcode_version}/microcode-${microcode_version}-2.txt ${microcode_version}/microcode-${microcode_version}-2.mem
microasm/hex2mem ${microcode_version}/microcode-${microcode_version}-3.txt ${microcode_version}/microcode-${microcode_version}-3.mem
python microasm/mem2bin.py ${microcode_version}/microcode-${microcode_version}-1.mem
python microasm/mem2bin.py ${microcode_version}/microcode-${microcode_version}-2.mem
python microasm/mem2bin.py ${microcode_version}/microcode-${microcode_version}-3.mem

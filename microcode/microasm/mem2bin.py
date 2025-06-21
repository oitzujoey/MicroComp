#!/usr/bin/env python

import sys
from pathlib import Path

argv = sys.argv

if len(argv) != 2:
    print("usage: ./mem2bin.py <path to .mem file>")
    exit(1)

mem_file_name = argv[1]

byte_strings_list = []
with open(mem_file_name) as mem_file:
    bytes_bytes = bytes(map(lambda line: int(line.strip(), 16),
                            mem_file.readlines()))
with open(Path(mem_file_name).with_suffix('.bin'), 'wb') as bin_file:
    bin_file.write(bytes_bytes)

#!/usr/bin/bash

source=$1
binary=$(basename ${source} .hna).bin
emulation_directory=$(pwd)

pushd "$(dirname "${source}")"
duck-lisp "$(basename "${source}")"
cp ${binary} ${emulation_directory}/build/
popd
pushd build
cmake --build . && ./microcomp_emulator ./${binary}
popd

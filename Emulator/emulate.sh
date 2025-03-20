#!/usr/bin/bash

source=$1
binary=$2

pushd "$(dirname "${source}")"
duck-lisp "$(basename "${source}")"
popd
pushd build
cmake --build . && ./microcomp_emulator "../${binary}"
popd

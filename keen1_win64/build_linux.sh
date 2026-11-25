#!/bin/sh
# same build on Linux/macOS (used to verify the package): ./build_linux.sh /path/to/KEEN1.EXE
cc -std=gnu99 -O2 -w -Isrc -o keen1_selftest src/selftest.c src/unlzexe.c src/lifted.c || exit 1
[ -n "$1" ] && ./keen1_selftest "$1" tests/vectors.txt

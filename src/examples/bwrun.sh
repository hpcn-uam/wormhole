#!/bin/sh

OUTFILE=$(echo $$)

#valgrind ./testBW/testBW &> /tmp/lisp.$OUTFILE.out
./testBW/testBW &> /tmp/bw.$OUTFILE.out

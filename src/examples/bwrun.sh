#!/bin/sh

OUTFILE=$(echo $$)

rm /tmp/*c /tmp/*so /tmp/*out;

#valgrind ./testBW/testBW &> /tmp/lisp.$OUTFILE.out
./testBW/testBW &> /tmp/bw.$OUTFILE.out

#!/bin/sh

OUTFILE=$(echo $$)

rm /tmp/*c /tmp/*so /tmp/*out;

#valgrind ./testBW/testBW &> /tmp/lisp.$OUTFILE.out
#gdbserver 0.0.0.0:2345 ./testBW/testBW &> /tmp/bw.$OUTFILE.out

./testBW/testBW &> /tmp/bw.$OUTFILE.out

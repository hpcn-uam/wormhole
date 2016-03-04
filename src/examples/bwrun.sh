#!/bin/sh

OUTFILE=$(echo $$)

#valgrind ./testBW/testBW &> /tmp/lisp.$OUTFILE.out
#gdbserver 0.0.0.0:2345 ./testBW/testBW &> /tmp/bw.$OUTFILE.out

./testBW/testBW &> /tmp/bw.$OUTFILE.out

#sleep 60
#rm -f /tmp/*c /tmp/*so /tmp/*out;

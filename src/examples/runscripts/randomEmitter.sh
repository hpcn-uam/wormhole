#!/bin/sh

OUTFILE=$(echo $$)

#valgrind ./testBW/testBW &> /tmp/lisp.$OUTFILE.out
#gdbserver 0.0.0.0:2345 ./testBW/testBW &> /tmp/bw.$OUTFILE.out

#valgrind  --vgdb=yes --vgdb-error=0 
./randomEmitter "$@" &> /tmp/rnde.$OUTFILE.out

#sleep 60
#rm -f /tmp/*c /tmp/*so /tmp/*out;

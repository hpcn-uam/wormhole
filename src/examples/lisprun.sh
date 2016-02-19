#!/bin/sh

OUTFILE=$(echo $$)

#valgrind ./testLisp/testLisp &> /tmp/lisp.$OUTFILE.out
./testLisp/testLisp &> /tmp/lisp.$OUTFILE.out

#!/bin/bash

CFILES=$(find src -name '*.[ch]' -or -name '*.[ch]pp')
JFILES=$(find src -name '*.java')

. ./dependencies/formatOptions.sh

./$STYLEDIRECTORY/a.out --mode=c    $COMMONFLAGS $CFILES
./$STYLEDIRECTORY/a.out --mode=java $COMMONFLAGS $JFILES

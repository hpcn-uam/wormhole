#!/bin/bash

FOLDERS="src include dependencies/repos/netlib/ dependencies/repos/hptimelib/"

CFILES=$(find $FOLDERS '(' -name '*.[ch]' -or -name '*.[ch]pp' ')' -and -not '(' -wholename 'src/netlib*' -or -wholename 'src/einstein/netlib*' -or -wholename 'include/netlib*' ')')
JFILES=$(find $FOLDERS -name '*.java')

. ./dependencies/formatOptions.sh

./$STYLEEXECUTABLE --mode=c    $COMMONFLAGS $CFILES
./$STYLEEXECUTABLE --mode=java $COMMONFLAGS $JFILES

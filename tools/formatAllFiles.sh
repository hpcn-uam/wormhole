#!/bin/bash

FOLDERS="src include dependencies/repos/netlib/ dependencies/repos/hptimelib/"

CFILES=$(find $FOLDERS '(' -name '*.[ch]' -or -name '*.[ch]pp' ')' -and -not '(' -wholename 'src/netlib*' -or -wholename 'include/netlib*' ')')
JFILES=$(find $FOLDERS -name '*.java')

. ./dependencies/formatOptions.sh

./$STYLEDIRECTORY/a.out --mode=c    $COMMONFLAGS $CFILES
./$STYLEDIRECTORY/a.out --mode=java $COMMONFLAGS $JFILES

#!/bin/bash

OUTFILE=$(echo $$)

ulimit -c unlimited
FILES="~/.bashrc ~/.profile /etc/profile"

for FILE in "~/.bashrc" "~/.profile" "/etc/profile"
do
    if [ -e "$FILE" ]
    then
        . $FILE &>> /tmp/detectPro.$OUTFILE.out
    fi
done

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:lib
echo "LIBPATH = " $LD_LIBRARY_PATH &>> /tmp/detectPro.$OUTFILE.out

date &> /tmp/detectPro.time
#valgrind --vgdb=yes --vgdb-error=0 --track-origins=yes \
./detectPro extra/cfg/global.cfg &>> /tmp/detectPro.$OUTFILE.out
date &>> /tmp/detectPro.time

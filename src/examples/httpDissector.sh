#!/bin/bash

OUTFILE=$(echo $$)

#valgrind ./testBW/testBW &> /tmp/lisp.$OUTFILE.out
#gdbserver 0.0.0.0:2345 ./testBW/testBW &> /tmp/bw.$OUTFILE.out

FILES="~/.bashrc ~/.profile /etc/profile"

for FILE in "~/.bashrc" "~/.profile" "/etc/profile"
do
    if [ -e "$FILE" ]
    then
        . $FILE &>> /tmp/httpDissector.$OUTFILE.out
    fi
done

#cd testJBW
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:lib
echo "LIBPATH = " $LD_LIBRARY_PATH &>> /tmp/httpDissector.$OUTFILE.out

date &> /tmp/httpDissector.time
#valgrind --vgdb=yes --vgdb-error=0 --track-origins=yes \
./httpDissector -i prueba --raw &>> /tmp/httpDissector.$OUTFILE.out
date &>> /tmp/httpDissector.time

#sleep 60
#rm -f /tmp/*c /tmp/*so /tmp/*out;

#!/bin/sh

OUTFILE=$(echo $$)

ulimit -c unlimited
FILES="~/.bashrc ~/.profile /etc/profile"

for FILE in "~/.bashrc" "~/.profile" "/etc/profile"
do
    if [ -e "$FILE" ]
    then
        . $FILE &>> /tmp/throughput_server.$OUTFILE.out
    fi
done

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:lib

./perf_throughput_server "$@" &>> /tmp/throughput_server.$OUTFILE.out

#!/bin/bash

OUTFILE=$(echo $WORM_ID)

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

mkdir -p /root/detectPro/instance/$WORM_ID/data
mkdir -p /root/detectPro/instance/$WORM_ID/capture
mkdir -p /root/detectPro/instance/$WORM_ID/flows
mkdir -p /root/detectPro/instance/$WORM_ID/cfg


cp /root/detectPro/extra/cfg/{alarms.cfg,networks.cfg,ordenMenuRedes.cfg,supernetworks.cfg} /root/detectPro/instance/$WORM_ID/cfg/.
sed -e 's/detectPro\/extra/detectPro\/instance\/'$WORM_ID'/g' extra/cfg/global.cfg > /root/detectPro/instance/$WORM_ID/cfg/global.cfg

date &> /tmp/detectPro.time
#valgrind --vgdb=yes --vgdb-error=0 --track-origins=yes \
./detectPro /root/detectPro/instance/$WORM_ID/cfg/global.cfg &>> /tmp/detectPro.$OUTFILE.out
date &>> /tmp/detectPro.time

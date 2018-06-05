#!/bin/bash

OUTFILE=$(echo $WORM_ID)
ROOTDIR=/mnt/raid

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

mkdir -p $ROOTDIR/$WORM_ID/data
mkdir -p $ROOTDIR/$WORM_ID/capture
mkdir -p $ROOTDIR/$WORM_ID/flows
mkdir -p $ROOTDIR/$WORM_ID/cfg


cp /root/detectPro/extra/cfg/{alarms.cfg,networks.cfg,ordenMenuRedes.cfg,supernetworks.cfg} $ROOTDIR/$WORM_ID/cfg/.
sed -e 's/detectPro\/extra/detectPro\/instance\/'$WORM_ID'/g' extra/cfg/global.cfg > $ROOTDIR/$WORM_ID/cfg/global.cfg

date &>  $ROOTDIR/$WORM_ID/detectPro.$OUTFILE.log
#valgrind --vgdb=yes --vgdb-error=0 --track-origins=yes \
./detectPro $ROOTDIR/$WORM_ID/cfg/global.cfg &>> $ROOTDIR/$WORM_ID/detectPro.$OUTFILE.log
date &>> $ROOTDIR/$WORM_ID/detectPro.$OUTFILE.log

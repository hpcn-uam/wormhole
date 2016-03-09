#!/bin/bash

LIBS=/root/flink/build-target/lib/flink-dist_2.10-1.1-SNAPSHOT.jar
SRC=org/apache/flink/streaming/examples/socket
FOLDER=org


rm -rf $SRC/*class
javac -cp $LIBS $SRC/*java && jar cvfm bw.jar META-INF/manifest.txt $FOLDER

gcc -O3 -Wall gentext.c -o gentext -std=gnu11

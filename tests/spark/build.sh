#!/bin/bash

SPARKDIR="/root/streaming/spark/tar/spark-2.3.1-bin-hadoop2.7"

javac -cp "$SPARKDIR/jars/*" $(find wh -iname "*.java") && jar cf tests.jar wh #-Xlint:unchecked

echo "======= DONE ======="

# Example: ../bin/spark-submit --class org.apache.spark.examples.streaming.JavaNetworkWordCount tests.jar 127.0.0.1 9999
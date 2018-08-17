#!/bin/bash

STORMDIR="/root/streaming/storm/tar/apache-storm-1.2.2"

javac -cp "$STORMDIR/lib/*" $(find wh -iname "*.java") && jar cf tests.jar wh #-Xlint:unchecked

echo "======= DONE ======="

# Example: ../bin/spark-submit --class org.apache.spark.examples.streaming.JavaNetworkWordCount tests.jar 127.0.0.1 9999
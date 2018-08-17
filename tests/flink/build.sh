#!/bin/bash

FLINKDIR="/root/streaming/flink/tar/flink-1.5.0"

javac -cp "$FLINKDIR/lib/*" $(find wh -iname "*.java") && jar cf tests.jar wh #-Xlint:unchecked

echo "======= DONE ======="

# Example: ../bin/spark-submit --class org.apache.spark.examples.streaming.JavaNetworkWordCount tests.jar 127.0.0.1 9999
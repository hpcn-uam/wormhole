#!/bin/bash

KAFKADIR="/root/streaming/kafka/tar/kafka_2.11-1.1.0/"

javac -cp "$KAFKADIR/libs/*" $(find wh -iname "*.java") && jar cf tests.jar wh

# TOPICS

# Delete old
$KAFKADIR/bin/kafka-topics.sh --delete --zookeeper 127.0.0.1:2181 --topic thr
$KAFKADIR/bin/kafka-topics.sh --delete --zookeeper 127.0.0.1:2181 --topic latin
$KAFKADIR/bin/kafka-topics.sh --delete --zookeeper 127.0.0.1:2181 --topic latout

# New
$KAFKADIR/bin/kafka-topics.sh --create --zookeeper 127.0.0.1:2181 --topic thr --replication-factor 1 --partitions 8
$KAFKADIR/bin/kafka-topics.sh --alter  --zookeeper 127.0.0.1:2181 --topic thr --config retention.ms=1000

$KAFKADIR/bin/kafka-topics.sh --create --zookeeper 127.0.0.1:2181 --topic latin --replication-factor 1 --partitions 1
$KAFKADIR/bin/kafka-topics.sh --alter  --zookeeper 127.0.0.1:2181 --topic latin --config retention.ms=1000
$KAFKADIR/bin/kafka-topics.sh --create --zookeeper 127.0.0.1:2181 --topic latout --replication-factor 1 --partitions 1
$KAFKADIR/bin/kafka-topics.sh --alter  --zookeeper 127.0.0.1:2181 --topic latout --config retention.ms=1000

echo "======= DONE ======="
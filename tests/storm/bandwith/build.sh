#!/bin/bash

wget -nc http://central.maven.org/maven2/org/apache/storm/storm-core/1.2.2/storm-core-1.2.2.jar

javac -cp storm-core-1.2.2.jar org/apache/storm/wh/tests/BandwithMeter.java && jar cf bw.jar org

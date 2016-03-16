#!/bin/bash

javac -cp /usr/hdp/2.4.0.0-169/storm/lib/storm-core-0.10.0.2.4.0.0-169.jar backtype/storm/wh/tests/BandwithMeter.java && jar cf bw.jar backtype

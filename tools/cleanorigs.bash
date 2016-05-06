#!/bin/bash

for file in $(find . -name "*orig")
do
	echo removing $file ...
	rm -rf $file
done

for file in $(find . -name "*class")
do
	echo removing $file ...
	rm -rf $file
done

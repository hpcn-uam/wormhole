#!/bin/bash

for file in $(find . -name "*orig")
do
	echo removing $file ...
	rm -rf $file
done

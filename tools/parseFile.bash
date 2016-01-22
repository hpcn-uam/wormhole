#!/bin/bash

FILES=$(ls include/*.h)


for f in $FILES
do
	flag=0

	cat $f | while read in
	do

		if [[ flag -eq 0 ]]
		then

			if [[ $in == *"struct"* || $in == *"enum"* ]]
			then
				echo "${in}"
				flag=1
			fi
		else
                        echo "${in}"
		fi


                if [[ $in == *"}"* ]]
                then
                        flag=0
                fi

	done

done

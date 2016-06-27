#!/bin/bash

FILES=$(ls include/*.h)


for f in $FILES
do
	flag=0

	cat $f | while read in
	do

		if [[ $in == *"WH_DymRoute_send"*";"* || $in == *"typedef"*";"* ]] #Functions and inline typedefs
		then
			echo "${in}"
		fi

		if [[ $in == *"struct "* || $in == *"enum"* || $in == *"union"* ]]
		then

			if [[ $in == *"{"* ]]
			then
				((flag+=1))
#				echo AAAAAADDDD $flag
			fi
		fi

		if [ $flag -gt 0 ]
		then
                        echo "${in}"
		fi

                if [[ $in == *"}"* ]]
        	then
      		        ((flag-=1))
#			echo LEEEESS $flag
               	fi

	done

done

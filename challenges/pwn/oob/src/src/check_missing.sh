#!/bin/bash

function usage () {

	echo "$0 <directory> [<directory> [ ...]]"

}

if [ $# -lt 1 ]
then
	usage $0
fi

for D in $@
do

	echo "Checking $D for missing functions"

	for F in $D/*.o
	do

		MISSING=$(readelf -r $F | grep PLT32)

		if [ $(echo $MISSING | grep "\S" | wc -l) -gt 0 ]
		then

			while IFS= read -r LINE
			do

				N=$(echo $LINE | rev | cut -d' ' -f3 | rev)
				echo -e "\e[31;1mWARNING\e[0m: You may be missing $N in $D/$F"

			done <<< "$MISSING"

		fi

	done

done
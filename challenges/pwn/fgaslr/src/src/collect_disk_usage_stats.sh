#!/bin/bash

PROGS=(toy md5sum nc)

for P in ${PROGS[@]}
do

	S=$(find $P -type f \( -name '*.o' -o -name "*.bin" \) -exec wc -c {} + | grep total | awk '{print $1}')
	SO=$(wc -c ${P}_orig/${P}.bin | awk '{print $1}')

	echo "$P,$S"
	echo "${P}_orig,$SO"

done
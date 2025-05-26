#!/bin/bash

# This script depends on...
# - GDB + GEF
# - Radare2

function get_maps_fgaslr () {

	BP=$(r2 -A -s sym.main -c 'pdf ~call r8' -qq $1 | awk '{print $2}')

	gdb \
		-ex "gef config gef.disable_color True" \
		-ex "break *$BP" -ex "run ${@:2}" -ex 'vmmap' \
		-ex 'quit' $1 2>&1 \
		| grep -A9999 "Perm Path" | tail -n +2

}

function get_maps_orig () {

	gdb \
		-ex "gef config gef.disable_color True" \
		-ex 'break *main' -ex "run  ${@:2}" -ex 'vmmap' \
		-ex 'quit' $1 2>&1 \
		| grep -A9999 "Perm Path" | tail -n +2

}

function maps_sum () {

	TOTAL=0

	while read LINE
	do

		START=$(echo $LINE | awk '{print $1}')
		END=$(echo $LINE | awk '{print $2}')
		SIZE=$(($END - $START))
		TOTAL=$(($TOTAL + $SIZE))

	done

	echo $TOTAL

}

TOY_ORIG=$(get_maps_orig ./toy_orig/toy.bin asdf | maps_sum)
TOY_FGASLR=$(get_maps_fgaslr ./toy/toy.bin asdf | maps_sum)

echo "toy,$TOY_FGASLR"
echo "toy_orig,$TOY_ORIG"

MD5SUM_ORIG=$(get_maps_orig ./md5sum_orig/md5sum.bin -x < ./md5sum_orig/input | maps_sum)
MD5SUM_FGASLR=$(get_maps_fgaslr ./md5sum/md5sum.bin -x < ./md5sum/input | maps_sum)

echo "md5sum,$MD5SUM_FGASLR"
echo "md5sum_orig,$MD5SUM_ORIG"

NC_ORIG=$(get_maps_orig ./nc_orig/nc.bin -h | maps_sum)
NC_FGASLR=$(get_maps_fgaslr ./nc/nc.bin -h | maps_sum)

echo "nc,$NC_FGASLR"
echo "nc_orig,$NC_ORIG"

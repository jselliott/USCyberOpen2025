#!/bin/sh 

mips-linux-gnu-gcc -c uhttpd-utils.c -o uhttpd-utils.o -g
mips-linux-gnu-gcc -c cgi_utils.c -o cgi_utils.o -g
mips-linux-gnu-gcc -g -c uhttpd.c -o uhttpd.o

mips-linux-gnu-gcc cgi_utils.o uhttpd-utils.o uhttpd.o -o binary

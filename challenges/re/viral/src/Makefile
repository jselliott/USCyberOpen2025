CC=gcc
CFLAGS=-O2 -Wall -fPIC
LDLIBS=-ldl

all: final_binary

flag_check.bin: flag_check.c
	$(CC) $(CFLAGS) -c flag_check.c -o flag_check.o
	objcopy -O binary flag_check.o flag_check.bin

payload.enc: flag_check.bin encrypt.py
	python3 encrypt.py

loader: loader.c
	$(CC) $(CFLAGS) loader.c -o loader $(LDLIBS)

final_binary: loader payload.enc append_payload.py
	python3 append_payload.py loader payload.enc final_binary
	chmod +x final_binary

clean:
	rm -f *.o *.bin *.enc loader final_binary

.PHONY: all clean

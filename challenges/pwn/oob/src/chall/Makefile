
CFLAGS   := -fPIC -fno-jump-tables -Wall -fno-stack-protector -no-pie -ffixed-rbp -fomit-frame-pointer -Wno-incompatible-pointer-types



CFLAGS		+= -I./

all:

	$(CC) $(CFLAGS) -c -o check_winner.o check_winner.c
	$(CC) $(CFLAGS) -c -o fake_fini.o fake_fini.c
	$(CC) $(CFLAGS) -c -o fake_init.o fake_init.c
	$(CC) $(CFLAGS) -c -o fake_rtld_fini.o fake_rtld_fini.c	
	$(CC) $(CFLAGS) -c -o get_index_from_input.o get_index_from_input.c
	$(CC) $(CFLAGS) -c -o __libc_start_main_sim.o __libc_start_main_sim.c
	$(CC) $(CFLAGS) -c -o play_game.o play_game.c
	$(CC) $(CFLAGS) -c -o print_board.o print_board.c
	$(CC) $(CFLAGS) -c -o print.o print.c
	$(CC) $(CFLAGS) -c -o _start.o _start.c
	$(CC) $(CFLAGS) -c -o sys_exit.o sys_exit.c
	$(CC) $(CFLAGS) -c -o sys_open.o sys_open.c
	$(CC) $(CFLAGS) -c -o sys_read.o sys_read.c
	$(CC) $(CFLAGS) -c -o write.o write.c
	$(CC) $(CFLAGS) -c -o win.o win.c

	$(CC) $(CFLAGS) -fPIE -g -o chall.bin chall.c \
		../src/fgaslr.c -D'PROG_NAME="chall"' ../src/start.c \
		../src/cache.c ../src/mappings.c ../src/graph.c ../src/stats.c

clean:

	rm *.o *.bin


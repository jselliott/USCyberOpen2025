obj-m += netdev.o
netdev-objs := src/netdev.o src/ringbuf.o

all:
	make -C ../linux-6.14.7 M=$(PWD) ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- modules

clean:
	make -C ../linux-6.14.7 M=$(PWD) ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- clean

obj-m += filter.o
filter-objs := src/filter.o src/filter_vm.o

all:
	make -C ../linux-6.14.7 M=$(PWD) ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- modules

clean:
	make -C ../linux-6.14.7 M=$(PWD) ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- clean

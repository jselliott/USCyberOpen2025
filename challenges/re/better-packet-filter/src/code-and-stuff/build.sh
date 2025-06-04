#!/bin/bash

# Download source
wget https://cdn.kernel.org/pub/linux/kernel/v6.x/linux-6.14.7.tar.xz
tar xvf linux-6.14.7.tar.xz

# Compile the kernel
make -C linux-6.14.7 ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- defconfig
make -C linux-6.14.7 ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -j$(nproc) all

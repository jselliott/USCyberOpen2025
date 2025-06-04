#!/bin/sh

echo "Booting kernel..."

timeout 120s qemu-system-aarch64 \
    -kernel /opt/Image.gz \
    -initrd /opt/initramfs.cpio.gz \
    -m 256M \
    -nographic \
    -machine virt \
    -cpu cortex-a76 \
    -no-reboot \
    -monitor none \
    -append 'panic=-1 oops=panic sysctl.kernel.dmesg_restrict=1 quiet loglevel=0'

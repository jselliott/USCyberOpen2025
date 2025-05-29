#!/bin/bash

unset $(env | cut -d= -f1)          
PATH="/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin"

if [ ! -f /flag.txt ]; then
    echo "[!] /flag.txt not found."
    exit 1
fi

flag=$(cat /flag.txt)

echo "Enter base64-encoded payload:"
read -r base64_payload

echo $base64_payload
echo $base64_payload | base64 -d | md5sum


if [ $? -ne 0 ]; then
    echo "[!] Failed to decode base64 input."
    exit 1
fi

echo $payload

for i in `seq 1 5`; do
    lib="./libs/$i/"

    echo "[*] Running Step $i"
    val=$(echo "$base64_payload" | grep -q "DEBUG")
    if [ $? -eq 0 ]; then
	    output=$(echo "$base64_payload" | base64 -d | /usr/sbin/chroot --userspec=1000:1000 ./ ./qemu-x86_64-static -d page -L $lib ./library_independence_binary_001 )
    else
	    output=$(echo "$base64_payload" | base64 -d | /usr/sbin/chroot --userspec=1000:1000 ./ ./qemu-x86_64-static -L $lib ./library_independence_binary_001 )
    fi

    val=$(echo "$output" |  grep -q "$flag")

    if [ $? -eq 0 ]; then
        echo "[✓] Step $i passed."
    else
        echo "[✗] Step $i failed: flag not found in output."
        exit $i
    fi
done

echo "[✔] All steps passed successfully."
echo "Flag:" $flag


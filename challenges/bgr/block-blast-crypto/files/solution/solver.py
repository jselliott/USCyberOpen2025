#!/usr/bin/env python3
"""
Docker-compatible solver for the AES-ECB byte-at-a-time oracle
"""

import socket
import binascii

BLOCK_SIZE = 16
HOST = "localhost"
PORT = 1337

def recv_until(sock, delim=b"> "):
    """Receive until prompt delimiter is found."""
    data = b""
    while not data.endswith(delim):
        chunk = sock.recv(4096)
        if not chunk:
            raise ConnectionError("Socket closed")
        data += chunk
    return data

def oracle(sock, user_bytes: bytes) -> bytes:
    """Send input to the oracle and return decrypted ciphertext."""
    if not user_bytes:
        user_bytes = b"\x00"

    payload = binascii.hexlify(user_bytes) + b"\n"
    sock.sendall(payload)

    data = recv_until(sock)
    lines = [line.strip() for line in data.decode().splitlines() if line.strip() and line.strip() != '>']

    if not lines:
        raise ValueError("No usable response from oracle")

    ct_hex = lines[-1]
    try:
        return binascii.unhexlify(ct_hex)
    except binascii.Error:
        raise ValueError(f"Expected hex, got: {ct_hex!r}")

def main():
    with socket.create_connection((HOST, PORT)) as sock:
        recv_until(sock)  # consume the welcome banner
        recovered = b""
        print("[*]  Starting byte-at-a-time attack")

        while True:
            pad_len   = (BLOCK_SIZE - len(recovered) % BLOCK_SIZE) - 1
            prefix    = b"A" * pad_len
            block_idx = (len(prefix) + len(recovered)) // BLOCK_SIZE

            if pad_len == 0:
                prefix = b"A" * BLOCK_SIZE
                block_idx += 1

            full_ct = oracle(sock, prefix)
            target_blk = full_ct[block_idx * BLOCK_SIZE : (block_idx + 1) * BLOCK_SIZE]

            found_byte = None
            for guess in range(256):
                test_ct = oracle(sock, prefix + recovered + bytes([guess]))
                guess_blk = test_ct[block_idx * BLOCK_SIZE : (block_idx + 1) * BLOCK_SIZE]
                if guess_blk == target_blk:
                    found_byte = guess
                    recovered += bytes([guess])
                    print(f"[+]  Recovered byte {len(recovered):02d}: {bytes([guess])!r}")
                    break

            if found_byte is None:
                print("[!]  Failed to match any byte — attack aborted.")
                break

            if recovered.endswith(b"}"):
                print("\nFLAG:", recovered.decode())
                break

        sock.sendall(b"exit\n")

if __name__ == "__main__":
    main()

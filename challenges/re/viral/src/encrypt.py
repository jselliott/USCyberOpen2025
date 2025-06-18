def lcg(seed, a, c, m, length):
    x = seed
    for _ in range(length):
        x = (a * x + c) % m
        yield x & 0xFF

def encrypt(data, seed=0x1337, a=1103515245, c=12345, m=2**31):
    keystream = lcg(seed, a, c, m, len(data))
    return bytes([b ^ k for b, k in zip(data, keystream)])

with open("flag_check.bin", "rb") as f:
    raw = f.read()

encrypted = encrypt(raw)

with open("payload.enc", "wb") as f:
    f.write(encrypted)

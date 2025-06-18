def lcg_stream(seed, a, c, m, length):
    x = seed
    for _ in range(length):
        x = (a * x + c) % m
        yield x & 0xFF

def encrypt(data, seed=0x1337, a=1103515245, c=12345, m=2**31):
    key_stream = lcg_stream(seed, a, c, m, len(data))
    return bytes([b ^ k for b, k in zip(data, key_stream)])

with open("check_flag.bin", "rb") as f:
    code = f.read()

enc = encrypt(code)

with open("payload.enc", "wb") as f:
    f.write(enc)

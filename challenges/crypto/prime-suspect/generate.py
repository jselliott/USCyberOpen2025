import random
from Crypto.Util.number import getPrime, bytes_to_long, long_to_bytes

# Parameters
bit_length = 256  # size of n
e = 65537
flag = b"SVUSCG{sm4ll_pr1m3s}"

def generate_rsa_keypair(bits, e):
    while True:
        p = getPrime(bits // 2)
        q = getPrime(bits // 2)
        n = p * q
        phi = (p - 1) * (q - 1)
        try:
            d = pow(e, -1, phi)
            return p, q, n, d
        except ValueError:
            continue  # e not invertible mod phi, retry

# Generate keys
p, q, n, d = generate_rsa_keypair(bit_length, e)

# Encrypt the flag
m = bytes_to_long(flag)
if m >= n:
    raise ValueError("Flag is too large for the modulus size!")
c = pow(m, e, n)

# Decrypt for verification
decrypted_m = pow(c, d, n)
decrypted_flag = long_to_bytes(decrypted_m)

# Output
#print(f"p = {p}")
#print(f"q = {q}")
print(f"n = {n}")
#print(f"d = {d}")
print(f"e = {e}")
print(f"c = {c}")
print(f"Decrypted flag = {decrypted_flag}")

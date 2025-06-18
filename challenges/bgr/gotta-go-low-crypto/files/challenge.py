import random
from math import gcd
import sympy

def genKeypair(p, q):
    n = p * q
    phi = (p - 1) * (q - 1)

    print(n)
    e = 3

    def modinv(a, m):
        m0, x0, x1 = m, 0, 1
        while a > 1:
            q = a // m
            m, a = a % m, m
            x0, x1 = x1 - q * x0, x0
        return x1 + m0 if x1 < 0 else x1

    d = modinv(e, phi)
    return ((e, n), (d, n))

def encrypt(pubKey, plaintext):
    e, n = pubKey

    plaintextInt = int.from_bytes(plaintext.encode(), byteorder='big')

    encrypted = pow(plaintextInt, e, n)
    return encrypted

def genPrime(bits):
    while True:
        possible = random.getrandbits(bits)
        possible |= (1 << bits - 1) | 1

        if sympy.isprime(possible):
            return possible

if __name__ == "__main__":
    while True:
        p = genPrime(512)
        q = genPrime(512)

        if gcd(3, (p - 1) * (q - 1)) == 1:
            break

    pubKey, privKey = genKeypair(p, q)

    message = "SVBGR{test_flag}"
    encrypted = encrypt(pubKey, message)

    print("Encrypted message:", encrypted)
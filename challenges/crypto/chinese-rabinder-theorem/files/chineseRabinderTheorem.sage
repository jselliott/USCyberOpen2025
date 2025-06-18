from Crypto.Util.number import *
bunchaPrimes = list(set([getPrime(10) for _ in range(2000)]))
ittyBitty = list(filter(lambda x: x % 4 == 3, bunchaPrimes))
N = prod(ittyBitty)
msg = bytes_to_long(flag)
assert(GCD(msg, N) == 1)
assert(msg < N)
ct = pow(msg, 2, N)
print(N)
#30327208759781412025136331048643419536910103237132356122012246111636453702769363409081910784093069558218111
print(ct)
#17809769080654903649334892184111600681027552797084733634899402165936938957467437130366159467052858323916324
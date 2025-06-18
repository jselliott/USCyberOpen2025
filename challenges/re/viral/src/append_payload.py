import sys
import struct

ELF = sys.argv[1]
ENC = sys.argv[2]
OUT = sys.argv[3]

with open(ELF, "rb") as f:
    elf_data = f.read()
with open(ENC, "rb") as f:
    payload = f.read()

offset = len(elf_data)
patched = bytearray(elf_data)

placeholder = b'\xef\xbe\xad\xde'
replacement = struct.pack("<I", offset)

idx = patched.find(placeholder)
if idx == -1:
    print("Could not find shell_offset placeholder!")
    sys.exit(1)

patched[idx:idx+4] = replacement
with open(OUT, "wb") as f:
    f.write(patched)
    f.write(payload)

print(f"[+] Appended {len(payload)} bytes at offset {offset}")
print(f"[+] Output written to {OUT}")

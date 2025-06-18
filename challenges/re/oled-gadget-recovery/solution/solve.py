#!/usr/bin/env python3
from pathlib import Path
from PIL import Image

BIN_PATH   = Path("oled-gadget.bin") # input file
OFFSET     = 0x5C88                  # starting offset
N_BYTES    = 2_560                   # 128×160 / 8
WIDTH      = 160
HEIGHT     = 128
OUT_PATH   = Path("flag.png")         # output image
# ----------------------------------------------------------------------

with BIN_PATH.open("rb") as f:
    f.seek(OFFSET)
    data = f.read(N_BYTES)
    if len(data) != N_BYTES:
        raise ValueError("File too short -- couldn’t read the full 2560 bytes")

img = Image.new("1", (WIDTH, HEIGHT), color=1)
pixels = img.load()

# Pillow indexing makes this a bit more confusing.

BYTES_PER_PAGE = HEIGHT
for page in range(WIDTH // 8):
    col = page * 8
    for y in range(HEIGHT):
        byte_val = data[page * BYTES_PER_PAGE + y]
        py = HEIGHT - 1 - y
        for bit in range(8):
            px = col + bit
            bit_val = (byte_val >> bit) & 1
            pixels[px, py] = 1 if bit_val else 0

img.save(OUT_PATH)
print(f"✓ Wrote {OUT_PATH} ({WIDTH}×{HEIGHT} 1-bit PNG)")
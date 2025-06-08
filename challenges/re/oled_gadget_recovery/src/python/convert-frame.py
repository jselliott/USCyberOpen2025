# -----------------------------------------------------------------------------
# Copyright (c) 2025 Troy Callahan / Cybolt Solutions LLC
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the “Software”), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, provided that the following attribution requirement is
# met:
#
#     ▶  All copies or substantial portions of the Software (source or binary)
#        must retain this copyright notice, the permission notice, and a clear
#        attribution to Troy Callahan / Cybolt Solutions LLC.
#
# THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
# -----------------------------------------------------------------------------


from PIL import Image
from os import listdir, makedirs
from os.path import isfile, join, splitext, exists

def make_frames(frame_dir, output_dir):
    if not exists(output_dir):
        makedirs(output_dir)

    # Get all PNG files in the directory
    onlyfiles = [f for f in listdir(frame_dir) if isfile(join(frame_dir, f)) and f.endswith('.png')]

    frame_headers = []  # Store references for frame.h

    for file in onlyfiles:
        base_name, _ = splitext(file)
        png_path = join(frame_dir, file)
        output_path = join(output_dir, f"{base_name}.h")
        array_name = base_name.replace("-", "_")

        # Convert the PNG to a C header
        convert_frame_to_c_header(png_path, output_path, raw_array_name=array_name)

        # Add reference for frame.h
        frame_headers.append(f"extern const uint8_t {array_name}[];")

    # Generate the frame.h file
    frame_h_path = join(output_dir, "frames.h")
    with open(frame_h_path, "w") as f:
        # Header guard start
        f.write("#ifndef FRAMES_H\n")
        f.write("#define FRAMES_H\n\n")

        # C++ extern guards
        f.write("#ifdef __cplusplus\n")
        f.write('extern "C" {\n')
        f.write("#endif\n\n")

        # Include all generated frame headers
        for file in onlyfiles:
            base_name, _ = splitext(file)
            f.write(f'#include "{base_name}.h"\n')
        f.write("\n")

        # C++ extern guards close
        f.write("#ifdef __cplusplus\n")
        f.write("}\n")
        f.write("#endif\n\n")

        # Header guard end
        f.write("#endif // FRAMES_H\n")

    print(f"Master frame header file generated at: {frame_h_path}")

def convert_frame_to_c_header(png_path, output_path, raw_array_name="raw_image_data"):
    # Open the image and convert it to monochrome (1-bit pixels)
    img = Image.open(png_path).convert("1")
    width, height = img.size

    if width != 160 or height != 128:
        raise ValueError(f"Image must be exactly 160x128 pixels. Fond {width}x{height}")

    # Get the pixel data
    pixels = img.load()

    # Generate the packed data
    c_array_raw = []
    for x in range(0, width, 8):
        page = []
        for y in range(0, height):
            value = 0x00
            for b in range(8):  # Process each bit in the byte
                bit = 1 if pixels[x+b, y] == 255 else 0
                value |= (bit << b)
            page.append(value)
        c_array_raw.append(page)

    # Write the C header file
    with open(output_path, "w") as f:
        # Header guard start
        f.write(f"#ifndef {raw_array_name.upper()}_H\n")
        f.write(f"#define {raw_array_name.upper()}_H\n\n")

        # C++ extern guards
        f.write("#ifdef __cplusplus\n")
        f.write('extern "C" {\n')
        f.write("#endif\n\n")

        # Include necessary headers
        # f.write('#include "frame.h"\n')
        f.write("#include <stdint.h>\n\n")

        # Define the data array
        num_bytes = sum(len(page) for page in c_array_raw)
        f.write(f"static const uint8_t {raw_array_name}_frame[{num_bytes}] = {{\n")
        for index, page in enumerate(c_array_raw):
            f.write("    " + ", ".join(f"0x{val:02X}" for val in reversed(page)))
            if index != len(c_array_raw) - 1:  # Avoid trailing comma on last line
                f.write(",\n")
            else:
                f.write("\n")
        f.write("};\n\n")

        # C++ extern guards close
        f.write("#ifdef __cplusplus\n")
        f.write("}\n")
        f.write("#endif\n\n")

        # Header guard end
        f.write(f"#endif // {raw_array_name.upper()}_H\n")

    print(f"Header file generated at: {output_path}")

# Usage example
make_frames("./python/png/frames", "./Inc/frame")



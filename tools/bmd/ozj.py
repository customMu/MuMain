"""Converts the textures of the MU client: OZJ = JPEG, OZT = TGA, OZB = BMP, each with a dump prefix.

The client writes them in CGlobalBitmap::Save_Image (GlobalBitmap.cpp): the first 24 (OZJ) or 4 (OZT, OZB)
bytes of the image, then the whole image. It reads OZT as uncompressed 32-bit TGA, rows bottom-up, without an
image id (OpenTga), so write_tga writes exactly that.

    python ozj.py unpack Data/Monster/golem.OZJ textures/      -> textures/golem.jpg
    python ozj.py pack textures/golem.jpg Data/Monster/         -> Data/Monster/golem.OZJ
"""
import os
import struct
import sys

PREFIX_SIZE = {'.ozj': 24, '.ozt': 4, '.ozb': 4}
PACKED_EXT = {'.jpg': '.OZJ', '.jpeg': '.OZJ', '.tga': '.OZT', '.bmp': '.OZB'}
UNPACKED_EXT = {'.ozj': '.jpg', '.ozt': '.tga', '.ozb': '.bmp'}
_TGA_HEADER = struct.Struct('<BBBHHBHHHHBB')
_TGA_TRUECOLOR = 2
_TGA_BPP = 32
_TGA_ALPHA_BITS = 8


def unpack_bytes(data, ext):
    return data[PREFIX_SIZE[ext.lower()]:]


def pack_bytes(data, packed_ext):
    return data[:PREFIX_SIZE[packed_ext.lower()]] + data


def unpack(path, out_dir):
    """OZJ/OZT/OZB -> jpg/tga/bmp in out_dir; returns the written path."""
    stem, ext = os.path.splitext(os.path.basename(path))
    out = os.path.join(out_dir, stem + UNPACKED_EXT[ext.lower()])
    with open(path, 'rb') as f:
        data = unpack_bytes(f.read(), ext)
    with open(out, 'wb') as f:
        f.write(data)
    return out


def pack(path, out_dir):
    """jpg/tga/bmp -> OZJ/OZT/OZB in out_dir; returns the written path."""
    stem, ext = os.path.splitext(os.path.basename(path))
    packed_ext = PACKED_EXT[ext.lower()]
    out = os.path.join(out_dir, stem + packed_ext)
    with open(path, 'rb') as f:
        data = pack_bytes(f.read(), packed_ext)
    with open(out, 'wb') as f:
        f.write(data)
    return out


def write_tga(path, width, height, rgba_bottom_up):
    """Uncompressed 32-bit TGA the client can read; rgba_bottom_up = bytes, rows from the bottom row up."""
    header = _TGA_HEADER.pack(0, 0, _TGA_TRUECOLOR, 0, 0, 0, 0, 0, width, height, _TGA_BPP, _TGA_ALPHA_BITS)
    bgra = bytearray(rgba_bottom_up)
    bgra[0::4], bgra[2::4] = rgba_bottom_up[2::4], rgba_bottom_up[0::4]
    with open(path, 'wb') as f:
        f.write(header + bytes(bgra))


def find_texture(name, folder):
    """The file the client would load for a texture name of a mesh ("golem.jpg" -> golem.OZJ), any case."""
    stem, ext = os.path.splitext(name)
    wanted = {(stem + PACKED_EXT.get(ext.lower(), ext)).lower(), name.lower()}
    try:
        entries = os.listdir(folder)
    except OSError:
        return None
    for entry in entries:
        if entry.lower() in wanted:
            return os.path.join(folder, entry)
    return None


if __name__ == '__main__':
    if len(sys.argv) != 4 or sys.argv[1] not in ('pack', 'unpack'):
        sys.exit(__doc__)
    print((pack if sys.argv[1] == 'pack' else unpack)(sys.argv[2], sys.argv[3]))

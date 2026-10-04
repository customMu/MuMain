"""Reads and writes the OZT images of the MU client (4 bytes prefix + uncompressed 32-bit TGA) and PNG previews."""
import struct
import zlib


def read_ozt(path):
    data = open(path, 'rb').read()[4:]
    id_length, _, image_type = data[0], data[1], data[2]
    width, height, bpp, descriptor = struct.unpack_from('<HHBB', data, 12)
    assert image_type == 2 and bpp == 32, (image_type, bpp)
    pixels = data[18 + id_length:]
    rows = []
    for y in range(height):
        row = pixels[y * width * 4:(y + 1) * width * 4]
        rows.append(row)
    if not descriptor & 0x20:
        rows.reverse()  # bottom-up -> top-down
    # BGRA -> RGBA
    out = []
    for row in rows:
        r = bytearray(row)
        r[0::4], r[2::4] = row[2::4], row[0::4]
        out.append(bytes(r))
    return width, height, out


def write_png(path, width, height, rows_rgba):
    raw = b''.join(b'\x00' + row for row in rows_rgba)

    def chunk(tag, payload):
        return struct.pack('>I', len(payload)) + tag + payload + struct.pack('>I', zlib.crc32(tag + payload) & 0xFFFFFFFF)

    png = b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', struct.pack('>IIBBBBB', width, height, 8, 6, 0, 0, 0)) + chunk(b'IDAT', zlib.compress(raw, 9)) + chunk(b'IEND', b'')
    open(path, 'wb').write(png)


def write_ozt(path, width, height, rows_rgba, prefix=b'\x00\x00\x02\x00'):
    header = struct.pack('<BBBHHBHHHHBB', 0, 0, 2, 0, 0, 0, 0, 0, width, height, 32, 8)
    body = bytearray()
    for row in reversed(rows_rgba):  # bottom-up like the existing files
        r = bytearray(row)
        r[0::4], r[2::4] = row[2::4], row[0::4]
        body += r
    open(path, 'wb').write(prefix + header + bytes(body))


if __name__ == '__main__':
    import sys
    w, h, rows = read_ozt(sys.argv[1])
    print(w, h)
    raw = open(sys.argv[1], 'rb').read()
    print('prefix', raw[:4].hex(), 'header', raw[4:22].hex(), 'size', len(raw))
    # downscale x4 for preview
    small = [bytes(b''.join(rows[y][x * 4:x * 4 + 4] for x in range(0, w, 4))) for y in range(0, h, 4)]
    write_png(sys.argv[2], w // 4, h // 4, small)

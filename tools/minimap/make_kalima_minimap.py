"""Draws the minimap of Kalima (World25) from the walk map of the server, in the style of the game:
a blue floor, rocky borders and pillars, transparent outside. 1024x1024, 4 pixels per field; the image row
is the map X and the column the map Y, like the client positions the hero on the minimap."""
import math
import sys
from ozt import write_ozt, write_png

SIZE = 256
SCALE = 4
data = bytes.fromhex(open(sys.argv[1]).read().strip())[3:]
walk = [[0.0] * SIZE for _ in range(SIZE)]  # walk[x][y]
for i, v in enumerate(data[:SIZE * SIZE]):
    walk[i & 0xFF][(i >> 8) & 0xFF] = 1.0 if v in (0, 1) else 0.0

# Only the largest connected walkable area: stray walkable fields at the map edge can't be reached.
seen = [[False] * SIZE for _ in range(SIZE)]
best = []
for sx in range(SIZE):
    for sy in range(SIZE):
        if walk[sx][sy] and not seen[sx][sy]:
            area, stack = [], [(sx, sy)]
            seen[sx][sy] = True
            while stack:
                x, y = stack.pop()
                area.append((x, y))
                for nx, ny in ((x - 1, y), (x + 1, y), (x, y - 1), (x, y + 1)):
                    if 0 <= nx < SIZE and 0 <= ny < SIZE and walk[nx][ny] and not seen[nx][ny]:
                        seen[nx][ny] = True
                        stack.append((nx, ny))
            if len(area) > len(best):
                best = area
walk = [[0.0] * SIZE for _ in range(SIZE)]
for x, y in best:
    walk[x][y] = 1.0

# Distance (in fields) of each non-walkable field to the next walkable one, up to 3: the rocky border.
INF = 99
dist = [[0 if walk[x][y] else INF for y in range(SIZE)] for x in range(SIZE)]
for _ in range(3):
    nxt = [row[:] for row in dist]
    for x in range(SIZE):
        for y in range(SIZE):
            if dist[x][y] == INF:
                best = min(dist[nx][ny] for nx, ny in ((x - 1, y), (x + 1, y), (x, y - 1), (x, y + 1)) if 0 <= nx < SIZE and 0 <= ny < SIZE)
                if best < INF:
                    nxt[x][y] = best + 1
    dist = nxt
rock = [[1.0 if 0 < dist[x][y] <= 3 else 0.0 for y in range(SIZE)] for x in range(SIZE)]


def sample(grid, fx, fy):
    """Bilinear sample of a field grid at a fractional field position."""
    x0, y0 = int(math.floor(fx)), int(math.floor(fy))
    tx, ty = fx - x0, fy - y0

    def g(x, y):
        return grid[x][y] if 0 <= x < SIZE and 0 <= y < SIZE else 0.0

    return (g(x0, y0) * (1 - tx) * (1 - ty) + g(x0 + 1, y0) * tx * (1 - ty)
            + g(x0, y0 + 1) * (1 - tx) * ty + g(x0 + 1, y0 + 1) * tx * ty)


def noise(x, y, seed):
    n = (x * 374761393 + y * 668265263 + seed * 2147483647) & 0xFFFFFFFF
    n = (n ^ (n >> 13)) * 1274126177 & 0xFFFFFFFF
    return ((n ^ (n >> 16)) & 0xFFFF) / 65535.0


def smooth_noise(fx, fy, cell, seed):
    x0, y0 = int(fx // cell), int(fy // cell)
    tx, ty = (fx / cell) - x0, (fy / cell) - y0
    a, b = noise(x0, y0, seed), noise(x0 + 1, y0, seed)
    c, d = noise(x0, y0 + 1, seed), noise(x0 + 1, y0 + 1, seed)
    return (a * (1 - tx) + b * tx) * (1 - ty) + (c * (1 - tx) + d * tx) * ty


rows = []
pixels = SIZE * SCALE
for py in range(pixels):  # image row = map X
    row = bytearray()
    fx = (py + 0.5) / SCALE - 0.5
    for px in range(pixels):  # image column = map Y
        fy = (px + 0.5) / SCALE - 0.5
        w = sample(walk, fx, fy)
        r = sample(rock, fx, fy)
        grain = smooth_noise(py, px, 6, 1) * 0.6 + noise(py, px, 2) * 0.4
        if w > 0.5:
            # Blue floor with tiles and a light rim along the walls.
            edge = max(0.0, min(1.0, (0.85 - w) / 0.35))
            tile = 0.06 if ((py // 16) + (px // 16)) % 2 == 0 else 0.0
            base = 0.75 + grain * 0.35 + tile
            red, green, blue = 38 * base, 68 * base, 118 * base
            red += 60 * edge
            green += 110 * edge
            blue += 120 * edge
            row += bytes((min(255, int(red)), min(255, int(green)), min(255, int(blue)), 255))
        elif r > 0.35 or w > 0.15:
            # Rocks: grey-brown, darker to the outside.
            shade = 0.55 + grain * 0.6 - (0.25 if r < 0.6 else 0.0)
            red, green, blue = 120 * shade, 112 * shade, 100 * shade
            alpha = 255 if r > 0.5 or w > 0.15 else int(255 * (r - 0.35) / 0.15)
            row += bytes((min(255, int(red)), min(255, int(green)), min(255, int(blue)), max(0, min(255, alpha))))
        else:
            row += b'\x00\x00\x00\x00'
    rows.append(bytes(row))

write_ozt(sys.argv[2], pixels, pixels, rows)
preview = [bytes(b''.join(rows[y][x * 4:x * 4 + 4] for x in range(0, pixels, 2))) for y in range(0, pixels, 2)]
write_png(sys.argv[3], pixels // 2, pixels // 2, preview)
print('ok')

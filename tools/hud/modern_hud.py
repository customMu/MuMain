"""Textures of the modern bottom HUD (the main frame: potion and skill slots, HP / SD / AG / mana, experience, menu buttons).

    python tools/hud/modern_hud.py                      -> src/bin/Data/Interface/Modern/*.OZJ / *.OZT
    python tools/hud/modern_hud.py --preview shot.png   -> also preview.png: the HUD over a game screenshot
    python tools/hud/modern_hud.py --skip-render        -> reuses the renders of the last run (work folder)

A forged-steel band with gold trims, sunk slots, HP and mana as glass orbs of glowing liquid with gold rings, SD and AG as
glass tubes, a gold experience bar and steel medallions with engraved gold icons for the menu buttons.

How: this script draws the maps of the band (height, gold, slot insides) and the icons, Blender renders the parts with
Cycles (render_hud.py), and this script cuts the renders into the textures of the client. The band, gauge, experience and
button textures keep the size and the layout of the classic ones (newui_menu01/02/03, the gauges, the buttons), so the
client draws them with the same code; CNewUIMainFrameWindow::LoadImages picks the folder (`$hud modern` / `$hud classic`).
The glass of the orbs and tubes (orb_glass, tube_glass) is drawn over the liquid by the modern HUD only, at a higher
resolution. Coordinates are the 640 x 480 reference of the client (NewUIMainFrameWindow.cpp).
"""
import argparse
import os
import subprocess
import sys

import numpy
from PIL import Image, ImageChops, ImageDraw, ImageEnhance, ImageFilter, ImageFont

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', 'bmd'))
import ozj  # noqa: E402

REPO = os.path.abspath(os.path.join(HERE, '..', '..'))
OUT_DIR = os.path.join(REPO, 'src', 'bin', 'Data', 'Interface', 'Modern')
WORK_DIR = os.path.join(REPO, 'out', 'hud')
BLENDER = os.environ.get('BLENDER', r'C:\Program Files\Blender Foundation\Blender 5.2\blender.exe')
FONT = 'C:/Windows/Fonts/segoeuib.ttf'
SS = 4  # the band is rendered SS times larger than the reference pixels (render_hud.SS)

HUD_TOP = 429  # the band: y 429..480, 51 high; the experience bar is its last 10 rows
HUD_HEIGHT = 51
WIDTH = 640

# rectangles in reference pixels: x0, y0, x1, y1 (exclusive)
POTION_SLOTS = [(3 + 38 * i, 431, 37 + 38 * i, 468) for i in range(4)]
SKILL_SLOTS = [(223 + 32 * i, 431, 253 + 32 * i, 469) for i in range(5)]
CURRENT_SKILL = (386, 432, 418, 469)
HP_GAUGE = (158, 432, 203, 471)  # the fill texture: 45 x 39, the liquid a circle of ORB_RADIUS in it
SD_GAUGE = (204, 431, 220, 470)  # 16 x 39
AG_GAUGE = (420, 431, 436, 470)
MANA_GAUGE = (437, 432, 482, 471)
EXP_TRACK = (2, 473, 631, 477)
BUTTONS_X, BUTTON_W, BUTTON_H = 489, 30, 41
ORB_RADIUS = 19.5
ORB_OVERLAY = 56  # the ring overlay of an orb with its shadow, reference pixels (NewUIMainFrameWindow.cpp kOrbGlassSize)
TUBE_OVERLAY = (26, 48)  # kTubeGlassWidth / Height
MEDALLION = 26  # diameter of a button medallion
MEDALLION_DRAWN = MEDALLION * 2.1 / 2  # a cell of the medallion atlas covers the coin render: 2.1 units for the coin's 2 (kMedallionSize)
ICONS = ('shop', 'character', 'inventory', 'friend', 'menu')
BUTTONS = (('Bt05', 'shop'), ('Bt01', 'character'), ('Bt02', 'inventory'), ('Bt03', 'friend'), ('Bt04', 'menu'))


def s(v):
    return int(round(v * SS))


# ---------- maps for the render ----------

def shape(size, draw):
    img = Image.new('L', size, 0)
    draw(ImageDraw.Draw(img))
    return img


def band_maps(work):
    """band_height (white = high), band_gold (gold metal), band_inner (dark glassy inside of the slots), SS x the band."""
    size = (s(WIDTH), s(HUD_HEIGHT))
    height = Image.new('L', size, 150)
    gold = Image.new('L', size, 0)
    inner = Image.new('L', size, 0)

    def rect(r, inset=0.0):
        return (s(r[0] + inset), s(r[1] - HUD_TOP + inset), s(r[2] - inset) - 1, s(r[3] - HUD_TOP - inset) - 1)

    def sunk(r, radius, rim=True):
        hole = shape(size, lambda d: d.rounded_rectangle(rect(r), radius=s(radius), fill=255))
        depth = hole.filter(ImageFilter.GaussianBlur(s(0.6)))
        height.paste(ImageChops.subtract(height, depth.point(lambda v: v * 0.55)))
        inner.paste(ImageChops.lighter(inner, shape(size, lambda d: d.rounded_rectangle(rect(r, 0.8), radius=s(max(radius - 0.8, 0.5)), fill=255))))
        if rim:
            ring = shape(size, lambda d: d.rounded_rectangle(rect(r, -0.3), radius=s(radius + 0.3), outline=255, width=s(1.1)))
            gold.paste(ImageChops.lighter(gold, ring))
            height.paste(ImageChops.lighter(height, ring.point(lambda v: v * 0.85)))

    def sunk_circle(cx, cy, r):
        box = (s(cx - r), s(cy - HUD_TOP - r), s(cx + r), s(cy - HUD_TOP + r))
        hole = shape(size, lambda d: d.ellipse(box, fill=255))
        height.paste(ImageChops.subtract(height, hole.filter(ImageFilter.GaussianBlur(s(0.6))).point(lambda v: v * 0.6)))
        inner.paste(ImageChops.lighter(inner, hole))

    # the top trim and the trim above the experience bar: raised gold
    for y0, y1 in ((0, 1.8), (40.6, 41.8)):
        band = shape(size, lambda d: d.rectangle((0, s(y0), size[0], s(y1)), fill=255))
        gold.paste(ImageChops.lighter(gold, band))
        height.paste(ImageChops.lighter(height, band.point(lambda v: v * 0.95)))
    # grooves between the sections
    for xd in (152.5, 221.5, 383.5, 419.0, 485.5):
        groove = shape(size, lambda d: d.rectangle((s(xd - 0.6), s(2), s(xd + 0.6), s(40.5)), fill=255))
        height.paste(ImageChops.subtract(height, groove.filter(ImageFilter.GaussianBlur(s(0.4))).point(lambda v: v * 0.4)))
    for r in POTION_SLOTS + SKILL_SLOTS:
        sunk(r, 3.5)
    sunk(CURRENT_SKILL, 3.5)
    sunk(SD_GAUGE, 7.5)
    sunk(AG_GAUGE, 7.5)
    for r in (HP_GAUGE, MANA_GAUGE):
        sunk_circle((r[0] + r[2]) / 2, (r[1] + r[3]) / 2, ORB_RADIUS + 0.5)
    # the experience groove with gold studs every tenth
    x0, y0, x1, y1 = EXP_TRACK
    sunk((x0 - 0.5, y0 - 0.8, x1 + 0.5, y1 + 0.8), 2.0)
    for k in list(range(1, 10)):
        cx = x0 + (x1 - x0) * k / 10
        stud = shape(size, lambda d: d.regular_polygon((s(cx), s((y0 + y1) / 2 - HUD_TOP), s(1.6)), 4, fill=255))
        gold.paste(ImageChops.lighter(gold, stud))
        height.paste(ImageChops.lighter(height, stud))
        inner.paste(ImageChops.subtract(inner, stud))
    # buttons sit on a darker plate: a shallow recess
    plate = shape(size, lambda d: d.rounded_rectangle((s(487), s(2.5), s(639.5), s(40)), radius=s(3), fill=255))
    height.paste(ImageChops.subtract(height, plate.filter(ImageFilter.GaussianBlur(s(0.8))).point(lambda v: v * 0.2)))
    height = height.filter(ImageFilter.GaussianBlur(s(0.25)))
    height.save(os.path.join(work, 'band_height.png'))
    gold.filter(ImageFilter.GaussianBlur(s(0.15))).save(os.path.join(work, 'band_gold.png'))
    inner.filter(ImageFilter.GaussianBlur(s(0.2))).save(os.path.join(work, 'band_inner.png'))


def icon(d, kind, cx, cy, k):
    """Filled icons, white = raised gold, black lines cut into them; a 16 x 16 box around cx, cy, k pixels per unit.
    shop: a cut gem (the item shop), character: a knight's helmet, inventory: a bag, friend: two people, menu: a gear."""
    def p(x, y):
        return cx + x * k, cy + y * k

    def cut(points, w=0.8):
        d.line([p(*q) for q in points], fill=0, width=max(1, int(w * k)), joint='curve')

    if kind == 'shop':
        d.polygon([p(-4.2, -5.5), p(4.2, -5.5), p(7.5, -1.5), p(0, 7.5), p(-7.5, -1.5)], fill=255)
        cut([(-7.5, -1.5), (7.5, -1.5)])
        cut([(-4.2, -5.5), (-2.2, -1.5), (0, 7.5)])
        cut([(4.2, -5.5), (2.2, -1.5), (0, 7.5)])
        cut([(-2.2, -1.5), (0, -5.5), (2.2, -1.5)])
    elif kind == 'character':
        d.chord((*p(-6.2, -7.5), *p(6.2, 5)), 180, 360, fill=255)
        d.rounded_rectangle((*p(-6.2, -1.4), *p(6.2, 7)), radius=1.6 * k, fill=255)
        d.polygon([p(-6.2, 4), p(-2.2, 7.2), p(-6.2, 7.2)], fill=0)
        d.polygon([p(6.2, 4), p(2.2, 7.2), p(6.2, 7.2)], fill=0)
        d.rectangle((*p(-5, -0.6), *p(5, 0.9)), fill=0)  # the eye slit
        d.rectangle((*p(-0.7, 0.9), *p(0.7, 5.2)), fill=0)  # the nose guard gap
        cut([(0, -7.2), (0, -2.2)], 0.7)  # the crest
    elif kind == 'inventory':
        d.ellipse((*p(-3.6, -7.6), *p(3.6, -1.2)), outline=255, width=int(1.7 * k))  # the handle
        d.rounded_rectangle((*p(-7, -3), *p(7, 7.5)), radius=3 * k, fill=255)
        cut([(-7, 0.6), (7, 0.6)], 0.9)  # the flap
        d.rounded_rectangle((*p(-1.6, -0.3), *p(1.6, 2.6)), radius=0.6 * k, fill=0)  # the buckle
        d.rounded_rectangle((*p(-0.8, 0.5), *p(0.8, 1.8)), radius=0.3 * k, fill=255)
    elif kind == 'friend':
        d.ellipse((*p(1.2, -6.8), *p(6.2, -1.8)), fill=255)  # the one behind
        d.chord((*p(-0.6, -0.2), *p(8, 13)), 180, 360, fill=255)
        d.ellipse((*p(-6.6, -6.2), *p(0.6, 1.0)), fill=0)  # a gap around the one in front
        d.chord((*p(-9.4, 0.2), *p(3.4, 15.8)), 180, 360, fill=0)
        d.ellipse((*p(-5.8, -5.4), *p(-0.2, 0.2)), fill=255)  # the one in front
        d.chord((*p(-8.6, 1.4), *p(2.6, 14.6)), 180, 360, fill=255)
        d.rectangle((*p(-9.5, 7.5), *p(9.5, 9.5)), fill=0)
    elif kind == 'menu':
        teeth = []
        for i in range(16):
            a = numpy.radians(i * 22.5 + 11.25)
            r = 7.4 if i % 2 == 0 else 5.4
            for da in (-7, 7):
                b = a + numpy.radians(da)
                teeth.append(p(r * numpy.cos(b), r * numpy.sin(b)))
        d.polygon(teeth, fill=255)
        d.ellipse((*p(-2.6, -2.6), *p(2.6, 2.6)), fill=0)


def icon_maps(work):
    """The icons on the coin: 512 px for the coin's diameter of 2 units (MEDALLION reference pixels)."""
    n = 512
    k = n / MEDALLION
    for kind in ICONS:
        img = Image.new('L', (n, n), 0)
        icon(ImageDraw.Draw(img), kind, n / 2, n / 2 + 0.3 * k, k)
        img.filter(ImageFilter.GaussianBlur(3)).save(os.path.join(work, f'icon_{kind}.png'))


def run_blender(work):
    script = os.path.join(HERE, 'render_hud.py')
    subprocess.run([BLENDER, '-b', '--factory-startup', '-P', script, '--', work], check=True,
                   stdout=subprocess.DEVNULL)


# ---------- the textures ----------

def label(img, xy, text, size=6.5, color=(232, 200, 128)):
    d = ImageDraw.Draw(img)
    f = ImageFont.truetype(FONT, s(size))
    d.text((s(xy[0]) + s(0.5), s(xy[1]) + s(0.6)), text, font=f, fill=(0, 0, 0))
    d.text((s(xy[0]), s(xy[1])), text, font=f, fill=color)


def down(img, size):
    return img.resize(size, Image.LANCZOS)


def band_textures(band):
    full = band.copy()
    for i, r in enumerate(POTION_SLOTS):
        label(full, (r[0] + 2.5, r[1] - HUD_TOP + 0.5), 'QWER'[i])
    for i, r in enumerate(SKILL_SLOTS):
        label(full, (r[0] + 2.5, r[1] - HUD_TOP + 0.5), str(i + 1))
    full = down(full, (WIDTH, HUD_HEIGHT))
    overlay = band.crop((s(222), 0, s(382), s(42)))
    for i, r in enumerate(SKILL_SLOTS):
        label(overlay, (r[0] - 222 + 2.5, r[1] - HUD_TOP + 0.5), str((i + 6) % 10), color=(255, 230, 160))
    return {
        'newui_menu01': full.crop((0, 0, 256, HUD_HEIGHT)),
        'newui_menu02': full.crop((256, 0, 384, HUD_HEIGHT)),
        'newui_menu03': full.crop((384, 0, 640, HUD_HEIGHT)),
        'newui_menu02-03': down(overlay, (160, 42)),
    }


def fill(band, rect, part, part_size):
    """The fill texture of a gauge: the band under it with the liquid in it. The client cuts it from the top as the
    value drops, so the level of the liquid sinks."""
    x0, y0, x1, y1 = rect
    bg = band.crop((s(x0), s(y0 - HUD_TOP), s(x1), s(y1 - HUD_TOP))).convert('RGBA')
    liquid = part.resize((s(part_size[0]), s(part_size[1])), Image.LANCZOS)
    bg.alpha_composite(liquid, ((bg.width - liquid.width) // 2, (bg.height - liquid.height) // 2))
    return down(bg.convert('RGB'), (x1 - x0, y1 - y0))


def exp_fill(colors):
    light, mid, dark = (numpy.array(c, dtype=numpy.float64) for c in colors)
    rows = numpy.array([dark * 0.9 + mid * 0.1, light, (light + mid) / 2, mid * 0.75])
    return Image.fromarray(numpy.clip(numpy.repeat(rows[:, None, :], 6, axis=1), 0, 255).astype(numpy.uint8), 'RGB')


BUTTON_STATES = (  # brightness, glow, press offset: up, over, down, active (41 rows each)
    (1.0, 0.0, 0),
    (1.3, 0.8, 0),
    (0.7, 0.0, 1),
    (1.15, 0.45, 0),
)


def button(band, coin, index):
    x_ref = BUTTONS_X + BUTTON_W * index
    bg = band.crop((s(x_ref), 0, s(x_ref + BUTTON_W), s(BUTTON_H))).convert('RGBA')
    coin = coin.resize((s(MEDALLION + 1), s(MEDALLION + 1)), Image.LANCZOS)
    strip = Image.new('RGB', (s(BUTTON_W), s(BUTTON_H * 4)))
    for k, (bright, glow, offset) in enumerate(BUTTON_STATES):
        img = bg.copy()
        pos = ((img.width - coin.width) // 2, s(21.3 + offset) - coin.height // 2)
        alpha = coin.getchannel('A')
        shadow = Image.new('RGBA', coin.size, (0, 0, 0, 0))
        shadow.putalpha(alpha.point(lambda v: v * 0.7).filter(ImageFilter.GaussianBlur(s(1.2))))
        img.alpha_composite(shadow, (pos[0] + s(0.6), pos[1] + s(1.0)))
        if glow:
            halo = Image.new('RGBA', coin.size, (255, 214, 130, 0))
            halo.putalpha(alpha.filter(ImageFilter.GaussianBlur(s(2.2))).point(lambda v: v * glow))
            img.alpha_composite(halo, pos)
        c = ImageEnhance.Brightness(coin.convert('RGB')).enhance(bright).convert('RGBA')
        c.putalpha(alpha)
        img.alpha_composite(c, pos)
        strip.paste(img.convert('RGB'), (0, s(BUTTON_H * k)))
    return down(strip, (BUTTON_W, BUTTON_H * 4))


def with_shadow(img, reference_height):
    """A soft shadow under an overlay, down and to the right (the key light of the band is at the upper left)."""
    px = img.height / reference_height
    alpha = img.getchannel('A')
    shadow = Image.new('RGBA', img.size, (0, 0, 0, 0))
    shadow.putalpha(alpha.filter(ImageFilter.GaussianBlur(px * 1.4)).point(lambda v: int(v * 0.75)))
    out = Image.new('RGBA', img.size, (0, 0, 0, 0))
    out.alpha_composite(shadow, (int(px * 1.0), int(px * 1.6)))
    out.alpha_composite(img)
    return out


def medallion_atlas(work):
    """The medallions in the order of BUTTONS, 128 texels each, drawn by the modern HUD over the buttons."""
    atlas = Image.new('RGBA', (1024, 128), (0, 0, 0, 0))
    for i, (_, kind) in enumerate(BUTTONS):
        atlas.alpha_composite(down(Image.open(os.path.join(work, f'medallion_{kind}.png')).convert('RGBA'), (128, 128)), (i * 128, 0))
    return atlas


def build(work):
    band = Image.open(os.path.join(work, 'band.png')).convert('RGB')
    textures = band_textures(band)
    orb = {c: Image.open(os.path.join(work, f'orb_{c}.png')) for c in ('red', 'green', 'blue')}
    tube = {c: Image.open(os.path.join(work, f'tube_{c}.png')) for c in ('sd', 'ag')}
    d = 2 * ORB_RADIUS
    textures.update({
        'newui_menu_red': fill(band, HP_GAUGE, orb['red'], (d, d)),
        'newui_menu_green': fill(band, HP_GAUGE, orb['green'], (d, d)),
        'newui_menu_blue': fill(band, MANA_GAUGE, orb['blue'], (d, d)),
        'newui_menu_sd': fill(band, SD_GAUGE, tube['sd'], (15.2, 39)),
        'newui_menu_ag': fill(band, AG_GAUGE, tube['ag'], (15.2, 39)),
        'newui_exbar': exp_fill(((255, 236, 170), (232, 166, 40), (90, 50, 6))),
        'Exbar_Master': exp_fill(((236, 200, 255), (150, 80, 236), (50, 16, 90))),
    })
    for i, (name, kind) in enumerate(BUTTONS):
        textures['newui_menu_' + name] = button(band, Image.open(os.path.join(work, f'medallion_{kind}.png')), i)
    overlays = {
        'orb_glass': down(with_shadow(Image.open(os.path.join(work, 'orb_glass.png')).convert('RGBA'), ORB_OVERLAY), (128, 128)),
        'tube_glass': down(with_shadow(Image.open(os.path.join(work, 'tube_glass.png')).convert('RGBA'), TUBE_OVERLAY[1]), (64, 128)),
        'medallions': medallion_atlas(work),
    }
    return textures, overlays


def save(textures, overlays):
    os.makedirs(OUT_DIR, exist_ok=True)
    for name, img in textures.items():
        path = os.path.join(OUT_DIR, name + '.jpg')
        img.save(path, quality=95, subsampling=0)
        ozj.pack(path, OUT_DIR)
        os.remove(path)
    for name, img in overlays.items():
        path = os.path.join(OUT_DIR, name + '.tga')
        # uncompressed 32-bit, rows from the bottom up: OpenTga reads only that
        ozj.write_tga(path, img.width, img.height, img.transpose(Image.FLIP_TOP_BOTTOM).tobytes())
        ozj.pack(path, OUT_DIR)
        os.remove(path)
    print(f'{len(textures) + len(overlays)} textures -> {OUT_DIR}')


def preview(textures, overlays, shot, out_png, values=(0.7, 0.45, 0.9, 0.6, 0.37)):
    """The HUD as the client composes it, over a game screenshot (any 4:3 size), with sample values."""
    base = Image.open(shot).convert('RGBA')
    k = base.width / WIDTH
    off = 6  # room above the band for the orb rings
    q = 4  # drawn 4 x, then scaled to the screenshot
    hud = Image.new('RGBA', (WIDTH * q, (HUD_HEIGHT + off) * q))

    def put(img, x, y):
        img = img.convert('RGBA')
        hud.alpha_composite(img.resize((img.width * q, img.height * q), Image.LANCZOS), (int(x * q), int(y * q)))

    for name, x in (('newui_menu01', 0), ('newui_menu02', 256), ('newui_menu03', 384)):
        put(textures[name], x, off)
    hp, mana, sd, ag, exp = values
    for rect, name, value, glass in ((HP_GAUGE, 'newui_menu_red', hp, 'orb'), (MANA_GAUGE, 'newui_menu_blue', mana, 'orb'),
                                     (SD_GAUGE, 'newui_menu_sd', sd, 'tube'), (AG_GAUGE, 'newui_menu_ag', ag, 'tube')):
        x0, y0, x1, y1 = rect
        f = textures[name]
        cut = int(round(f.height * (1 - value)))
        put(f.crop((0, cut, f.width, f.height)), x0, y0 - HUD_TOP + cut + off)
        cx, cy = (x0 + x1) / 2, (y0 + y1) / 2 - HUD_TOP + off
        gw, gh = (ORB_OVERLAY, ORB_OVERLAY) if glass == 'orb' else TUBE_OVERLAY
        g = overlays[glass + '_glass'].resize((gw * q, gh * q), Image.LANCZOS)
        hud.alpha_composite(g, (int((cx - gw / 2) * q), int((cy - gh / 2) * q)))
    x0, y0, x1, y1 = EXP_TRACK
    put(textures['newui_exbar'].resize((int((x1 - x0) * exp), y1 - y0)), x0, y0 - HUD_TOP + off)
    for i, (name, _) in enumerate(BUTTONS):
        state = 1 if i == 2 else 0
        put(textures['newui_menu_' + name].crop((0, BUTTON_H * state, BUTTON_W, BUTTON_H * (state + 1))), BUTTONS_X + BUTTON_W * i, off)
        cell = overlays['medallions'].crop((i * 128, 0, i * 128 + 128, 128))
        size = int(MEDALLION_DRAWN * q)
        hud.alpha_composite(cell.resize((size, size), Image.LANCZOS),
                            (int((BUTTONS_X + BUTTON_W * i + BUTTON_W / 2 - MEDALLION_DRAWN / 2) * q), int((off + 21.3 - MEDALLION_DRAWN / 2) * q)))
    hud = hud.resize((base.width, int((HUD_HEIGHT + off) * k)), Image.LANCZOS)
    base.alpha_composite(hud, (0, base.height - hud.height))
    base.convert('RGB').save(out_png)
    print(f'preview -> {out_png}')


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--preview', metavar='SCREENSHOT')
    parser.add_argument('--preview-out', default='preview.png')
    parser.add_argument('--skip-render', action='store_true')
    parser.add_argument('--work', default=WORK_DIR)
    args = parser.parse_args()
    os.makedirs(args.work, exist_ok=True)
    if not args.skip_render:
        band_maps(args.work)
        icon_maps(args.work)
        run_blender(args.work)
    textures, overlays = build(args.work)
    save(textures, overlays)
    if args.preview:
        preview(textures, overlays, args.preview, args.preview_out)


if __name__ == '__main__':
    main()

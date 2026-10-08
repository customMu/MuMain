"""Textures of the windows in the modern style of the HUD (party, quests, menu, inventory, character, message boxes, ...).

    python tools/hud/modern_ui.py                       -> src/bin/Data/Interface/ModernUI/*.OZJ / *.OZT
    python tools/hud/modern_ui.py --preview window.png  -> also a sample window and the buttons
    python tools/hud/modern_ui.py --skip-render         -> reuses the render of the last run

The windows of the client are drawn from a few shared textures (the frame: back, top, sides, bottom; the buttons, the
close button, lines, scroll thumbs, table corners). With the modern windows on, the client loads a texture of
Data\\Interface\\ModernUI\\ instead of the classic one of the same name (UI/Theme/ModernTheme.h), so this script writes
them with the classic names, sizes and layouts (the windows keep their code).

How: the frame parts and buttons are drawn as maps (height, gold, dark inside, shape) into one atlas, Blender renders it
with the material of the HUD band (render_hud.py ... ui), and this script cuts the parts out, gives them their shape
(alpha) and the states of the buttons. The thin parts (table corners, scroll track) take the shape of the classic ones,
recoloured in gold.
"""
import argparse
import os
import subprocess
import sys

from PIL import Image, ImageChops, ImageDraw, ImageEnhance, ImageFilter

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', 'bmd'))
import ozj  # noqa: E402

REPO = os.path.abspath(os.path.join(HERE, '..', '..'))
CLASSIC_DIR = os.path.join(REPO, 'src', 'bin', 'Data', 'Interface')
OUT_DIR = os.path.join(CLASSIC_DIR, 'ModernUI')
WORK_DIR = os.path.join(REPO, 'out', 'hud')
BLENDER = os.environ.get('BLENDER', r'C:\Program Files\Blender Foundation\Blender 5.2\blender.exe')
SS = 3  # render_hud.UI_SS
ATLAS = (560, 520)  # render_hud.UI_ATLAS

# the parts in the atlas: name -> (x, y, width, height) in reference pixels (= texels of the game files)
PARTS = {
    'back': (0, 0, 225, 512),
    'top': (235, 0, 190, 64),
    'bottom': (235, 70, 190, 45),
    'rail_l': (505, 0, 21, 320),
    'rail_r': (535, 0, 21, 320),
    'msg_top': (235, 120, 230, 67),
    'msg_middle': (235, 192, 230, 15),
    'msg_bottom': (235, 212, 230, 50),
    'btn': (235, 270, 108, 29),
    'btn_big': (235, 380, 180, 29),
    'btn_small': (350, 270, 64, 29),
    'btn_very_small': (420, 270, 54, 23),
    'exit': (235, 310, 36, 29),
    'line': (235, 350, 188, 21),
    'thumb': (480, 330, 15, 30),
}

BASE, RAISED, GOLD_UP, SUNK = 150, 205, 235, 70  # heights (band_height convention: 150 = the panel surface)


def s(v):
    return int(round(v * SS))


class Maps:
    """The four maps of the atlas, drawn in reference pixels."""

    def __init__(self):
        size = (s(ATLAS[0]), s(ATLAS[1]))
        self.height = Image.new('L', size, BASE)
        self.gold = Image.new('L', size, 0)
        self.inner = Image.new('L', size, 0)
        self.alpha = Image.new('L', size, 0)

    def box(self, r):
        x0, y0, x1, y1 = r
        return (s(x0), s(y0), s(x1) - 1, s(y1) - 1)

    def draw(self, layer, fn):
        img = Image.new('L', self.height.size, 0)
        fn(ImageDraw.Draw(img))
        return img

    def put(self, r, height=None, gold=False, inner=False, alpha=True, radius=0.0, blur=0.0):
        mask = self.draw(None, lambda d: d.rounded_rectangle(self.box(r), radius=s(radius), fill=255))
        self.apply(mask, height, gold, inner, alpha, blur)

    def put_polygon(self, points, height=None, gold=False, inner=False, alpha=True, blur=0.0):
        mask = self.draw(None, lambda d: d.polygon([(s(x), s(y)) for x, y in points], fill=255))
        self.apply(mask, height, gold, inner, alpha, blur)

    def put_line(self, points, width, height=None, gold=True, alpha=True):
        mask = self.draw(None, lambda d: d.line([(s(x), s(y)) for x, y in points], fill=255, width=max(1, s(width))))
        self.apply(mask, height, gold, False, alpha, 0.0)

    def put_ellipse(self, r, height=None, gold=True, inner=False, alpha=True):
        mask = self.draw(None, lambda d: d.ellipse(self.box(r), fill=255))
        self.apply(mask, height, gold, inner, alpha, 0.0)

    def apply(self, mask, height, gold, inner, alpha, blur):
        soft = mask.filter(ImageFilter.GaussianBlur(s(blur))) if blur else mask
        if height is not None:
            self.height.paste(Image.new('L', mask.size, height), (0, 0), soft)
        if gold:
            self.gold.paste(255, (0, 0), mask)
        else:
            self.gold.paste(0, (0, 0), mask)
        if inner:
            self.inner.paste(255, (0, 0), mask)
        else:
            self.inner.paste(0, (0, 0), mask)
        if alpha:
            self.alpha.paste(255, (0, 0), mask)

    def save(self, work):
        self.height.filter(ImageFilter.GaussianBlur(s(0.35))).save(os.path.join(work, 'ui_height.png'))
        self.gold.filter(ImageFilter.GaussianBlur(s(0.15))).save(os.path.join(work, 'ui_gold.png'))
        self.inner.filter(ImageFilter.GaussianBlur(s(0.2))).save(os.path.join(work, 'ui_inner.png'))
        self.alpha.save(os.path.join(work, 'ui_alpha.png'))


# ---------- the parts ----------

def rail(m, x0, y0, x1, y1, inner_side):
    """A side rail: a raised forged-steel profile with a ridge, gold lines on both edges."""
    m.put((x0, y0, x1, y1), BASE + 30, blur=0.5)
    mid = (x0 + x1) / 2
    m.put((mid - 0.6, y0, mid + 0.6, y1), BASE + 55)
    m.put((x0, y0, x0 + 1.2, y1), GOLD_UP, gold=True)
    m.put((x1 - 1.4, y0, x1, y1), GOLD_UP, gold=True)


def bar(m, x0, y0, x1, y1, gold_at):
    """A horizontal bar of the frame: raised steel with a ridge and gold lines on both edges."""
    m.put((x0, y0, x1, y1), BASE + 30, blur=0.5)
    mid = (y0 + y1) / 2
    m.put((x0, mid - 0.6, x1, mid + 0.6), BASE + 55)
    m.put((x0, y0, x1, y0 + 1.2), GOLD_UP, gold=True)
    m.put((x0, y1 - 1.4, x1, y1), GOLD_UP, gold=True)


def gem(m, cx, cy, r):
    """A gold diamond stud."""
    m.put_polygon([(cx, cy - r), (cx + r, cy), (cx, cy + r), (cx - r, cy)], GOLD_UP, gold=True)


def corner(m, cx, cy, dx, dy, size=13):
    """A corner cap of the frame: a gold-rimmed steel block with a gem and gold scrolls along both edges;
    cx, cy is the outer corner, dx, dy point into the frame."""
    x0, x1 = sorted((cx, cx + dx * size))
    y0, y1 = sorted((cy, cy + dy * size))
    m.put((x0, y0, x1, y1), GOLD_UP, gold=True, radius=2)
    m.put((x0 + 1.4, y0 + 1.4, x1 - 1.4, y1 - 1.4), BASE + 45, radius=1.5, blur=0.4)
    gem(m, cx + dx * size / 2, cy + dy * size / 2, 3.4)
    for k in range(2):  # small scrolls leaving the cap along the two edges
        if k == 0:
            sx, sy = cx + dx * (size + 4), cy + dy * 4.5
        else:
            sx, sy = cx + dx * 4.5, cy + dy * (size + 4)
        m.put_ellipse((sx - 2.6, sy - 2.6, sx + 2.6, sy + 2.6), GOLD_UP)
        m.put_ellipse((sx - 1.2, sy - 1.2, sx + 1.2, sy + 1.2), BASE + 30, gold=False)


def wing(m, x, cy, direction, length):
    """A gold wing leaving the title plaque toward the edge of the frame."""
    tip = x + direction * length
    m.put_polygon([(x, cy - 4.5), (tip, cy - 0.6), (tip, cy + 0.6), (x, cy + 4.5)], GOLD_UP, gold=True)
    m.put_polygon([(x + direction * 2, cy - 2.2), (tip - direction * 6, cy - 0.3), (tip - direction * 6, cy + 0.3),
                   (x + direction * 2, cy + 2.2)], BASE + 45, gold=False)


def frame_top(m, x, y, w, h, title=True):
    """The top of a window: a heavy bar, a sunk title plaque with gold wings, the rails, corner caps."""
    bar(m, x, y, x + w, y + 9, 'bottom')
    rail(m, x, y, x + 8, y + h, 'right')
    rail(m, x + w - 8, y, x + w, y + h, 'left')
    if title:
        plaque = (x + 34, y + 4, x + w - 34, y + 25)
        cy = (plaque[1] + plaque[3]) / 2
        wing(m, plaque[0], cy, -1, 20)
        wing(m, plaque[2], cy, 1, 20)
        m.put(plaque, GOLD_UP, gold=True, radius=4)
        m.put((plaque[0] + 1.5, plaque[1] + 1.5, plaque[2] - 1.5, plaque[3] - 1.5), SUNK, inner=True, radius=3)
        line_y = y + 30
        m.put((x + 8, line_y, x + w - 8, line_y + 1.4), GOLD_UP, gold=True)  # the line under the title
        gem(m, x + w / 2, line_y + 0.7, 3.0)
    else:
        crest(m, x + w / 2, y + 4.5)
    corner(m, x, y, 1, 1)
    corner(m, x + w, y, -1, 1)


def crest(m, cx, cy):
    """A gold crest on the top bar of a message box (whose text and buttons start right below the bar)."""
    wing(m, cx - 7, cy, -1, 26)
    wing(m, cx + 7, cy, 1, 26)
    m.put_polygon([(cx, cy - 6.5), (cx + 7.5, cy), (cx, cy + 6.5), (cx - 7.5, cy)], GOLD_UP, gold=True)
    m.put_polygon([(cx, cy - 3.6), (cx + 4.2, cy), (cx, cy + 3.6), (cx - 4.2, cy)], SUNK + 40, gold=False, inner=True)


def frame_bottom(m, x, y, w, h):
    rail(m, x, y, x + 8, y + h, 'right')
    rail(m, x + w - 8, y, x + w, y + h, 'left')
    bar(m, x, y + h - 9, x + w, y + h, 'top')
    gem(m, x + w / 2, y + h - 4.5, 3.2)
    corner(m, x, y + h, 1, -1)
    corner(m, x + w, y + h, -1, -1)


def button(m, x, y, w, h):
    """A raised steel plate with a gold rim and a sunk dark face."""
    m.put((x + 0.5, y + 0.5, x + w - 0.5, y + h - 0.5), GOLD_UP, gold=True, radius=3.5)
    m.put((x + 1.7, y + 1.7, x + w - 1.7, y + h - 1.7), RAISED, radius=2.5, blur=0.6)
    m.put((x + 3.2, y + 3.2, x + w - 3.2, y + h - 3.2), BASE + 25, radius=1.8, blur=0.8)


def exit_button(m, x, y, w, h):
    button(m, x + 3, y + 1, w - 6, h - 2)
    cx, cy, r = x + w / 2, y + h / 2, 5.5
    m.put_line([(cx - r, cy - r), (cx + r, cy + r)], 2.2, GOLD_UP)
    m.put_line([(cx - r, cy + r), (cx + r, cy - r)], 2.2, GOLD_UP)


def separator(m, x, y, w, h):
    cy = y + h / 2
    m.put((x + 10, cy - 0.7, x + w - 10, cy + 0.7), GOLD_UP, gold=True)
    gem(m, x + w / 2, cy, 3.6)
    for ex, d in ((x + 5, 1), (x + w - 5, -1)):
        m.put_polygon([(ex - d * 4, cy), (ex + d * 5, cy - 3.2), (ex + d * 8, cy), (ex + d * 5, cy + 3.2)], GOLD_UP, gold=True)


def thumb(m, x, y, w, h):
    cx, cy = x + w / 2, y + h / 2
    m.put_polygon([(cx, y + 1), (x + w - 1, cy), (cx, y + h - 1), (x + 1, cy)], GOLD_UP, gold=True)
    m.put_ellipse((cx - 3.2, cy - 3.2, cx + 3.2, cy + 3.2), RAISED + 20, gold=False, inner=True)


def build_maps(work):
    m = Maps()
    x, y, w, h = PARTS['back']
    m.put((x, y, x + w, y + h), BASE)
    x, y, w, h = PARTS['top']
    frame_top(m, x, y, w, h)
    x, y, w, h = PARTS['bottom']
    frame_bottom(m, x, y, w, h)
    x, y, w, h = PARTS['rail_l']
    rail(m, x, y, x + 8, y + h, 'right')
    x, y, w, h = PARTS['rail_r']
    rail(m, x + w - 8, y, x + w, y + h, 'left')
    x, y, w, h = PARTS['msg_top']
    frame_top(m, x, y, w, h, title=False)
    x, y, w, h = PARTS['msg_middle']
    rail(m, x, y, x + 8, y + h, 'right')
    rail(m, x + w - 8, y, x + w, y + h, 'left')
    x, y, w, h = PARTS['msg_bottom']
    frame_bottom(m, x, y, w, h)
    for name in ('btn', 'btn_big', 'btn_small', 'btn_very_small'):
        button(m, *PARTS[name])
    exit_button(m, *PARTS['exit'])
    separator(m, *PARTS['line'])
    thumb(m, *PARTS['thumb'])
    m.save(work)


def run_blender(work):
    subprocess.run([BLENDER, '-b', '--factory-startup', '-P', os.path.join(HERE, 'render_hud.py'), '--', work, 'ui'],
                   check=True, stdout=subprocess.DEVNULL)


# ---------- the textures ----------

FRAME_LIGHT = 1.45  # the steel of frames and buttons lighter than the inside of the windows


def cut(render, alpha, name, light=FRAME_LIGHT):
    x, y, w, h = PARTS[name]
    box = (s(x), s(y), s(x + w), s(y + h))
    img = ImageEnhance.Brightness(render.crop(box)).enhance(light).convert('RGBA')
    img.putalpha(alpha.crop(box))
    return img.resize((w, h), Image.LANCZOS)


def states(img, count, shift_down=True):
    """count states stacked: normal, under the mouse (brighter, a gold glow), pressed (darker, one pixel lower)."""
    w, h = img.size
    out = Image.new('RGBA', (w, h * count), (0, 0, 0, 0))
    looks = [(1.0, 0.0, 0), (1.3, 0.5, 0), (0.72, 0.0, 1 if shift_down else 0)][:count]
    for k, (bright, glow, dy) in enumerate(looks):
        rgb = ImageEnhance.Brightness(img.convert('RGB')).enhance(bright).convert('RGBA')
        rgb.putalpha(img.getchannel('A'))
        if glow:
            halo = Image.new('RGBA', img.size, (255, 214, 130, 0))
            halo.putalpha(img.getchannel('A').filter(ImageFilter.GaussianBlur(1.2)).point(lambda v: int(v * glow * 0.6)))
            rgb = Image.alpha_composite(halo, rgb)
        out.alpha_composite(rgb, (0, h * k + dy))
    return out


def recolor(name, palette):
    """A classic thin part in gold or steel: its shape (alpha) and shading (luminance) kept."""
    img = Image.open(classic_file(name)).convert('RGBA')
    lum = img.convert('L')
    dark, light = palette
    tinted = Image.merge('RGB', [lum.point(lambda v, a=dark[i], b=light[i]: int(a + (b - a) * v / 255)) for i in range(3)])
    tinted.putalpha(img.getchannel('A'))
    return tinted


def classic_file(name):
    """The unpacked classic texture (a .jpg / .tga in the work folder)."""
    stem, ext = os.path.splitext(name)
    packed = {'.jpg': '.OZJ', '.tga': '.OZT'}[ext.lower()]
    for f in os.listdir(CLASSIC_DIR):
        if f.lower() == (stem + packed).lower():
            return ozj.unpack(os.path.join(CLASSIC_DIR, f), WORK_DIR)
    raise FileNotFoundError(name)


GOLD = ((70, 46, 14), (255, 222, 150))
STEEL = ((10, 11, 15), (120, 126, 140))


def build(work):
    render = Image.open(os.path.join(work, 'ui.png')).convert('RGB')
    alpha = Image.open(os.path.join(work, 'ui_alpha.png'))
    back = cut(render, Image.new('L', alpha.size, 255), 'back', light=1.0)
    # the inside of a window: the steel darkened for the text, a soft vignette
    back = ImageEnhance.Contrast(ImageEnhance.Brightness(back.convert('RGB')).enhance(0.85)).enhance(1.35)
    vignette = Image.new('L', back.size, 0)
    ImageDraw.Draw(vignette).rectangle((0, 0, back.width, back.height), outline=255, width=14)
    back = Image.composite(Image.new('RGB', back.size, (0, 0, 0)), back, vignette.filter(ImageFilter.GaussianBlur(14)).point(lambda v: int(v * 0.5)))
    btn = cut(render, alpha, 'btn')
    textures = {
        'newui_msgbox_back.jpg': back,
        'newui_item_back01.tga': cut(render, alpha, 'top'),
        'newui_item_back04.tga': cut(render, alpha, 'top'),
        'newui_item_back03.tga': cut(render, alpha, 'bottom'),
        'newui_item_back02-L.tga': cut(render, alpha, 'rail_l'),
        'newui_item_back02-R.tga': cut(render, alpha, 'rail_r'),
        'newui_msgbox_top.tga': cut(render, alpha, 'msg_top'),
        'newui_msgbox_middle.tga': cut(render, alpha, 'msg_middle'),
        'newui_msgbox_bottom.tga': cut(render, alpha, 'msg_bottom'),
        'newui_btn_empty.tga': states(btn, 3),
        'newui_btn_empty_big.tga': states(cut(render, alpha, 'btn_big'), 3),
        'newui_btn_empty_small.tga': states(cut(render, alpha, 'btn_small'), 3),
        'newui_btn_empty_very_small.tga': states(cut(render, alpha, 'btn_very_small'), 3),
        'newui_exit_00.tga': states(cut(render, alpha, 'exit'), 2, shift_down=False),
        'newui_myquest_Line.tga': cut(render, alpha, 'line'),
        'newui_scroll_on.tga': cut(render, alpha, 'thumb'),
        'newui_scroll_off.tga': ImageEnhance.Color(cut(render, alpha, 'thumb')).enhance(0.0),
    }
    for name in ('newui_item_table01(L).tga', 'newui_item_table01(R).tga', 'newui_item_table02(L).tga',
                 'newui_item_table02(R).tga', 'newui_item_table03(Up).tga', 'newui_item_table03(Dw).tga',
                 'newui_item_table03(L).tga', 'newui_item_table03(R).tga'):
        textures[name] = recolor(name, GOLD)
    for name in ('newui_scrollbar_up.tga', 'newui_scrollbar_m.tga', 'newui_scrollbar_down.tga'):
        textures[name] = recolor(name, STEEL)
    return textures


def check_sizes(textures):
    """The parts must keep the classic sizes: the windows draw them with the classic texel sizes."""
    for name, img in textures.items():
        classic = Image.open(classic_file(name))
        if classic.size != img.size:
            raise SystemExit(f'{name}: {img.size}, the classic one is {classic.size}')


def save(textures):
    os.makedirs(OUT_DIR, exist_ok=True)
    for name, img in textures.items():
        stem, ext = os.path.splitext(name)
        path = os.path.join(OUT_DIR, name)
        if ext.lower() == '.jpg':
            img.convert('RGB').save(path, quality=95, subsampling=0)
        else:
            img = img.convert('RGBA')
            ozj.write_tga(path, img.width, img.height, img.transpose(Image.FLIP_TOP_BOTTOM).tobytes())
        ozj.pack(path, OUT_DIR)
        os.remove(path)
    print(f'{len(textures)} textures -> {OUT_DIR}')


def preview(textures, out_png):
    """A window of 190 x 429 as the client composes it, the buttons and parts next to it, 2 x."""
    canvas = Image.new('RGBA', (560, 440), (40, 44, 52, 255))
    win = Image.new('RGBA', (190, 429), (0, 0, 0, 0))
    win.alpha_composite(textures['newui_msgbox_back.jpg'].convert('RGBA').crop((0, 0, 190, 429)))
    win.alpha_composite(textures['newui_item_back01.tga'], (0, 0))
    for y in range(64, 429 - 45, 10):
        win.alpha_composite(textures['newui_item_back02-L.tga'].crop((0, 0, 21, 10)), (0, y))
        win.alpha_composite(textures['newui_item_back02-R.tga'].crop((0, 0, 21, 10)), (190 - 21, y))
    win.alpha_composite(textures['newui_item_back03.tga'], (0, 429 - 45))
    win.alpha_composite(textures['newui_myquest_Line.tga'], (1, 120))
    win.alpha_composite(textures['newui_btn_empty.tga'].crop((0, 0, 108, 30)), (41, 200))
    win.alpha_composite(textures['newui_exit_00.tga'].crop((0, 0, 36, 29)), (13, 429 - 38))
    canvas.alpha_composite(win, (5, 5))
    x, y = 210, 5
    for name in ('newui_btn_empty.tga', 'newui_btn_empty_small.tga', 'newui_btn_empty_very_small.tga', 'newui_exit_00.tga',
                 'newui_scroll_on.tga', 'newui_scroll_off.tga'):
        img = textures[name]
        canvas.alpha_composite(img, (x, y))
        x += img.width + 8
    canvas.alpha_composite(textures['newui_msgbox_top.tga'], (210, 110))
    canvas.alpha_composite(textures['newui_msgbox_middle.tga'], (210, 177))
    canvas.alpha_composite(textures['newui_msgbox_bottom.tga'], (210, 192))
    canvas.resize((1120, 880), Image.LANCZOS).save(out_png)
    print(f'preview -> {out_png}')


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--preview', metavar='PNG')
    parser.add_argument('--skip-render', action='store_true')
    parser.add_argument('--work', default=WORK_DIR)
    args = parser.parse_args()
    os.makedirs(args.work, exist_ok=True)
    if not args.skip_render:
        build_maps(args.work)
        run_blender(args.work)
    textures = build(args.work)
    check_sizes(textures)
    save(textures)
    if args.preview:
        preview(textures, args.preview)


if __name__ == '__main__':
    main()

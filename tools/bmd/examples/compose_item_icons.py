"""Second step of render_item_icons.py (plain Python with Pillow): puts the rendered items into the pictures of the
AdminPanel (120 x 126, the item in the middle, item_14_<number>_0.png) and the site (WebP, trimmed).

    python examples/compose_item_icons.py <out folder of render_item_icons.py> <ItemEditor img/items> [<site img/items>]
"""

import glob
import os
import sys

from PIL import Image, ImageEnhance

CANVAS = (120, 126)
BRIGHTNESS = 2.1


def main():
    out_dir, editor_dir = sys.argv[1], sys.argv[2]
    site_dir = sys.argv[3] if len(sys.argv) > 3 else None
    raw_dir = os.path.join(out_dir, 'raw')
    pictures = []
    for path in sorted(glob.glob(os.path.join(raw_dir, 'item_14_*_0.png'))):
        name = os.path.basename(path)
        height = int(open(path[:-4] + '.txt').read())
        image = Image.open(path).convert('RGBA')
        image = image.crop(image.getbbox())
        # the studio light of Blender is darker than the jewels in the game, which shine
        alpha = image.getchannel('A')
        image = ImageEnhance.Color(ImageEnhance.Brightness(image.convert('RGB')).enhance(BRIGHTNESS)).enhance(1.15).convert('RGBA')
        image.putalpha(alpha)
        scale = height / image.height
        image = image.resize((max(1, round(image.width * scale)), height), Image.LANCZOS)
        canvas = Image.new('RGBA', CANVAS, (0, 0, 0, 0))
        canvas.alpha_composite(image, ((CANVAS[0] - image.width) // 2, (CANVAS[1] - image.height) // 2))
        canvas.save(os.path.join(editor_dir, name))
        if site_dir:
            image.save(os.path.join(site_dir, name[:-4] + '.webp'), quality=90)
        pictures.append(canvas)

    sheet = Image.new('RGBA', (CANVAS[0] * 9, CANVAS[1] * ((len(pictures) + 8) // 9)), (40, 40, 48, 255))
    for i, picture in enumerate(pictures):
        sheet.alpha_composite(picture, ((i % 9) * CANVAS[0], (i // 9) * CANVAS[1]))
    sheet.save(os.path.join(out_dir, 'sheet.png'))
    print(len(pictures), 'pictures')


main()

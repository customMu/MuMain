"""The icon of the state of the boss of the Illusion of Noria (10.10.2026): the Gilded Colossus in the top right corner of
the screen on the map of the illusion, drawn by Gemini in the style of the buff icons of the game. One picture, three
textures:

- awakened (alive): the picture as it is            -> newui_bossawakened.OZJ
- ritual (the kills of the map wake it): violet     -> newui_bossritual.OZJ
- banished (the cooldown after its defeat): grey    -> newui_bossbanished.OZJ

    python boss_state_icons.py --generate   draws a new picture with Gemini (needs GEMINI_API_KEY) -> boss_state_icon.png
    python boss_state_icons.py              converts the picture -> src/bin/Data/Interface/newui_boss*.OZJ

The client loads them in CNewUIBuffWindow::LoadImages (IMAGE_BOSS_BANISHED ... IMAGE_BOSS_AWAKENED) and draws the
40 x 56 part of the 64 x 64 texture (GameLogic/Events/IllusionOfNoria.cpp, RenderBossState).
"""
import os
import sys

from PIL import Image, ImageEnhance, ImageOps

import golden_curse_icon as base

HERE = os.path.dirname(os.path.abspath(__file__))
SOURCE = os.path.join(HERE, "boss_state_icon.png")
PROMPT = (
    "Draw one game icon in exactly the same style as the icons of the reference image (MU Online status icons): "
    "a small painted fantasy icon, portrait format 5:7, rich saturated colors, a colored radial gradient background, "
    "bold readable silhouette that fills the picture, no text, no letters, no border frame, no transparency. "
    "Subject: 'Gilded Colossus' - the head and shoulders of a huge golem made of cracked golden stone, seen from the "
    "front, glowing golden eyes, thin violet light in the cracks, a heavy brow; a dark brown to gold radial gradient "
    "background with a warm glow behind the head. Mighty, clearly a boss."
)


def tinted(img, state):
    if state == "awakened":
        return img
    grey = ImageOps.grayscale(img).convert("RGB")
    if state == "banished":
        return ImageEnhance.Brightness(grey).enhance(0.55)
    # the ritual: violet, a bit darker than awakened
    violet = Image.new("RGB", img.size, (150, 70, 230))
    return ImageEnhance.Brightness(Image.blend(ImageOps.colorize(ImageOps.grayscale(img), (20, 0, 40), (230, 190, 255)), violet, 0.15)).enhance(0.9)


def main():
    base.PROMPT = PROMPT
    base.SOURCE = SOURCE
    base.FOCUS = (0.05, 0.05, 0.95, 0.95)
    if "--generate" in sys.argv:
        base.generate()

    original = Image.open(SOURCE).convert("RGB")
    for state in ("awakened", "ritual", "banished"):
        temp = os.path.join(HERE, f"boss_state_{state}.tmp.png")
        tinted(original, state).save(temp)
        base.SOURCE = temp
        base.TARGET = os.path.join(base.REPO, "src", "bin", "Data", "Interface", f"newui_boss{state}.OZJ")
        base.convert()
        os.remove(temp)


if __name__ == "__main__":
    main()

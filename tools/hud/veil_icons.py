"""The buff icons of the Illusion of Noria which the warden sells for Illusion Shards (09.10.2026), drawn by Gemini in the
style of the buff icons of the game (Data/Interface/newui_statusicon.jpg - small painted icons on a colored gradient):

- Veil Ward (magic effect 189): the Golden Curse of the boss does no damage, 30 minutes -> newui_veilward.OZJ
- Blessing of the Veil (magic effect 185): +50 % damage to the monsters of the illusion, 1 hour -> newui_veilblessing.OZJ

    python veil_icons.py --generate   draws new pictures with Gemini (needs GEMINI_API_KEY) -> veil_*_icon.png
    python veil_icons.py              converts the pictures -> src/bin/Data/Interface/newui_veil*.OZJ

The client loads them in CNewUIBuffWindow::LoadImages (IMAGE_BUFF_VEIL_WARD, IMAGE_BUFF_VEIL_BLESSING) and draws the
40 x 56 part of the 64 x 64 texture, like the Golden Curse (golden_curse_icon.py).
"""
import os
import sys

import golden_curse_icon as base

HERE = os.path.dirname(os.path.abspath(__file__))
STYLE = (
    "Draw one game buff icon in exactly the same style as the icons of the reference image (MU Online status icons): "
    "a small painted fantasy icon, portrait format 5:7, rich saturated colors, a colored radial gradient background, "
    "bold readable silhouette that fills the picture, no text, no letters, no border frame, no transparency. "
)
ICONS = {
    "veil_ward": (
        "newui_veilward",
        STYLE + "Subject: 'Veil Ward' - a protective blessing against a golden curse. Show a translucent violet-silver "
        "magic shield with an elven leaf rune in the center, a thin shimmering veil of light wrapped around it, golden "
        "curse sparks breaking and fading on its surface; a deep blue-violet radial gradient background. Calm, clearly "
        "a protective buff.",
    ),
    "veil_blessing": (
        "newui_veilblessing",
        STYLE + "Subject: 'Blessing of the Veil' - a blessing which makes the weapon strike harder against illusions. Show "
        "a glowing sword blade pointing up, wrapped in swirling violet veil mist and bright violet-pink energy, a few "
        "shattering ghostly illusion shards around the tip; a dark violet to magenta radial gradient background with a "
        "bright core. Powerful, clearly an attack buff.",
    ),
}


def main():
    names = [n for n in ICONS if n in sys.argv] or list(ICONS)
    for name in names:
        target, prompt = ICONS[name]
        base.PROMPT = prompt
        base.SOURCE = os.path.join(HERE, f"{name}_icon.png")
        base.TARGET = os.path.join(base.REPO, "src", "bin", "Data", "Interface", f"{target}.OZJ")
        base.FOCUS = (0.08, 0.08, 0.92, 0.92)
        if "--generate" in sys.argv:
            base.generate()
        base.convert()


if __name__ == "__main__":
    main()

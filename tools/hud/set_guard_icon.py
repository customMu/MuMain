"""The buff icon of the Set Guard (a complete armor set reduces the damage taken), drawn by Gemini in the style of the
buff icons of the game (Data/Interface/newui_statusicon.jpg - small painted icons on a colored gradient, 20 x 28).

    python set_guard_icon.py --generate   draws a new picture with Gemini (needs GEMINI_API_KEY) -> set_guard_icon.png
    python set_guard_icon.py              converts set_guard_icon.png -> src/bin/Data/Interface/newui_setguard.OZJ

The client loads it in CNewUIBuffWindow::LoadImages (IMAGE_BUFF_SET_GUARD) and draws it for the Set Guard buff.
"""
import base64
import io
import json
import os
import sys
import urllib.request

from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.normpath(os.path.join(HERE, "..", ".."))
SOURCE = os.path.join(HERE, "set_guard_icon.png")
ATLAS = os.path.join(REPO, "src", "bin", "Data", "Interface", "newui_statusicon.OZJ")
TARGET = os.path.join(REPO, "src", "bin", "Data", "Interface", "newui_setguard.OZJ")
MODEL = "gemini-2.5-flash-image"
# twice the size of the icon on a 640 x 480 screen: sharp on larger screens, still small; the client pads it to 64 x 64
# and draws the 40 x 56 part (CNewUIBuffWindow) - change both together
SIZE = (40, 56)

PROMPT = (
    "Draw one game buff icon in exactly the same style as the icons of the reference image (MU Online status icons): "
    "a small painted fantasy icon, portrait format 5:7, rich saturated colors, a colored radial gradient background, "
    "bold readable silhouette, no text, no letters, no border frame, no transparency. "
    "Subject: 'Set Guard' - the protection of a complete matching armor set. Show the whole upper body of a heavy "
    "golden-steel knight's armor, centered and filling most of the picture: a closed helmet above the chest armor, both joined by a glowing protective rune shield aura; warm gold and deep blue "
    "colors, a soft golden glow behind it. It must look clearly different from a plain shield icon."
)


def atlas_sample():
    raw = open(ATLAS, "rb").read()
    img = Image.open(io.BytesIO(raw[24:])).convert("RGB")
    sample = img.crop((0, 0, 200, 56)).resize((800, 224), Image.NEAREST)
    buffer = io.BytesIO()
    sample.save(buffer, "PNG")
    return base64.b64encode(buffer.getvalue()).decode("ascii")


def generate():
    key = os.environ.get("GEMINI_API_KEY")
    if not key:
        sys.exit("GEMINI_API_KEY is not set")
    body = {
        "contents": [{"parts": [
            {"text": PROMPT},
            {"inlineData": {"mimeType": "image/png", "data": atlas_sample()}},
        ]}],
        # portrait like the icon (the reference strip is wide, it must not set the format)
        "generationConfig": {"responseModalities": ["IMAGE"], "imageConfig": {"aspectRatio": "3:4"}},
    }
    request = urllib.request.Request(
        f"https://generativelanguage.googleapis.com/v1beta/models/{MODEL}:generateContent",
        data=json.dumps(body).encode("utf-8"),
        headers={"Content-Type": "application/json", "x-goog-api-key": key},
    )
    with urllib.request.urlopen(request, timeout=180) as response:
        result = json.load(response)
    for part in result["candidates"][0]["content"]["parts"]:
        if "inlineData" in part:
            Image.open(io.BytesIO(base64.b64decode(part["inlineData"]["data"]))).convert("RGB").save(SOURCE)
            print("saved", SOURCE)
            return
    sys.exit("no image in the answer: " + json.dumps(result)[:500])


# the part of the picture around the armor (fractions of width and height): the icon is tiny, the subject must fill it
FOCUS = (0.12, 0.13, 0.88, 0.78)


def convert():
    img = Image.open(SOURCE).convert("RGB")
    w, h = img.size
    img = img.crop((int(FOCUS[0] * w), int(FOCUS[1] * h), int(FOCUS[2] * w), int(FOCUS[3] * h)))
    # crop to 5:7 around the center, then scale down
    w, h = img.size
    target_ratio = SIZE[0] / SIZE[1]
    if w / h > target_ratio:
        nw = int(h * target_ratio)
        img = img.crop(((w - nw) // 2, 0, (w - nw) // 2 + nw, h))
    else:
        nh = int(w / target_ratio)
        img = img.crop((0, (h - nh) // 2, w, (h - nh) // 2 + nh))
    img = img.resize(SIZE, Image.LANCZOS)
    buffer = io.BytesIO()
    img.save(buffer, "JPEG", quality=95)
    jpeg = buffer.getvalue()
    # .OZJ = 24 bytes in front of the JPEG (the client skips them); the game files repeat the start of the JPEG there
    with open(TARGET, "wb") as f:
        f.write(jpeg[:24] + jpeg)
    print("saved", TARGET)


if __name__ == "__main__":
    if "--generate" in sys.argv:
        generate()
    convert()

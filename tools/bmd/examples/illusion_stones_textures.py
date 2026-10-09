"""Textures of the stones of the skill fix option (Illusion of Noria): one crystal picture (Gemini,
illusion_stones/crystal_source.png) recoloured per stone.

    python examples/illusion_stones_textures.py

Writes illusion_stones/<name>.jpg (128 x 128): the Jewel of Illusion violet, the Echoes in the colour of their class
(DK red, DW blue, Elf green, MG magenta, DL gold, SUM rose, RF orange), the Lesser Mirage Stone silver blue and the
Greater Mirage Stone gold. examples/illusion_stones.py builds the models with them.
"""
import colorsys
import os

from PIL import Image

HERE = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'illusion_stones')
SIZE = 128
# name -> (hue in degrees, saturation factor, brightness factor)
COLOURS = {
    'jewelofillusion': (275, 1.0, 1.05),
    'echo_dk': (355, 1.0, 1.0),
    'echo_dw': (215, 1.0, 1.05),
    'echo_elf': (125, 0.95, 1.0),
    'echo_mg': (315, 1.0, 1.0),
    'echo_dl': (45, 1.0, 1.15),
    'echo_sum': (330, 0.75, 1.15),
    'echo_rf': (25, 1.0, 1.1),
    'miragelesser': (195, 0.45, 1.2),
    'miragegreater': (42, 0.9, 1.25),
}


def recolour(source, hue, saturation, brightness):
    """Keeps the light and the shape of the crystal, puts the hue (the source is violet)."""
    pixels = []
    target = hue / 360.0
    for r, g, b in source.getdata():
        h, s, v = colorsys.rgb_to_hsv(r / 255, g / 255, b / 255)
        r2, g2, b2 = colorsys.hsv_to_rgb(target, min(1.0, s * saturation), min(1.0, v * brightness))
        pixels.append((int(r2 * 255), int(g2 * 255), int(b2 * 255)))
    image = Image.new('RGB', source.size)
    image.putdata(pixels)
    return image


def main():
    source = Image.open(os.path.join(HERE, 'crystal_source.png')).convert('RGB').resize((SIZE, SIZE), Image.LANCZOS)
    for name, (hue, saturation, brightness) in COLOURS.items():
        recolour(source, hue, saturation, brightness).save(os.path.join(HERE, name + '.jpg'), quality=92)
        print('saved', name)


if __name__ == '__main__':
    main()

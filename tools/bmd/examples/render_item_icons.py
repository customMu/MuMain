"""Renders the inventory pictures of the items of the Illusion of Noria (14/171-197) from their BMD models.

The AdminPanel of the server (ItemEditor, item_{group}_{number}_{level}.png, 120 x 126, the item in the middle) and the
site (WebP, trimmed) show these pictures; the game files have none for the new items.

    blender -b -P examples/render_item_icons.py -- <Data/Item folder> <out folder>

Writes <out>/item_14_<number>_0.png; tools/import-item-pictures.sh of the portal (or render_item_icons_webp) makes WebP.
"""

import math
import os
import sys

import bpy
from mathutils import Vector

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
import blender_bmd  # noqa: E402

ECHO_CLASS = {173: 'DK', 174: 'DK', 175: 'DK', 176: 'MG', 177: 'MG', 178: 'DW', 179: 'MG', 180: 'MG', 181: 'DW', 182: 'DW',
              183: 'Elf', 184: 'Elf', 185: 'Elf', 186: 'DL', 187: 'DL', 188: 'DL', 189: 'SUM', 190: 'SUM', 191: 'SUM',
              192: 'RF', 193: 'RF', 194: 'RF'}

# item number -> (model, height of the picture in pixels: about 30 for a 1x1 item, like the jewels of the game)
ITEMS = {171: ('Whistle01', 56), 172: ('IllusionShard01', 32), 195: ('IllusionJewel01', 32),
         196: ('MirageLesser01', 26), 197: ('MirageGreater01', 34)}
ITEMS.update({number: (f'Echo{cls}01', 32) for number, cls in ECHO_CLASS.items()})

CANVAS = (120, 126)
RENDER = 512


def setup_scene():
    scene = bpy.context.scene
    scene.render.engine = 'BLENDER_WORKBENCH'
    scene.display.shading.light = 'STUDIO'
    scene.display.shading.color_type = 'TEXTURE'
    scene.display.shading.show_specular_highlight = True
    scene.render.film_transparent = True
    scene.render.resolution_x = RENDER
    scene.render.resolution_y = RENDER
    scene.render.image_settings.file_format = 'PNG'
    scene.render.image_settings.color_mode = 'RGBA'
    return scene


def frame(scene, objects):
    points = [obj.matrix_world @ Vector(corner) for obj in objects for corner in obj.bound_box]
    low = Vector((min(p.x for p in points), min(p.y for p in points), min(p.z for p in points)))
    high = Vector((max(p.x for p in points), max(p.y for p in points), max(p.z for p in points)))
    centre = (low + high) / 2
    size = max((high - low).x, (high - low).z, (high - low).y) * 1.3
    camera_data = bpy.data.cameras.new('Camera')
    camera_data.type = 'ORTHO'
    camera_data.ortho_scale = size
    camera = bpy.data.objects.new('Camera', camera_data)
    scene.collection.objects.link(camera)
    # from the front, a bit from above, like the inventory of the game
    tilt = math.radians(75)
    camera.location = centre + Vector((0, -math.sin(tilt), math.cos(tilt))) * size * 4
    camera.rotation_euler = (tilt, 0, 0)
    scene.camera = camera


def render(model_path, png_path):
    bpy.ops.wm.read_factory_settings(use_empty=True)
    scene = setup_scene()
    blender_bmd.import_bmd(model_path, os.path.dirname(png_path))
    meshes = [obj for obj in scene.objects if obj.type == 'MESH']
    frame(scene, meshes)
    scene.render.filepath = png_path
    bpy.ops.render.render(write_still=True)


def main():
    args = sys.argv[sys.argv.index('--') + 1:]
    data_dir, out_dir = args[0], args[1]
    os.makedirs(out_dir, exist_ok=True)
    raw_dir = os.path.join(out_dir, 'raw')
    os.makedirs(raw_dir, exist_ok=True)
    for number, (model, height) in sorted(ITEMS.items()):
        render(os.path.join(data_dir, model + '.bmd'), os.path.join(raw_dir, f'item_14_{number}_0.png'))
        with open(os.path.join(raw_dir, f'item_14_{number}_0.txt'), 'w') as f:
            f.write(str(height))


main()

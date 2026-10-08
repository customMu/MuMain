"""Example: a static item model built from code, the Jewelry Box (item 14/170).

    blender -b -P examples/jewelry_box.py -- out_dir

Writes out_dir/jewelry_box.blend, out_dir/JewelryBox01.bmd and its texture out_dir/jewelrybox.OZJ:
a ring box of red leather with a gold trim, the lid open, a gold ring with a ruby in blue velvet.
The client loads it as MODEL_JEWELRY_BOX from Data\\Item\\ (ZzzOpenData.cpp). The size follows the
Box of Luck (MagicBox01.bmd, about 31 x 31 x 26 units), so the default item scale of group 14 fits.
"""
import math
import os
import sys

import bmesh
import bpy
import numpy
from mathutils import Matrix, Vector

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
import blender_bmd  # noqa: E402

TEXTURE_SIZE = 64
CELLS = 4  # the texture is a 4 x 4 grid of colour cells
COLORS = {  # cell: (r, g, b, noise)
    'leather': ((0, 0), (0.42, 0.05, 0.07), 0.25),
    'gold': ((1, 0), (0.95, 0.72, 0.22), 0.12),
    'velvet': ((2, 0), (0.07, 0.09, 0.32), 0.30),
    'ruby': ((3, 0), (0.95, 0.08, 0.22), 0.10),
    'gold_dark': ((0, 1), (0.55, 0.38, 0.10), 0.10),
}
BASE = (30.0, 24.0, 14.0)  # width, depth, height of the lower part
LID_HEIGHT = 8.0
LID_OPEN = math.radians(105)  # from closed, around the hinge at the back


def build_texture(out_dir):
    """Colour cells with some noise; saved as JPEG, so the export writes jewelrybox.OZJ."""
    rng = numpy.random.default_rng(7)
    cell = TEXTURE_SIZE // CELLS
    rgba = numpy.ones((TEXTURE_SIZE, TEXTURE_SIZE, 4), dtype=numpy.float32)
    for (cx, cy), color, noise in COLORS.values():
        shade = 1.0 - noise + noise * rng.random((cell, cell))
        for c in range(3):
            rgba[cy * cell:(cy + 1) * cell, cx * cell:(cx + 1) * cell, c] = color[c] * shade
    image = bpy.data.images.new('jewelrybox', TEXTURE_SIZE, TEXTURE_SIZE)
    image.pixels.foreach_set(rgba.ravel())
    image.filepath_raw = os.path.join(out_dir, 'jewelrybox.jpg')
    image.file_format = 'JPEG'
    image.save()
    material = bpy.data.materials.new('jewelrybox')
    if bpy.app.version < (5, 0, 0):
        material.use_nodes = True  # always on from Blender 5
    node = material.node_tree.nodes.new('ShaderNodeTexImage')
    node.image = image
    node.interpolation = 'Closest'
    material.node_tree.links.new(node.outputs['Color'], material.node_tree.nodes['Principled BSDF'].inputs['Base Color'])
    return material


def paint(obj, color):
    """Maps every face into the cell of the colour, by its position: the noise of the cell shows."""
    (cx, cy), _, _ = COLORS[color]
    cell = 1.0 / CELLS
    margin = cell * 0.15
    me = obj.data
    while me.uv_layers:  # the primitives come with their own layer, the export reads the active one
        me.uv_layers.remove(me.uv_layers[0])
    uv = me.uv_layers.new(name='UV').data
    for poly in me.polygons:
        n = poly.normal
        axes = (1, 2) if abs(n.x) > max(abs(n.y), abs(n.z)) else (0, 2) if abs(n.y) > abs(n.z) else (0, 1)
        for loop in poly.loop_indices:
            co = me.vertices[me.loops[loop].vertex_index].co
            a, b = ((co[axes[0]] / 40.0) % 1.0, (co[axes[1]] / 40.0) % 1.0)
            uv[loop].uv = (cx * cell + margin + a * (cell - 2 * margin), cy * cell + margin + b * (cell - 2 * margin))


def add_box(name, size, location, color, material, bevel=0.0):
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=location)
    obj = bpy.context.active_object
    obj.name = name
    obj.scale = size
    bpy.ops.object.transform_apply(scale=True)
    if bevel:
        bm = bmesh.new()
        bm.from_mesh(obj.data)
        bmesh.ops.bevel(bm, geom=bm.edges[:], offset=bevel, segments=1, affect='EDGES')
        bm.to_mesh(obj.data)
        bm.free()
    finish(obj, color, material, smooth=False)
    return obj


def finish(obj, color, material, smooth):
    obj.data.materials.append(material)
    for poly in obj.data.polygons:
        poly.use_smooth = smooth
    paint(obj, color)


def build(material):
    w, d, h = BASE
    parts = [
        add_box('Base', (w, d, h), (0, 0, h / 2), 'leather', material, bevel=1.2),
        add_box('Trim', (w + 0.8, d + 0.8, 1.6), (0, 0, h - 0.9), 'gold', material),
        add_box('Cushion', (w - 3, d - 3, 1.0), (0, 0, h + 0.3), 'velvet', material),
        add_box('Lock', (4, 1.2, 3), (0, -d / 2 - 0.5, h - 2.5), 'gold_dark', material),
    ]
    # the lid: modelled closed on top of the base, then turned around the hinge at the back
    lid = [
        add_box('Lid', (w, d, LID_HEIGHT), (0, 0, h + LID_HEIGHT / 2 + 0.2), 'leather', material, bevel=1.2),
        add_box('LidTrim', (w + 0.8, d + 0.8, 1.6), (0, 0, h + 1.0), 'gold', material),
        add_box('LidVelvet', (w - 3, d - 3, 0.6), (0, 0, h - 0.15), 'velvet', material),
    ]
    hinge = Matrix.Translation((0, d / 2, h)) @ Matrix.Rotation(-LID_OPEN, 4, 'X') @ Matrix.Translation((0, -d / 2, -h))
    for obj in lid:
        obj.data.transform(hinge)
    # the ring stands in a slit of the cushion, facing the front, the ruby on top
    radius, thickness = 6.0, 1.3
    center = Vector((0, -1.5, h + 0.8 + radius - 2.0))
    bpy.ops.mesh.primitive_torus_add(major_radius=radius, minor_radius=thickness, major_segments=20, minor_segments=8,
                                     location=center, rotation=(math.pi / 2, 0, 0))
    ring = bpy.context.active_object
    ring.name = 'Ring'
    finish(ring, 'gold', material, smooth=True)
    bpy.ops.mesh.primitive_ico_sphere_add(subdivisions=1, radius=2.6, location=center + Vector((0, 0, radius + 1.6)))
    gem = bpy.context.active_object
    gem.name = 'Ruby'
    gem.scale = (1.0, 1.0, 1.25)
    finish(gem, 'ruby', material, smooth=False)
    bpy.ops.mesh.primitive_cylinder_add(vertices=8, radius=1.8, depth=1.6, location=center + Vector((0, 0, radius + 0.2)))
    setting = bpy.context.active_object
    setting.name = 'Setting'
    finish(setting, 'gold', material, smooth=False)
    for obj in bpy.context.scene.objects:
        obj.select_set(True)
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
    return parts + lid + [ring, gem, setting]


def main(out_dir):
    os.makedirs(out_dir, exist_ok=True)
    out_dir = os.path.abspath(out_dir)
    blender_bmd._reset_scene()
    build(build_texture(out_dir))
    bpy.ops.wm.save_as_mainfile(filepath=os.path.join(out_dir, 'jewelry_box.blend'))
    blender_bmd.export_bmd(os.path.join(out_dir, 'JewelryBox01.bmd'))


if __name__ == '__main__':
    args = sys.argv[sys.argv.index('--') + 1:] if '--' in sys.argv else sys.argv[1:]
    main(args[0] if args else '.')

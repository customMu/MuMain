"""Example: a static item model built from code, the Jewelry Box (item 14/170).

    blender -b -P examples/jewelry_box.py -- out_dir

Writes out_dir/jewelry_box.blend, out_dir/JewelryBox01.bmd and its texture out_dir/jewelrybox.OZJ:
a small box of soft rose leather with a champagne gold trim, the lid open; inside on pale velvet a ring
with a rose stone and a necklace with an aqua pendant (the box gives a ring or a pendant).
The client loads it as MODEL_JEWELRY_BOX from Data\\Item\\ (ZzzOpenData.cpp). It is built at the size of
the Box of Luck (MagicBox01.bmd, about 31 x 31 x 26 units) and scaled down by SCALE: a miniature.
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
    'leather': ((0, 0), (0.66, 0.38, 0.42), 0.15),
    'gold': ((1, 0), (0.93, 0.82, 0.56), 0.08),
    'velvet': ((2, 0), (0.80, 0.78, 0.90), 0.15),
    'rose': ((3, 0), (0.95, 0.55, 0.65), 0.06),
    'gold_dark': ((0, 1), (0.74, 0.62, 0.40), 0.08),
    'aqua': ((1, 1), (0.58, 0.86, 0.90), 0.06),
}
BASE = (30.0, 24.0, 14.0)  # width, depth, height of the lower part
LID_HEIGHT = 8.0
LID_OPEN = math.radians(105)  # from closed, around the hinge at the back
SCALE = 0.75  # the whole model, after it is built
ROUND = 2.5  # the radius of the rounded edges of the box and the lid
ROUND_SEGMENTS = 3


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


def add_box(name, size, location, color, material, round_all=0.0, round_corners=0.0):
    """A box with rounded edges: round_all rounds every edge, round_corners only the four upright ones."""
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=location)
    obj = bpy.context.active_object
    obj.name = name
    obj.scale = size
    bpy.ops.object.transform_apply(scale=True)
    radius = round_all or round_corners
    if radius:
        bm = bmesh.new()
        bm.from_mesh(obj.data)
        edges = [e for e in bm.edges if round_all or abs((e.verts[0].co - e.verts[1].co).normalized().z) > 0.9]
        bmesh.ops.bevel(bm, geom=edges, offset=radius, segments=ROUND_SEGMENTS, profile=0.5, affect='EDGES')
        bm.to_mesh(obj.data)
        bm.free()
    finish(obj, color, material, smooth=bool(radius))
    return obj


def add(primitive, name, color, material, smooth, scale=None, **options):
    getattr(bpy.ops.mesh, primitive)(**options)
    obj = bpy.context.active_object
    obj.name = name
    if scale:
        obj.scale = scale
    finish(obj, color, material, smooth)
    return obj


def finish(obj, color, material, smooth):
    obj.data.materials.append(material)
    for poly in obj.data.polygons:
        poly.use_smooth = smooth
    paint(obj, color)


def build(material):
    w, d, h = BASE
    parts = [
        add_box('Base', (w, d, h), (0, 0, h / 2), 'leather', material, round_all=ROUND),
        add_box('Trim', (w + 0.8, d + 0.8, 1.6), (0, 0, h - 0.9), 'gold', material, round_corners=ROUND + 0.4),
        add_box('Cushion', (w - 3, d - 3, 1.0), (0, 0, h + 0.3), 'velvet', material, round_corners=ROUND - 1.4),
        add_box('Lock', (4, 1.2, 3), (0, -d / 2 - 0.5, h - 2.5), 'gold_dark', material, round_corners=0.5),
    ]
    # the lid: modelled closed on top of the base, then turned around the hinge at the back
    lid = [
        add_box('Lid', (w, d, LID_HEIGHT), (0, 0, h + LID_HEIGHT / 2 + 0.2), 'leather', material, round_all=ROUND),
        add_box('LidTrim', (w + 0.8, d + 0.8, 1.6), (0, 0, h + 1.0), 'gold', material, round_corners=ROUND + 0.4),
        add_box('LidVelvet', (w - 3, d - 3, 0.6), (0, 0, h - 0.15), 'velvet', material, round_corners=ROUND - 1.4),
    ]
    hinge = Matrix.Translation((0, d / 2, h)) @ Matrix.Rotation(-LID_OPEN, 4, 'X') @ Matrix.Translation((0, -d / 2, -h))
    for obj in lid:
        obj.data.transform(hinge)
    # the ring stands in a slit of the cushion, facing the front, the stone on top
    top = h + 0.8  # the velvet
    radius, thickness = 5.2, 1.1
    center = Vector((0, 1.0, top + radius - 2.0))
    jewels = [
        add('primitive_torus_add', 'Ring', 'gold', material, True, major_radius=radius, minor_radius=thickness,
            major_segments=20, minor_segments=8, location=center, rotation=(math.pi / 2, 0, 0)),
        add('primitive_ico_sphere_add', 'Stone', 'rose', material, False, scale=(1.0, 1.0, 1.25),
            subdivisions=1, radius=2.6, location=center + Vector((0, 0, radius + 1.6))),
        add('primitive_cylinder_add', 'Setting', 'gold', material, False,
            vertices=8, radius=1.8, depth=1.6, location=center + Vector((0, 0, radius + 0.2))),
    ]
    # the necklace lies on the velvet around the ring, the pendant at the front
    chain_center, chain_radius = Vector((0, 0.5, top + 0.45)), (11.5, 8.0)
    front = chain_center + Vector((0, -chain_radius[1] - 2.4, 0.6))
    jewels += [
        add('primitive_torus_add', 'Chain', 'gold', material, True, scale=(chain_radius[0], chain_radius[1], 10.0),  # the tube about 0.45 thick
            major_radius=1.0, minor_radius=0.045, major_segments=32, minor_segments=6, location=chain_center),
        add('primitive_torus_add', 'Bail', 'gold', material, False, major_radius=0.9, minor_radius=0.35,
            major_segments=8, minor_segments=4, location=front + Vector((0, 2.0, 0)), rotation=(0, math.pi / 2, 0)),
        add('primitive_ico_sphere_add', 'Pendant', 'aqua', material, False, scale=(0.9, 1.3, 0.6),
            subdivisions=1, radius=2.8, location=front),
    ]
    for obj in bpy.context.scene.objects:
        obj.select_set(True)
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
    for obj in bpy.context.scene.objects:
        obj.data.transform(Matrix.Scale(SCALE, 4))
    return parts + lid + jewels


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

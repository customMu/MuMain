"""Example: the Illusion Shard (item 14/172), the currency of the warden of the Illusion of Noria.

    blender -b -P examples/illusion_shard.py -- out_dir

Writes out_dir/IllusionShard01.bmd and its texture out_dir/illusionshard.OZJ (copy both to src/bin/Data/Item):
a cluster of three faceted violet crystals, about the size of the jewels (Jewel15.bmd: 16 x 12 x 34 units).
The texture is the crystal tile of the whistle atlas (illusion_shard.jpg next to this script, made with Gemini).
"""
import math
import os
import shutil
import sys

import bmesh
import bpy
from mathutils import Matrix, Vector

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
import blender_bmd  # noqa: E402

TEXTURE = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'illusion_shard.jpg')
# crystals: base position, radius, height, tilt (degrees around X, Y), sides
CRYSTALS = [
    ((0.0, 0.0, 0.0), 5.0, 30.0, (0, 0), 6),
    ((5.5, 1.5, -2.0), 3.4, 19.0, (-10, 28), 6),
    ((-5.0, -1.0, -2.0), 3.0, 16.0, (12, -30), 5),
]


def crystal(bm, base, radius, height, tilt, sides):
    """A faceted crystal: a prism with a pointed top and a short point at the bottom; UVs around it."""
    rot = Matrix.Rotation(math.radians(tilt[0]), 4, 'X') @ Matrix.Rotation(math.radians(tilt[1]), 4, 'Y')
    base = Vector(base)

    def point(x, y, z):
        return base + rot @ Vector((x, y, z))

    ring_low = [point(math.cos(2 * math.pi * i / sides) * radius, math.sin(2 * math.pi * i / sides) * radius, height * 0.12)
                for i in range(sides)]
    ring_high = [point(math.cos(2 * math.pi * i / sides) * radius * 0.92, math.sin(2 * math.pi * i / sides) * radius * 0.92, height * 0.72)
                 for i in range(sides)]
    top = point(0, 0, height)
    bottom = point(0, 0, -height * 0.05)
    low = [bm.verts.new(p) for p in ring_low]
    high = [bm.verts.new(p) for p in ring_high]
    v_top = bm.verts.new(top)
    v_bottom = bm.verts.new(bottom)
    uv = bm.loops.layers.uv.verify()
    faces = []
    for i in range(sides):
        j = (i + 1) % sides
        u0, u1 = i / sides, (i + 1) / sides
        faces.append((bm.faces.new((low[i], low[j], high[j], high[i])), [(u0, 0.15), (u1, 0.15), (u1, 0.7), (u0, 0.7)]))
        faces.append((bm.faces.new((high[i], high[j], v_top)), [(u0, 0.7), (u1, 0.7), ((u0 + u1) / 2, 1.0)]))
        faces.append((bm.faces.new((low[j], low[i], v_bottom)), [(u1, 0.15), (u0, 0.15), ((u0 + u1) / 2, 0.0)]))
    for face, coords in faces:
        for loop, coord in zip(face.loops, coords):
            loop[uv].uv = coord


def main(out_dir):
    os.makedirs(out_dir, exist_ok=True)
    out_dir = os.path.abspath(out_dir)
    blender_bmd._reset_scene()
    texture = os.path.join(out_dir, 'illusionshard.jpg')
    shutil.copyfile(TEXTURE, texture)
    material = bpy.data.materials.new('illusionshard')
    if bpy.app.version < (5, 0, 0):
        material.use_nodes = True  # always on from Blender 5
    node = material.node_tree.nodes.new('ShaderNodeTexImage')
    node.image = bpy.data.images.load(texture)
    bm = bmesh.new()
    for spec in CRYSTALS:
        crystal(bm, *spec)
    mesh = bpy.data.meshes.new('IllusionShard')
    bm.normal_update()
    bm.to_mesh(mesh)
    bm.free()
    mesh.shade_flat()
    mesh.materials.append(material)
    obj = bpy.data.objects.new('IllusionShard', mesh)
    bpy.context.scene.collection.objects.link(obj)
    mesh.transform(Matrix.Translation((0, 0, 2.0)))
    blender_bmd.export_bmd(os.path.join(out_dir, 'IllusionShard01.bmd'))


if __name__ == '__main__':
    args = sys.argv[sys.argv.index('--') + 1:] if '--' in sys.argv else sys.argv[1:]
    main(args[0] if args else '.')

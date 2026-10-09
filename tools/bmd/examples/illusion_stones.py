"""Example: the stones of the skill fix option of the Illusion of Noria.

    python examples/illusion_stones_textures.py          (once: the coloured textures)
    blender -b -P examples/illusion_stones.py -- out_dir [preview]

Writes to out_dir (copy all *.bmd and *.OZJ to src/bin/Data/Item):
- IllusionJewel01.bmd - the Jewel of Illusion (14/195): a faceted jewel, violet;
- EchoDK01.bmd ... EchoRF01.bmd - the Echoes (14/173-194): the same jewel in the colour of the class of the skill
  (DK red, DW blue, Elf green, MG magenta, DL gold, SUM rose, RF orange);
- MirageLesser01.bmd / MirageGreater01.bmd - the Mirage Stones (14/196, 14/197): a smooth stone in a ring, another shape
  than the jewels; the Greater one (keeps the level on a fail) is bigger, gold and has two rings.
With "preview", out_dir/illusion_stones_preview.png shows them side by side.
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

TEXTURES = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'illusion_stones')
JEWELS = {
    'IllusionJewel': 'jewelofillusion',
    'EchoDK': 'echo_dk', 'EchoDW': 'echo_dw', 'EchoElf': 'echo_elf', 'EchoMG': 'echo_mg',
    'EchoDL': 'echo_dl', 'EchoSUM': 'echo_sum', 'EchoRF': 'echo_rf',
}
MIRAGE = {
    'MirageLesser': ('miragelesser', 1.0, 1),
    'MirageGreater': ('miragegreater', 1.35, 2),
}
SIDES = 8


def material(out_dir, texture):
    path = os.path.join(out_dir, texture + '.jpg')
    shutil.copyfile(os.path.join(TEXTURES, texture + '.jpg'), path)
    mat = bpy.data.materials.new(texture)
    if bpy.app.version < (5, 0, 0):
        mat.use_nodes = True  # always on from Blender 5
    node = mat.node_tree.nodes.new('ShaderNodeTexImage')
    node.image = bpy.data.images.load(path)
    bsdf = mat.node_tree.nodes.get('Principled BSDF')
    if bsdf is not None:
        mat.node_tree.links.new(node.outputs['Color'], bsdf.inputs['Base Color'])
    return mat


def link(name, bm, mat, flat):
    mesh = bpy.data.meshes.new(name)
    bm.normal_update()
    bm.to_mesh(mesh)
    bm.free()
    if flat:
        mesh.shade_flat()
    else:
        mesh.shade_smooth()
    mesh.materials.append(mat)
    obj = bpy.data.objects.new(name, mesh)
    bpy.context.scene.collection.objects.link(obj)
    return obj


def jewel(name, mat):
    """A faceted jewel standing upright (like the jewels of the game, about 22 x 22 x 36): table, crown, girdle, pavilion."""
    rings = [(5.5, 14.0), (9.0, 9.5), (11.0, 4.0), (11.0, 1.5)]  # (radius, height)
    bm = bmesh.new()
    uv = bm.loops.layers.uv.verify()
    verts = []
    for k, (radius, z) in enumerate(rings):
        offset = math.pi / SIDES if k % 2 else 0.0  # every other ring turned: triangle facets like a cut stone
        verts.append([bm.verts.new((math.cos(2 * math.pi * i / SIDES + offset) * radius,
                                    math.sin(2 * math.pi * i / SIDES + offset) * radius, z)) for i in range(SIDES)])
    top = bm.verts.new((0, 0, 14.0))
    bottom = bm.verts.new((0, 0, -20.0))
    faces = []
    for i in range(SIDES):
        j = (i + 1) % SIDES
        faces.append(bm.faces.new((verts[0][i], verts[0][j], top)))
        for k in range(len(rings) - 1):
            faces.append(bm.faces.new((verts[k + 1][i], verts[k + 1][j], verts[k][j], verts[k][i])))
        faces.append(bm.faces.new((verts[-1][j], verts[-1][i], bottom)))
    for face in faces:
        for loop in face.loops:
            co = loop.vert.co
            loop[uv].uv = (0.5 + math.atan2(co.y, co.x) / (2 * math.pi), (co.z + 20.0) / 34.0)
    return link(name, bm, mat, True)


def mirage(name, mat, scale, ring_count):
    """A smooth flat stone with a thin ring (two crossed rings on the Greater one)."""
    bm = bmesh.new()
    bmesh.ops.create_uvsphere(bm, u_segments=20, v_segments=12, radius=10.0)
    bmesh.ops.scale(bm, vec=(1.0, 0.75, 1.15), verts=bm.verts)
    uv = bm.loops.layers.uv.verify()
    for face in bm.faces:
        for loop in face.loops:
            co = loop.vert.co
            loop[uv].uv = (0.5 + math.atan2(co.y, co.x) / (2 * math.pi), 0.5 + co.z / 24.0)
    for r in range(ring_count):
        ring = bmesh.new()
        bmesh.ops.create_circle(ring, segments=24, radius=14.0)
        geom = bmesh.ops.extrude_edge_only(ring, edges=ring.edges)['geom']
        new_verts = [g for g in geom if isinstance(g, bmesh.types.BMVert)]
        bmesh.ops.scale(ring, vec=(0.88, 0.88, 1.0), verts=new_verts)
        bmesh.ops.translate(ring, vec=(0, 0, 0.0), verts=new_verts)
        bmesh.ops.solidify(ring, geom=ring.faces, thickness=1.2)
        tilt = Matrix.Rotation(math.radians(70), 4, 'X') @ Matrix.Rotation(math.radians(90 * r + 20), 4, 'Z')
        bmesh.ops.transform(ring, matrix=tilt, verts=ring.verts)
        ring_uv = ring.loops.layers.uv.verify()
        for face in ring.faces:
            for loop in face.loops:
                loop[ring_uv].uv = (0.9, 0.9)  # a bright spot of the texture
        mesh = bpy.data.meshes.new('ring')
        ring.to_mesh(mesh)
        ring.free()
        bm.from_mesh(mesh)
    bmesh.ops.scale(bm, vec=(scale, scale, scale), verts=bm.verts)
    return link(name, bm, mat, False)


def export(obj, out_dir, name):
    for other in list(bpy.context.scene.objects):
        other.hide_set(other != obj)
        other.hide_render = other != obj
    keep = [o for o in bpy.context.scene.objects if o != obj]
    for o in keep:
        bpy.context.scene.collection.objects.unlink(o)
    blender_bmd.export_bmd(os.path.join(out_dir, name + '01.bmd'))
    for o in keep:
        bpy.context.scene.collection.objects.link(o)


def main(out_dir, preview):
    os.makedirs(out_dir, exist_ok=True)
    out_dir = os.path.abspath(out_dir)
    blender_bmd._reset_scene()
    built = []
    for name, texture in JEWELS.items():
        built.append((name, jewel(name, material(out_dir, texture))))
    for name, (texture, scale, rings) in MIRAGE.items():
        built.append((name, mirage(name, material(out_dir, texture), scale, rings)))
    for name, obj in built:
        export(obj, out_dir, name)
    if preview:
        scene = bpy.context.scene
        for i, (_, obj) in enumerate(built):
            obj.hide_set(False)
            obj.hide_render = False
            obj.location = (i * 34.0 - 153.0, 0, 0)
        cam = bpy.data.objects.new('cam', bpy.data.cameras.new('cam'))
        scene.collection.objects.link(cam)
        cam.location = (0, -430, 30)
        cam.rotation_euler = (Vector((0, 0, 0)) - cam.location).to_track_quat('-Z', 'Y').to_euler()
        scene.camera = cam
        sun = bpy.data.objects.new('sun', bpy.data.lights.new('sun', 'SUN'))
        sun.data.energy = 4
        sun.rotation_euler = (math.radians(50), 0, math.radians(30))
        scene.collection.objects.link(sun)
        scene.render.engine = 'BLENDER_EEVEE'
        scene.render.resolution_x, scene.render.resolution_y = 1600, 260
        scene.render.filepath = os.path.join(out_dir, 'illusion_stones_preview.png')
        bpy.ops.render.render(write_still=True)


if __name__ == '__main__':
    args = sys.argv[sys.argv.index('--') + 1:] if '--' in sys.argv else sys.argv[1:]
    main(args[0] if args else '.', len(args) > 1 and args[1] == 'preview')

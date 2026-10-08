"""Example: the Whistle of the Veil (item 14/171), the quest item of the Illusion of Noria.

    blender -b -P examples/whistle.py -- out_dir [preview]

Writes out_dir/Whistle01.bmd and its texture out_dir/whistle.OZJ (copy both to src/bin/Data/Item): a small curved
horn whistle of dark carved bone with gold bands, a gold mouthpiece, a violet crystal in a gold claw setting on top
and a gold loop for a cord. The texture is an atlas of three tiles made with Gemini (whistle_atlas.jpg next to this
script): carved bone on the left half, engraved gold bottom right, violet crystal top right; every part is unwrapped
into the tile of its material. The model stands upright like the scrolls (1 x 2 slots, about 50 units, the bell on top).
With "preview", out_dir/whistle_preview.png is rendered too.
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

ATLAS = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'whistle_atlas.jpg')
args = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
OUT = os.path.abspath(args[0] if args else '.')
PREVIEW = len(args) > 1 and args[1] == "preview"
os.makedirs(OUT, exist_ok=True)

# atlas zones (u0, v0, u1, v1): bone left half, gold bottom right, gem top right
ZONES = {"bone": (0.0, 0.0, 0.5, 1.0), "gold": (0.5, 0.0, 1.0, 0.5), "gem": (0.5, 0.5, 1.0, 1.0)}

blender_bmd._reset_scene()
scene = bpy.context.scene


def curve_point(t):
    """Centre line of the horn: 0 = mouthpiece, 1 = bell; a gentle upward curve along +Y (length 14)."""
    y = 14.0 * t
    z = 1.6 * math.sin(t * math.pi * 0.9) + 0.9 * t * t
    return Vector((0.0, y, z))


def curve_frame(t):
    d = (curve_point(min(1, t + 0.01)) - curve_point(max(0, t - 0.01))).normalized()
    side = Vector((1, 0, 0))
    up = d.cross(side).normalized()
    return d, side, -up if up.z < 0 else up


def radius(t):
    return 0.75 + 0.55 * t + 1.1 * t ** 3.2


def tube(name, t0, t1, rings, segs, rad, zone, flare=0.0):
    bm = bmesh.new()
    rows = []
    for i in range(rings + 1):
        t = t0 + (t1 - t0) * i / rings
        c = curve_point(t)
        _, side, up = curve_frame(t)
        r = rad(t)
        row = []
        for j in range(segs):
            a = 2 * math.pi * j / segs
            row.append(bm.verts.new(c + side * math.cos(a) * r + up * math.sin(a) * r))
        rows.append(row)
    for i in range(rings):
        for j in range(segs):
            bm.faces.new((rows[i][j], rows[i][(j + 1) % segs], rows[i + 1][(j + 1) % segs], rows[i + 1][j]))
    # inner wall of the bell (open end) and a cap at the mouthpiece
    cap = bm.faces.new(list(reversed(rows[0])))
    inner = []
    c = curve_point(t1)
    d, side, up = curve_frame(t1)
    for j in range(segs):
        a = 2 * math.pi * j / segs
        inner.append(bm.verts.new(c - d * 0.9 + side * math.cos(a) * rad(t1) * 0.78 + up * math.sin(a) * rad(t1) * 0.78))
    for j in range(segs):
        bm.faces.new((rows[-1][j], inner[j], inner[(j + 1) % segs], rows[-1][(j + 1) % segs]))
    bm.faces.new(inner)
    return finish(bm, name, zone)


def ring(name, t, r_major, r_minor, zone, segs=20, minor_segs=6):
    """A gold band around the horn at t (a torus following the horn frame)."""
    bm = bmesh.new()
    c = curve_point(t)
    d, side, up = curve_frame(t)
    rows = []
    for i in range(segs):
        a = 2 * math.pi * i / segs
        ring_dir = side * math.cos(a) + up * math.sin(a)
        centre = c + ring_dir * r_major
        row = []
        for j in range(minor_segs):
            b = 2 * math.pi * j / minor_segs
            row.append(bm.verts.new(centre + ring_dir * math.cos(b) * r_minor + d * math.sin(b) * r_minor * 1.6))
        rows.append(row)
    for i in range(segs):
        for j in range(minor_segs):
            bm.faces.new((rows[i][j], rows[(i + 1) % segs][j], rows[(i + 1) % segs][(j + 1) % minor_segs], rows[i][(j + 1) % minor_segs]))
    return finish(bm, name, zone)


def loop(name, centre, normal_axis, r_major, r_minor, zone, segs=16, minor_segs=6):
    bm = bmesh.new()
    a_axis = Vector((0, 1, 0)) if abs(normal_axis.y) < 0.9 else Vector((1, 0, 0))
    u = normal_axis.cross(a_axis).normalized()
    v = normal_axis.cross(u).normalized()
    rows = []
    for i in range(segs):
        a = 2 * math.pi * i / segs
        rd = u * math.cos(a) + v * math.sin(a)
        row = []
        for j in range(minor_segs):
            b = 2 * math.pi * j / minor_segs
            row.append(bm.verts.new(centre + rd * (r_major + math.cos(b) * r_minor) + normal_axis * math.sin(b) * r_minor))
        rows.append(row)
    for i in range(segs):
        for j in range(minor_segs):
            bm.faces.new((rows[i][j], rows[(i + 1) % segs][j], rows[(i + 1) % segs][(j + 1) % minor_segs], rows[i][(j + 1) % minor_segs]))
    return finish(bm, name, zone)


def gem(name, centre, size, zone):
    """An elongated faceted crystal (8-sided bipyramid) standing on the horn."""
    bm = bmesh.new()
    top = bm.verts.new(centre + Vector((0, 0, size * 2.2)))
    bottom = bm.verts.new(centre + Vector((0, 0, -size * 0.6)))
    mid = [bm.verts.new(centre + Vector((math.cos(a) * size, math.sin(a) * size * 0.8, size * 0.55)))
           for a in (2 * math.pi * i / 8 for i in range(8))]
    for i in range(8):
        bm.faces.new((mid[i], mid[(i + 1) % 8], top))
        bm.faces.new((mid[(i + 1) % 8], mid[i], bottom))
    return finish(bm, name, zone)


def setting(name, centre, zone):
    """Gold claws holding the crystal: 4 small prongs + a base disc."""
    bm = bmesh.new()
    base = []
    for i in range(10):
        a = 2 * math.pi * i / 10
        base.append(bm.verts.new(centre + Vector((math.cos(a) * 1.05, math.sin(a) * 0.9, -0.15))))
    top_ring = [bm.verts.new(v.co + Vector((0, 0, 0.35))) for v in base]
    for i in range(10):
        bm.faces.new((base[i], base[(i + 1) % 10], top_ring[(i + 1) % 10], top_ring[i]))
    bm.faces.new(list(reversed(base)))
    bm.faces.new(top_ring)
    for i in range(4):
        a = 2 * math.pi * i / 4 + math.pi / 4
        p = centre + Vector((math.cos(a) * 0.8, math.sin(a) * 0.7, 0.2))
        q = centre + Vector((math.cos(a) * 0.62, math.sin(a) * 0.52, 1.25))
        w = Vector((-math.sin(a), math.cos(a), 0)) * 0.13
        o = Vector((math.cos(a), math.sin(a), 0)) * 0.1
        vs = [bm.verts.new(x) for x in (p - w, p + w, q + w * 0.4, q - w * 0.4, p - w + o, p + w + o, q + w * 0.4 + o, q - w * 0.4 + o)]
        for f in ((0, 1, 2, 3), (5, 4, 7, 6), (0, 4, 5, 1), (1, 5, 6, 2), (2, 6, 7, 3), (3, 7, 4, 0)):
            bm.faces.new([vs[k] for k in f])
    return finish(bm, name, zone)


def finish(bm, name, zone):
    mesh = bpy.data.meshes.new(name)
    bm.normal_update()
    bm.to_mesh(mesh)
    bm.free()
    obj = bpy.data.objects.new(name, mesh)
    obj["zone"] = zone
    scene.collection.objects.link(obj)
    return obj


parts = [
    tube("horn", 0.06, 1.0, 18, 14, radius, "bone"),
    tube("mouthpiece", 0.0, 0.1, 3, 16, lambda t: 0.62 + 0.4 * t, "gold"),
    ring("band0", 0.12, radius(0.12) + 0.05, 0.18, "gold"),
    ring("band1", 0.55, radius(0.55) + 0.05, 0.2, "gold"),
    ring("band2", 0.985, radius(0.985) + 0.06, 0.26, "gold"),
]
c = curve_point(0.36)
_, _, up = curve_frame(0.36)
top = c + up * (radius(0.36) - 0.05)
parts.append(setting("setting", top, "gold"))
parts.append(gem("crystal", top + Vector((0, 0, 0.5)), 0.85, "gem"))
cl = curve_point(0.8)
_, side, up = curve_frame(0.8)
parts.append(loop("loop", cl - up * (radius(0.8) + 0.55), side, 0.7, 0.17, "gold"))

# UVs: smart project per part, packed into the zone of its material
for obj in parts:
    bpy.ops.object.select_all(action="DESELECT")
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj
    bpy.ops.object.mode_set(mode="EDIT")
    bpy.ops.mesh.select_all(action="SELECT")
    bpy.ops.uv.smart_project(angle_limit=math.radians(55), island_margin=0.02)
    bpy.ops.object.mode_set(mode="OBJECT")
    obj.data.shade_smooth() if obj["zone"] != "gem" else obj.data.shade_flat()

# keep the zone layout: islands of a zone share the zone rectangle
for zone in ZONES:
    objs = [o for o in parts if o["zone"] == zone]
    u0, v0, u1, v1 = ZONES[zone]
    n = len(objs)
    cols = math.ceil(math.sqrt(n))
    rows_n = math.ceil(n / cols)
    for k, obj in enumerate(objs):
        cx, cy = k % cols, k // cols
        cu0 = u0 + (u1 - u0) * cx / cols
        cv0 = v0 + (v1 - v0) * cy / rows_n
        cw, ch = (u1 - u0) / cols, (v1 - v0) / rows_n
        uv = obj.data.uv_layers.active.data
        for d in uv:
            d.uv = (cu0 + 0.01 + d.uv[0] * (cw - 0.02), cv0 + 0.01 + d.uv[1] * (ch - 0.02))

bpy.ops.object.select_all(action="SELECT")
bpy.context.view_layer.objects.active = parts[0]
bpy.ops.object.join()
whistle = bpy.context.view_layer.objects.active

# one material with the atlas (JPEG: the export writes whistle.OZJ)
texture = os.path.join(OUT, "whistle.jpg")
shutil.copyfile(ATLAS, texture)
mat = bpy.data.materials.new("whistle")
if bpy.app.version < (5, 0, 0):
    mat.use_nodes = True  # always on from Blender 5
node = mat.node_tree.nodes.new("ShaderNodeTexImage")
node.image = bpy.data.images.load(texture)
whistle.data.materials.clear()
whistle.data.materials.append(mat)
centre = sum((Vector(v.co) for v in whistle.data.vertices), Vector()) / len(whistle.data.vertices)
whistle.data.transform(Matrix.Translation(-centre))
whistle.data.transform(Matrix.Rotation(math.radians(90), 4, "X"))  # the bell (+Y) up (+Z), the crystal to -Y
whistle.data.transform(Matrix.Scale(3.3, 4))
blender_bmd.export_bmd(os.path.join(OUT, "Whistle01.bmd"))

if PREVIEW:
    bsdf = mat.node_tree.nodes.get("Principled BSDF") or mat.node_tree.nodes.new("ShaderNodeBsdfPrincipled")
    mat.node_tree.links.new(node.outputs["Color"], bsdf.inputs["Base Color"])
    scene = bpy.context.scene
    cam = bpy.data.objects.new("cam", bpy.data.cameras.new("cam"))
    scene.collection.objects.link(cam)
    cam.location = (55, -60, 12)
    cam.rotation_euler = (Vector((0, 0, 0)) - cam.location).to_track_quat("-Z", "Y").to_euler()
    scene.camera = cam
    for loc, power in (((40, -30, 50), 60000), ((-30, 40, 20), 20000)):
        light = bpy.data.objects.new("l", bpy.data.lights.new("l", "POINT"))
        light.data.energy = power
        light.location = loc
        scene.collection.objects.link(light)
    scene.render.engine = "BLENDER_EEVEE"
    scene.render.resolution_x = scene.render.resolution_y = 512
    scene.render.filepath = os.path.join(OUT, "whistle_preview.png")
    bpy.ops.render.render(write_still=True)

"""Example: a rigged, animated low-poly monster built from code and exported as a game model.

    blender -b -P examples/golem.py -- out_dir
    (or: python examples/golem.py out_dir   with the bpy module from pip)

Writes out_dir/golem.blend, out_dir/Monster_Golem.bmd and its texture out_dir/golem.OZT.
The actions follow the order of the monster actions of the client (MONSTER01_STOP1 ... MONSTER01_DIE).
"""
import math
import os
import sys

import bpy
import numpy
from mathutils import Euler, Vector

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
import blender_bmd  # noqa: E402

TEXTURE_SIZE = 64
BONES = (  # name, head, tail, parent
    ('Bip01', (0, 0, 90), (0, 0, 110), None),
    ('Spine', (0, 0, 110), (0, 0, 160), 'Bip01'),
    ('Head', (0, 0, 160), (0, 0, 190), 'Spine'),
    ('L_Arm', (35, 0, 150), (55, 0, 90), 'Spine'),
    ('R_Arm', (-35, 0, 150), (-55, 0, 90), 'Spine'),
    ('L_Leg', (18, 0, 90), (18, 0, 0), 'Bip01'),
    ('R_Leg', (-18, 0, 90), (-18, 0, 0), 'Bip01'),
)
PARTS = (  # primitive, bone, location, scale, options
    ('primitive_ico_sphere_add', 'Bip01', (0, 0, 100), (30, 22, 18), {'subdivisions': 1}),
    ('primitive_ico_sphere_add', 'Spine', (0, 0, 135), (38, 26, 30), {'subdivisions': 1}),
    ('primitive_ico_sphere_add', 'Head', (0, 0, 178), (16, 16, 16), {'subdivisions': 1}),
    ('primitive_cone_add', 'L_Arm', (48, 0, 115), (12, 12, 35), {'vertices': 6}),
    ('primitive_cone_add', 'R_Arm', (-48, 0, 115), (12, 12, 35), {'vertices': 6}),
    ('primitive_cylinder_add', 'L_Leg', (18, 0, 45), (11, 11, 45), {'vertices': 6}),
    ('primitive_cylinder_add', 'R_Leg', (-18, 0, 45), (11, 11, 45), {'vertices': 6}),
)


def wave(t, amount):
    return amount * math.sin(t * 2 * math.pi)


def swing(t, amount):
    return amount * math.sin(t * math.pi)


# name: (keys, looped, pose(t) -> {bone: (rotation XYZ in armature axes, offset)})
ACTIONS = {
    '00_STOP1': (16, True, lambda t: {'Spine': ((wave(t, 0.05), 0, 0), (0, 0, wave(t, 2)))}),
    '01_STOP2': (16, True, lambda t: {'Head': ((0, 0, wave(t, 0.6)), (0, 0, 0))}),
    '02_WALK': (16, True, lambda t: {'L_Leg': ((wave(t, 0.5), 0, 0), (0, 0, 0)),
                                     'R_Leg': ((wave(t, -0.5), 0, 0), (0, 0, 0)),
                                     'L_Arm': ((wave(t, -0.4), 0, 0), (0, 0, 0)),
                                     'R_Arm': ((wave(t, 0.4), 0, 0), (0, 0, 0))}),
    '03_ATTACK1': (12, False, lambda t: {'R_Arm': ((swing(t, -2.2), 0, 0), (0, 0, 0)),
                                         'Spine': ((swing(t, 0.25), 0, 0), (0, 0, 0))}),
    '04_ATTACK2': (12, False, lambda t: {'L_Arm': ((swing(t, -2.2), 0, 0), (0, 0, 0)),
                                         'Spine': ((swing(t, 0.25), 0, 0), (0, 0, 0))}),
    '05_SHOCK': (8, False, lambda t: {'Spine': ((swing(t, -0.4), 0, 0), (0, 0, 0))}),
    '06_DIE': (16, False, lambda t: {'Bip01': ((-1.5 * min(t * 1.3, 1), 0, 0), (0, 0, -70 * min(t * 1.3, 1)))}),
}


def build_rig():
    data = bpy.data.armatures.new('Rig')
    rig = bpy.data.objects.new('Rig', data)
    bpy.context.scene.collection.objects.link(rig)
    bpy.context.view_layer.objects.active = rig
    bpy.ops.object.mode_set(mode='EDIT')
    for name, head, tail, parent in BONES:
        eb = data.edit_bones.new(name)
        eb.head, eb.tail = head, tail
        if parent:
            eb.parent = data.edit_bones[parent]
    bpy.ops.object.mode_set(mode='OBJECT')
    return rig


def build_texture(out_dir):
    """A stone-blue noise texture; the export writes it as golem.OZT."""
    image = bpy.data.images.new('golem', TEXTURE_SIZE, TEXTURE_SIZE)
    rng = numpy.random.default_rng(1)
    shade = 0.75 + 0.25 * rng.random((TEXTURE_SIZE, TEXTURE_SIZE))
    rgba = numpy.stack([0.35 * shade, 0.55 * shade, 0.9 * shade, numpy.ones_like(shade)], axis=-1)
    image.pixels.foreach_set(rgba.astype(numpy.float32).ravel())
    image.filepath_raw = os.path.join(out_dir, 'golem.png')
    image.file_format = 'PNG'
    image.save()
    material = bpy.data.materials.new('golem')
    if bpy.app.version < (5, 0, 0):
        material.use_nodes = True  # always on from Blender 5
    node = material.node_tree.nodes.new('ShaderNodeTexImage')
    node.image = image
    material.node_tree.links.new(node.outputs['Color'], material.node_tree.nodes['Principled BSDF'].inputs['Base Color'])
    return material


def build_parts(rig, material):
    """One rigid part per bone: the client binds every vertex to exactly one bone."""
    for primitive, bone, location, scale, options in PARTS:
        getattr(bpy.ops.mesh, primitive)(location=location, **options)
        obj = bpy.context.active_object
        obj.scale = scale
        bpy.ops.object.transform_apply(scale=True)
        obj.data.materials.append(material)
        obj.vertex_groups.new(name=bone).add(range(len(obj.data.vertices)), 1.0, 'REPLACE')
        obj.modifiers.new('Armature', 'ARMATURE').object = rig


def pose(rig, offsets):
    for pb in rig.pose.bones:
        pb.rotation_mode = 'XYZ'
        pb.rotation_euler = (0, 0, 0)
        pb.location = (0, 0, 0)
    for name, (rotation, offset) in offsets.items():
        pb = rig.pose.bones[name]
        axes = pb.bone.matrix_local.to_3x3()
        pb.rotation_euler = (axes.inverted() @ Euler(rotation, 'XYZ').to_matrix() @ axes).to_euler('XYZ')
        pb.location = axes.inverted() @ Vector(offset)


def build_actions(rig):
    rig.animation_data_create()
    for name, (keys, looped, fn) in ACTIONS.items():
        action = bpy.data.actions.new(name)
        action.use_fake_user = True
        rig.animation_data.action = action
        for frame in range(keys):
            pose(rig, fn(frame / keys if looped else frame / (keys - 1)))  # a loop must not repeat its first key
            for pb in rig.pose.bones:
                pb.keyframe_insert('location', frame=frame)
                pb.keyframe_insert('rotation_euler', frame=frame)


def main(out_dir):
    os.makedirs(out_dir, exist_ok=True)
    out_dir = os.path.abspath(out_dir)
    blender_bmd._reset_scene()
    rig = build_rig()
    build_parts(rig, build_texture(out_dir))
    build_actions(rig)
    bpy.ops.wm.save_as_mainfile(filepath=os.path.join(out_dir, 'golem.blend'))
    blender_bmd.export_bmd(os.path.join(out_dir, 'Monster_Golem.bmd'))


if __name__ == '__main__':
    args = sys.argv[sys.argv.index('--') + 1:] if '--' in sys.argv else sys.argv[1:]
    main(args[0] if args else '.')

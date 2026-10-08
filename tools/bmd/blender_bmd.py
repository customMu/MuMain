"""BMD <-> Blender: import game models to edit them, export Blender scenes as game models, render previews.

Run with Blender (4.4 or newer):
    blender -b -P blender_bmd.py -- import Data/Monster/Monster01.bmd monster01.blend
    blender -b -P blender_bmd.py -- export monster01.blend Monster01.bmd [--scale 100] [--encrypt]
    blender -b -P blender_bmd.py -- preview Monster01.bmd preview.png [--action 0] [--frames 6]
or with the bpy module from pip (`pip install bpy`): python blender_bmd.py import ...

import writes .blend, .glb/.gltf or .fbx (by the extension) and unpacks the textures next to it.
export takes .blend, .glb/.gltf, .fbx or .obj and writes the textures as OZJ/OZT next to the .bmd.
See README.md for how a scene has to look (bones, actions, materials).
"""
import argparse
import os
import sys

import bpy
import numpy  # bundled with Blender
from mathutils import Matrix, Vector

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import bmd  # noqa: E402
import ozj  # noqa: E402

BONE_LENGTH = 5.0           # display length of imported bones, MU units
ROOT_BONE = 'Bip01'
KEY_PRECISION = 5           # decimals used to merge equal normals / texcoords
PREVIEW_SIZE = 320
JPEG_EXTS = ('.jpg', '.jpeg')


# ---------- small helpers ----------

def _matrix(m34):
    return Matrix((m34[0], m34[1], m34[2], (0.0, 0.0, 0.0, 1.0)))


def _local_matrix(position, rotation):
    m = Matrix(bmd.euler_to_matrix(rotation)).to_4x4()
    m.translation = Vector(position)
    return m


def _safe_name(name):
    """Blender names must be valid UTF-8; some game names hold cut Korean characters."""
    return name.encode('utf-8', errors='replace').decode('utf-8')


def _keep_name(id_data, original):
    """Remembers the exact game name when Blender changed it (invalid UTF-8, duplicate bone names)."""
    if id_data.name != original:
        id_data['mu_name_hex'] = original.encode('cp949', errors='surrogateescape').hex()
        id_data['mu_name_blender'] = id_data.name


def _game_name(id_data, blender_name):
    """The game name of an imported item, unless it was renamed in Blender since."""
    if 'mu_name_hex' in id_data and id_data.get('mu_name_blender') == id_data.name:
        return bytes.fromhex(id_data['mu_name_hex']).decode('cp949', errors='surrogateescape')
    return blender_name


def _reset_scene():
    bpy.ops.wm.read_factory_settings(use_empty=True)


def _new_action(name, rig):
    """An action with one slot for the armature, Blender 4.4+ layered API."""
    from bpy_extras import anim_utils
    action = bpy.data.actions.new(name)
    action.use_fake_user = True
    slot = action.slots.new(id_type='OBJECT', name=rig.name)
    channelbag = anim_utils.action_ensure_channelbag_for_slot(action, slot)
    return action, slot, channelbag


def _assign_action(rig, action):
    rig.animation_data_create()
    rig.animation_data.action = action
    if action.slots:
        rig.animation_data.action_slot = action.slots[0]


# ---------- import ----------

def _import_armature(model, rest):
    data = bpy.data.armatures.new('Armature')
    rig = bpy.data.objects.new('Armature', data)
    bpy.context.scene.collection.objects.link(rig)
    bpy.context.view_layer.objects.active = rig
    bpy.ops.object.mode_set(mode='EDIT')
    names = []
    for i, bone in enumerate(model.bones):
        eb = data.edit_bones.new(_safe_name(bone.name) or f'Dummy{i:02d}')
        eb.head, eb.tail = (0, 0, 0), (0, BONE_LENGTH, 0)
        eb.matrix = rest[i]
        names.append(eb.name)
    for i, bone in enumerate(model.bones):
        if not bone.dummy and 0 <= bone.parent < len(names):
            data.edit_bones[names[i]].parent = data.edit_bones[names[bone.parent]]
    bpy.ops.object.mode_set(mode='OBJECT')
    for i, bone in enumerate(model.bones):
        data.bones[names[i]]['mu_index'] = i
        data.bones[names[i]]['mu_dummy'] = bone.dummy
        _keep_name(data.bones[names[i]], bone.name)
    return rig, names


def _import_actions(model, rig, names, rest):
    rest_local = []
    for i, bone in enumerate(model.bones):
        parent = bone.parent if not bone.dummy and 0 <= bone.parent < len(rest) else -1
        rest_local.append(rest[i] if parent < 0 else rest[parent].inverted() @ rest[i])
    for a, action_info in enumerate(model.actions):
        action, _, channelbag = _new_action(f'{a:03d}', rig)
        action['mu_index'] = a
        action['mu_keys'] = action_info.num_keys
        if action_info.lock_positions:
            action['mu_lock_positions'] = [c for p in action_info.positions for c in p]
        for i, bone in enumerate(model.bones):
            if bone.dummy or action_info.num_keys == 0:
                continue
            positions, rotations = bone.keys[a]
            basis = [rest_local[i].inverted() @ _local_matrix(positions[k], rotations[k])
                     for k in range(action_info.num_keys)]
            _key_bone(channelbag, names[i], basis)
    if bpy.data.actions:
        _assign_action(rig, bpy.data.actions[0])


def _key_bone(channelbag, bone_name, basis):
    path = f'pose.bones["{bone_name}"]'
    loc = [m.to_translation() for m in basis]
    rot = [m.to_quaternion() for m in basis]
    for k in range(1, len(rot)):
        rot[k].make_compatible(rot[k - 1])
    for prop, values, size in (('location', loc, 3), ('rotation_quaternion', rot, 4)):
        for axis in range(size):
            fc = channelbag.fcurves.new(f'{path}.{prop}', index=axis)
            fc.group = channelbag.groups.get(bone_name) or channelbag.groups.new(bone_name)
            fc.keyframe_points.add(len(values))
            coords = [c for k, v in enumerate(values) for c in (k, v[axis])]
            fc.keyframe_points.foreach_set('co', coords)
            fc.keyframe_points.foreach_set('interpolation', [1] * len(values))  # LINEAR, the client lerps keys
            fc.update()


def _texture_material(texture, texture_dir, out_dir):
    material = bpy.data.materials.get(_safe_name(texture)) or bpy.data.materials.new(_safe_name(texture))
    material['mu_texture'] = _safe_name(texture)
    _keep_name(material, texture)
    if material.node_tree and any(n.type == 'TEX_IMAGE' for n in material.node_tree.nodes):
        return material
    found = ozj.find_texture(texture, texture_dir)
    if not found:
        print(f'texture not found: {texture} in {texture_dir}')
        return material
    path = ozj.unpack(found, out_dir) if found.lower().endswith(tuple(ozj.PREFIX_SIZE)) else found
    if bpy.app.version < (5, 0, 0):
        material.use_nodes = True  # always on from Blender 5
    nodes = material.node_tree.nodes
    image_node = nodes.new('ShaderNodeTexImage')
    image_node.image = bpy.data.images.load(path)
    bsdf = nodes['Principled BSDF']
    material.node_tree.links.new(image_node.outputs['Color'], bsdf.inputs['Base Color'])
    if path.lower().endswith('.tga'):
        material.node_tree.links.new(image_node.outputs['Alpha'], bsdf.inputs['Alpha'])
    return material


def _import_mesh(model, index, rig, names, rest, texture_dir, out_dir):
    mesh_info = model.meshes[index]
    verts = [rest[node] @ Vector(pos) for node, pos in mesh_info.vertices]
    faces, loop_uvs, loop_normals = [], [], []
    for t in mesh_info.triangles:
        corners = t.vertices[:t.polygon]
        if len(set(corners)) < len(corners) or max(corners) >= len(verts):
            continue  # degenerate in the game file
        faces.append(corners)
        for c in range(t.polygon):
            u, v = mesh_info.texcoords[t.texcoords[c]]
            loop_uvs.append((u, 1.0 - v))
            node, normal, _ = mesh_info.normals[t.normals[c]]
            loop_normals.append((rest[node].to_3x3() @ Vector(normal)).normalized())
    me = bpy.data.meshes.new(f'Mesh{index:02d}')
    me.from_pydata(verts, [], faces)
    uv_layer = me.uv_layers.new(name='UVMap')
    uv_layer.data.foreach_set('uv', [c for uv in loop_uvs for c in uv])
    me.normals_split_custom_set(loop_normals)
    me.polygons.foreach_set('use_smooth', [True] * len(me.polygons))
    me.materials.append(_texture_material(mesh_info.texture, texture_dir, out_dir))
    obj = bpy.data.objects.new(f'Mesh{index:02d}', me)
    obj['mu_texture_index'] = mesh_info.texture_index
    bpy.context.scene.collection.objects.link(obj)
    for vi, (node, _) in enumerate(mesh_info.vertices):
        group = obj.vertex_groups.get(names[node]) or obj.vertex_groups.new(name=names[node])
        group.add([vi], 1.0, 'REPLACE')
    obj.parent = rig
    obj.modifiers.new('Armature', 'ARMATURE').object = rig


def import_bmd(path, out_dir=None):
    """Loads a BMD into the current (emptied) scene; returns the armature object."""
    model = bmd.read(path)
    out_dir = out_dir or os.path.dirname(os.path.abspath(path))
    has_pose = model.actions and model.actions[0].num_keys > 0
    rest = [_matrix(m) for m in bmd.bone_matrices(model, 0, 0)] if has_pose else [Matrix()] * len(model.bones)
    rig, names = _import_armature(model, rest)
    # Blender rebuilds a bone from head, tail and roll, which drifts near straight up/down bones:
    # place the meshes and the keys on the bones Blender really made.
    rest = [rig.data.bones[n].matrix_local.copy() for n in names]
    rig['mu_name'] = _safe_name(model.name)
    _keep_name(rig, model.name)
    for i in range(len(model.meshes)):
        _import_mesh(model, i, rig, names, rest, os.path.dirname(os.path.abspath(path)), out_dir)
    _import_actions(model, rig, names, rest)
    return rig


# ---------- export ----------

def _ordered_bones(rig):
    """Bones in BMD order: the stored game order when the model was imported, else parents first."""
    bones = list(rig.data.bones)
    if all('mu_index' in b for b in bones):
        return sorted(bones, key=lambda b: b['mu_index'])
    ordered = []
    pending = [b for b in bones if b.parent is None]
    while pending:  # depth first, a parent always before its children
        bone = pending.pop(0)
        ordered.append(bone)
        pending[:0] = list(bone.children)
    return ordered


def _export_actions(rig):
    actions = list(bpy.data.actions)
    if any('mu_index' in a for a in actions):
        actions = [a for a in actions if 'mu_index' in a]
        return sorted(actions, key=lambda a: a['mu_index'])
    return sorted(actions, key=lambda a: a.name)


def _frames(action):
    start, end = int(action.frame_range[0]), int(action.frame_range[1])
    if 'mu_keys' in action:
        return list(range(start, start + action['mu_keys']))
    return list(range(start, end + 1))


def _sample_bones(model, rig, bones, scale):
    """Keys of every action: positions/rotations of each bone relative to its parent."""
    index = {b.name: i for i, b in enumerate(bones)}
    for bone in bones:
        dummy = bool(bone.get('mu_dummy', False))
        parent = index[bone.parent.name] if bone.parent else -1
        model.bones.append(bmd.Bone('' if dummy else _game_name(bone, bone.name), parent, dummy))
    actions = _export_actions(rig) if rig.animation_data is not None or bpy.data.actions else []
    for action in actions:
        _assign_action(rig, action)
        frames = _frames(action)
        lock = list(action.get('mu_lock_positions', []))
        info = bmd.Action(len(frames), bool(lock))
        info.positions = [tuple(c * scale for c in lock[i:i + 3]) for i in range(0, len(lock), 3)]
        model.actions.append(info)
        samples = [_pose_at(rig, bones, f) for f in frames]
        for i, bone in enumerate(model.bones):
            if bone.dummy:
                continue
            bone.keys.append(([tuple(c * scale for c in s[i][0]) for s in samples], [s[i][1] for s in samples]))


def _pose_at(rig, bones, frame):
    """Position and rotation of every bone relative to its parent, computed in double precision."""
    bpy.context.scene.frame_set(frame)
    world = numpy.array(rig.matrix_world, dtype=numpy.float64)
    result = []
    for bone in bones:
        pb = rig.pose.bones[bone.name]
        m = numpy.array(pb.matrix, dtype=numpy.float64)
        m = world @ m if pb.parent is None else numpy.linalg.inv(numpy.array(pb.parent.matrix, dtype=numpy.float64)) @ m
        rotation = m[:3, :3] / numpy.linalg.norm(m[:3, :3], axis=0)  # drop any scale
        result.append((tuple(m[:3, 3]), bmd.matrix_to_euler(rotation.tolist())))
    return result


def _static_root(model):
    """A model without an armature gets one root bone and one action with one key, like the static objects."""
    model.bones.append(bmd.Bone(ROOT_BONE, -1, False, [([(0.0, 0.0, 0.0)], [(0.0, 0.0, 0.0)])]))
    model.actions.append(bmd.Action(1))


def _vertex_bone(obj, vertex, bone_index, fallback):
    best, weight = fallback, 0.0
    for g in vertex.groups:
        name = obj.vertex_groups[g.group].name
        if name in bone_index and g.weight > weight:
            best, weight = bone_index[name], g.weight
    return best


def _material_texture(material, out_dir, written):
    """Texture name stored in the BMD; writes the image as OZJ/OZT next to the model once."""
    if material is None:
        return 'white.jpg'
    image = next((n.image for n in (material.node_tree.nodes if material.node_tree else [])
                  if n.type == 'TEX_IMAGE' and n.image), None)
    if image is None:
        return _game_name(material, material.get('mu_texture', material.name + '.jpg'))
    stem = os.path.splitext(material.get('mu_texture', image.name))[0]
    jpeg = _jpeg_bytes(image)
    texture = stem + ('.jpg' if jpeg else '.tga')
    if texture not in written:
        written.add(texture)
        _write_texture(image, jpeg, os.path.join(out_dir, texture))
    return texture


def _jpeg_bytes(image):
    """The original JPEG of an image (a file or packed into a .blend/.glb), None for other formats."""
    if image.packed_file and image.file_format == 'JPEG':
        return bytes(image.packed_file.data)
    source = bpy.path.abspath(image.filepath) if image.filepath else ''
    if source.lower().endswith(JPEG_EXTS) and os.path.isfile(source):
        with open(source, 'rb') as f:
            return f.read()
    return None


def _write_texture(image, jpeg, path):
    if jpeg:
        data = jpeg
    else:
        width, height = image.size
        pixels = numpy.empty(width * height * 4, dtype=numpy.float32)
        image.pixels.foreach_get(pixels)
        rgba = (numpy.clip(pixels, 0.0, 1.0) * 255 + 0.5).astype(numpy.uint8).tobytes()  # bottom-up already
        ozj.write_tga(path, width, height, rgba)
        with open(path, 'rb') as f:
            data = f.read()
        os.remove(path)
    packed_ext = ozj.PACKED_EXT[os.path.splitext(path)[1].lower()]
    with open(os.path.splitext(path)[0] + packed_ext, 'wb') as f:
        f.write(ozj.pack_bytes(data, packed_ext))


def _export_meshes(rig, bones, scale, out_dir):
    """BMD meshes from the visible mesh objects, evaluated (other modifiers applied) in the rest pose."""
    if rig:
        rig.data.pose_position = 'REST'
    bpy.context.view_layer.update()
    depsgraph = bpy.context.evaluated_depsgraph_get()
    builders, written = {}, set()
    for obj in [o for o in bpy.context.scene.objects if o.type == 'MESH' and o.visible_get()]:
        evaluated = obj.evaluated_get(depsgraph)
        _export_mesh(obj, evaluated.to_mesh(), rig, bones, scale, out_dir, builders, written)
        evaluated.to_mesh_clear()
    if rig:
        rig.data.pose_position = 'POSE'
    ordered = sorted(builders.values(), key=lambda b: b.mesh.texture_index)
    return ordered, written


class _MeshBuilder:
    """Collects one BMD mesh (one material), merging equal vertices, normals and texcoords."""

    def __init__(self, texture, texture_index):
        self.mesh = bmd.Mesh(texture, texture_index)
        self.vertex_map, self.normal_map, self.uv_map = {}, {}, {}

    def vertex(self, key, node, pos):
        if key not in self.vertex_map:
            self.vertex_map[key] = len(self.mesh.vertices)
            self.mesh.vertices.append((node, pos))
        return self.vertex_map[key]

    def normal(self, vertex, node, normal):
        key = (vertex, tuple(round(c, KEY_PRECISION) for c in normal))
        if key not in self.normal_map:
            self.normal_map[key] = len(self.mesh.normals)
            self.mesh.normals.append((node, tuple(normal), vertex))
        return self.normal_map[key]

    def texcoord(self, uv):
        key = tuple(round(c, KEY_PRECISION) for c in uv)
        if key not in self.uv_map:
            self.uv_map[key] = len(self.mesh.texcoords)
            self.mesh.texcoords.append(key)
        return self.uv_map[key]


def _export_mesh(obj, me, rig, bones, scale, out_dir, builders, written):
    bone_index = {b.name: i for i, b in enumerate(bones)}
    inv_rest = [(rig.matrix_world @ b.matrix_local).inverted() if rig else Matrix() for b in bones] or [Matrix()]
    fallback = bone_index.get(obj.parent_bone, 0) if obj.parent_type == 'BONE' else 0
    me.calc_loop_triangles()
    uv = me.uv_layers.active.data if me.uv_layers.active else None
    normals = me.corner_normals
    for tri in me.loop_triangles:
        material = obj.material_slots[tri.material_index].material if obj.material_slots else None
        key = (material.name if material else '', obj.get('mu_texture_index'))
        if key not in builders:
            builders[key] = _MeshBuilder(_material_texture(material, out_dir, written), len(builders))
            if obj.get('mu_texture_index') is not None:
                builders[key].mesh.texture_index = obj['mu_texture_index']
        b = builders[key]
        indices = [[], [], []]
        for corner, loop in enumerate(tri.loops):
            v = me.vertices[tri.vertices[corner]]
            node = _vertex_bone(obj, v, bone_index, fallback)
            world = obj.matrix_world @ v.co
            local = inv_rest[node] @ world
            vi = b.vertex((obj.name, v.index), node, tuple(c * scale for c in local))
            n = (inv_rest[node].to_3x3() @ (obj.matrix_world.to_3x3() @ Vector(normals[loop].vector))).normalized()
            indices[0].append(vi)
            indices[1].append(b.normal(vi, node, tuple(n)))
            u, v_ = uv[loop].uv if uv else (0.5, 0.5)
            indices[2].append(b.texcoord((u, 1.0 - v_)))
        b.mesh.triangles.append(bmd.Triangle(3, indices[0] + [0], indices[1] + [0], indices[2] + [0]))


def export_bmd(path, scale=1.0, encrypt=False, verify=False):
    """Writes the current scene as a BMD (and its textures as OZJ/OZT) next to path."""
    rig = next((o for o in bpy.context.scene.objects if o.type == 'ARMATURE'), None)
    default_name = os.path.splitext(os.path.basename(path))[0] + '.smd'  # the game files keep the source name
    model = bmd.Model(name=_game_name(rig, rig.get('mu_name', default_name)) if rig else default_name)
    bones = _ordered_bones(rig) if rig else []
    if rig:
        _sample_bones(model, rig, bones, scale)
    else:
        _static_root(model)
    if rig and not model.actions:
        model.actions.append(bmd.Action(1))
        for bone in model.bones:
            if not bone.dummy:
                bone.keys.append(([(0.0, 0.0, 0.0)], [(0.0, 0.0, 0.0)]))
        _fill_rest_keys(model, rig, bones, scale)
    builders, written = _export_meshes(rig, bones, scale, os.path.dirname(os.path.abspath(path)))
    model.meshes = [b.mesh for b in builders]
    bmd.write(path, model, encrypt)
    print(f'{path}: {len(model.meshes)} meshes, {len(model.bones)} bones, {len(model.actions)} actions, '
          f'textures {sorted(written) or "(none written)"}')
    if verify:
        _verify(model, rig, builders, scale)
    return model


def _verify(model, rig, builders, scale):
    """Compares every key of every action as Blender deforms the meshes and as the client will."""
    actions = _export_actions(rig) if rig else []
    worst = 0.0
    for a, action in enumerate(actions):
        _assign_action(rig, action)
        for k, frame in enumerate(_frames(action)):
            bpy.context.scene.frame_set(frame)
            posed = bmd.posed_vertices(model, a, k)
            depsgraph = bpy.context.evaluated_depsgraph_get()
            for m, builder in enumerate(builders):
                for (obj_name, vertex), mu_index in builder.vertex_map.items():
                    obj = bpy.context.scene.objects[obj_name].evaluated_get(depsgraph)
                    blender = obj.matrix_world @ obj.data.vertices[vertex].co * scale
                    worst = max(worst, max(abs(blender[i] - posed[m][mu_index][i]) for i in range(3)))
    print(f'verify: largest difference between Blender and the client pose: {worst:.4f} units'
          + ('' if worst < 0.1 else ' - vertices weighted to several bones are bound to one bone only'))


def _fill_rest_keys(model, rig, bones, scale):
    """Armature without actions: one key with the rest pose."""
    for i, bone in enumerate(bones):
        m = rig.matrix_world @ bone.matrix_local if bone.parent is None else \
            bone.parent.matrix_local.inverted() @ bone.matrix_local
        loc, rot, _ = m.decompose()
        model.bones[i].keys[0] = ([tuple(c * scale for c in loc)], [tuple(rot.to_euler('XYZ'))])


# ---------- preview ----------

def _setup_render(engine):
    scene = bpy.context.scene
    scene.render.resolution_x = scene.render.resolution_y = PREVIEW_SIZE
    scene.render.film_transparent = False
    world = bpy.data.worlds.new('Preview')
    world.color = (0.2, 0.2, 0.22)
    scene.world = world
    sun = bpy.data.objects.new('Sun', bpy.data.lights.new('Sun', 'SUN'))
    sun.data.energy = 4
    sun.rotation_euler = (0.7, 0.2, 0.6)
    scene.collection.objects.link(sun)
    if engine == 'workbench':
        scene.render.engine = 'BLENDER_WORKBENCH'
        scene.display.shading.color_type = 'TEXTURE'
    else:
        scene.render.engine = 'CYCLES'
        scene.cycles.samples = 16
        scene.cycles.device = 'GPU' if engine == 'cycles-gpu' else 'CPU'


def _frame_bounds(frames):
    """Center of the model at every frame and the largest radius, so walking models stay in the picture."""
    scene = bpy.context.scene
    centers, radius = [], 1.0
    for f in frames:
        scene.frame_set(f)
        depsgraph = bpy.context.evaluated_depsgraph_get()
        points = [o.evaluated_get(depsgraph).matrix_world @ Vector(c)
                  for o in scene.objects if o.type == 'MESH' for c in o.evaluated_get(depsgraph).bound_box]
        if not points:
            points = [Vector()]
        lo = Vector([min(p[i] for p in points) for i in range(3)])
        hi = Vector([max(p[i] for p in points) for i in range(3)])
        centers.append((lo + hi) / 2)
        radius = max(radius, (hi - lo).length / 2)
    return centers, radius


def _add_camera(radius):
    scene = bpy.context.scene
    cam = bpy.data.objects.new('Camera', bpy.data.cameras.new('Camera'))
    scene.collection.objects.link(cam)
    cam.data.clip_end = radius * 20
    scene.camera = cam
    return cam


def _aim_camera(cam, center, radius):
    direction = Vector((0.6, -1.0, 0.45)).normalized()
    cam.location = center + direction * radius * 2.6
    cam.rotation_euler = (center - cam.location).to_track_quat('-Z', 'Y').to_euler()


def preview(path, out_png, action_index=0, frame_count=6, engine='cycles'):
    """Renders frame_count poses of one action side by side into one PNG."""
    rig = import_bmd(path, os.path.dirname(os.path.abspath(out_png))) if path.lower().endswith('.bmd') \
        else _load_scene(path)
    actions = _export_actions(rig) if rig else []
    frames = [1]
    if actions:
        action = actions[min(action_index, len(actions) - 1)]
        _assign_action(rig, action)
        all_frames = _frames(action)
        step = max(1, len(all_frames) // frame_count)
        frames = all_frames[::step][:frame_count]
    _setup_render(engine)
    centers, radius = _frame_bounds(frames)
    cam = _add_camera(radius)
    tiles = []
    for i, f in enumerate(frames):
        bpy.context.scene.frame_set(f)
        _aim_camera(cam, centers[i], radius)
        bpy.context.scene.render.filepath = f'{out_png}.{i}.png'
        bpy.ops.render.render(write_still=True)
        tiles.append(f'{out_png}.{i}.png')
    _join_tiles(tiles, out_png)


def _join_tiles(tiles, out_png):
    images = [bpy.data.images.load(t) for t in tiles]
    w, h = images[0].size
    strip = bpy.data.images.new('Preview', w * len(images), h, alpha=True)
    pixels = numpy.zeros(w * len(images) * h * 4, dtype=numpy.float32)
    for n, img in enumerate(images):
        src = numpy.empty(w * h * 4, dtype=numpy.float32)
        img.pixels.foreach_get(src)
        for y in range(h):
            row = (y * w * len(images) + n * w) * 4
            pixels[row:row + w * 4] = src[y * w * 4:(y + 1) * w * 4]
    strip.pixels.foreach_set(pixels)
    strip.filepath_raw = out_png
    strip.file_format = 'PNG'
    strip.save()
    for t in tiles:
        os.remove(t)


# ---------- files ----------

def _load_scene(path):
    ext = os.path.splitext(path)[1].lower()
    if ext == '.blend':
        bpy.ops.wm.open_mainfile(filepath=path)
    else:
        _reset_scene()
        if ext in ('.glb', '.gltf'):
            bpy.ops.import_scene.gltf(filepath=path)
        elif ext == '.fbx':
            bpy.ops.import_scene.fbx(filepath=path)
        elif ext == '.obj':
            bpy.ops.wm.obj_import(filepath=path)
        elif ext == '.bmd':
            return import_bmd(path)
        else:
            raise ValueError(f'unsupported input {path}')
    return next((o for o in bpy.context.scene.objects if o.type == 'ARMATURE'), None)


def _save_scene(path):
    ext = os.path.splitext(path)[1].lower()
    if ext == '.blend':
        bpy.ops.wm.save_as_mainfile(filepath=os.path.abspath(path))
    elif ext in ('.glb', '.gltf'):
        bpy.ops.export_scene.gltf(filepath=path, export_animation_mode='ACTIONS')
    elif ext == '.fbx':
        bpy.ops.export_scene.fbx(filepath=path, bake_anim_use_all_actions=True, add_leaf_bones=False,
                                 bake_anim_simplify_factor=0.0)
    else:
        raise ValueError(f'unsupported output {path}')


def main(argv):
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('command', choices=('import', 'export', 'preview'))
    parser.add_argument('source')
    parser.add_argument('target')
    parser.add_argument('--scale', type=float, default=1.0, help='export: multiply all sizes (glTF in meters: 100)')
    parser.add_argument('--encrypt', action='store_true', help='export: write version 0x0C like the game files')
    parser.add_argument('--verify', action='store_true', help='export: compare all poses with Blender')
    parser.add_argument('--action', type=int, default=0, help='preview: action index')
    parser.add_argument('--frames', type=int, default=6, help='preview: number of poses')
    parser.add_argument('--engine', choices=('cycles', 'cycles-gpu', 'workbench'), default='cycles',
                        help='preview: workbench is fastest but needs a GPU')
    args = parser.parse_args(argv)
    if args.command == 'import':
        _reset_scene()
        import_bmd(args.source, os.path.dirname(os.path.abspath(args.target)))
        _save_scene(args.target)
    elif args.command == 'export':
        _load_scene(args.source)
        export_bmd(args.target, args.scale, args.encrypt, args.verify)
    else:
        _reset_scene()
        preview(args.source, args.target, args.action, args.frames, args.engine)


if __name__ == '__main__':
    main(sys.argv[sys.argv.index('--') + 1:] if '--' in sys.argv else sys.argv[1:])

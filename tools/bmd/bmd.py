"""Reads and writes the BMD models of the MU client (meshes, bones, animations).

The layout mirrors BMD::Open2 / BMD::Save2 in src/source/Render/Models/ZzzBMD.cpp:
version 0x0A is plain, version 0x0C is encrypted with MapFileEncrypt (ZzzLodTerrain.h).
Pure Python, no dependencies, so it also runs inside Blender.

    python bmd.py info Data/Monster/Monster01.bmd
    python bmd.py check Data            # parses every .bmd below a folder
"""
import math
import os
import struct
import sys
from dataclasses import dataclass, field

VERSION_PLAIN = 0x0A
VERSION_ENCRYPTED = 0x0C
NAME_SIZE = 32
MAX_BONES = 200  # ZzzBMD.h
MAX_MESH = 50
MAX_VERTICES = 15000
GIMBAL_EPSILON = 1e-9
_XOR_KEY = bytes((0xD1, 0x73, 0x52, 0xF6, 0xD2, 0x9A, 0xCB, 0x27, 0x3E, 0xAF, 0x59, 0x31, 0x37, 0xB3, 0xE7, 0xA2))
_MAP_KEY_START = 0x5E
_MAP_KEY_STEP = 0x3D

# Struct sizes as the MSVC compiler lays them out (natural alignment, no pragma pack).
_VERTEX = struct.Struct('<hxx3f')            # Vertex_t: short Node, vec3 Position = 16
_NORMAL = struct.Struct('<hxx3fhxx')         # Normal_t: short Node, vec3 Normal, short BindVertex = 20
_TEXCOORD = struct.Struct('<2f')             # TexCoord_t = 8
_TRIANGLE = struct.Struct('<bx4h4h4h2x32xhxx')  # Triangle_t2 (with the unused lightmap fields) = 64
_VEC3 = struct.Struct('<3f')


@dataclass
class Triangle:
    polygon: int                 # 3 or 4 corners
    vertices: list               # 4 indices into Mesh.vertices
    normals: list                # 4 indices into Mesh.normals
    texcoords: list              # 4 indices into Mesh.texcoords


@dataclass
class Mesh:
    texture: str                 # e.g. "golem.jpg"; the client loads golem.OZJ next to the model
    texture_index: int = 0
    vertices: list = field(default_factory=list)   # (bone, (x, y, z)) in the space of the bone
    normals: list = field(default_factory=list)    # (bone, (x, y, z), bind vertex)
    texcoords: list = field(default_factory=list)  # (u, v), v = 0 at the top of the image
    triangles: list = field(default_factory=list)


@dataclass
class Bone:
    name: str
    parent: int = -1
    dummy: bool = False
    # Per action: (positions, rotations), one (x, y, z) per key, relative to the parent.
    # Rotations are Euler angles in radians, applied X, then Y, then Z (AngleQuaternion).
    keys: list = field(default_factory=list)


@dataclass
class Action:
    num_keys: int
    lock_positions: bool = False
    positions: list = field(default_factory=list)  # (x, y, z) per key when lock_positions


@dataclass
class Model:
    name: str = "model.smd"
    meshes: list = field(default_factory=list)
    bones: list = field(default_factory=list)
    actions: list = field(default_factory=list)
    version: int = VERSION_PLAIN


def map_file_decrypt(data):
    out = bytearray(len(data))
    key = _MAP_KEY_START
    for i, b in enumerate(data):
        out[i] = ((b ^ _XOR_KEY[i % 16]) - key) & 0xFF
        key = (b + _MAP_KEY_STEP) & 0xFF
    return bytes(out)


def map_file_encrypt(data):
    out = bytearray(len(data))
    key = _MAP_KEY_START
    for i, b in enumerate(data):
        out[i] = ((b + key) & 0xFF) ^ _XOR_KEY[i % 16]
        key = (out[i] + _MAP_KEY_STEP) & 0xFF
    return bytes(out)


def _decode_name(raw):
    return raw.split(b'\0', 1)[0].decode('cp949', errors='surrogateescape')


def _encode_name(text):
    raw = text.encode('cp949', errors='surrogateescape')[:NAME_SIZE]  # the game files use all 32 bytes, unterminated
    return raw.ljust(NAME_SIZE, b'\0')


class _Reader:
    def __init__(self, data):
        self.data = data
        self.pos = 0

    def take(self, fmt):
        values = fmt.unpack_from(self.data, self.pos)
        self.pos += fmt.size
        return values

    def short(self):
        return self.take(_SHORT)[0]

    def name(self):
        raw = self.data[self.pos:self.pos + NAME_SIZE]
        self.pos += NAME_SIZE
        return _decode_name(raw)

    def vec3s(self, count):
        return [self.take(_VEC3) for _ in range(count)]


_SHORT = struct.Struct('<h')
_ACTION = struct.Struct('<h?')
_MESH_HEADER = struct.Struct('<5h')
_MODEL_HEADER = struct.Struct('<3h')


def _payload(raw, path):
    if raw[:3] != b'BMD':
        raise ValueError(f"{path}: not a BMD file")
    version = raw[3]
    if version == VERSION_PLAIN:
        return version, raw[4:]
    if version == VERSION_ENCRYPTED:
        size = struct.unpack_from('<i', raw, 4)[0]
        return version, map_file_decrypt(raw[8:8 + size])
    raise ValueError(f"{path}: unsupported BMD version 0x{version:02X}")


def _read_mesh(r):
    nv, nn, nt, ntri, tex_index = r.take(_MESH_HEADER)
    mesh = Mesh(texture='', texture_index=tex_index)
    for _ in range(nv):
        node, *pos = r.take(_VERTEX)
        mesh.vertices.append((node, tuple(pos)))
    for _ in range(nn):
        node, x, y, z, bind = r.take(_NORMAL)
        mesh.normals.append((node, (x, y, z), bind))
    mesh.texcoords = [r.take(_TEXCOORD) for _ in range(nt)]
    for _ in range(ntri):
        t = r.take(_TRIANGLE)
        mesh.triangles.append(Triangle(t[0], list(t[1:5]), list(t[5:9]), list(t[9:13])))
    mesh.texture = r.name()
    return mesh


def read(path):
    raw = open(path, 'rb').read()
    version, data = _payload(raw, path)
    r = _Reader(data)
    model = Model(name=r.name(), version=version)
    num_meshes, num_bones, num_actions = r.take(_MODEL_HEADER)
    model.meshes = [_read_mesh(r) for _ in range(num_meshes)]
    for _ in range(num_actions):
        keys, lock = r.take(_ACTION)
        action = Action(keys, lock)
        if lock and keys > 0:
            action.positions = r.vec3s(keys)
        model.actions.append(action)
    for _ in range(num_bones):
        dummy = data[r.pos] != 0
        r.pos += 1
        if dummy:
            model.bones.append(Bone('', dummy=True))
            continue
        bone = Bone(r.name(), r.short())
        for action in model.actions:
            positions = r.vec3s(action.num_keys)
            bone.keys.append((positions, r.vec3s(action.num_keys)))
        model.bones.append(bone)
    if r.pos != len(data):
        raise ValueError(f"{path}: {len(data) - r.pos} bytes left after the model")
    return model


def _pack_mesh(mesh, out):
    out += _MESH_HEADER.pack(len(mesh.vertices), len(mesh.normals), len(mesh.texcoords),
                             len(mesh.triangles), mesh.texture_index)
    for node, pos in mesh.vertices:
        out += _VERTEX.pack(node, *pos)
    for node, normal, bind in mesh.normals:
        out += _NORMAL.pack(node, *normal, bind)
    for uv in mesh.texcoords:
        out += _TEXCOORD.pack(*uv)
    for t in mesh.triangles:
        out += _TRIANGLE.pack(t.polygon, *t.vertices, *t.normals, *t.texcoords, 0)
    out += _encode_name(mesh.texture)


def validate(model):
    """Raises ValueError when the client would reject or overflow on the model."""
    problems = []
    if len(model.bones) > MAX_BONES:
        problems.append(f"{len(model.bones)} bones, the client allows {MAX_BONES}")
    if len(model.meshes) > MAX_MESH:
        problems.append(f"{len(model.meshes)} meshes, the client allows {MAX_MESH}")
    for i, mesh in enumerate(model.meshes):
        if len(mesh.vertices) > MAX_VERTICES:
            problems.append(f"mesh {i}: {len(mesh.vertices)} vertices, the client allows {MAX_VERTICES}")
    for i, bone in enumerate(model.bones):
        if not bone.dummy and bone.parent >= len(model.bones):
            problems.append(f"bone {i} ({bone.name}): parent {bone.parent} does not exist")
    if problems:
        raise ValueError("; ".join(problems))


def write(path, model, encrypt=False):
    validate(model)
    out = bytearray(_encode_name(model.name))
    out += _MODEL_HEADER.pack(len(model.meshes), len(model.bones), len(model.actions))
    for mesh in model.meshes:
        _pack_mesh(mesh, out)
    for action in model.actions:
        out += _ACTION.pack(action.num_keys, action.lock_positions)
        if action.lock_positions:
            for p in action.positions:
                out += _VEC3.pack(*p)
    for bone in model.bones:
        out.append(1 if bone.dummy else 0)
        if bone.dummy:
            continue
        out += _encode_name(bone.name) + _SHORT.pack(bone.parent)
        for positions, rotations in bone.keys:
            for p in positions:
                out += _VEC3.pack(*p)
            for r in rotations:
                out += _VEC3.pack(*r)
    if encrypt:
        body = b'BMD' + bytes((VERSION_ENCRYPTED,)) + struct.pack('<i', len(out)) + map_file_encrypt(bytes(out))
    else:
        body = b'BMD' + bytes((VERSION_PLAIN,)) + bytes(out)
    with open(path, 'wb') as f:
        f.write(body)


# ---------- pose math, same as the client (AngleQuaternion / QuaternionMatrix / R_ConcatTransforms) ----------

def euler_to_matrix(angles):
    """3x3 rotation of AngleQuaternion: X first, then Y, then Z."""
    sr, cr = math.sin(angles[0]), math.cos(angles[0])
    sp, cp = math.sin(angles[1]), math.cos(angles[1])
    sy, cy = math.sin(angles[2]), math.cos(angles[2])
    return [[cp * cy, sr * sp * cy - cr * sy, cr * sp * cy + sr * sy],
            [cp * sy, sr * sp * sy + cr * cy, cr * sp * sy - sr * cy],
            [-sp, sr * cp, cr * cp]]


def matrix_to_euler(m):
    """Inverse of euler_to_matrix. Peels the angles off one after another, so the result stays exact for the
    whole matrix even at pitch +-90 degrees, where the game files often sit and yaw and roll are ambiguous."""
    cos_pitch = math.hypot(m[0][0], m[1][0])
    yaw = math.atan2(m[1][0], m[0][0]) if cos_pitch > GIMBAL_EPSILON else 0.0
    cy, sy = math.cos(yaw), math.sin(yaw)
    n = [[cy * m[0][j] + sy * m[1][j] for j in range(3)],      # Rz(-yaw) * m = Ry(pitch) * Rx(roll)
         [-sy * m[0][j] + cy * m[1][j] for j in range(3)],
         list(m[2])]
    pitch = math.atan2(-n[2][0], n[0][0])
    cp, sp = math.cos(pitch), math.sin(pitch)
    p11, p21 = n[1][1], sp * n[0][1] + cp * n[2][1]             # Ry(-pitch) * n = Rx(roll)
    return (math.atan2(p21, p11), pitch, yaw)


def _concat(a, b):
    """3x4 matrices: a * b."""
    out = [[0.0] * 4 for _ in range(3)]
    for i in range(3):
        for j in range(4):
            out[i][j] = sum(a[i][k] * b[k][j] for k in range(3)) + (a[i][3] if j == 3 else 0.0)
    return out


def bone_matrices(model, action, key):
    """World 3x4 matrix of every bone at one key of one action (identity for dummy bones)."""
    result = []
    for bone in model.bones:
        if bone.dummy or not bone.keys or model.actions[action].num_keys == 0:
            result.append([[1.0, 0, 0, 0], [0, 1.0, 0, 0], [0, 0, 1.0, 0]])
            continue
        positions, rotations = bone.keys[action]
        rot = euler_to_matrix(rotations[key])
        local = [rot[i] + [positions[key][i]] for i in range(3)]
        result.append(local if bone.parent < 0 else _concat(result[bone.parent], local))
    return result


def transform(m, v):
    return tuple(sum(m[i][k] * v[k] for k in range(3)) + m[i][3] for i in range(3))


def rotate(m, v):
    return tuple(sum(m[i][k] * v[k] for k in range(3)) for i in range(3))


def posed_vertices(model, action, key):
    """World positions of every vertex of every mesh, like BMD::Transform does."""
    mats = bone_matrices(model, action, key)
    return [[transform(mats[node], pos) for node, pos in mesh.vertices] for mesh in model.meshes]


# ---------- command line ----------

def _info(path):
    m = read(path)
    print(f"{path}: version 0x{m.version:02X}, name {m.name!r}")
    for i, mesh in enumerate(m.meshes):
        print(f"  mesh {i}: {len(mesh.vertices)} vertices, {len(mesh.triangles)} triangles, texture {mesh.texture!r}")
    print("  actions (keys):", [a.num_keys for a in m.actions])
    for i, b in enumerate(m.bones):
        print(f"  bone {i}: " + ("(dummy)" if b.dummy else f"{b.name!r} parent {b.parent}"))


def _check(folder):
    ok, other, failed = 0, 0, []
    for root, _, files in os.walk(folder):
        for name in files:
            if not name.lower().endswith('.bmd'):
                continue
            path = os.path.join(root, name)
            with open(path, 'rb') as f:
                if f.read(3) != b'BMD':
                    other += 1  # the client also keeps text tables (Item.bmd ...) under .bmd
                    continue
            try:
                read(path)
                ok += 1
            except Exception as e:  # report every broken file, keep going
                failed.append(f"{path}: {e}")
    print(f"{ok} models read, {len(failed)} failed, {other} files are not models")
    for line in failed:
        print("  " + line)


if __name__ == '__main__':
    if len(sys.argv) != 3 or sys.argv[1] not in ('info', 'check'):
        sys.exit(__doc__)
    (_info if sys.argv[1] == 'info' else _check)(sys.argv[2])

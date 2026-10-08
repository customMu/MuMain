"""The Jewelry Box item (14/200): a lacquered box with gold trims, the lid half open, a ring and a pendant inside.

    blender -b -P examples/jewelry_box.py -- out_dir
    (or: python examples/jewelry_box.py out_dir   with the bpy module from pip)

Writes out_dir/jewelry_box.blend, out_dir/JewelryBox.bmd and its textures (JewelryBoxWood.OZJ, JewelryBoxGold.OZJ,
JewelryBoxVelvet.OZJ, JewelryBoxGem.OZJ). Static model: no armature, one bone and one key, like the map objects.
Sized and oriented like the Box of Luck (Data/Item/MagicBox01.bmd): Z up, standing on z = 0, about 30 units wide,
the front towards -Y, which the client turns to the viewer in the inventory (angle 270, -10, 0).
"""
import math
import os
import sys

import bpy
import numpy

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
import blender_bmd  # noqa: E402

TEXTURE_SIZE = 128
WIDTH, DEPTH = 30.0, 22.0          # outer size of the box body
BODY_HEIGHT = 13.0
LID_HEIGHT = 5.0
LID_OPEN_DEGREES = 40.0            # the lid swings up around the back edge
WALL = 1.6
TRIM = 1.0                         # thickness of the gold bands
SEED = 7


# ---------- textures ----------

def _noise(rng, size, scale):
    """Smooth value noise: random values on a coarse grid, bilinear up to the texture size."""
    grid = rng.random((scale + 1, scale + 1))
    axis = numpy.linspace(0, scale, size, endpoint=False)
    i = axis.astype(int)
    f = axis - i
    a, b = grid[i][:, i], grid[i][:, i + 1]
    c, d = grid[i + 1][:, i], grid[i + 1][:, i + 1]
    fx, fy = f[None, :], f[:, None]
    return (a * (1 - fx) + b * fx) * (1 - fy) + (c * (1 - fx) + d * fx) * fy


def _wood(rng):
    y = numpy.linspace(0, 1, TEXTURE_SIZE)[:, None]
    warp = _noise(rng, TEXTURE_SIZE, 4) * 6.0
    grain = 0.5 + 0.5 * numpy.sin((y * 38.0 + warp) * math.pi)
    shade = 0.55 + 0.3 * grain + 0.15 * _noise(rng, TEXTURE_SIZE, 32)
    return numpy.stack([0.42 * shade, 0.07 * shade, 0.09 * shade], axis=-1)  # dark wine lacquer


def _gold(rng):
    brushed = 0.8 + 0.2 * _noise(rng, TEXTURE_SIZE, 64) * _noise(rng, TEXTURE_SIZE, 8)
    return numpy.stack([1.0 * brushed, 0.78 * brushed, 0.32 * brushed], axis=-1)


def _velvet(rng):
    pile = 0.65 + 0.35 * _noise(rng, TEXTURE_SIZE, 48)
    return numpy.stack([0.08 * pile, 0.12 * pile, 0.42 * pile], axis=-1)


def _gem(rng):
    y, x = numpy.mgrid[0:TEXTURE_SIZE, 0:TEXTURE_SIZE] / TEXTURE_SIZE
    facets = 0.6 + 0.4 * numpy.abs(numpy.sin(x * 9.0) * numpy.cos(y * 7.0))
    sparkle = (_noise(rng, TEXTURE_SIZE, 40) > 0.93) * 0.5
    shade = numpy.clip(facets + sparkle, 0, 1)
    return numpy.stack([0.75 * shade + 0.1, 0.1 * shade, 0.3 * shade + 0.1], axis=-1)  # ruby-violet


def _material(name, rgb, out_dir):
    """A material whose texture is saved as JPEG; the export packs it as name.OZJ."""
    image = bpy.data.images.new(name, TEXTURE_SIZE, TEXTURE_SIZE)
    rgba = numpy.concatenate([rgb, numpy.ones(rgb.shape[:2] + (1,))], axis=-1)
    image.pixels.foreach_set(numpy.clip(rgba, 0, 1).astype(numpy.float32).ravel())
    image.filepath_raw = os.path.join(out_dir, name + '.jpg')
    image.file_format = 'JPEG'
    image.save()
    material = bpy.data.materials.new(name)
    if bpy.app.version < (5, 0, 0):
        material.use_nodes = True  # always on from Blender 5
    node = material.node_tree.nodes.new('ShaderNodeTexImage')
    node.image = image
    material.node_tree.links.new(node.outputs['Color'], material.node_tree.nodes['Principled BSDF'].inputs['Base Color'])
    return material


# ---------- geometry ----------

def _finish(obj, material, smooth=False, bevel=0.0):
    obj.data.materials.append(material)
    for polygon in obj.data.polygons:
        polygon.use_smooth = smooth
    if bevel:
        modifier = obj.modifiers.new('Bevel', 'BEVEL')
        modifier.width = bevel
        modifier.segments = 2
        modifier.limit_method = 'ANGLE'
    return obj


def _cube(name, size, location, material, bevel=0.0):
    bpy.ops.mesh.primitive_cube_add(location=location)
    obj = bpy.context.active_object
    obj.name = name
    obj.scale = (size[0] / 2, size[1] / 2, size[2] / 2)
    bpy.ops.object.transform_apply(scale=True)
    return _finish(obj, material, bevel=bevel)


def _body(m):
    """An open box: a floor and four walls, with gold bands at the rim and the bottom and gold feet."""
    h = BODY_HEIGHT
    _cube('Floor', (WIDTH, DEPTH, WALL), (0, 0, 1.5 + WALL / 2), m['wood'], 0.3)
    for sign in (-1, 1):
        _cube('WallFB', (WIDTH, WALL, h), (0, sign * (DEPTH - WALL) / 2, 1.5 + h / 2), m['wood'], 0.3)
        _cube('WallLR', (WALL, DEPTH - 2 * WALL, h), (sign * (WIDTH - WALL) / 2, 0, 1.5 + h / 2), m['wood'], 0.3)
    for z in (1.5 + TRIM / 2, 1.5 + h - TRIM / 2):  # bands around the box
        for sign in (-1, 1):
            _cube('BandFB', (WIDTH + 0.4, WALL + 0.4, TRIM), (0, sign * (DEPTH - WALL) / 2, z), m['gold'])
            _cube('BandLR', (WALL + 0.4, DEPTH + 0.4, TRIM), (sign * (WIDTH - WALL) / 2, 0, z), m['gold'])
    for sx in (-1, 1):  # corner posts and feet
        for sy in (-1, 1):
            _cube('Corner', (2.0, 2.0, h + 0.4), (sx * (WIDTH / 2 - 0.6), sy * (DEPTH / 2 - 0.6), 1.5 + h / 2),
                  m['gold'], 0.3)
            bpy.ops.mesh.primitive_uv_sphere_add(segments=8, ring_count=5, radius=1.6,
                                                 location=(sx * (WIDTH / 2 - 2.5), sy * (DEPTH / 2 - 2.5), 1.6))
            _finish(bpy.context.active_object, m['gold'], smooth=True)
    _cube('Clasp', (4.0, 1.2, 4.5), (0, -DEPTH / 2 - 0.3, 1.5 + h - 2.0), m['gold'], 0.3)


def _lid(m):
    """The lid: a slab with a gold rim and a gem on top, hinged at the back edge and swung open."""
    bpy.ops.object.empty_add(location=(0, DEPTH / 2, 1.5 + BODY_HEIGHT))
    hinge = bpy.context.active_object
    parts = [
        _cube('Lid', (WIDTH + 0.6, DEPTH + 0.6, LID_HEIGHT), (0, 0, 1.5 + BODY_HEIGHT + LID_HEIGHT / 2), m['wood'], 0.8),
        _cube('LidLining', (WIDTH - 1.0, DEPTH - 1.0, 0.4), (0, 0, 1.5 + BODY_HEIGHT - 0.1), m['velvet']),
        _cube('LidBand', (3.0, DEPTH + 1.0, 0.6), (0, 0, 1.5 + BODY_HEIGHT + LID_HEIGHT + 0.1), m['gold']),
    ]
    z = 1.5 + BODY_HEIGHT + TRIM / 2
    for sign in (-1, 1):  # gold frame around the lid edge
        parts.append(_cube('LidRimFB', (WIDTH + 1.0, TRIM, TRIM), (0, sign * (DEPTH + 0.5) / 2, z), m['gold']))
        parts.append(_cube('LidRimLR', (TRIM, DEPTH + 1.0, TRIM), (sign * (WIDTH + 0.5) / 2, 0, z), m['gold']))
    bpy.ops.mesh.primitive_ico_sphere_add(subdivisions=1, radius=2.6,
                                          location=(0, 0, 1.5 + BODY_HEIGHT + LID_HEIGHT + 1.2))
    gem = bpy.context.active_object
    gem.scale = (1.0, 1.0, 0.7)
    parts.append(_finish(gem, m['gem']))
    for part in parts:
        part.parent = hinge
        part.matrix_parent_inverse = hinge.matrix_world.inverted()
    hinge.rotation_euler = (math.radians(-LID_OPEN_DEGREES), 0, 0)  # opens up and back around X
    bpy.context.view_layer.update()
    for part in parts:  # bake the hinge: the export takes plain meshes
        matrix = part.matrix_world.copy()
        part.parent = None
        part.matrix_world = matrix


def _contents(m):
    """Velvet cushion with a ring standing in a slot and a pendant lying beside it."""
    top = 1.5 + BODY_HEIGHT - 3.0
    _cube('Cushion', (WIDTH - 2 * WALL, DEPTH - 2 * WALL, 6.0), (0, 0, top - 3.0), m['velvet'], 0.8)
    bpy.ops.mesh.primitive_torus_add(major_radius=3.4, minor_radius=0.7, major_segments=20, minor_segments=8,
                                     location=(-5.5, 1.0, top + 2.2), rotation=(math.radians(90), 0, math.radians(15)))
    _finish(bpy.context.active_object, m['gold'], smooth=True)
    bpy.ops.mesh.primitive_ico_sphere_add(subdivisions=1, radius=1.5, location=(-6.4, 0.75, top + 6.0))
    _finish(bpy.context.active_object, m['gem'])
    bpy.ops.mesh.primitive_torus_add(major_radius=4.2, minor_radius=0.35, major_segments=24, minor_segments=6,
                                     location=(5.5, 0.5, top + 0.4))
    _finish(bpy.context.active_object, m['gold'], smooth=True)
    bpy.ops.mesh.primitive_cone_add(vertices=6, radius1=1.8, depth=3.6, location=(5.5, -4.6, top + 0.9),
                                    rotation=(math.radians(-90), 0, 0))
    _finish(bpy.context.active_object, m['gem'])


def main(out_dir):
    os.makedirs(out_dir, exist_ok=True)
    out_dir = os.path.abspath(out_dir)
    blender_bmd._reset_scene()
    rng = numpy.random.default_rng(SEED)
    materials = {
        'wood': _material('JewelryBoxWood', _wood(rng), out_dir),
        'gold': _material('JewelryBoxGold', _gold(rng), out_dir),
        'velvet': _material('JewelryBoxVelvet', _velvet(rng), out_dir),
        'gem': _material('JewelryBoxGem', _gem(rng), out_dir),
    }
    _body(materials)
    _lid(materials)
    _contents(materials)
    bpy.ops.wm.save_as_mainfile(filepath=os.path.join(out_dir, 'jewelry_box.blend'))
    blender_bmd.export_bmd(os.path.join(out_dir, 'JewelryBox.bmd'))


if __name__ == '__main__':
    args = sys.argv[sys.argv.index('--') + 1:] if '--' in sys.argv else sys.argv[1:]
    main(args[0] if args else '.')

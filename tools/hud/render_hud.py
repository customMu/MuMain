"""Blender (Cycles) renders of the modern HUD parts; called by modern_hud.py:

    blender -b -P tools/hud/render_hud.py -- <work_dir>        the HUD parts
    blender -b -P tools/hud/render_hud.py -- <work_dir> ui     the window parts atlas (modern_ui.py): ui.png

Reads the maps modern_hud.py wrote to work_dir (band_height.png, band_gold.png, band_inner.png, icon_<kind>.png) and
writes there:
    band.png                 the metal band, 640 x 51 reference pixels at SS
    orb_<colour>.png         a sphere of liquid (RGBA), the HP / mana fill
    orb_glass.png            the gold ring of an orb (RGBA), drawn over the liquid
    tube_<colour>.png        a capsule of liquid (RGBA), the SD / AG fill
    tube_glass.png           the gold caps and the glass of a tube (RGBA)
    medallion_<kind>.png     a round metal button with an engraved icon (RGBA)
"""
import math
import os
import sys

import bpy
from mathutils import Vector

SS = 4
BAND = (640, 51)
LIQUIDS = {  # colour of the liquid (linear-ish RGB), glow
    'red': ((0.70, 0.02, 0.03), 1.0),
    'green': ((0.10, 0.55, 0.06), 1.0),
    'blue': ((0.03, 0.16, 0.85), 1.0),
    'sd': ((0.95, 0.62, 0.08), 0.9),
    'ag': ((0.05, 0.60, 0.80), 0.9),
}
ICONS = ('shop', 'character', 'inventory', 'friend', 'menu')
GOLD = (1.0, 0.72, 0.36)
STEEL = (0.035, 0.038, 0.048)
RELIEF_DEPTH = 5.0  # reference pixels between white and black of band_height
ORB_OVERLAY = 56  # the ring overlay, reference pixels (modern_hud.ORB_OVERLAY); the liquid is 39
TUBE_OVERLAY = (26, 48)  # the tube overlay; the liquid is 16 x 39
UI_ATLAS = (560, 520)  # the window parts atlas of modern_ui.py, reference pixels
UI_SS = 3


# ---------- scene helpers ----------

def reset():
    bpy.ops.wm.read_factory_settings(use_empty=True)
    scene = bpy.context.scene
    scene.render.engine = 'CYCLES'
    scene.cycles.device = 'CPU'
    scene.cycles.samples = 96
    scene.cycles.use_denoising = True
    scene.view_settings.view_transform = 'Standard'
    scene.view_settings.look = 'None'
    scene.render.image_settings.file_format = 'PNG'
    scene.render.image_settings.color_mode = 'RGBA'
    scene.render.film_transparent = True
    world = bpy.data.worlds.new('World')
    scene.world = world
    if world.node_tree is None:
        world.use_nodes = True
    nodes = world.node_tree.nodes
    bg = nodes['Background']
    coord = nodes.new('ShaderNodeTexCoord')
    ramp = nodes.new('ShaderNodeValToRGB')
    sep = nodes.new('ShaderNodeSeparateXYZ')
    world.node_tree.links.new(coord.outputs['Generated'], sep.inputs[0])
    world.node_tree.links.new(sep.outputs['Z'], ramp.inputs['Fac'])
    ramp.color_ramp.elements[0].color = (0.02, 0.02, 0.025, 1)
    ramp.color_ramp.elements[1].color = (0.22, 0.22, 0.25, 1)
    world.node_tree.links.new(ramp.outputs['Color'], bg.inputs['Color'])
    bg.inputs['Strength'].default_value = 0.8
    return scene


def camera(scene, location, rotation, ortho_scale, resolution):
    cam_data = bpy.data.cameras.new('Camera')
    cam_data.type = 'ORTHO'
    cam_data.ortho_scale = ortho_scale
    cam = bpy.data.objects.new('Camera', cam_data)
    scene.collection.objects.link(cam)
    cam.location = location
    cam.rotation_euler = rotation
    scene.camera = cam
    scene.render.resolution_x, scene.render.resolution_y = resolution
    scene.render.resolution_percentage = 100


def area_light(scene, location, size, energy, color=(1, 1, 1), target=(0, 0, 0)):
    """An area light at location, aimed at target."""
    data = bpy.data.lights.new('Light', 'AREA')
    data.size = size
    data.energy = energy
    data.color = color
    obj = bpy.data.objects.new('Light', data)
    scene.collection.objects.link(obj)
    obj.location = location
    direction = Vector(target) - Vector(location)
    obj.rotation_euler = direction.to_track_quat('-Z', 'Y').to_euler()
    return obj


def material(name):
    mat = bpy.data.materials.new(name)
    mat.use_nodes = True
    return mat, mat.node_tree.nodes, mat.node_tree.links, mat.node_tree.nodes['Principled BSDF']


def principled(mat_nodes, base, metallic, roughness, coat=0.0):
    p = mat_nodes.new('ShaderNodeBsdfPrincipled')
    p.inputs['Base Color'].default_value = (*base, 1)
    p.inputs['Metallic'].default_value = metallic
    p.inputs['Roughness'].default_value = roughness
    p.inputs['Coat Weight'].default_value = coat
    return p


def image_node(nodes, path, non_color=True):
    node = nodes.new('ShaderNodeTexImage')
    node.image = bpy.data.images.load(path)
    if non_color:
        node.image.colorspace_settings.name = 'Non-Color'
    node.interpolation = 'Cubic'
    node.extension = 'EXTEND'
    return node


def render(scene, path):
    scene.render.filepath = path
    bpy.ops.render.render(write_still=True)
    print('rendered', path)


# ---------- the band ----------

def render_band(work):
    scene = reset()
    scene.render.film_transparent = False
    w, h = BAND
    metal_relief(work, 'band', w, h, 3)
    camera(scene, (0, 0, 100), (0, 0, 0), w, (w * SS, h * SS))
    # a warm key light low from the upper left (long shadows in the slots), a cool dim fill from below
    area_light(scene, (-260, 170, 110), 90, 1.6e6, (1.0, 0.93, 0.82))
    area_light(scene, (60, -160, 80), 500, 0.3e6, (0.75, 0.84, 1.0))
    scene.cycles.samples = 64
    render(scene, os.path.join(work, 'band.png'))


def render_ui_atlas(work):
    """The window parts (modern_ui.py): one atlas of maps ui_height / ui_gold / ui_inner, UI_ATLAS reference pixels,
    lit by a sun (the same light everywhere on the atlas) from the upper left like the band."""
    scene = reset()
    scene.render.film_transparent = False
    w, h = UI_ATLAS
    metal_relief(work, 'ui', w, h, 2)
    camera(scene, (0, 0, 100), (0, 0, 0), max(w, h), (w * UI_SS, h * UI_SS))
    sun = bpy.data.lights.new('Sun', 'SUN')
    sun.energy = 3.2
    sun.angle = math.radians(8)
    sun.color = (1.0, 0.93, 0.82)
    obj = bpy.data.objects.new('Sun', sun)
    scene.collection.objects.link(obj)
    obj.rotation_euler = (Vector((-260, 170, 110)) * -1).to_track_quat('-Z', 'Y').to_euler()
    fill = bpy.data.lights.new('Fill', 'SUN')
    fill.energy = 0.5
    fill.angle = math.radians(40)
    fill.color = (0.75, 0.84, 1.0)
    obj = bpy.data.objects.new('Fill', fill)
    scene.collection.objects.link(obj)
    obj.rotation_euler = (Vector((60, -160, 80)) * -1).to_track_quat('-Z', 'Y').to_euler()
    scene.cycles.samples = 48
    render(scene, os.path.join(work, 'ui.png'))


def metal_relief(work, prefix, w, h, density):
    """A w x h plane of forged steel, gold and dark insides from the maps <prefix>_height / _gold / _inner: a grid of
    density vertices per reference pixel, displaced by the height map, so the rims cast real shadows."""
    bpy.ops.mesh.primitive_grid_add(x_subdivisions=w * density, y_subdivisions=h * density, size=1)
    plane = bpy.context.active_object
    plane.scale = (w, h, 1)
    bpy.ops.object.transform_apply(scale=True)
    relief = bpy.data.textures.new('relief', 'IMAGE')
    relief.image = bpy.data.images.load(os.path.join(work, f'{prefix}_height.png'))
    relief.image.colorspace_settings.name = 'Non-Color'
    relief.extension = 'EXTEND'
    displace = plane.modifiers.new('Relief', 'DISPLACE')
    displace.texture = relief
    displace.texture_coords = 'UV'
    displace.mid_level = 150 / 255  # the panel stays at z 0
    displace.strength = RELIEF_DEPTH
    bpy.ops.object.shade_smooth()
    mat, nodes, links, out_bsdf = material(prefix)
    out = nodes['Material Output']
    nodes.remove(out_bsdf)
    height = image_node(nodes, os.path.join(work, f'{prefix}_height.png'))
    gold_mask = image_node(nodes, os.path.join(work, f'{prefix}_gold.png'))
    inner_mask = image_node(nodes, os.path.join(work, f'{prefix}_inner.png'))
    bump = nodes.new('ShaderNodeBump')
    bump.inputs['Strength'].default_value = 0.6  # fine detail on top of the displaced relief
    bump.inputs['Distance'].default_value = 1.0
    links.new(height.outputs['Color'], bump.inputs['Height'])
    # forged steel: a noise in the roughness and the colour, brushed along x
    coord = nodes.new('ShaderNodeTexCoord')
    mapping = nodes.new('ShaderNodeMapping')
    mapping.inputs['Scale'].default_value = (40, 400, 40)
    links.new(coord.outputs['Object'], mapping.inputs['Vector'])
    noise = nodes.new('ShaderNodeTexNoise')
    noise.inputs['Scale'].default_value = 3.0
    noise.inputs['Detail'].default_value = 8
    links.new(mapping.outputs['Vector'], noise.inputs['Vector'])
    steel = principled(nodes, STEEL, 1.0, 0.42)
    rough = nodes.new('ShaderNodeMapRange')
    rough.inputs['To Min'].default_value = 0.38
    rough.inputs['To Max'].default_value = 0.62
    links.new(noise.outputs['Fac'], rough.inputs['Value'])
    links.new(rough.outputs['Result'], steel.inputs['Roughness'])
    gold = principled(nodes, GOLD, 1.0, 0.22)
    inner = principled(nodes, (0.004, 0.005, 0.008), 0.0, 0.7)
    inner.inputs['Specular IOR Level'].default_value = 0.12  # a dark, almost matte inside
    for bsdf in (steel, gold, inner):
        links.new(bump.outputs['Normal'], bsdf.inputs['Normal'])
    mix_gold = nodes.new('ShaderNodeMixShader')
    links.new(gold_mask.outputs['Color'], mix_gold.inputs['Fac'])
    links.new(steel.outputs['BSDF'], mix_gold.inputs[1])
    links.new(gold.outputs['BSDF'], mix_gold.inputs[2])
    mix_inner = nodes.new('ShaderNodeMixShader')
    links.new(inner_mask.outputs['Color'], mix_inner.inputs['Fac'])
    links.new(mix_gold.outputs['Shader'], mix_inner.inputs[1])
    links.new(inner.outputs['BSDF'], mix_inner.inputs[2])
    links.new(mix_inner.outputs['Shader'], out.inputs['Surface'])
    plane.data.materials.append(mat)
    return plane


# ---------- orbs and tubes ----------

def liquid_material(name, color, glow):
    mat, nodes, links, bsdf = material(name)
    bsdf.inputs['Base Color'].default_value = (*color, 1)
    bsdf.inputs['Roughness'].default_value = 0.18
    bsdf.inputs['Coat Weight'].default_value = 1.0
    bsdf.inputs['Coat Roughness'].default_value = 0.05
    bsdf.inputs['Subsurface Weight'].default_value = 0.6
    bsdf.inputs['Subsurface Radius'].default_value = (1.0, 0.4, 0.3)
    bsdf.inputs['Emission Color'].default_value = (*color, 1)
    # swirls: a distorted noise lights the liquid from inside
    noise = nodes.new('ShaderNodeTexNoise')
    noise.inputs['Scale'].default_value = 2.2
    noise.inputs['Detail'].default_value = 6
    noise.inputs['Distortion'].default_value = 2.5
    ramp = nodes.new('ShaderNodeMapRange')
    ramp.inputs['From Min'].default_value = 0.45
    ramp.inputs['From Max'].default_value = 0.75
    ramp.inputs['To Min'].default_value = 0.15 * glow
    ramp.inputs['To Max'].default_value = 1.4 * glow
    links.new(noise.outputs['Fac'], ramp.inputs['Value'])
    links.new(ramp.outputs['Result'], bsdf.inputs['Emission Strength'])
    return mat


def gold_material():
    mat, nodes, links, bsdf = material('gold')
    bsdf.inputs['Base Color'].default_value = (*GOLD, 1)
    bsdf.inputs['Metallic'].default_value = 1.0
    bsdf.inputs['Roughness'].default_value = 0.2
    return mat


def glass_shell_material():
    """Only the reflections of a glass shell: transparent where nothing is reflected, so it can lie over the liquid."""
    mat, nodes, links, bsdf = material('glass')
    out = nodes['Material Output']
    nodes.remove(bsdf)
    glossy = nodes.new('ShaderNodeBsdfGlossy')
    glossy.inputs['Roughness'].default_value = 0.04
    transparent = nodes.new('ShaderNodeBsdfTransparent')
    fresnel = nodes.new('ShaderNodeLayerWeight')
    fresnel.inputs['Blend'].default_value = 0.35
    mix = nodes.new('ShaderNodeMixShader')
    links.new(fresnel.outputs['Fresnel'], mix.inputs['Fac'])
    links.new(transparent.outputs['BSDF'], mix.inputs[1])
    links.new(glossy.outputs['BSDF'], mix.inputs[2])
    links.new(mix.outputs['Shader'], out.inputs['Surface'])
    return mat


def orb_lights(scene):
    area_light(scene, (-3, -4, 4), 2.0, 600)
    area_light(scene, (3, -3, -1.5), 3.0, 120, (0.8, 0.85, 1.0))


def render_orbs(work):
    """Front view of a sphere of radius 1; the ring overlay is ORB_OVERLAY / (2 * ORB_RADIUS) wider than the liquid."""
    for colour, (rgb, glow) in LIQUIDS.items():
        if colour in ('sd', 'ag'):
            continue
        scene = reset()
        bpy.ops.mesh.primitive_uv_sphere_add(radius=1.0, segments=96, ring_count=48, rotation=(math.pi / 2, 0, 0))
        sphere = bpy.context.active_object
        bpy.ops.object.shade_smooth()
        sphere.data.materials.append(liquid_material(colour, rgb, glow))
        camera(scene, (0, -10, 0), (math.pi / 2, 0, 0), 2.0, (512, 512))
        orb_lights(scene)
        render(scene, os.path.join(work, f'orb_{colour}.png'))
    # the ring: the camera sees ORB_OVERLAY / 39 of the liquid's diameter (room for its shadow)
    scene = reset()
    scale = ORB_OVERLAY / 39.0
    ring_r = 1.0 + 2.2 / 19.5
    bpy.ops.mesh.primitive_torus_add(major_radius=ring_r, minor_radius=2.4 / 19.5, major_segments=96, minor_segments=24,
                                     rotation=(math.pi / 2, 0, 0), location=(0, 0.05, 0))
    ring = bpy.context.active_object
    bpy.ops.object.shade_smooth()
    gold = gold_material()
    ring.data.materials.append(gold)
    for angle in (45, 135, 225, 315):
        a = math.radians(angle)
        bpy.ops.mesh.primitive_uv_sphere_add(radius=1.6 / 19.5, location=(ring_r * math.cos(a), -0.1, ring_r * math.sin(a)))
        bpy.ops.object.shade_smooth()
        bpy.context.active_object.data.materials.append(gold)
    camera(scene, (0, -10, 0), (math.pi / 2, 0, 0), 2.0 * scale, (512, 512))
    orb_lights(scene)
    render(scene, os.path.join(work, 'orb_glass.png'))


def capsule(radius, half_length):
    bpy.ops.mesh.primitive_uv_sphere_add(radius=radius, segments=48, ring_count=25)  # odd: no ring on the equator
    obj = bpy.context.active_object
    for v in obj.data.vertices:
        v.co.z += half_length if v.co.z > 0 else -half_length
    bpy.ops.object.shade_smooth()
    return obj


def render_tubes(work):
    """The liquid fills 16 x 39 reference pixels; the glass overlay is TUBE_OVERLAY."""
    tube_r, tube_half = 7.6 / 39, (19.5 - 7.6) / 39  # in units of the tube height (1.0 = 39 px)
    for colour in ('sd', 'ag'):
        rgb, glow = LIQUIDS[colour]
        scene = reset()
        obj = capsule(tube_r, tube_half)
        obj.data.materials.append(liquid_material(colour, rgb, glow))
        camera(scene, (0, -10, 0), (math.pi / 2, 0, 0), 1.0, (int(256 * 16 / 39), 256))
        orb_lights(scene)
        render(scene, os.path.join(work, f'tube_{colour}.png'))
    scene = reset()
    shell = capsule(tube_r + 0.4 / 39, tube_half)
    shell.data.materials.append(glass_shell_material())
    gold = gold_material()
    for z in (0.5 - 2.5 / 39, -0.5 + 2.5 / 39):
        bpy.ops.mesh.primitive_torus_add(major_radius=tube_r * 0.9, minor_radius=1.6 / 39, location=(0, 0, z))
        bpy.ops.object.shade_smooth()
        bpy.context.active_object.data.materials.append(gold)
    camera(scene, (0, -10, 0), (math.pi / 2, 0, 0), TUBE_OVERLAY[1] / 39, (round(512 * TUBE_OVERLAY[0] / TUBE_OVERLAY[1]), 512))
    orb_lights(scene)
    render(scene, os.path.join(work, 'tube_glass.png'))


# ---------- medallions ----------

def render_medallions(work):
    """A coin of radius 1 seen from the front, the icon raised in polished gold on dark steel, a gold rim."""
    for kind in ICONS:
        scene = reset()
        bpy.ops.mesh.primitive_cylinder_add(radius=1.0, depth=0.25, vertices=128, rotation=(math.pi / 2, 0, 0))
        coin = bpy.context.active_object
        bevel = coin.modifiers.new('Bevel', 'BEVEL')
        bevel.width = 0.12
        bevel.segments = 6
        bpy.ops.object.shade_smooth()
        bpy.ops.object.modifier_apply(modifier='Bevel')
        mat, nodes, links, bsdf = material('coin')
        out = nodes['Material Output']
        nodes.remove(bsdf)
        coord = nodes.new('ShaderNodeTexCoord')
        mapping = nodes.new('ShaderNodeMapping')
        # object x, z (-1..1) -> image u, v (0..1)
        mapping.inputs['Location'].default_value = (0.5, 0.5, 0)
        mapping.inputs['Scale'].default_value = (0.5, 0.5, 1)
        sep = nodes.new('ShaderNodeSeparateXYZ')
        comb = nodes.new('ShaderNodeCombineXYZ')
        links.new(coord.outputs['Object'], sep.inputs[0])
        links.new(sep.outputs['X'], comb.inputs['X'])
        links.new(sep.outputs['Y'], comb.inputs['Y'])
        links.new(comb.outputs['Vector'], mapping.inputs['Vector'])
        icon = image_node(nodes, os.path.join(work, f'icon_{kind}.png'))
        links.new(mapping.outputs['Vector'], icon.inputs['Vector'])
        bump = nodes.new('ShaderNodeBump')  # raised
        bump.inputs['Strength'].default_value = 1.0
        bump.inputs['Distance'].default_value = 0.09
        links.new(icon.outputs['Color'], bump.inputs['Height'])
        steel = principled(nodes, (0.05, 0.055, 0.07), 1.0, 0.38)
        gold = principled(nodes, GOLD, 1.0, 0.14)
        gold_fac = nodes.new('ShaderNodeMapRange')
        gold_fac.inputs['From Min'].default_value = 0.35
        gold_fac.inputs['From Max'].default_value = 0.65
        links.new(icon.outputs['Color'], gold_fac.inputs['Value'])
        for b in (steel, gold):
            links.new(bump.outputs['Normal'], b.inputs['Normal'])
        # the rim of the coin is gold too
        rim = nodes.new('ShaderNodeMath')
        rim.operation = 'GREATER_THAN'
        rim.inputs[1].default_value = 0.86
        length = nodes.new('ShaderNodeVectorMath')
        length.operation = 'LENGTH'
        flat = nodes.new('ShaderNodeCombineXYZ')
        links.new(sep.outputs['X'], flat.inputs['X'])
        links.new(sep.outputs['Y'], flat.inputs['Y'])
        links.new(flat.outputs['Vector'], length.inputs[0])
        links.new(length.outputs['Value'], rim.inputs[0])
        either = nodes.new('ShaderNodeMath')
        either.operation = 'MAXIMUM'
        links.new(gold_fac.outputs['Result'], either.inputs[0])
        links.new(rim.outputs['Value'], either.inputs[1])
        mix = nodes.new('ShaderNodeMixShader')
        links.new(either.outputs['Value'], mix.inputs['Fac'])
        links.new(steel.outputs['BSDF'], mix.inputs[1])
        links.new(gold.outputs['BSDF'], mix.inputs[2])
        links.new(mix.outputs['Shader'], out.inputs['Surface'])
        coin.data.materials.append(mat)
        camera(scene, (0, -10, 0), (math.pi / 2, 0, 0), 2.1, (512, 512))
        orb_lights(scene)
        render(scene, os.path.join(work, f'medallion_{kind}.png'))


def main(work, what='hud'):
    if what == 'ui':
        render_ui_atlas(work)
        return
    render_band(work)
    render_orbs(work)
    render_tubes(work)
    render_medallions(work)


if __name__ == '__main__':
    args = sys.argv[sys.argv.index('--') + 1:] if '--' in sys.argv else sys.argv[1:]
    main(os.path.abspath(args[0]), *args[1:])

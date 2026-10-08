# BMD model tools

Read, write, convert and preview the 3D models of the client (`Data/**/*.bmd`: meshes, bones, animations)
and their textures (`.OZJ`, `.OZT`). The file layout follows `BMD::Open2` / `BMD::Save2`
(`src/source/Render/Models/ZzzBMD.cpp`) and `CGlobalBitmap::OpenJpegTurbo` / `OpenTga` / `Save_Image`
(`src/source/Render/Sprites/GlobalBitmap.cpp`).

| File | What | Needs |
|---|---|---|
| `bmd.py` | read/write BMD (plain `0x0A` and encrypted `0x0C`), pose math of the client, `info` / `check` commands | Python 3 |
| `ozj.py` | OZJ/OZT/OZB ⇄ JPG/TGA/BMP | Python 3 |
| `blender_bmd.py` | BMD → Blender (`.blend`, `.glb`, `.fbx`); Blender scene / `.glb` / `.fbx` / `.obj` → BMD + textures; preview renders | Blender 4.4+ |
| `roundtrip_test.py` | regression test: game models → Blender → BMD, compared pose by pose | Blender 4.4+ |
| `examples/golem.py` | a rigged, animated monster built from code | Blender 4.4+ |

Checked against the game data in `src/bin/Data`: all 5114 models are read to the last byte and written back
without loss (4 files of version `0x00` the client does not load either); 600 random models went through
Blender and back with every vertex of every key within 0.002 units (`roundtrip_test.py`), and through
`.glb` / `.fbx` and back within 0.001.

## Setup on the PC

1. **Blender** 4.4 or newer (tested with 5.2): <https://www.blender.org/download/>. Blender brings its own
   Python, nothing else is needed. Without a GPU everything works too; only the `workbench` preview needs one.
2. Optional, to call it as `blender` in a shell: add the Blender folder
   (`C:\Program Files\Blender Foundation\Blender 5.2\`) to `PATH`.
3. Optional, Blender as a Python module instead of the program: `pip install bpy` — it needs exactly the
   Python version of that Blender release (bpy 5.x: Python 3.13), see <https://pypi.org/project/bpy/>.
   Then `python blender_bmd.py ...` works instead of `blender -b -P blender_bmd.py -- ...`.

## Commands

```bash
# what is inside a model
python bmd.py info ../../src/bin/Data/Monster/Monster13.bmd

# game model -> Blender (textures are unpacked next to the output: snowman.OZJ -> snowman.jpg)
blender -b -P blender_bmd.py -- import ../../src/bin/Data/Monster/Monster13.bmd work/monster13.blend
blender -b -P blender_bmd.py -- import ../../src/bin/Data/Monster/Monster13.bmd work/monster13.glb

# Blender / glTF / FBX -> game model (+ textures as OZJ/OZT next to it); --verify compares every pose
blender -b -P blender_bmd.py -- export work/monster13.blend out/Monster13.bmd --verify
blender -b -P blender_bmd.py -- export bought_model.glb out/Monster200.bmd --scale 100

# preview: 6 poses of action 2 side by side (--engine workbench is fast with a GPU)
blender -b -P blender_bmd.py -- preview out/Monster13.bmd out/preview.png --action 2 --frames 6

# textures by hand
python ozj.py unpack ../../src/bin/Data/Monster/snowman.OZJ work/
python ozj.py pack work/snowman.jpg out/

# a monster from code
blender -b -P examples/golem.py -- out/golem
```

## How a scene must look for the export

- **One armature** (or none: a static model gets one bone `Bip01` and one key, like the map objects).
- **One bone per vertex.** The client has no skin weights: every vertex follows the bone with the largest
  weight (a vertex without a bone group follows the bone the object is parented to, else the first bone).
  Model in rigid parts or paint weights 0/1; `--verify` reports how far a weighted mesh drifts.
- **Actions = animations**, in the order of their names (`00_STOP1`, `01_STOP2`, …), every frame of the
  action's frame range is one key. The client loops by going from the last key back to the first, so a loop
  must not repeat its first pose at the end. Monster actions (`_define.h`): 0 STOP1, 1 STOP2, 2 WALK,
  3 ATTACK1, 4 ATTACK2, 5 SHOCK, 6 DIE, 7 APEAR, 8 ATTACK3, 9 ATTACK4, 10 RUN, 11 ATTACK5.
  Player, NPC and item models have their own action lists in the client code.
- **Materials → meshes.** Each material becomes one mesh with one texture: the first Image Texture node of
  the material. A JPEG stays JPEG (→ `.OZJ`), anything else is written as 32-bit TGA with alpha (→ `.OZT`).
  The client rounds textures up to a power of two, at most 1024.
- **Units and axes:** Z up, 1 unit ≈ 1 cm: a human-sized monster is 150–200 units tall. A glTF in meters
  needs `--scale 100`. Apply the object transforms (Ctrl+A) before the export; bones have no scale in BMD.
- **Limits of the client** (`ZzzBMD.h`): 200 bones, 50 meshes, 15000 vertices per mesh — the export stops
  with a message when a model is over.

Models imported with this tool keep the game order of bones and actions, the exact game names
(custom properties `mu_index`, `mu_name_hex`, `mu_keys`, `mu_lock_positions`), so editing an existing
model and exporting it changes only what was edited.

## Trying a model in the game

The client loads models by fixed names (e.g. monster type N → `Data\Monster\Monster<N+1>.bmd` with at least two digits, `Monster01.bmd`;
`OpenMonsterModel` in `ZzzOpenData.cpp`) and the textures named inside the model from the same folder.
The quickest test without code: back up a model of the game, put the new `.bmd` and its `.OZJ`/`.OZT`
under that name, start the client. A new model slot needs code in the client (a model index and its
`AccessModel` / `OpenTexture` call) and, for monsters and items, the matching definition on the server.

## Tests

```bash
python bmd.py check ../../src/bin/Data                     # every model reads to the last byte
blender -b -P roundtrip_test.py -- --sample 50 ../../src/bin/Data
```

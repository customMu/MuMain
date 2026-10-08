"""Regression test of the tools: game models -> Blender -> BMD, compared pose by pose with the original.

    python roundtrip_test.py ../../src/bin/Data/Monster/Monster13.bmd ../../src/bin/Data/Player/player.bmd
    python roundtrip_test.py --sample 30 ../../src/bin/Data      # 30 random models below a folder

Needs Blender's Python (bpy). Every vertex of every key of every action is compared; a model passes when
no corner of a triangle moves more than TOLERANCE units (models are about 100-300 units tall).
"""
import argparse
import os
import random
import sys
import tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import bmd  # noqa: E402
import blender_bmd  # noqa: E402

TOLERANCE = 0.05


def _corners(model, posed):
    out = []
    for m, mesh in enumerate(model.meshes):
        for t in mesh.triangles:
            corners = t.vertices[:t.polygon]
            if len(set(corners)) == len(corners):  # the import drops degenerate triangles
                out.append([posed[m][i] for i in corners])
    return out


def largest_difference(original, exported):
    a, b = bmd.read(original), bmd.read(exported)
    if [x.num_keys for x in a.actions] != [x.num_keys for x in b.actions]:
        raise AssertionError('actions differ')
    if [(x.name, x.parent, x.dummy) for x in a.bones] != [(x.name, x.parent, x.dummy) for x in b.bones]:
        raise AssertionError('bones differ')
    worst = 0.0
    for action, info in enumerate(a.actions):
        for key in range(info.num_keys):
            ca = _corners(a, bmd.posed_vertices(a, action, key))
            cb = _corners(b, bmd.posed_vertices(b, action, key))
            if len(ca) != len(cb):
                raise AssertionError(f'{len(ca)} triangles became {len(cb)}')
            for ta, tb in zip(ca, cb):
                worst = max(worst, max(abs(x - y) for p, q in zip(ta, tb) for x, y in zip(p, q)))
    return worst


def _models(paths, sample):
    files = []
    for path in paths:
        if os.path.isdir(path):
            files += [os.path.join(root, n) for root, _, names in os.walk(path) for n in names
                      if n.lower().endswith('.bmd')]
        else:
            files.append(path)
    files = [f for f in files if open(f, 'rb').read(4)[:3] == b'BMD']
    return random.Random(1).sample(files, min(sample, len(files))) if sample else files


def main(argv):
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('paths', nargs='+')
    parser.add_argument('--sample', type=int, default=0)
    args = parser.parse_args(argv)
    work = tempfile.mkdtemp()
    failed = 0
    for path in _models(args.paths, args.sample):
        try:
            bmd.read(path)
        except ValueError as e:
            print(f'skip  {path}: {e}')
            continue
        try:
            blender_bmd._reset_scene()
            blender_bmd.import_bmd(path, work)
            blender_bmd.export_bmd(os.path.join(work, 'out.bmd'))
            worst = largest_difference(path, os.path.join(work, 'out.bmd'))
            ok = worst < TOLERANCE
            print(f'{"ok  " if ok else "FAIL"}  {worst:9.5f}  {path}')
        except Exception as e:  # report and go on with the next model
            ok = False
            print(f'FAIL  {path}: {e!r}')
        failed += not ok
    print(f'{failed} failed')
    return 1 if failed else 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[sys.argv.index('--') + 1:] if '--' in sys.argv else sys.argv[1:]))

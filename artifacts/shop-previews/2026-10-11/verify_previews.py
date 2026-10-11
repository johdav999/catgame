"""Clean-process checks of retained shop render masters and delivered PNGs."""
import bpy, json
from pathlib import Path
job = Path(__file__).resolve().parent
root = job.parents[2]
report = {}
for name in ['cushion', 'bowl']:
    bpy.ops.wm.open_mainfile(filepath=str(job / (name + '-preview.blend')))
    images = [i for i in bpy.data.images if i.type == 'IMAGE' and i.users]
    assert images and all(i.packed_file for i in images), 'Preview master needs packed textures'
    meshes = [o for o in bpy.context.scene.objects if o.type == 'MESH']
    assert len(meshes) == (2 if name == 'bowl' else 1)
    png = bpy.data.images.load(str(root / 'assets/runtime/models/garden' / (name + '-preview.png')))
    assert tuple(png.size) == (512, 512) and png.channels == 4
    report[name] = {'meshes': len(meshes), 'packed_images': [i.name for i in images],
                    'png_size': list(png.size), 'channels': png.channels}
(job / 'verification.json').write_text(json.dumps(report, indent=2) + '\n')

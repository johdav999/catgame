"""Render the unchanged accepted garden GLBs for phase-7 shop cards."""
import bpy, hashlib, json
from pathlib import Path
from mathutils import Vector

root = Path(__file__).resolve().parents[3]
job = Path(__file__).resolve().parent
runtime = root / 'assets/runtime/models/garden'
records = []
for asset, scale, target_y in [('cushion', 1.65, .10), ('bowl', 1.0, .20)]:
    bpy.ops.object.select_all(action='SELECT')
    bpy.ops.object.delete(use_global=False)
    source = runtime / (asset + '.glb')
    bpy.ops.import_scene.gltf(filepath=str(source))
    if asset == 'bowl':
        bpy.ops.import_scene.gltf(filepath=str(runtime / 'bowl_water.glb'))
        for obj in bpy.context.selected_objects:
            obj.location.z += .383
    scene = bpy.context.scene
    scene.render.engine = 'CYCLES'
    scene.cycles.samples = 48
    scene.render.resolution_x = scene.render.resolution_y = 512
    scene.render.resolution_percentage = 100
    scene.render.film_transparent = True
    scene.view_settings.view_transform = 'Standard'
    scene.view_settings.look = 'Medium High Contrast'
    scene.world.use_nodes = True
    scene.world.node_tree.nodes['Background'].inputs[0].default_value = (.65, .70, .74, 1)
    scene.world.node_tree.nodes['Background'].inputs[1].default_value = .55
    bpy.ops.object.camera_add(location=(1.6, -2.5, 2.4))
    camera = bpy.context.object
    camera.rotation_euler = (Vector((0, 0, target_y)) - camera.location).to_track_quat('-Z', 'Y').to_euler()
    camera.data.type = 'ORTHO'
    camera.data.ortho_scale = scale
    scene.camera = camera
    bpy.ops.object.light_add(type='AREA', location=(-2, -3, 4))
    bpy.context.object.data.energy = 220
    bpy.context.object.data.size = 4
    output = runtime / (asset + '-preview.png')
    scene.render.image_settings.file_format = 'PNG'
    scene.render.image_settings.color_mode = 'RGBA'
    scene.render.filepath = str(output)
    bpy.ops.file.pack_all()
    bpy.ops.wm.save_as_mainfile(filepath=str(job / (asset + '-preview.blend')))
    bpy.ops.render.render(write_still=True)
    records.append(dict(id='garden/' + asset + '-preview@1', source=source.relative_to(root).as_posix(),
                        source_sha256=hashlib.sha256(source.read_bytes()).hexdigest(),
                        output=output.relative_to(root).as_posix(), sha256=hashlib.sha256(output.read_bytes()).hexdigest(),
                        method='Blender 3.5.1 render of unchanged accepted GLB; transparent 512px PNG'))
    if asset == 'bowl':
        water = runtime / 'bowl_water.glb'
        records[-1]['water'] = dict(source=water.relative_to(root).as_posix(),
                                   sha256=hashlib.sha256(water.read_bytes()).hexdigest(),
                                   source_translation=[0, 0, .383])
(job / 'manifest.json').write_text(json.dumps(records, indent=2) + '\n')

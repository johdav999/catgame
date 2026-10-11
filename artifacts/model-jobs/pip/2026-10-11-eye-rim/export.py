import bpy,runpy,json,sys
from pathlib import Path
J=Path(__file__).parent
rev=int(sys.argv[sys.argv.index('--')+1])
bpy.ops.wm.open_mainfile(filepath=str(J/f'pip-blink-r{rev}.blend'))
s=bpy.context.scene;r=bpy.data.objects['PipRig'];m=bpy.data.objects['Pip repaired Tripo skin']
r.animation_data.action=bpy.data.actions['relaxed_idle'];s.frame_set(1)
bpy.ops.file.pack_all();bpy.ops.wm.save_as_mainfile(filepath=str(J/'pip-blink.blend'))
bpy.ops.object.select_all(action='DESELECT');r.select_set(True);m.select_set(True);bpy.context.view_layer.objects.active=r
bpy.ops.export_scene.gltf(filepath=str(J/'pip-blink.glb'),export_format='GLB',use_selection=True,export_animations=True,export_force_sampling=True,export_frame_range=False,export_nla_strips=True)
runpy.run_path(str(J.parent/'2026-10-10-tripo-repair/normalize_glb.py'))['normalize'](J/'pip-blink.glb')
bpy.data.objects.remove(m,do_unlink=True);bpy.data.objects.remove(r,do_unlink=True)
for a in list(bpy.data.actions):bpy.data.actions.remove(a)
bpy.ops.import_scene.gltf(filepath=str(J/'pip-blink.glb'))
r=next(o for o in s.objects if o.type=='ARMATURE')
for track in r.animation_data.nla_tracks:track.mute=True
r.animation_data.action=next(a for a in bpy.data.actions if 'relaxed_idle' in a.name)
s.frame_set(76)
m=next(o for o in s.objects if o.type=='MESH' and any(mod.type=='ARMATURE' for mod in o.modifiers))
(J/'clean-reimport.json').write_text(json.dumps({'triangles':sum(len(p.vertices)-2 for p in m.data.polygons),'materials':len(m.data.materials),'bones':len(r.data.bones),'actions':[a.name for a in bpy.data.actions],'max_influences':max(len(v.groups) for v in m.data.vertices)},indent=2))
bpy.ops.wm.save_as_mainfile(filepath=str(J/'clean-reimport.blend'))

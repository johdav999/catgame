import bpy,runpy,json
from pathlib import Path
from mathutils import Vector
J=Path(__file__).parent
bpy.ops.wm.open_mainfile(filepath=str(J/'pip-settle-r9.blend'))
s=bpy.context.scene;r=bpy.data.objects['PipRig'];m=bpy.data.objects['Pip repaired Tripo skin']
r.animation_data.action=bpy.data.actions['relaxed_idle'];s.frame_set(1)
idle={p.name:p.matrix_basis.copy() for p in r.pose.bones}
r.animation_data.action=bpy.data.actions['settle_cushion']
for p in r.pose.bones:
 p.matrix_basis=idle[p.name]
 for prop in ['location','rotation_quaternion','scale']:p.keyframe_insert(prop,frame=1)
for fc in r.animation_data.action.fcurves:
 for k in fc.keyframe_points:k.interpolation='LINEAR'
r.animation_data.action=bpy.data.actions['relaxed_idle'];s.frame_set(1)
bpy.ops.file.pack_all();bpy.ops.wm.save_as_mainfile(filepath=str(J/'pip-settle.blend'))
bpy.ops.object.select_all(action='DESELECT');r.select_set(True);m.select_set(True);bpy.context.view_layer.objects.active=r
bpy.ops.export_scene.gltf(filepath=str(J/'pip-settle.glb'),export_format='GLB',use_selection=True,export_animations=True,export_force_sampling=True,export_frame_range=False,export_nla_strips=True)
runpy.run_path(str(J.parent/'2026-10-10-tripo-repair/normalize_glb.py'))['normalize'](J/'pip-settle.glb')
# Clean reimport into the same neutral review rig, removing the source objects.
bpy.data.objects.remove(m,do_unlink=True);bpy.data.objects.remove(r,do_unlink=True)
for a in list(bpy.data.actions):bpy.data.actions.remove(a)
bpy.ops.import_scene.gltf(filepath=str(J/'pip-settle.glb'))
r=next(o for o in s.objects if o.type=='ARMATURE')
for track in r.animation_data.nla_tracks:track.mute=True
r.animation_data.action=next(a for a in bpy.data.actions if 'settle_cushion' in a.name)
s.frame_set(72)
cam=s.camera;cam.data.ortho_scale=1.8
for label,loc in [('front',(1.4,-2.4,1.15)),('side',(2.8,0,.85)),('rear',(-1.6,2.4,1.1))]:
 cam.location=loc;cam.rotation_euler=(Vector((0,0,.35))-cam.location).to_track_quat('-Z','Y').to_euler()
 s.render.filepath=str(J/('clean-'+label+'.png'));bpy.ops.render.render(write_still=True)
mesh=next(o for o in s.objects if o.type=='MESH' and any(mod.type=='ARMATURE' for mod in o.modifiers))
(J/'clean-reimport.json').write_text(json.dumps({'triangles':sum(len(p.vertices)-2 for p in mesh.data.polygons),'materials':len(mesh.data.materials),'bones':len(r.data.bones),'actions':[a.name for a in bpy.data.actions],'max_influences':max(len(v.groups) for v in mesh.data.vertices)},indent=2))

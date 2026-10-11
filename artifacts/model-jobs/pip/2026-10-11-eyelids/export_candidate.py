import bpy,sys,json,runpy
from pathlib import Path
from mathutils import Matrix,Vector
JOB=Path(__file__).parent;REV=int(sys.argv[sys.argv.index('--')+1])
bpy.ops.wm.open_mainfile(filepath=str(JOB/('pip-eyelids-r%d.blend'%REV)))
scene=bpy.context.scene;rig=bpy.data.objects['PipRig'];body=bpy.data.objects['Pip repaired Tripo skin']
# Bring accepted non-facial action edits into the editable master and FBX.
root=JOB.parents[3];manifest=json.loads((root/'assets/runtime/models/pip/manifest.json').read_text())
for name,revision in manifest.get('animation_revisions',{}).items():
 source=root/revision['source_master']
 if not source.exists() or name not in bpy.data.actions:continue
 target=bpy.data.actions[name]
 with bpy.data.libraries.load(str(source),link=False) as (available,loaded):
  assert name in available.actions,(source,name)
  loaded.actions=[name]
 incoming=loaded.actions[0]
 for curve in list(target.fcurves):
  if 'pose.bones["blink' not in curve.data_path:target.fcurves.remove(curve)
 for curve in incoming.fcurves:
  if 'pose.bones["blink' in curve.data_path:continue
  out=target.fcurves.new(curve.data_path,index=curve.array_index)
  out.keyframe_points.add(len(curve.keyframe_points))
  for a,b in zip(curve.keyframe_points,out.keyframe_points):
   b.co=a.co;b.interpolation=a.interpolation;b.handle_left_type=a.handle_left_type;b.handle_right_type=a.handle_right_type;b.handle_left=a.handle_left;b.handle_right=a.handle_right
 bpy.data.actions.remove(incoming)
for v in body.data.vertices:
 groups=sorted([(g.group,g.weight) for g in v.groups if g.weight>1e-7],key=lambda p:-p[1])[:4]
 total=sum(w for g,w in groups);assert total>0
 for g in list(v.groups):body.vertex_groups[g.group].remove([v.index])
 for g,w in groups:body.vertex_groups[g].add([v.index],w/total,'REPLACE')
scene.render.fps=30
rig.animation_data.action=bpy.data.actions['relaxed_idle'];scene.frame_set(1)
for action in bpy.data.actions:action.use_fake_user=True
export=JOB/'exports';export.mkdir(exist_ok=True)
cam=scene.camera;cam.location=(1.45,-2.25,1.15);cam.rotation_euler=(Vector((0,0,.5))-cam.location).to_track_quat('-Z','Y').to_euler();cam.data.ortho_scale=1.55
scene.render.resolution_x=960;scene.render.resolution_y=960
for frame,label in [(1,'open'),(75,'half'),(77,'closed')]:
 scene.frame_set(frame);scene.render.filepath=str(JOB/('r%d-game-%s.png'%(REV,label)));bpy.ops.render.render(write_still=True)
scene.frame_set(1)
bpy.ops.wm.save_as_mainfile(filepath=str(export/'pip-eyelids.blend'))
rig.animation_data.action=None
for p in rig.pose.bones:p.matrix_basis=Matrix.Identity(4)
bpy.context.view_layer.update();bpy.ops.object.select_all(action='DESELECT');body.select_set(True);rig.select_set(True);bpy.context.view_layer.objects.active=body
bpy.ops.export_scene.gltf(filepath=str(export/'pip-eyelids.glb'),export_format='GLB',use_selection=True,export_animations=True,export_nla_strips=True,export_force_sampling=True,export_frame_range=False,export_skins=True,export_morph=False,export_yup=True)
runpy.run_path(str(JOB.parent/'2026-10-10-tripo-repair/normalize_glb.py'))['normalize'](export/'pip-eyelids.glb')
bpy.ops.export_scene.fbx(filepath=str(export/'pip-eyelids-raw.fbx'),use_selection=True,object_types={'MESH','ARMATURE'},add_leaf_bones=False,apply_unit_scale=True,apply_scale_options='FBX_SCALE_ALL',axis_forward='-Z',axis_up='Y',bake_anim=True,bake_anim_use_all_actions=True,bake_anim_use_nla_strips=False,bake_anim_simplify_factor=0,path_mode='COPY',embed_textures=True)
from io_scene_fbx import parse_fbx,encode_bin
root,version=parse_fbx.parse(str(export/'pip-eyelids-raw.fbx'))
methods={'C':'add_bool','Y':'add_int16','I':'add_int32','L':'add_int64','F':'add_float32','D':'add_float64','S':'add_string','R':'add_bytes','f':'add_float32_array','d':'add_float64_array','i':'add_int32_array','l':'add_int64_array','b':'add_bool_array','c':'add_byte_array'}
def clone(node):
 out=encode_bin.FBXElem(node.id)
 for index,(typ,value) in enumerate(zip(node.props_type,node.props)):
  if node.id==b'AnimationStack' and index==1 and typ==ord('S'):
   name,suffix=value.split(b'\x00\x01',1);value=name.split(b'|')[-1]+b'\x00\x01'+suffix
  getattr(out,methods[chr(typ)])(value)
 out.elems=[clone(c) for c in node.elems];return out
encode_bin.write(str(export/'pip-eyelids.fbx'),clone(root),version)
report={'revision':REV,'triangles':sum(len(p.vertices)-2 for p in body.data.polygons),'vertices':len(body.data.vertices),'materials':len(body.data.materials),'bones':[b.name for b in rig.data.bones],'max_influences':max(len(v.groups) for v in body.data.vertices),'clips':[a.name for a in bpy.data.actions]}
(export/'measurements.json').write_text(json.dumps(report,indent=2));print(json.dumps(report))

"""Clean-process import and full-cycle, five-camera review of the merged GLB."""
import bpy,json
from pathlib import Path
from mathutils import Vector
JOB=Path(__file__).resolve().parents[1]
bpy.ops.wm.open_mainfile(filepath=str(JOB/'pip-walk-r1.blend'))
report={'master_images':[{'name':i.name,'packed':bool(i.packed_file)} for i in bpy.data.images if i.type=='IMAGE' and i.users]}
assert all(i['packed'] for i in report['master_images'])
for o in list(bpy.context.scene.objects):
 if o.type=='ARMATURE' or o.type=='MESH' and o.name!='Review ground':bpy.data.objects.remove(o,do_unlink=True)
for a in list(bpy.data.actions):bpy.data.actions.remove(a)
bpy.ops.import_scene.gltf(filepath=str(JOB/'exports/pip.glb'))
rig=next(o for o in bpy.context.scene.objects if o.type=='ARMATURE')
body=next(o for o in bpy.context.scene.objects if o.type=='MESH' and o.find_armature()==rig)
assert len(rig.data.bones)==29
assert len(bpy.data.actions)==9
assert all(abs(sum(g.weight for g in v.groups)-1)<1e-4 for v in body.data.vertices)
report.update(bones=len(rig.data.bones),triangles=sum(len(p.vertices)-2 for p in body.data.polygons),materials=len(body.data.materials),actions=[a.name for a in bpy.data.actions])
for track in rig.animation_data.nla_tracks:track.mute=True
rig.animation_data.action=next(a for a in bpy.data.actions if 'garden_walk' in a.name)
scene=bpy.context.scene;cam=scene.camera;scene.render.resolution_x=640;scene.render.resolution_y=640
scene.eevee.taa_render_samples=32;scene.frame_start=0;scene.frame_end=35
for view,loc in [('side',(2.8,0,.65)),('front',(0,-2.8,.65)),('rear',(0,2.8,.65)),('three-quarter',(1.45,-2.25,1.15)),('puzzle',(2.4,-3.5,3.5))]:
 cam.location=loc;cam.rotation_euler=(Vector((0,0,.50))-cam.location).to_track_quat('-Z','Y').to_euler();cam.data.ortho_scale=1.55 if view!='puzzle' else 2.4
 for frame in range(36):
  scene.frame_set(frame);scene.render.filepath=str(JOB/'renders'/('clean-%s-%02d.png'%(view,frame)));bpy.ops.render.render(write_still=True)
(JOB/'exports/clean-reimport.json').write_text(json.dumps(report,indent=2))
print(json.dumps(report))

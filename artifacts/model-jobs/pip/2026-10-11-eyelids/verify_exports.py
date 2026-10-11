import bpy,json,sys
from pathlib import Path
from mathutils import Vector
JOB=Path(__file__).parent;report={}
bpy.ops.wm.open_mainfile(filepath=str(JOB/'exports/pip-eyelids.blend'))
rig=bpy.data.objects['PipRig'];master={b.name:(b.parent.name if b.parent else None,[list(r) for r in b.matrix_local]) for b in rig.data.bones}
assert len(master)==31 and len(bpy.data.actions)==9
with bpy.data.libraries.load(str(JOB.parent/'2026-10-10-tripo-repair/pip-repaired.blend'),link=False) as (a,b):b.armatures=['Paws cat rig v1']
old=b.armatures[0]
for bone in old.bones:
 parent,matrix=master[bone.name];assert parent==(bone.parent.name if bone.parent else None)
 assert max(abs(matrix[i][j]-bone.matrix_local[i][j]) for i in range(4) for j in range(4))<1e-7
report['original_29_bones']='names, parents and all local bind matrix entries unchanged'
for extension,filename in [('glb','pip-merged.glb'),('fbx','pip-eyelids.fbx')]:
 bpy.ops.wm.read_factory_settings(use_empty=True)
 bpy.context.scene.render.fps=30
 if extension=='glb':bpy.ops.import_scene.gltf(filepath=str(JOB/'exports'/filename))
 else:bpy.ops.import_scene.fbx(filepath=str(JOB/'exports'/filename))
 rig=next(o for o in bpy.context.scene.objects if o.type=='ARMATURE');meshes=[o for o in bpy.context.scene.objects if o.type=='MESH'];assert len(meshes)==1
 mesh=meshes[0];assert set(b.name for b in rig.data.bones)==set(master)
 assert len(bpy.data.actions)==9
 assert all(abs(sum(g.weight for g in v.groups)-1)<1e-4 and 1<=len(v.groups)<=4 for v in mesh.data.vertices)
 triangles=sum(len(p.vertices)-2 for p in mesh.data.polygons);assert triangles<=22000
 for action in bpy.data.actions:
  rig.animation_data.action=action
  for f in range(round(action.frame_range[0]),round(action.frame_range[1])+1):
   bpy.context.scene.frame_set(f)
   for name in ['blink.L','blink.R','blink.lower.L','blink.lower.R']:
    assert max(abs(v-1) for v in rig.pose.bones[name].scale)<1e-4
 report[extension]={'triangles':triangles,'vertices':len(mesh.data.vertices),'materials':len(mesh.data.materials),'bones':len(rig.data.bones),'clips':[a.name for a in bpy.data.actions],'normalized_weights':True,'unit_eyelid_scales_every_frame':True}
 if extension=='fbx':continue
 scene=bpy.context.scene;scene.render.engine='BLENDER_EEVEE';scene.render.resolution_x=960;scene.render.resolution_y=960;scene.render.resolution_percentage=100
 scene.view_settings.view_transform='Standard';scene.view_settings.look='Medium High Contrast';scene.world=bpy.data.worlds.new('Review world');scene.world.color=(.25,.25,.25)
 for name,loc,energy,size in [('Key',(-3,-4,6),450,4),('Fill',(4,-1,3),250,5),('Rim',(0,4,5),300,3)]:
  d=bpy.data.lights.new(name,'AREA');o=bpy.data.objects.new(name,d);scene.collection.objects.link(o);o.location=loc;d.energy=energy;d.size=size;o.rotation_euler=(Vector((0,0,.5))-o.location).to_track_quat('-Z','Y').to_euler()
 bpy.ops.object.camera_add();cam=bpy.context.object;scene.camera=cam;cam.data.type='ORTHO';cam.data.ortho_scale=1.55
 for track in rig.animation_data.nla_tracks:track.mute=True
 rig.animation_data.action=next(a for a in bpy.data.actions if 'relaxed_idle' in a.name)
 for label,frame,location in [('open',1,(1.45,-2.25,1.15)),('closed',77,(1.45,-2.25,1.15)),('side-closed',77,(2.3,-.8,1.05))]:
  scene.frame_set(frame);cam.location=location;cam.rotation_euler=(Vector((0,0,.5))-cam.location).to_track_quat('-Z','Y').to_euler()
  scene.render.filepath=str(JOB/('clean-'+label+'.png'));bpy.ops.render.render(write_still=True)
(JOB/'exports/clean-reimport.json').write_text(json.dumps(report,indent=2));print(json.dumps(report))

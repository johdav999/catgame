"""Independent packed-master and clean FBX/GLB reimport checks."""
import bpy,json,sys
from pathlib import Path
from mathutils import Vector
JOB=Path(__file__).parent
rev=int(sys.argv[sys.argv.index('--')+1]);report={}
bpy.ops.wm.open_mainfile(filepath=str(JOB/('pip-repaired-r%d.blend'%rev)))
source_rig=bpy.data.objects['PipRig']
assert len(bpy.data.actions)==9,'Packed master must preserve all actions'
master_bones={b.name:{'parent':b.parent.name if b.parent else None,'matrix':[list(row) for row in b.matrix_local]} for b in source_rig.data.bones}
packed=[{'name':i.name,'packed':bool(i.packed_file),'size':list(i.size)} for i in bpy.data.images if i.type=='IMAGE' and i.users]
assert all(i['packed'] for i in packed)
report['packed_master_images']=packed
for extension in ['glb','fbx']:
 bpy.ops.wm.read_factory_settings(use_empty=True)
 file=JOB/'exports'/('pip-repaired-r%d.glb'%rev if extension=='glb' else 'pip-repaired.fbx')
 if extension=='glb':bpy.ops.import_scene.gltf(filepath=str(file))
 else:bpy.ops.import_scene.fbx(filepath=str(file))
 meshes=[o for o in bpy.context.scene.objects if o.type=='MESH'];assert len(meshes)==1
 mesh=meshes[0];rig=next(o for o in bpy.context.scene.objects if o.type=='ARMATURE')
 weights=[sum(g.weight for g in v.groups) for v in mesh.data.vertices]
 assert all(abs(w-1)<1e-4 for w in weights)
 assert max(len(v.groups) for v in mesh.data.vertices)<=4
 assert set(b.name for b in rig.data.bones)==set(master_bones)
 tris=sum(len(p.vertices)-2 for p in mesh.data.polygons);assert tris<=18000
 assert len(mesh.data.materials)<=2
 assert len(bpy.data.actions)==9
 report[extension]={'triangles':tris,'vertices':len(mesh.data.vertices),'bones':len(rig.data.bones),'materials':len(mesh.data.materials),'actions':[a.name for a in bpy.data.actions],'unweighted':sum(w<1e-6 for w in weights),'max_influences':max(len(v.groups) for v in mesh.data.vertices),'bounds':[[min((mesh.matrix_world@v.co)[i] for v in mesh.data.vertices) for i in range(3)],[max((mesh.matrix_world@v.co)[i] for v in mesh.data.vertices) for i in range(3)]]}
 if extension=='fbx':continue
 scene=bpy.context.scene;scene.render.engine='BLENDER_EEVEE';scene.eevee.use_gtao=True;scene.eevee.gtao_distance=.08
 scene.render.resolution_x=960;scene.render.resolution_y=960;scene.render.resolution_percentage=100
 scene.view_settings.view_transform='Standard';scene.view_settings.look='Medium High Contrast';scene.world=bpy.data.worlds.new('Review world');scene.world.color=(.25,.25,.25)
 bpy.ops.mesh.primitive_plane_add(size=200,location=(0,0,-.006));floor=bpy.context.object
 mat=bpy.data.materials.new('Review ground');mat.diffuse_color=(.50,.57,.49,1);floor.data.materials.append(mat)
 for name,loc,energy,size in [('Key',(-3,-4,6),450,4),('Fill',(4,-1,3),250,5),('Rim',(0,4,5),300,3)]:
  d=bpy.data.lights.new(name,'AREA');o=bpy.data.objects.new(name,d);scene.collection.objects.link(o);o.location=loc;d.energy=energy;d.size=size;o.rotation_euler=(Vector((0,0,.5))-o.location).to_track_quat('-Z','Y').to_euler()
 bpy.ops.object.camera_add();cam=bpy.context.object;scene.camera=cam;cam.data.type='ORTHO';cam.data.ortho_scale=1.85
 for track in rig.animation_data.nla_tracks:track.mute=True
 for name,frame in [('relaxed_idle',1),('garden_walk',10),('curled_rest',30),('drink',16)]:
  rig.animation_data.action=next(a for a in bpy.data.actions if name in a.name);scene.frame_set(frame)
  cam.location=(1.45,-2.25,1.15);cam.rotation_euler=(Vector((.14,0,.5))-cam.location).to_track_quat('-Z','Y').to_euler()
  scene.render.filepath=str(JOB/'renders'/('clean-'+name+'.png'));bpy.ops.render.render(write_still=True)
(JOB/'exports/clean-reimport.json').write_text(json.dumps(report,indent=2));print(json.dumps(report))


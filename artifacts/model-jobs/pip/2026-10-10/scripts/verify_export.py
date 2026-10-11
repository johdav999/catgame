"""Clean-process packed master and GLB inspection, independent of source scene."""
import bpy,json,math
from pathlib import Path
from mathutils import Vector
JOB=Path(__file__).resolve().parents[1]
bpy.ops.wm.open_mainfile(filepath=str(JOB/'pip-master.blend'))
packed=[{'name':i.name,'packed':bool(i.packed_file),'size':list(i.size)} for i in bpy.data.images if i.type=='IMAGE']
assert all(i['packed'] for i in packed),packed
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.gltf(filepath=str(JOB/'exports/pip.glb'))
meshes=[o for o in bpy.context.scene.objects if o.type=='MESH'];rig=next(o for o in bpy.context.scene.objects if o.type=='ARMATURE')
assert len(meshes)==1
mesh=meshes[0];tris=sum(len(p.vertices)-2 for p in mesh.data.polygons)
assert tris<=18000,tris
assert len(mesh.data.materials)<=2
assert len(bpy.data.actions)==9,[a.name for a in bpy.data.actions]
points=[mesh.matrix_world@v.co for v in mesh.data.vertices]
bounds=[[min(p[i] for p in points) for i in range(3)],[max(p[i] for p in points) for i in range(3)]]
weights=[sum(g.weight for g in v.groups) for v in mesh.data.vertices]
assert all(abs(w-1)<1e-4 for w in weights)
assert max(len(v.groups) for v in mesh.data.vertices)<=4
# Retained import measurements; map names can be exporter-generated.
report={'triangles':tris,'vertices':len(mesh.data.vertices),'materials':len(mesh.data.materials),'bones':len(rig.data.bones),'actions':[a.name for a in bpy.data.actions],'bounds_blender':bounds,'packed_master_images':packed,'max_influences':max(len(v.groups) for v in mesh.data.vertices)}
(JOB/'exports/clean-reimport.json').write_text(json.dumps(report,indent=2))
scene=bpy.context.scene;scene.render.engine='CYCLES';scene.cycles.samples=32;scene.render.resolution_x=960;scene.render.resolution_y=960;scene.render.resolution_percentage=100
scene.view_settings.view_transform='Standard';scene.view_settings.look='Medium High Contrast';scene.world=bpy.data.worlds.new('Review world');scene.world.color=(.34,.34,.34)
floor=bpy.data.materials.new('Review sandstone');floor.diffuse_color=(.57,.62,.52,1)
bpy.ops.mesh.primitive_plane_add(size=200);plane=bpy.context.object;plane.location.z=-.005;plane.data.materials.append(floor)
for name,loc,energy,size in [('Key',(-3,-4,6),430,4),('Fill',(4,-1,3),180,5),('Rim',(0,4,5),260,3)]:
    d=bpy.data.lights.new(name,'AREA');o=bpy.data.objects.new(name,d);scene.collection.objects.link(o);o.location=loc;d.energy=energy;d.size=size;o.rotation_euler=(Vector((0,0,.45))-o.location).to_track_quat('-Z','Y').to_euler()
bpy.ops.object.camera_add(location=(1.45,-2.25,1.28));camera=bpy.context.object;camera.rotation_euler=(Vector((0,0,.51))-camera.location).to_track_quat('-Z','Y').to_euler();camera.data.type='ORTHO';camera.data.ortho_scale=1.47;scene.camera=camera
for track in rig.animation_data.nla_tracks:track.mute=True
def find(name):return next(a for a in bpy.data.actions if name in a.name)
for name,frame in [('relaxed_idle',1),('curled_rest',30),('drink',16),('garden_walk',10)]:
    rig.animation_data.action=find(name);scene.frame_set(frame);scene.render.filepath=str(JOB/('renders/reimport-'+name+'.png'));bpy.ops.render.render(write_still=True)
bpy.ops.wm.save_as_mainfile(filepath=str(JOB/'exports/clean-reimport.blend'))
print(json.dumps(report))

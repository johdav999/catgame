import bpy,sys,math,json
from pathlib import Path
from mathutils import Vector,Matrix
JOB=Path(__file__).parent
ROOT=JOB.parents[3]
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.fbx(filepath=str(ROOT/'assets/Pip/pip.fbx'))
mesh=next(o for o in bpy.context.scene.objects if o.type=='MESH')
for mod in list(mesh.modifiers):mesh.modifiers.remove(mod)
mesh.parent=None
# Bake source object transformation, orient the cat to canonical -Y forward.
transform=Matrix.Diagonal((.92,1.25,1.25,1))@Matrix.Rotation(math.radians(72),4,'Z')@mesh.matrix_world
for v in mesh.data.vertices:v.co=transform@v.co
mesh.matrix_world=Matrix.Identity(4)
mesh.vertex_groups.clear()
# Centre left/right; the imported cat is intentionally a little asymmetric.
coords=[v.co for v in mesh.data.vertices]
xmid=(min(p.x for p in coords)+max(p.x for p in coords))*.5
zmin=min(p.z for p in coords)
for v in mesh.data.vertices:v.co.x-=xmid;v.co.z-=zmin
(JOB/'source-transform.json').write_text(json.dumps({'xmid':xmid,'zmin':zmin,'yaw_degrees':72}))
scene=bpy.context.scene
scene.render.engine='BLENDER_EEVEE'
scene.eevee.use_gtao=True;scene.eevee.gtao_distance=.08
scene.render.resolution_x=960;scene.render.resolution_y=960;scene.render.resolution_percentage=100
scene.view_settings.view_transform='Standard';scene.view_settings.look='Medium High Contrast'
scene.world=bpy.data.worlds.new('Review world');scene.world.color=(.4,.4,.4)
bpy.ops.mesh.primitive_plane_add(size=200,location=(0,0,-.006))
floor=bpy.context.object;floor.name='Review ground'
mat=bpy.data.materials.new('Review ground');mat.diffuse_color=(.50,.57,.49,1);floor.data.materials.append(mat)
for name,loc,energy,size in [('Key',(-3,-4,6),450,4),('Fill',(4,-1,3),250,5),('Rim',(0,4,5),300,3)]:
 d=bpy.data.lights.new(name,'AREA');o=bpy.data.objects.new(name,d);scene.collection.objects.link(o);o.location=loc;d.energy=energy;d.size=size;o.rotation_euler=(Vector((0,0,.5))-o.location).to_track_quat('-Z','Y').to_euler()
bpy.ops.object.camera_add()
cam=bpy.context.object;scene.camera=cam;cam.data.type='ORTHO';cam.data.ortho_scale=1.55
(JOB/'renders').mkdir(exist_ok=True)
for name,loc in [('source-three-quarter',(1.45,-2.25,1.15)),('source-side',(2.8,0,.75)),('source-front',(0,-3,.8))]:
 cam.location=loc;cam.rotation_euler=(Vector((0,0,.5))-cam.location).to_track_quat('-Z','Y').to_euler()
 scene.render.filepath=str(JOB/'renders'/f'{name}.png');bpy.ops.render.render(write_still=True)
bpy.ops.wm.save_as_mainfile(filepath=str(JOB/'source-oriented.blend'))
print('READY')


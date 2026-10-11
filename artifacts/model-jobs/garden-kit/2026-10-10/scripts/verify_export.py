"""Reopen packed master, independently import every GLB, measure and render."""
import bpy,json,sys,math
from pathlib import Path
from mathutils import Vector
JOB=Path(__file__).resolve().parents[1];ROOT=JOB.parents[3]
revision=int(sys.argv[-1]) if sys.argv[-1].isdigit() else 3
folder=JOB/'exports'/f'r{revision}';manifest=json.loads((folder/'asset-manifest.json').read_text())
bpy.ops.wm.open_mainfile(filepath=str(JOB/f'garden-kit-r{revision}.blend'))
packed=[{'name':i.name,'size':list(i.size),'packed':bool(i.packed_file)} for i in bpy.data.images if i.type=='IMAGE']
assert all(i['packed'] for i in packed),packed
bpy.ops.wm.read_factory_settings(use_empty=True)
groups={};measurements=[]
for a in manifest['assets']:
    before=set(bpy.data.objects);bpy.ops.import_scene.gltf(filepath=str(folder/a['file']));objects=list(set(bpy.data.objects)-before);meshes=[o for o in objects if o.type=='MESH'];groups[a['file'][:-4]]=objects
    points=[o.matrix_world@v.co for o in meshes for v in o.data.vertices]
    lo=[min(p[i] for p in points) for i in range(3)];hi=[max(p[i] for p in points) for i in range(3)]
    actual={'min':[lo[0],lo[2],-hi[1]],'max':[hi[0],hi[2],-lo[1]]}
    assert all(abs(actual[k][i]-a['bounds'][k][i])<.001 for k in actual for i in range(3)),(a['id'],actual,a['bounds'])
    triangles=sum(len(p.vertices)-2 for o in meshes for p in o.data.polygons);assert triangles==a['triangles']
    assert all(o.data.uv_layers for o in meshes)
    for o in meshes:
        for v in o.data.vertices:assert all(math.isfinite(c) for c in v.co)
    measurements.append({'id':a['id'],'bounds':actual,'triangles':triangles,'material_slots':sum(len(o.data.materials) for o in meshes)})
for i in manifest['review_instances']:
    x,z,ny=i['position']
    for o in groups[i['asset']]:
        if not o.parent:o.location+=Vector((x,-ny,z))
scene=bpy.context.scene;scene.render.engine='CYCLES';scene.cycles.samples=32;scene.render.resolution_x=1400;scene.render.resolution_y=1100;scene.render.resolution_percentage=100
scene.view_settings.view_transform='Standard';scene.view_settings.look='Medium High Contrast';scene.world=bpy.data.worlds.new('Review world');scene.world.color=(.3,.3,.3)
floor=bpy.data.materials.new('Neutral review floor');floor.diffuse_color=(.6,.64,.53,1)
bpy.ops.mesh.primitive_plane_add(size=200);bpy.context.object.location.z=-.005;bpy.context.object.data.materials.append(floor)
for loc,power,size in [((-3,-4,8),850,5),((4,2,6),650,5)]:
    bpy.ops.object.light_add(type='AREA',location=loc);l=bpy.context.object;l.data.energy=power;l.data.shape='DISK';l.data.size=size;l.rotation_euler=(Vector((0,0,0))-l.location).to_track_quat('-Z','Y').to_euler()
bpy.ops.object.camera_add(location=(7,-10,10));c=bpy.context.object;c.rotation_euler=(Vector((0,0,.2))-c.location).to_track_quat('-Z','Y').to_euler();c.data.type='ORTHO';c.data.ortho_scale=7.3;scene.camera=c
scene.render.filepath=str(JOB/'renders'/f'r{revision}-reimport.png');bpy.ops.render.render(write_still=True)
for name,loc in [('rear',(-7,10,8)),('side',(10,0,5)),('top',(0,0,12))]:
    c.location=loc;c.rotation_euler=(Vector((0,0,.2))-c.location).to_track_quat('-Z','Y').to_euler()
    scene.render.filepath=str(JOB/'renders'/f'r{revision}-{name}.png');bpy.ops.render.render(write_still=True)
# Include delivered Pip, never a proportion stand-in.
bpy.ops.import_scene.gltf(filepath=str(ROOT/'assets/runtime/models/pip/pip.glb'))
rig=next(o for o in scene.objects if o.type=='ARMATURE')
for track in rig.animation_data.nla_tracks:track.mute=True
rig.location=(1.7,-1.2,.16)
rig.animation_data.action=next(a for a in bpy.data.actions if 'curled_rest' in a.name);scene.frame_set(1)
c.location=(3.5,-4.3,3.0);c.rotation_euler=(Vector((1.7,-1.2,.3))-c.location).to_track_quat('-Z','Y').to_euler();c.data.ortho_scale=1.9
scene.render.filepath=str(JOB/'renders'/f'r{revision}-pip-cushion.png');bpy.ops.render.render(write_still=True)
rig.location=(2.45,.36327,0);rig.animation_data.action=next(a for a in bpy.data.actions if 'drink' in a.name);scene.frame_set(1)
c.location=(4.4,-2.4,1.5);c.rotation_euler=(Vector((2.45,-.0,.28))-c.location).to_track_quat('-Z','Y').to_euler();c.data.ortho_scale=1.65
scene.render.filepath=str(JOB/'renders'/f'r{revision}-pip-bowl.png');bpy.ops.render.render(write_still=True)
(folder/'clean-reimport.json').write_text(json.dumps({'packed_images':packed,'assets':measurements},indent=2))
bpy.ops.wm.save_as_mainfile(filepath=str(folder/'clean-reimport.blend'))
print('CLEAN_REIMPORT_PASSED',len(measurements))

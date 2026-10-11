"""Repair the supplied Tripo Pip using the exact existing Paws bind rig."""
import bpy,math,json,ast,sys,argparse
import numpy as np
from pathlib import Path
from mathutils import Vector,Matrix
JOB=Path(__file__).parent;ROOT=JOB.parents[3];OLD=JOB.parent/'2026-10-10'
parser=argparse.ArgumentParser();parser.add_argument('--revision',type=int,default=1);args=parser.parse_args(sys.argv[sys.argv.index('--')+1:] if '--' in sys.argv else []);REV=args.revision
bpy.ops.wm.open_mainfile(filepath=str(JOB/'source-oriented.blend'))
scene=bpy.context.scene
scene.render.fps=30;scene.render.fps_base=1
body=next(o for o in scene.objects if o.type=='MESH' and o.name!='Review ground')
body.name='Pip repaired Tripo skin'
for o in list(scene.objects):
 if o.type=='ARMATURE':bpy.data.objects.remove(o,do_unlink=True)
with bpy.data.libraries.load(str(OLD/'pip-master.blend'),link=False) as (source,destination):
 destination.objects=['PipRig'];destination.actions=list(source.actions)
rig=bpy.data.objects['PipRig'];scene.collection.objects.link(rig)
rig.animation_data_clear()
for b in rig.pose.bones:b.matrix_basis=Matrix.Identity(4)
bones={b.name:tuple(b.head_local) for b in rig.data.bones}
if REV==3:
 # Align the source tail centreline to the canonical bind curve. Preserve its
 # cross section, UVs and markings rather than binding a displaced hook.
 source_report=json.loads((JOB.parent/'2026-10-10-fbx-check/blender-inspection.json').read_text())
 oldbones={b['name']:b for b in source_report['armatures'][0]['bones']}
 transform=Matrix.Diagonal((.92,1.25,1.25,1))@Matrix.Rotation(math.radians(72),4,'Z')
 shift=json.loads((JOB/'source-transform.json').read_text())
 def source_point(p):
  q=transform@Vector(p);q.x-=shift['xmid'];q.z-=shift['zmin'];return q
 source_curve=[source_point(oldbones['bone_%d'%i]['head']) for i in range(28,33)]+[source_point(oldbones['bone_32']['tail'])]
 target_curve=[Vector(bones['tail.%02d'%i]) for i in range(7)]+[Vector((.04,.316,1.025))]
 def lengths(curve):
  out=[0]
  for a,b in zip(curve,curve[1:]):out.append(out[-1]+(b-a).length)
  return [v/out[-1] for v in out]
 source_t=lengths(source_curve);target_t=lengths(target_curve)
 def smooth(a,b,v):
  t=max(0,min(1,(v-a)/(b-a)));return t*t*(3-2*t)
 for v in body.data.vertices:
  influence=smooth(.48,.65,v.co.z)*smooth(.22,.34,v.co.y)
  if influence<1e-6:continue
  best=None
  for i,(a,b) in enumerate(zip(source_curve,source_curve[1:])):
   direction=b-a;t=max(0,min(1,(v.co-a).dot(direction)/direction.length_squared));p=a+t*direction;distance=(v.co-p).length
   if best is None or distance<best[0]:best=(distance,i,t,p)
  _,i,t,p=best;u=source_t[i]+t*(source_t[i+1]-source_t[i])
  j=next((j for j in range(7) if u<=target_t[j+1]),6);f=(u-target_t[j])/(target_t[j+1]-target_t[j])
  dest=target_curve[j].lerp(target_curve[j+1],f)
  rotation=(source_curve[i+1]-source_curve[i]).rotation_difference(target_curve[j+1]-target_curve[j])
  corrected=dest+rotation@(v.co-p);v.co=v.co.lerp(corrected,influence)
# Original geometry/material/UVs retained. Decimation preserves UV and silhouette.
bpy.ops.object.select_all(action='DESELECT');body.select_set(True);bpy.context.view_layer.objects.active=body
dec=body.modifiers.new('Paws triangle budget','DECIMATE');dec.ratio=.88
bpy.ops.object.modifier_apply(modifier=dec.name)
for p in body.data.polygons:p.use_smooth=True
# Downsample originals once in linear-aware Blender image storage; keep sources.
(JOB/'textures').mkdir(exist_ok=True)
material=body.data.materials[0];material.name='Pip Tripo portable coat'
material.use_nodes=True;nodes=material.node_tree.nodes;links=material.node_tree.links
nodes.clear();out=nodes.new('ShaderNodeOutputMaterial');pbr=nodes.new('ShaderNodeBsdfPrincipled');links.new(pbr.outputs['BSDF'],out.inputs['Surface'])
pbr.inputs['Roughness'].default_value=.82;pbr.inputs['Metallic'].default_value=0
source_dir=next((ROOT/'assets/Pip').glob('*.fbm'))
def image_node(kind,ext,color):
 file=next(source_dir.glob('*_'+kind+'.'+ext));im=bpy.data.images.load(str(file),check_existing=False);im.name='pip-'+kind+'-1024';im.colorspace_settings.name=color;im.scale(1024,1024)
 im.filepath_raw=str(JOB/'textures'/('pip-'+kind+'.png'));im.file_format='PNG';im.save();im.pack()
 n=nodes.new('ShaderNodeTexImage');n.image=im;return n
base=image_node('basecolor','JPEG','sRGB');rough=image_node('roughness','JPEG','Non-Color');metal=image_node('metallic','JPEG','Non-Color');normal=image_node('normal','PNG','Non-Color')
links.new(base.outputs['Color'],pbr.inputs['Base Color']);links.new(rough.outputs['Color'],pbr.inputs['Roughness']);links.new(metal.outputs['Color'],pbr.inputs['Metallic'])
nm=nodes.new('ShaderNodeNormalMap');nm.inputs['Strength'].default_value=.45;links.new(normal.outputs['Color'],nm.inputs['Color']);links.new(nm.outputs['Normal'],pbr.inputs['Normal'])
# Reuse existing anatomical distance fields. No dependency on old Tripo weights.
tree=ast.parse((OLD/'scripts/build_pip.py').read_text())
functions=[n for n in tree.body if isinstance(n,ast.FunctionDef) and n.name in ['smoothstep','weights']]
namespace={'Vector':Vector,'math':math,'REV':9,'bones':bones,'tailpoints':[tuple(rig.data.bones['tail.%02d'%i].head_local) for i in range(7)]+[(.04,.316,1.025)]}
exec(compile(ast.Module(body=functions,type_ignores=[]),str(OLD/'scripts/build_pip.py'),'exec'),namespace)
body.vertex_groups.clear()
for name in bones:body.vertex_groups.new(name=name)
for v in body.data.vertices:
 values=dict(namespace['weights'](v.co))
 if REV>=2:
  # Source head is wider/turned; rigid face prevents cheek and eye warping.
  h=namespace['smoothstep'](.47,.59,v.co.z)*namespace['smoothstep'](-.19,-.28,v.co.y)
  values={n:w*(1-h) for n,w in values.items()}
  e=namespace['smoothstep'](.81,.92,v.co.z)*namespace['smoothstep'](.08,.14,abs(v.co.x))
  values['head']=values.get('head',0)+h*(1-e)
  ear='ear.R' if v.co.x>0 else 'ear.L';values[ear]=values.get(ear,0)+h*e
 best=sorted(values.items(),key=lambda item:item[1],reverse=True)[:4];total=sum(w for n,w in best)
 for n,w in best:
  if w/total>1e-7:body.vertex_groups[n].add([v.index],w/total,'REPLACE')
mod=body.modifiers.new('Canonical Paws skin','ARMATURE');mod.object=rig;body.parent=rig;body.matrix_parent_inverse=Matrix.Identity(4)
# Small opaque eyelid caps let the canonical blink bones close textured eyes.
# Rest bone transforms stay exact; cap centres are compensated in pose channels.
eye_centres={}
if REV>=4:
 pixels=np.array(base.image.pixels[:]).reshape(1024,1024,4)
 dark_points=[]
 for loop in body.data.loops:
  v=body.data.vertices[loop.vertex_index];uv=body.data.uv_layers.active.data[loop.index].uv
  col=pixels[int((uv.y%1)*1023),int((uv.x%1)*1023),:3]
  if .57<v.co.z<.81 and v.co.y<-.27 and sum(col)<.35:
   dark_points.append(v.co.copy())
 data=np.array(dark_points);assert len(data)>10,'Cannot find eye regions'
 centres=np.array([data[data[:,0].argmin()],data[data[:,0].argmax()]])
 for iteration in range(20):
  assignment=np.argmin(((data[:,None,:]-centres[None,:,:])**2).sum(axis=2),axis=1)
  centres=np.array([np.mean(data[assignment==i],axis=0) for i in range(2)])
 dark={'L':data[assignment==0],'R':data[assignment==1]}
 caps=[]
 lidmat=bpy.data.materials.new('Pip soft ginger eyelids');lidmat.use_nodes=True
 lidshader=lidmat.node_tree.nodes.get('Principled BSDF');lidshader.inputs['Base Color'].default_value=(.78,.36,.105,1);lidshader.inputs['Roughness'].default_value=.86
 eye_front=Vector((centres[1,1]-centres[0,1],centres[0,0]-centres[1,0],0)).normalized()
 lidimage=bpy.data.images.new('Pip closed-eyelid colour',width=64,height=64)
 yy,xx=np.mgrid[0:64,0:64];u=xx/63;v=yy/63
 lid_pixels=np.ones((64,64,4));lid_pixels[:,:,:3]=(.84,.46,.18)
 line=np.abs(v-(.48+.12*(u-.5)**2))<.018
 lid_pixels[line,:3]=(.16,.055,.022);lidimage.pixels=lid_pixels.ravel();lidimage.pack()
 lidtex=lidmat.node_tree.nodes.new('ShaderNodeTexImage');lidtex.image=lidimage;lidmat.node_tree.links.new(lidtex.outputs['Color'],lidshader.inputs['Base Color'])
 for label in ['L','R']:
  assert len(dark[label]),('Cannot find eye',label)
  centre=Vector(np.median(np.array(dark[label]),axis=0))+eye_front*.020
  eye_centres['blink.'+label]=centre.copy()
  bpy.ops.mesh.primitive_uv_sphere_add(segments=16,ring_count=8,location=centre)
  cap=bpy.context.object;cap.name='Pip eyelid '+label;cap.scale=(.052,.018,.054);cap.rotation_euler.z=math.atan2(eye_front.x,-eye_front.y)
  bpy.ops.object.transform_apply(location=True,rotation=True,scale=True)
  tangent=Vector((-eye_front.y,eye_front.x,0))
  for loop in cap.data.loops:
   q=cap.data.vertices[loop.vertex_index].co-centre
   cap.data.uv_layers.active.data[loop.index].uv=(.5+q.dot(tangent)/.104,.5+q.z/.108)
  for p in cap.data.polygons:p.use_smooth=True
  cap.data.materials.append(lidmat);vg=cap.vertex_groups.new(name='blink.'+label);vg.add(list(range(len(cap.data.vertices))),1,'REPLACE');caps.append(cap)
 bpy.ops.object.select_all(action='DESELECT');body.select_set(True)
 for cap in caps:cap.select_set(True)
 bpy.context.view_layer.objects.active=body;bpy.ops.object.join()
 print('EYE CENTRES',eye_centres)
# Duplicate the accepted actions; original masters stay untouched.
clip_names=['relaxed_idle','attentive_request','happy_pounce','garden_walk','settle_cushion','curled_rest','approach_bowl','drink','return_idle']
rig.animation_data_create()
positions=np.array([(*v.co,1) for v in body.data.vertices])
skin_weights=np.zeros((len(body.data.vertices),len(bones)));names=list(bones)
for v in body.data.vertices:
 for g in v.groups:skin_weights[v.index,names.index(body.vertex_groups[g.group].name)]=g.weight
inverse={n:rig.data.bones[n].matrix_local.inverted() for n in names}
def posed_points():
 bpy.context.view_layer.update();pts=np.zeros((len(positions),3))
 for i,n in enumerate(names):
  m=np.array(rig.pose.bones[n].matrix@inverse[n]);pts+=(positions@m.T)[:,:3]*skin_weights[:,i,None]
 return pts
for name in clip_names:
 action=bpy.data.actions[name];action.use_fake_user=True;rig.animation_data.action=action
 # Preserve all canonical rotations/scales; adapt pelvis contact to actual mesh.
 for f in range(1,round(action.frame_range[1])+1):
  scene.frame_set(f);pts=posed_points()
  if REV>=4:
   # Retarget the resting tail as a continuous bend, keeping source silhouette
   # and avoiding the rejected geometry warping in revision 3.
   if name in ['settle_cushion','curled_rest']:
    t=namespace['smoothstep'](0,1,(f-1)/(round(action.frame_range[1])-1)) if name=='settle_cushion' else 1
    bind=Vector(bones['tail.00']);pelvis=rig.pose.bones['pelvis'];anchor=pelvis.matrix@inverse['pelvis']@bind
    delta=Matrix.Translation(anchor)@Matrix.Rotation(math.radians(95)*t,4,'Y')@Matrix.Translation(-bind)
    for i in range(7):
     n='tail.%02d'%i;b=rig.data.bones[n];p=rig.pose.bones[n];world=delta@b.matrix_local
     local=b.convert_local_to_pose(world,b.matrix_local,parent_matrix=p.parent.matrix,parent_matrix_local=b.parent.matrix_local,invert=True)
     p.location,p.rotation_quaternion,p.scale=local.decompose();bpy.context.view_layer.update()
     p.keyframe_insert('location',frame=f);p.keyframe_insert('rotation_quaternion',frame=f);p.keyframe_insert('scale',frame=f)
   for n,centre in eye_centres.items():
    p=rig.pose.bones[n];amount=max(0,min(1,(1-p.scale.y)/.955));scale=.002+.998*amount
    local_centre=rig.data.bones[n].matrix_local.inverted()@centre
    offset=rig.data.bones[n].matrix_local.to_quaternion().inverted()@(eye_front*(-.10*(1-amount)))
    p.scale=(1,scale,1);p.location=Vector((0,(1-scale)*local_centre.y,0))+offset
    p.keyframe_insert('scale',frame=f);p.keyframe_insert('location',frame=f)
   pts=posed_points()
  lift=max(0,.003-float(pts[:,2].min()))
  if lift:
   rig.pose.bones['pelvis'].location+=rig.data.bones['pelvis'].matrix_local.to_quaternion().inverted()@Vector((0,0,lift))
   rig.pose.bones['pelvis'].keyframe_insert('location',frame=f)
  if REV>=5 and name in ['settle_cushion','curled_rest']:
   # Source hind feet are offset from the procedural mesh's foot centres.
   # Adapt the lower chains so both hind contacts reach the same ground plane.
   t=namespace['smoothstep'](0,1,(f-1)/(round(action.frame_range[1])-1)) if name=='settle_cushion' else 1
   for iteration in range(2):
    pts=posed_points()
    for side in ['L','R']:
     mask=(positions[:,1]>.10)&(positions[:,2]<.12)&((positions[:,0]<0) if side=='L' else (positions[:,0]>0))
     if not mask.any():continue
     dz=-max(0,float(pts[mask,2].min())-.004)*t
     for limb in ['hind_lower.','hind_paw.']:
      n=limb+side;p=rig.pose.bones[n];b=rig.data.bones[n];world=p.matrix.copy();world.translation.z+=dz
      local=b.convert_local_to_pose(world,b.matrix_local,parent_matrix=p.parent.matrix,parent_matrix_local=b.parent.matrix_local,invert=True)
      p.location,p.rotation_quaternion,p.scale=local.decompose();bpy.context.view_layer.update();p.keyframe_insert('location',frame=f)
   pts=posed_points();final_lift=max(0,.003-float(pts[:,2].min()))
   if final_lift:
    rig.pose.bones['pelvis'].location+=rig.data.bones['pelvis'].matrix_local.to_quaternion().inverted()@Vector((0,0,final_lift))
    rig.pose.bones['pelvis'].keyframe_insert('location',frame=f)
 for fc in action.fcurves:
  for key in fc.keyframe_points:key.interpolation='LINEAR'
rig.animation_data.action=bpy.data.actions['relaxed_idle'];scene.frame_set(1)
# Material review: no excessive baked-source highlights from EEVEE environment.
scene.world.color=(.25,.25,.25)
cam=scene.camera
def render(label,action,frame,loc=(1.45,-2.25,1.15),target=(0,0,.5),scale=1.55):
 rig.animation_data.action=bpy.data.actions[action];scene.frame_set(frame)
 cam.location=loc;cam.rotation_euler=(Vector(target)-cam.location).to_track_quat('-Z','Y').to_euler();cam.data.ortho_scale=scale
 scene.render.filepath=str(JOB/'renders'/('r%d-'%REV+label+'.png'));bpy.ops.render.render(write_still=True)
for label,action,frame in [('idle','relaxed_idle',1),('walk','garden_walk',10),('request','attentive_request',37),('rest','curled_rest',30),('drink','drink',16)]:
 render(label,action,frame)
render('side','relaxed_idle',1,(2.8,0,.75))
# Same-model portraits are delivered with the corrected asset.
scene.render.resolution_x=512;scene.render.resolution_y=512;scene.render.film_transparent=True;scene.render.image_settings.color_mode='RGBA';bpy.data.objects['Review ground'].hide_render=True
for label,action,frame in [('portrait-dialogue','attentive_request',37),('portrait-shop','relaxed_idle',1),('portrait-ending','curled_rest',30)]:
 render(label,action,frame,(.55,-2,.95) if 'ending' not in label else (1.45,-2.25,1.15),(0,-.37,.70) if 'ending' not in label else (0,0,.3),.78 if 'ending' not in label else 1.4)
scene.render.film_transparent=False;bpy.data.objects['Review ground'].hide_render=False;scene.render.resolution_x=960;scene.render.resolution_y=960
rig.animation_data.action=bpy.data.actions['relaxed_idle'];scene.frame_set(1)
for im in list(bpy.data.images):
 if im.type=='IMAGE' and im.users and not im.packed_file:im.pack()
# Remove unused 4K imported images from packed delivery.
for im in list(bpy.data.images):
 if im.users==0:bpy.data.images.remove(im)
bpy.ops.wm.save_as_mainfile(filepath=str(JOB/('pip-repaired-r%d.blend'%REV)))
# All actions exported by exact names, sampled in place.
rig.animation_data.action=None
for p in rig.pose.bones:p.matrix_basis=Matrix.Identity(4)
bpy.context.view_layer.update()
bpy.ops.object.select_all(action='DESELECT');body.select_set(True);rig.select_set(True);bpy.context.view_layer.objects.active=body
(JOB/'exports').mkdir(exist_ok=True)
bpy.ops.export_scene.gltf(filepath=str(JOB/'exports'/('pip-repaired-r%d.glb'%REV)),export_format='GLB',use_selection=True,export_animations=True,export_nla_strips=True,export_force_sampling=True,export_frame_range=False,export_skins=True,export_morph=False,export_yup=True)
import runpy
runpy.run_path(str(JOB/'normalize_glb.py'))['normalize'](JOB/'exports'/('pip-repaired-r%d.glb'%REV))
bpy.ops.export_scene.fbx(filepath=str(JOB/'exports'/('pip-repaired-r%d.fbx'%REV)),use_selection=True,object_types={'MESH','ARMATURE'},add_leaf_bones=False,apply_unit_scale=True,apply_scale_options='FBX_SCALE_ALL',axis_forward='-Z',axis_up='Y',bake_anim=True,bake_anim_use_all_actions=True,bake_anim_use_nla_strips=False,bake_anim_simplify_factor=0,path_mode='COPY',embed_textures=True)
report={'revision':REV,'vertices':len(body.data.vertices),'triangles':sum(len(p.vertices)-2 for p in body.data.polygons),'bones':list(bones),'max_influences':max(len(v.groups) for v in body.data.vertices),'unweighted':sum(not len(v.groups) for v in body.data.vertices),'clips':clip_names,'source_transform':'rotate +72 Z; scale X .92, Y/Z 1.25; recenter X and ground Z','bind_rig':'exact copied Paws rig v1','textures':[{ 'name':im.name,'size':list(im.size)} for im in bpy.data.images if im.users and im.type=='IMAGE']}
(JOB/'exports'/('measurements-r%d.json'%REV)).write_text(json.dumps(report,indent=2));print(json.dumps(report))


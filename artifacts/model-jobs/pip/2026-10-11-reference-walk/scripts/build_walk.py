"""Reference-directed stance/swing targets, analytical two-bone IK, baked canonical action.
Run with Blender 3.5: --background --python-exit-code 1 --python this.py -- --revision 1
"""
import bpy, math, json, argparse, sys, runpy
import numpy as np
from pathlib import Path
from mathutils import Vector, Matrix, Euler
JOB=Path(__file__).resolve().parents[1]
parser=argparse.ArgumentParser();parser.add_argument('--revision',type=int,default=1);parser.add_argument('--render',action='store_true')
args=parser.parse_args(sys.argv[sys.argv.index('--')+1:] if '--' in sys.argv else [])
spec=json.loads((JOB/'references/walk-spec.json').read_text())
bpy.ops.wm.open_mainfile(filepath=str(JOB.parent/'2026-10-10-tripo-repair/pip-repaired.blend'))
scene=bpy.context.scene;scene.render.fps=30;scene.render.fps_base=1
rig=bpy.data.objects['PipRig'];body=next(o for o in scene.objects if o.type=='MESH' and o.find_armature()==rig)
rig.animation_data.action=bpy.data.actions['relaxed_idle'];scene.frame_set(1)
# Retain repaired eyelid poses and every other action. Only replace garden_walk.
base={p.name:p.matrix_basis.copy() for p in rig.pose.bones}
rig.animation_data.action=None
bpy.data.actions.remove(bpy.data.actions['garden_walk'])
action=bpy.data.actions.new('garden_walk');action.use_fake_user=True;rig.animation_data.action=action
bones=rig.data.bones;names=[b.name for b in bones]
positions=np.array([(*v.co,1) for v in body.data.vertices]);weights=np.zeros((len(positions),len(names)))
for v in body.data.vertices:
 for g in v.groups:weights[v.index,names.index(body.vertex_groups[g.group].name)]=g.weight
inverse={n:bones[n].matrix_local.inverted() for n in names}
feet={}
for key in spec['contact_phase']:
 limb,side=key.split('.');mask=(positions[:,2]<.028)&((positions[:,0]<0) if side=='L' else (positions[:,0]>0))&((positions[:,1]<0) if limb=='front' else (positions[:,1]>0))
 sole=Vector(positions[mask,:3].mean(axis=0));sole.z=float(positions[mask,2].min())
 feet[key]={'sole':sole,'mask':mask,'neutral':Vector(((-1 if side=='L' else 1)*(.115 if limb=='front' else .157),-.365 if limb=='front' else .26,.003))}

def set_world(n,position,rotation):
 p=rig.pose.bones[n];b=bones[n]
 world=Matrix.Translation(position)@rotation.to_matrix().to_4x4()@b.matrix_local.to_quaternion().to_matrix().to_4x4()
 local=b.convert_local_to_pose(world,b.matrix_local,parent_matrix=p.parent.matrix,parent_matrix_local=b.parent.matrix_local,invert=True) if p.parent else world
 p.location,p.rotation_quaternion,p.scale=local.decompose();p.rotation_mode='QUATERNION'
 bpy.context.view_layer.update()

def interpolate(keys,u,column,periodic=False):
 # Cubic Hermite through artist-defined poses. Periodic tangents preserve body seams.
 i=next((i for i in range(len(keys)-1) if u<=keys[i+1][0]),len(keys)-2)
 a,b=keys[i],keys[i+1];dt=b[0]-a[0];v=(u-a[0])/dt
 def tangent(j):
  if periodic:
   prev=keys[j-1] if j>0 else [keys[-2][0]-1,*keys[-2][1:]]
   nex=keys[j+1] if j<len(keys)-1 else [keys[1][0]+1,*keys[1][1:]]
  else:
   if j==0 or j==len(keys)-1:return 0
   prev,nex=keys[j-1],keys[j+1]
  return (nex[column]-prev[column])/(nex[0]-prev[0])
 return (2*v**3-3*v*v+1)*a[column]+(v**3-2*v*v+v)*dt*tangent(i)+(-2*v**3+3*v*v)*b[column]+(v**3-v*v)*dt*tangent(i+1)

def target(key,phase):
 q=(phase-spec['contact_phase'][key])%1;d=spec['stance_fraction'];span=spec['stride_metres']*d
 point=feet[key]['neutral'].copy();pitch=0
 if q<d:
  point.y+=-span/2+spec['stride_metres']*q
 else:
  u=(q-d)/(1-d);ks=[[k['u'],k['travel'],k['height'],k['pitch_degrees']] for k in spec['swing_keys']]
  # End tangents continue the planted backward velocity through lift-off/landing.
  travel=interpolate(ks,u,1)
  edge=.10
  if u<edge:travel-=.5*u*(1-u/edge)**2
  if u>1-edge:travel+=.5*(1-u)*(1-(1-u)/edge)**2
  point.y+=span/2-span*travel;point.z+=max(0,interpolate(ks,u,2));pitch=interpolate(ks,u,3)
 return point,pitch,q<d

def solve(key,point,pitch):
 limb,side=key.split('.');upper=limb+'_upper.'+side;lower=limb+'_lower.'+side;paw=limb+'_paw.'+side
 A=rig.pose.bones[upper].head.copy();restA=bones[upper].head_local;restB=bones[lower].head_local;restC=bones[paw].head_local
 footRot=Euler((math.radians(pitch),0,0)).to_quaternion()
 C=point-footRot@(feet[key]['sole']-restC)
 l1=(restB-restA).length;l2=(restC-restB).length;axis=C-A;distance=axis.length
 if distance>l1+l2-.0001:
  # Move shoulder/hip subtly with scapula motion, never stretch the lower chain.
  A+=axis.normalized()*(distance-(l1+l2-.0001));axis=C-A;distance=axis.length
 direction=axis.normalized();bend=Vector((0,1 if limb=='front' else -1,0));bend=(bend-direction*bend.dot(direction)).normalized()
 along=(l1*l1-l2*l2+distance*distance)/(2*distance);height=math.sqrt(max(0,l1*l1-along*along));B=A+direction*along+bend*height
 set_world(upper,A,(restB-restA).rotation_difference(B-A))
 set_world(lower,B,(restC-restB).rotation_difference(C-B))
 set_world(paw,C,footRot)

def skin():
 pts=np.zeros((len(positions),3))
 for i,n in enumerate(names):pts+=(positions@np.array(rig.pose.bones[n].matrix@inverse[n]).T)[:,:3]*weights[:,i,None]
 return pts

records=[]
for f in range(37):
 phase=f/36;scene.frame_set(f+1)
 for p in rig.pose.bones:p.matrix_basis=base[p.name].copy()
 # Artist-defined weight shift, counter-rotation of chest and level gaze.
 b=spec['body_keys'];shift=interpolate(b,phase,1,True);lift=interpolate(b,phase,2,True);pitch=interpolate(b,phase,3,True);roll=interpolate(b,phase,4,True)
 pelvis=Vector(bones['pelvis'].head_local)+Vector((shift,0,lift))
 set_world('pelvis',pelvis,Euler((math.radians(pitch),math.radians(roll),0)).to_quaternion())
 set_world('chest',rig.pose.bones['chest'].head.copy(),Euler((math.radians(-pitch*.5),math.radians(-roll*.4),0)).to_quaternion())
 set_world('head',rig.pose.bones['head'].head.copy(),Euler((0,0,math.radians(roll*.3))).to_quaternion())
 for n in ['tail.01','tail.02','tail.03']:
  p=rig.pose.bones[n];p.rotation_mode='QUATERNION';p.rotation_quaternion=Euler((0,0,math.radians(-roll*.7))).to_quaternion()
 bpy.context.view_layer.update()
 targets={key:target(key,phase) for key in feet}
 for key,(point,angle,stance) in targets.items():solve(key,point,angle)
 # Solve actual sole height/centroid, including blended lower-leg skin influences.
 for iteration in range(3):
  pts=skin()
  for key,(point,angle,stance) in targets.items():
   values=pts[feet[key]['mask']];actual=Vector(values.mean(axis=0));actual.z=float(values[:,2].min())
   error=point-actual
   feet[key]['correction']=feet[key].get('correction',Vector())+error
   solve(key,point+feet[key]['correction'],angle)
 pts=skin();entry={'frame':f+1,'phase':phase,'floor_min':float(pts[:,2].min()),'feet':{}}
 for key,(point,angle,stance) in targets.items():
  values=pts[feet[key]['mask']];actual=values.mean(axis=0);actual[2]=values[:,2].min()
  entry['feet'][key]={'stance':stance,'target':list(point),'actual':list(actual)}
  feet[key]['correction']=Vector()
 records.append(entry)
 for p in rig.pose.bones:
  for channel in ['location','rotation_quaternion','scale']:p.keyframe_insert(channel,frame=f+1,group=p.name)
for fc in action.fcurves:
 for k in fc.keyframe_points:k.interpolation='LINEAR'
action['reference']='references/muybridge-716.jpg (walking only); references/walk-spec.json'
action['stride_metres']=spec['stride_metres'];action['stance_fraction']=spec['stance_fraction']
scene.frame_start=1;scene.frame_end=37;scene.frame_set(1)
rig['walk_source']='Reference-directed contact targets and two-bone IK; baked 30 fps'
scene.render.resolution_x=640;scene.render.resolution_y=640;scene.render.resolution_percentage=100
scene.eevee.taa_render_samples=32
master=JOB/('pip-walk-r%d.blend'%args.revision);bpy.ops.wm.save_as_mainfile(filepath=str(master))
(JOB/'exports'/('contacts-r%d.json'%args.revision)).write_text(json.dumps(records,indent=2))
rig.animation_data.action=None
for p in rig.pose.bones:p.matrix_basis=Matrix.Identity(4)
bpy.context.view_layer.update();bpy.ops.object.select_all(action='DESELECT');rig.select_set(True);body.select_set(True);bpy.context.view_layer.objects.active=body
output=JOB/'exports'/('pip-walk-r%d.glb'%args.revision)
bpy.ops.export_scene.gltf(filepath=str(output),export_format='GLB',use_selection=True,export_animations=True,export_nla_strips=True,export_force_sampling=True,export_frame_range=False,export_skins=True,export_morph=False,export_yup=True)
runpy.run_path(str(JOB.parent/'2026-10-10-tripo-repair/normalize_glb.py'))['normalize'](output)
if args.render:
 rig.animation_data.action=action;cam=scene.camera
 for view,loc in [('side',(2.8,0,.65)),('front',(0,-2.8,.65)),('rear',(0,2.8,.65)),('three-quarter',(1.45,-2.25,1.15)),('puzzle',(2.4,-3.5,3.5))]:
  cam.location=loc;cam.rotation_euler=(Vector((0,0,.50))-cam.location).to_track_quat('-Z','Y').to_euler();cam.data.ortho_scale=1.55 if view!='puzzle' else 2.4
  for f in range(1,37,3):
   scene.frame_set(f);scene.render.filepath=str(JOB/'renders'/('r%d-%s-%02d.png'%(args.revision,view,f)));bpy.ops.render.render(write_still=True)
print('WALK COMPLETE',master,output)

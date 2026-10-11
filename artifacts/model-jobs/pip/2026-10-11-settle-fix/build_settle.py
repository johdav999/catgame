"""Connected-joint settle authoring; preserve canonical bind and unrelated actions."""
import bpy, math, json, sys, runpy
import numpy as np
from pathlib import Path
from mathutils import Vector, Matrix, Euler, Quaternion
JOB=Path(__file__).resolve().parent
SOURCE=JOB.parent/'2026-10-10-tripo-repair/pip-repaired-r8.blend'
REV=int(sys.argv[sys.argv.index('--')+1]) if '--' in sys.argv else 1
bpy.ops.wm.open_mainfile(filepath=str(SOURCE))
scene=bpy.context.scene;rig=bpy.data.objects['PipRig'];body=bpy.data.objects['Pip repaired Tripo skin']
scene.render.fps=30;scene.render.fps_base=1
names=[b.name for b in rig.data.bones]
positions=np.array([(*v.co,1) for v in body.data.vertices])
weights=np.zeros((len(positions),len(names)))
for v in body.data.vertices:
 for g in v.groups: weights[v.index,names.index(body.vertex_groups[g.group].name)]=g.weight
inverse={n:rig.data.bones[n].matrix_local.inverted() for n in names}
def points():
 bpy.context.view_layer.update();pts=np.zeros((len(positions),3))
 for i,n in enumerate(names):pts+=(positions@np.array(rig.pose.bones[n].matrix@inverse[n]).T)[:,:3]*weights[:,i,None]
 return pts
def rotate(n,xyz):
 p=rig.pose.bones[n];q=p.bone.matrix_local.to_quaternion()
 p.rotation_quaternion=q.inverted()@Euler(tuple(math.radians(x) for x in xyz)).to_quaternion()@q
 bpy.context.view_layer.update()
def world_rotate(n,axis,angle):
 p=rig.pose.bones[n];m=p.matrix.copy();o=m.translation.copy()
 p.matrix=Matrix.Translation(o)@Matrix.Rotation(math.radians(angle),4,axis)@Matrix.Translation(-o)@m
 bpy.context.view_layer.update()
def align(n,child,target):
 p=rig.pose.bones[n];m=p.matrix.copy();origin=m.translation.copy()
 direction=rig.pose.bones[child].matrix.translation-origin
 q=direction.rotation_difference(target-origin)
 p.matrix=Matrix.Translation(origin)@q.to_matrix().to_4x4()@Matrix.Translation(-origin)@m
 bpy.context.view_layer.update()
def limb(kind,side,target):
 a,b,c=[kind+'_'+part+'.'+side for part in ['upper','lower','paw']]
 shoulder=rig.pose.bones[a].matrix.translation.copy()
 l1=(rig.data.bones[b].head_local-rig.data.bones[a].head_local).length
 l2=(rig.data.bones[c].head_local-rig.data.bones[b].head_local).length
 delta=target-shoulder;d=min(delta.length,l1+l2-.00001);axis=delta.normalized()
 along=(l1*l1-l2*l2+d*d)/(2*d);height=math.sqrt(max(0,l1*l1-along*along))
 pole=Vector((0,1 if kind=='front' else -1,0));pole=(pole-axis*pole.dot(axis)).normalized()
 elbow=shoulder+axis*along+pole*height
 align(a,b,elbow);align(b,c,target)
 # Keep the sole level without shortening or translating either leg segment.
 m=rig.data.bones[c].matrix_local.copy();m.translation=rig.pose.bones[c].matrix.translation
 rig.pose.bones[c].matrix=m;bpy.context.view_layer.update()
def ease(x):x=max(0,min(1,x));return x*x*x*(x*(x*6-15)+10)
idle=bpy.data.actions['relaxed_idle'];rig.animation_data.action=idle;scene.frame_set(1)
initial={n:rig.pose.bones[n].matrix_basis.copy() for n in names}
closed={}
rig.animation_data.action=bpy.data.actions['curled_rest'];scene.frame_set(1)
for n in ['blink.L','blink.R']:closed[n]=rig.pose.bones[n].matrix_basis.copy()
report={}
for name,duration in [('settle_cushion',2.4),('curled_rest',4.)]:
 action=bpy.data.actions[name];action.fcurves.clear();action.use_fake_user=True;rig.animation_data.action=action
 samples=[]
 for f in range(1,round(duration*30)+2):
  if name=='curled_rest' and f>1:
   for p in rig.pose.bones:
    for prop in ['location','rotation_quaternion','scale']:p.keyframe_insert(prop,frame=f)
   samples.append(dict(samples[-1],time=(f-1)/30))
   continue
  scene.frame_set(f)
  t=(f-1)/30 if name=='settle_cushion' else 2.4
  lower=ease(t/1.7);relax=ease((t-.7)/1.7)
  for n,m in initial.items():rig.pose.bones[n].matrix_basis=m
  rig.pose.bones['pelvis'].location=rig.data.bones['pelvis'].matrix_local.to_quaternion().inverted()@Vector((0,0,-.205*lower))
  if REV>=8:
   rig.pose.bones['pelvis'].location=rig.data.bones['pelvis'].matrix_local.to_quaternion().inverted()@Vector((0,0,-.245*lower))
   rotate('pelvis',(-10*lower,0,0))
  rotate('spine',(0,0,12*relax));rotate('chest',(0,0,4*relax))
  rotate('neck',(18*relax,0,-8*relax));rotate('head',(12*relax,0,-20*relax))
  for side in ['L','R']:
   for kind in ['front','hind']:
    target=rig.data.bones[kind+'_paw.'+side].head_local.copy()
    target.y+=(-.035 if kind=='front' else -.018)*lower
    limb(kind,side,target)
  if 2<=REV<=4:
   # Side-supported rest: carry the connected body onto its flank, then let
   # the head follow the neck arc. No independent neck translation.
   rotate('pelvis',(0,58*relax,0))
   rotate('spine',(0,0,25*relax));rotate('chest',(0,0,10*relax))
   rotate('neck',(20*relax,0,25*relax));rotate('head',(12*relax,0,20*relax))
  if 3<=REV<=4:
   rotate('pelvis',(0,(45 if REV>=4 else 80)*relax,0))
   for n in ['spine','chest','neck','head']:rotate(n,(0,0,0))
   for n,angle in [('spine',22),('chest',12),('neck',28),('head',20)]:world_rotate(n,'Z',angle*relax)
   world_rotate('head','Y',-25*relax)
  if REV==4:
   for side in ['L','R']:
    for n,angle in [('front_upper.',55),('front_lower.',-95),('front_paw.',40),('hind_upper.',-65),('hind_lower.',100),('hind_paw.',-35)]:
     rotate(n+side,(angle*lower,0,0))
  # Lay the source's upright tail alongside the haunch as one coherent chain.
  # This respects its actual curved geometry and does not resample joint centres.
  rotate('tail.00',(0,78*relax,8*relax))
  if 2<=REV<=4:
   rotate('tail.00',(25*relax,15*relax,-35*relax))
   rotate('tail.01',(0,0,-12*relax));rotate('tail.02',(0,0,-18*relax))
  if 3<=REV<=4:
   for i in range(7):rotate('tail.%02d'%i,(0,0,0))
   world_rotate('tail.00','Y',(50 if REV>=4 else 10)*relax)
   world_rotate('tail.00','Z',-35*relax)
  if REV==5 or REV>=8:
   rotate('tail.00',(0,(90 if REV>=8 else 105)*relax,8*relax))
   # Solve the actual supplied skin's toe contacts, not the procedural rig's
   # nominal paw centres. Correction rotates each connected two-bone chain.
   for side in ['L','R']:
    for kind in ['front','hind']:
     target=rig.data.bones[kind+'_paw.'+side].head_local.copy()
     target.y+=((-.12 if kind=='front' else -.16) if REV>=8 else (-.08 if kind=='front' else -.09))*lower
     mask=(positions[:,2]<.09)&((positions[:,1]<0) if kind=='front' else (positions[:,1]>.1))&((positions[:,0]<0) if side=='L' else (positions[:,0]>0))
     for iteration in range(4):
      limb(kind,side,target);pts=points();target.z+=(.003-float(pts[mask,2].min()))*.85
  if 6<=REV<=7:
   rotate('pelvis',(0,(60 if REV>=7 else 105)*relax,0))
   for n in ['spine','chest','neck','head']:rotate(n,(0,0,0))
   for n,angle in [('spine',15),('chest',10),('neck',30),('head',25)]:world_rotate(n,'Z',angle*relax)
   world_rotate('head','Y',(-55 if REV>=7 else -25)*relax)
   for side in ['L','R']:
    for n,angle in [('front_upper.',35),('front_lower.',-60),('front_paw.',25),('hind_upper.',-40),('hind_lower.',65),('hind_paw.',-25)]:rotate(n+side,(angle*lower,0,0))
   for i in range(7):rotate('tail.%02d'%i,(0,0,0))
   world_rotate('tail.00','Y',(25 if REV>=7 else -25)*relax);world_rotate('tail.00','Z',-40*relax)
   if REV>=7:
    for side in ['L','R']:
     for kind in ['front','hind']:world_rotate(kind+'_upper.'+side,'Y',45*relax)
    world_rotate('neck','X',20*relax)
  if REV>=9:world_rotate('tail.00','Z',-75*relax)
  for n in closed:
   a,b,c=initial[n].decompose();x,y,z=closed[n].decompose()
   q=ease((t-1.2)/1.2)
   p=rig.pose.bones[n];p.location=a.lerp(x,q);p.rotation_quaternion=b.slerp(y,q);p.scale=c.lerp(z,q)
  pts=points()
  if REV>=2:
   p=rig.pose.bones['pelvis'];m=p.matrix.copy()
   m.translation.z+=max(0,.003-float(pts[:,2].min()));p.matrix=m
   pts=points()
  # Contact residual is a diagnostic, not a whole-body lift to conceal bad posing.
  samples.append({'time':(f-1)/30,'min':pts.min(0).tolist(),'max':pts.max(0).tolist()})
  for p in rig.pose.bones:
   for prop in ['location','rotation_quaternion','scale']:p.keyframe_insert(prop,frame=f)
 for fc in action.fcurves:
  for k in fc.keyframe_points:k.interpolation='LINEAR'
 report[name]=samples
rig.animation_data.action=bpy.data.actions['settle_cushion'];scene.frame_set(73)
scene.render.engine='CYCLES';scene.cycles.samples=16
scene.render.resolution_x=scene.render.resolution_y=700;scene.render.resolution_percentage=100
camera=scene.camera
for view,loc in [('front',(1.4,-2.4,1.15)),('side',(2.8,0,.85)),('rear',(-1.6,2.4,1.1)),('top',(0,-.1,3))]:
 camera.location=loc;camera.rotation_euler=(Vector((0,0,.35))-camera.location).to_track_quat('-Z','Y').to_euler();camera.data.ortho_scale=1.75
 scene.render.filepath=str(JOB/('r%d-%s.png'%(REV,view)));bpy.ops.render.render(write_still=True)
rig.animation_data.action=idle;scene.frame_set(1)
bpy.ops.file.pack_all();bpy.ops.wm.save_as_mainfile(filepath=str(JOB/('pip-settle-r%d.blend'%REV)))
bpy.ops.object.select_all(action='DESELECT');rig.select_set(True);body.select_set(True);bpy.context.view_layer.objects.active=rig
bpy.ops.export_scene.gltf(filepath=str(JOB/('pip-settle-r%d.glb'%REV)),export_format='GLB',use_selection=True,export_animations=True,export_force_sampling=True,export_frame_range=False,export_nla_strips=True)
runpy.run_path(str(JOB.parent/'2026-10-10-tripo-repair/normalize_glb.py'))['normalize'](JOB/('pip-settle-r%d.glb'%REV))
(JOB/('r%d-measurements.json'%REV)).write_text(json.dumps(report,indent=2))

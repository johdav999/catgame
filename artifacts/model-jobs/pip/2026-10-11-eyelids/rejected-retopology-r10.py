"""Pip fitted eyelids. All changes remain on the canonical blink bones."""
import bpy,bmesh,math,json,sys,runpy
import numpy as np
from pathlib import Path
from mathutils import Vector,Matrix
from mathutils.bvhtree import BVHTree
JOB=Path(__file__).parent
REV=int(sys.argv[sys.argv.index('--')+1]) if '--' in sys.argv else 1
bpy.ops.wm.open_mainfile(filepath=str(JOB/'eyes-source.blend'))
scene=bpy.context.scene;rig=bpy.data.objects['PipRig'];body=bpy.data.objects['Pip repaired Tripo skin']
rig.animation_data.action=None
for p in rig.pose.bones:p.matrix_basis.identity()
bm=bmesh.new();bm.from_mesh(body.data);bmesh.ops.triangulate(bm,faces=list(bm.faces));bm.to_mesh(body.data);bm.free();body.data.update()
tree=BVHTree.FromPolygons([v.co for v in body.data.vertices],[p.vertices[:] for p in body.data.polygons],all_triangles=True)
coat=body.data.materials[0];shader=coat.node_tree.nodes.get('Principled BSDF');tex=shader.inputs['Base Color'].links[0].from_node.image
pixels=np.array(tex.pixels[:]).reshape(tex.size[1],tex.size[0],4)
def surface(origin,direction):
 hit,normal,index,dist=tree.ray_cast(origin,direction)
 if hit is None:return None
 face=body.data.polygons[index];v=[body.data.vertices[i].co for i in face.vertices]
 from mathutils.geometry import barycentric_transform
 uv=[Vector((*body.data.uv_layers.active.data[i].uv,0)) for i in face.loop_indices]
 t=barycentric_transform(hit,*v,*uv)
 return hit,normal,Vector((t.x,t.y))
def color(uv):
 return pixels[int((uv.y%1)*(tex.size[1]-1)),int((uv.x%1)*(tex.size[0]-1)),:3]
samples=json.loads((JOB/'eye-samples.json').read_text());a=np.array([r['p'] for r in samples if sum(r['c'])<.35])
c=np.array([a[a[:,0].argmin()],a[a[:,0].argmax()]])
for i in range(20):
 k=((a[:,None,:]-c[None,:,:])**2).sum(2).argmin(1);c=np.array([a[k==j].mean(0) for j in range(2)])
front=Vector((c[1,1]-c[0,1],c[0,0]-c[1,0],0)).normalized();right=Vector((-front.y,front.x,0));up=Vector((0,0,1))
eyes=[]
for i,label in enumerate(['L','R']):
 # Local optical plane follows each side of the curved face.
 normal=np.mean([r['n'] for r in samples if sum(r['c'])<.35 and np.linalg.norm(np.array(r['p'])-c[i])<.055],0)
 n=Vector((normal[0],normal[1],0)).normalized();t=Vector((-n.y,n.x,0))
 centre=Vector(c[i]);centre.z-=.011
 eyes.append({'name':'blink.'+label,'centre':centre,'n':n,'t':t})
 print('EYE FRAME',label,list(centre),list(n))
# Render orthographic face with the old patches absent, useful for fitting.
target=(Vector(c[0])+Vector(c[1]))*.5;target.z-=.013
cam=scene.camera;cam.location=target+front*2+up*.04;cam.rotation_euler=(target-cam.location).to_track_quat('-Z','Y').to_euler();cam.data.ortho_scale=.33
scene.render.resolution_x=1100;scene.render.resolution_y=650
scene.render.filepath=str(JOB/'front-reference.png');bpy.ops.render.render(write_still=True)
# Reconstruct socket topology, preserving the source eye and outer face.
newobjects=runpy.run_path(str(JOB/'socket_topology.py'))['construct'](globals())
def pose_eye(eye,amount):
 b=rig.data.bones[eye['name']];p=rig.pose.bones[eye['name']]
 deform=Matrix.Diagonal((1,1,1-.999*amount,1))
 delta=Matrix.Translation(eye['pivot']+eye['n']*(.012*amount))@deform@Matrix.Translation(-eye['pivot'])
 local=b.matrix_local.inverted()@delta@b.matrix_local
 p.location,p.rotation_quaternion,p.scale=local.decompose()
def blink(t):
 # 100 ms close, 67 ms hold, 200 ms release. Smooth velocity at contacts.
 def ease(x):x=max(0,min(1,x));return x*x*(3-2*x)
 if t<2.4:return 0
 if t<2.5:return ease((t-2.4)/.1)
 if t<2.5666667:return 1
 return 1-ease((t-2.5666667)/.2)
for action in bpy.data.actions:
 action.use_fake_user=True;rig.animation_data.action=action
 for fc in list(action.fcurves):
  if 'pose.bones["blink.' in fc.data_path:action.fcurves.remove(fc)
 duration=(action.frame_range[1]-1)/30
 for f in range(1,round(action.frame_range[1])+1):
  scene.frame_set(f);t=(f-1)/30
  amount=blink(t) if action.name=='relaxed_idle' else 0
  if action.name=='curled_rest':amount=1
  if action.name=='settle_cushion':
   q=max(0,min(1,t/duration));amount=q*q*(3-2*q)
  for eye in eyes:
   pose_eye(eye,amount)
   p=rig.pose.bones[eye['name']]
   for prop in ['location','rotation_quaternion','scale']:p.keyframe_insert(prop,frame=f)
 for fc in action.fcurves:
  if 'pose.bones["blink.' in fc.data_path:
   for key in fc.keyframe_points:key.interpolation='LINEAR'
bpy.ops.object.select_all(action='DESELECT');body.select_set(True)
for obj in newobjects:obj.select_set(True)
bpy.context.view_layer.objects.active=body;bpy.ops.object.join()
bm=bmesh.new();bm.from_mesh(body.data);boundary=[v for v in bm.verts if v.is_boundary];bmesh.ops.remove_doubles(bm,verts=boundary,dist=.000001);bm.to_mesh(body.data);bm.free();body.data.update()
rig.animation_data.action=None
for p in rig.pose.bones:p.matrix_basis.identity()
for amount in [0,.25,.5,.75,1]:
 for eye in eyes:pose_eye(eye,amount)
 bpy.context.view_layer.update()
 scene.render.filepath=str(JOB/('r%d-blink-%03d.png'%(REV,amount*100)));bpy.ops.render.render(write_still=True)
rig.animation_data.action=bpy.data.actions['relaxed_idle'];scene.frame_set(1)
bpy.ops.wm.save_as_mainfile(filepath=str(JOB/('pip-eyelids-r%d.blend'%REV)))
(JOB/'fit.json').write_text(json.dumps([{k:list(v) if isinstance(v,Vector) else v for k,v in e.items()} for e in eyes],indent=2))

"""Welded eyelid topology and translation-only upper/lower skin controls."""
import bpy,bmesh,math,json,sys,runpy
import numpy as np
from pathlib import Path
from mathutils import Vector,Matrix
from mathutils.bvhtree import BVHTree
J=Path(__file__).resolve().parent;ROOT=J.parents[3]
REV=int(sys.argv[sys.argv.index('--')+1]) if '--' in sys.argv else 1
bpy.ops.wm.open_mainfile(filepath=str(J/'source.blend'))
scene=bpy.context.scene;rig=bpy.data.objects['PipRig'];body=bpy.data.objects['Pip repaired Tripo skin']
rig.animation_data.action=None
for p in rig.pose.bones:p.matrix_basis.identity()
bm=bmesh.new();bm.from_mesh(body.data);bmesh.ops.triangulate(bm,faces=list(bm.faces));bm.to_mesh(body.data);bm.free();body.data.update()
tree=BVHTree.FromPolygons([v.co for v in body.data.vertices],[p.vertices[:] for p in body.data.polygons],all_triangles=True)
body.data.calc_normals_split()
source_normals=[l.normal.copy() for l in body.data.loops]
source_verts=[v.co.copy() for v in body.data.vertices]
source_faces=[(p.vertices[:],p.loop_indices[:]) for p in body.data.polygons]
coat=body.data.materials[0];body.data.materials.clear();body.data.materials.append(coat)
tex=coat.node_tree.nodes.get('Principled BSDF').inputs['Base Color'].links[0].from_node.image
pixels=np.array(tex.pixels[:]).reshape(tex.size[1],tex.size[0],4)
def surface(q,n):
 hit,normal,index,dist=tree.ray_cast(q+n*.25,-n)
 assert hit is not None,('Missing face surface',q)
 from mathutils.geometry import barycentric_transform
 face=body.data.polygons[index];v=[source_verts[i] for i in face.vertices]
 uv=[Vector((*body.data.uv_layers.active.data[i].uv,0)) for i in face.loop_indices]
 return hit,barycentric_transform(hit,*v,*uv)
def color(q,n):
 _,uv=surface(q,n)
 return pixels[int((uv.y%1)*(tex.size[1]-1)),int((uv.x%1)*(tex.size[0]-1)),:3]
samples=json.loads((J/'eye-samples.json').read_text());a=np.array([r['p'] for r in samples if sum(r['c'])<.35]);centres=np.array([a[a[:,0].argmin()],a[a[:,0].argmax()]])
for _ in range(20):
 k=((a[:,None,:]-centres[None,:,:])**2).sum(2).argmin(1);centres=np.array([a[k==j].mean(0) for j in range(2)])
up=Vector((0,0,1));eyes=[]
for i,side in enumerate(['L','R']):
 normal=np.mean([r['n'] for r in samples if sum(r['c'])<.35 and np.linalg.norm(np.array(r['p'])-centres[i])<.055],0)
 n=Vector((normal[0],normal[1],0)).normalized();t=Vector((-n.y,n.x,0));c=Vector(centres[i]);c.z-=.011
 eyes.append(dict(name='blink.'+side,lower='blink.lower.'+side,n=n,t=t,c=c))
# Additive facial controls preserve every original canonical bone and bind matrix.
bpy.context.view_layer.objects.active=rig;rig.select_set(True);bpy.ops.object.mode_set(mode='EDIT')
for eye in eyes:
 old=rig.data.edit_bones[eye['name']];b=rig.data.edit_bones.new(eye['lower']);b.head=old.head;b.tail=old.tail;b.roll=old.roll;b.parent=rig.data.edit_bones['head']
bpy.ops.object.mode_set(mode='OBJECT')
skin=bpy.data.materials.new('Pip continuous eyelid coat');skin.use_nodes=True;shader=skin.node_tree.nodes.get('Principled BSDF');shader.inputs['Roughness'].default_value=.86
atlas=np.ones((256,512,4));created=[];removed=set();boundary_positions=[];report={'eyes':[]}
for ei,eye in enumerate(eyes):
 c,n,t=eye['c'],eye['n'],eye['t'];rx=.035 if ei==0 else .039;rz=.041
 eye.update(rx=rx,rz=rz)
 selected={p.index for p in body.data.polygons if (((p.center-c).dot(t)/(rx*1.27))**2+((p.center-c).z/.054)**2<1 and abs((p.center-c).dot(n))<.085)}
 assert not removed.intersection(selected);removed.update(selected)
 neighbors={};edgefaces={}
 for p in body.data.polygons:
  for edge in p.edge_keys:edgefaces.setdefault(tuple(sorted(edge)),[]).append(p.index)
 for edge,faces in edgefaces.items():
  if len(faces)==2 and sum(f in selected for f in faces)==1:
   x,y=edge;neighbors.setdefault(x,[]).append(y);neighbors.setdefault(y,[]).append(x)
 assert all(len(v)==2 for v in neighbors.values())
 ring=[next(iter(neighbors))];prev=None
 while True:
  x=ring[-1];nxt=next(v for v in neighbors[x] if v!=prev)
  if nxt==ring[0]:break
  ring.append(nxt);prev=x
 assert len(ring)==len(neighbors)
 area=sum((body.data.vertices[ring[i]].co-c).dot(t)*(body.data.vertices[ring[(i+1)%len(ring)]].co-c).z-(body.data.vertices[ring[(i+1)%len(ring)]].co-c).dot(t)*(body.data.vertices[ring[i]].co-c).z for i in range(len(ring)))
 if area<0:ring.reverse()
 first=max(range(len(ring)),key=lambda i:(body.data.vertices[ring[i]].co-c).dot(t));ring=ring[first:]+ring[:first]
 outer=[body.data.vertices[i].co.copy() for i in ring]
 lengths=[(outer[(i+1)%len(outer)]-p).length for i,p in enumerate(outer)];angles=[0]
 for length in lengths[:-1]:angles.append(angles[-1]+math.tau*length/sum(lengths))
 # Replace the jagged cut with a convex, fitted loop before bridging it.
 # Using the raw cut as a loft boundary produces crossed quads at concave notches.
 raw=np.unwrap([math.atan2((p.z-c.z)/.054,(p-c).dot(t)/(rx*1.27)) for p in outer])
 for _ in range(8):raw=np.array([(raw[i-1] if i else raw[-1]-math.tau)*.2+raw[i]*.6+(raw[i+1] if i+1<len(raw) else raw[0]+math.tau)*.2 for i in range(len(raw))])
 angles=list(raw);phase=angles[0]
 outer=[]
 for idx,theta in zip(ring,angles):
  q=c+t*(rx*1.27*math.cos(theta))+up*(.054*math.sin(theta))
  hit,_=surface(q,n);q+=n*(hit-q).dot(n)
  body.data.vertices[idx].co=q;outer.append(q.copy())
 boundary_positions.extend(outer)
 # Extend boundary fitting through neighboring facial vertices, rather than
 # pulling one isolated ring away from its original triangulated neighbors.
 neighbors_all={v.index:set() for v in body.data.vertices}
 for edge in body.data.edges:
  i,j=edge.vertices;neighbors_all[i].add(j);neighbors_all[j].add(i)
 anchors=dict(zip(ring,[q-source_verts[idx] for idx,q in zip(ring,outer)]))
 mobile=[v.index for v in body.data.vertices if v.index not in anchors and all((source_verts[v.index]-c).length<=(source_verts[v.index]-other['c']).length for other in eyes) and math.sqrt(((source_verts[v.index]-c).dot(t)/(rx*1.27))**2+((source_verts[v.index].z-c.z)/.054)**2)<1.7 and abs((source_verts[v.index]-c).dot(n))<.075]
 delta={i:anchors.get(i,Vector((0,0,0))) for i in neighbors_all}
 for _ in range(60):
  updates={i:sum((delta[j] for j in neighbors_all[i]),Vector())/len(neighbors_all[i]) for i in mobile if neighbors_all[i]}
  delta.update(updates)
 for i in mobile:body.data.vertices[i].co=source_verts[i]+delta[i]
 def outer_at(theta):
  for i in range(len(angles)):
   a=angles[i];b=angles[i+1] if i+1<len(angles) else phase+math.tau
   if a<=theta<=b:return outer[i].lerp(outer[(i+1)%len(outer)],(theta-a)/(b-a))
  return outer[-1]
 # Keep the original eye texture on a recessed, stationary eye surface.
 eye_points=[]
 for vertex in body.data.vertices:
  q=source_verts[vertex.index].copy();rho=math.sqrt(((q-c).dot(t)/rx)**2+((q.z-c.z)/rz)**2);a=max(0,min(1,(rho-.75)/.50));eye_points.append(q-n*(.004+.025*a*a*(3-2*a)))
 mesh=bpy.data.meshes.new('Pip fixed eye '+eye['name']);mesh.from_pydata(eye_points,[],[body.data.polygons[i].vertices[:] for i in sorted(selected)]);mesh.update()
 obj=bpy.data.objects.new(mesh.name,mesh);scene.collection.objects.link(obj);mesh.materials.append(coat);uv=mesh.uv_layers.new()
 for p,oldidx in zip(mesh.polygons,sorted(selected)):
  for new,old in zip(p.loop_indices,body.data.polygons[oldidx].loop_indices):uv.data[new].uv=body.data.uv_layers.active.data[old].uv
  p.use_smooth=True
 g=obj.vertex_groups.new(name='head');g.add(list(range(len(mesh.vertices))),1,'REPLACE')
 bm=bmesh.new();bm.from_mesh(mesh);bmesh.ops.delete(bm,geom=[v for v in bm.verts if not v.link_faces],context='VERTS');bm.to_mesh(mesh);bm.free();created.append(obj)
 center_hit,_=surface(c,n);front=(center_hit-c).dot(n)+.005
 eye['front']=front;eye['seam']=c.z-.012
 # Continuous skin band: preserved irregular outside edge, smooth aperture.
 # Rim depth is chosen so translation-only controls meet at one curved seam.
 verts=[];faces=[];uvs=[];deform=[];fractions=[0,.08,.28,.52,.78,1]
 for v in fractions:
  for theta,pouter in zip(angles,outer):
   x=rx*math.cos(theta);seam=eye['seam']+.002*(1-(x/rx)**2)
   z=seam+(.053 if math.sin(theta)>=0 else .029)*math.sin(theta);upper=z>=seam;distance=.075 if upper else .060
   w=abs(z-seam)/distance
   depth=front-.014*(x/rx)**2-w*.006
   inner=c+t*x+up*(z-c.z)+n*depth
   q=inner.lerp(pouter,v)
   projected,_=surface(q,n); source_depth=(projected-c).dot(n)
   # Keep the exact welded outside positions.
   verts.append(q)
   radius=math.sqrt(((pouter-c).dot(t)/rx)**2+((pouter.z-c.z)/rz)**2)
   falloff=max(0,min(1,(1.9-radius)/.9));falloff=falloff*falloff*(3-2*falloff)
   seamq=eye['seam']+.002*(1-min(1,((pouter-c).dot(t)/rx)**2))
   signed=(1-v)*(z-seam)+v*(pouter.z-seamq)*falloff;upper=signed>=0
   influence=abs(signed)/(.075 if upper else .060)
   deform.append((eye['name'] if upper else eye['lower'],influence));uvs.append(((theta-phase)/math.tau,(ei*128+.5+v*127)/256))
 count=len(ring)
 for j in range(len(fractions)-1):
  for i in range(count):
   a=j*count+i;b=j*count+(i+1)%count;faces.append((a,b,b+count,a+count))
 # Bake the source's surrounding coat, without importing iris pixels into skin.
 for yy in range(128):
  v=yy/127
  for xx in range(512):
   theta=phase+xx/511*math.tau;p=outer_at(theta);q=p
   edge=color(q,n); base=color(c+up*(.071 if math.sin(theta)>0 else -.074),n); blend=v**3; col=base*(1-blend)+edge*blend
   margin=max(0,1-v/.060); atlas[ei*128+yy,xx,:3]=col*(1-margin)+np.array([.105,.047,.024])*margin
 mesh=bpy.data.meshes.new('Pip welded eyelids '+eye['name']);mesh.from_pydata(verts,[],faces);mesh.update()
 obj=bpy.data.objects.new(mesh.name,mesh);scene.collection.objects.link(obj);mesh.materials.append(skin);uv=mesh.uv_layers.new()
 for p in mesh.polygons:
  wrap=max(uvs[mesh.loops[l].vertex_index][0] for l in p.loop_indices)-min(uvs[mesh.loops[l].vertex_index][0] for l in p.loop_indices)>.5
  for l in p.loop_indices:
   u,v=uvs[mesh.loops[l].vertex_index];uv.data[l].uv=(u+1 if wrap and u<.5 else u,v)
  p.use_smooth=True
 if mesh.polygons[0].normal.dot(n)<0:
  bm=bmesh.new();bm.from_mesh(mesh);bmesh.ops.reverse_faces(bm,faces=list(bm.faces));bm.to_mesh(mesh);bm.free()
 groups={name:obj.vertex_groups.new(name=name) for name in [eye['name'],eye['lower'],'head']}
 for i,(bone,w) in enumerate(deform):
  if w>1e-7:groups[bone].add([i],w,'REPLACE')
  if w<1:groups['head'].add([i],1-w,'REPLACE')
 created.append(obj)
 # Let the neighboring original cheek and brow follow the lid with a smooth falloff.
 head=body.vertex_groups['head'];groups_body={name:body.vertex_groups.get(name) or body.vertex_groups.new(name=name) for name in [eye['name'],eye['lower']]}
 for vertex in body.data.vertices:
  q=vertex.co;radius=math.sqrt(((q-c).dot(t)/rx)**2+((q.z-c.z)/rz)**2)
  if radius>=1.9 or abs((q-c).dot(n))>.075 or any((q-other['c']).length<(q-c).length for other in eyes):continue
  falloff=max(0,min(1,(1.9-radius)/.9));falloff=falloff*falloff*(3-2*falloff)
  seamq=eye['seam']+.002*(1-min(1,((q-c).dot(t)/rx)**2));upper=q.z>=seamq
  weight=min(.95,abs(q.z-seamq)/(.075 if upper else .060)*falloff)
  for group in list(vertex.groups):body.vertex_groups[group.group].remove([vertex.index])
  groups_body[eye['name'] if upper else eye['lower']].add([vertex.index],weight,'REPLACE');head.add([vertex.index],1-weight,'REPLACE')
 report['eyes'].append({'name':eye['name'],'boundary_vertices':count,'removed_faces':len(selected),'rim_vertices':count})
# Delete source socket surfaces after every UV/raycast sample has completed.
bm=bmesh.new();bm.from_mesh(body.data);bm.faces.ensure_lookup_table();bmesh.ops.delete(bm,geom=[bm.faces[i] for i in removed],context='FACES');bm.to_mesh(body.data);bm.free();body.data.update()
im=bpy.data.images.new('Pip eyelid coat from original UVs',width=512,height=256);im.pixels=atlas.ravel();im.pack()
node=skin.node_tree.nodes.new('ShaderNodeTexImage');node.image=im;skin.node_tree.links.new(node.outputs['Color'],shader.inputs['Base Color'])
bpy.ops.object.select_all(action='DESELECT');body.select_set(True)
for obj in created:obj.select_set(True)
bpy.context.view_layer.objects.active=body;bpy.ops.object.join()
bm=bmesh.new();bm.from_mesh(body.data)
seamverts=[v for v in bm.verts if any((v.co-p).length<1e-6 for p in boundary_positions)]
bmesh.ops.remove_doubles(bm,verts=seamverts,dist=1e-6);bm.to_mesh(body.data);bm.free()
# Match the untouched face normals at the welded boundary. Stable smooth normals
# also avoid baking the triangulated socket cut into the facial shading.
from mathutils.geometry import barycentric_transform
normals=[]
for v in body.data.vertices:
 hit,normal,idx,dist=tree.find_nearest(v.co)
 ids,loops=source_faces[idx]; normal=barycentric_transform(hit,*[source_verts[i] for i in ids],*[source_normals[i] for i in loops]).normalized()
 normals.append(normal)
body.data.use_auto_smooth=True
body.data.normals_split_custom_set([normals[l.vertex_index] for l in body.data.loops])
def pose_eye(eye,a):
 for name,dz in [(eye['name'],-.075),(eye['lower'],.060)]:
  b=rig.data.bones[name];p=rig.pose.bones[name]
  move=up*(dz*a)+eye['n']*(.006*math.sin(a*math.pi/2))
  local=b.matrix_local.inverted()@Matrix.Translation(move)@b.matrix_local
  p.location,p.rotation_quaternion,p.scale=local.decompose()
def ease(x):x=max(0,min(1,x));return x*x*(3-2*x)
def blink(t):
 if t<2.4:return 0
 if t<2.5:return ease((t-2.4)/.1)
 if t<2.566666667:return 1
 return 1-ease((t-2.566666667)/.2)
for action in bpy.data.actions:
 action.use_fake_user=True;rig.animation_data.action=action
 for fc in list(action.fcurves):
  if 'pose.bones["blink' in fc.data_path:action.fcurves.remove(fc)
 end=round(action.frame_range[1]);duration=(end-1)/30
 for f in range(1,end+1):
  scene.frame_set(f);t=(f-1)/30;a=blink(t) if action.name=='relaxed_idle' else 0
  if action.name=='curled_rest':a=1
  if action.name=='settle_cushion':a=ease(max(0,(t-1.2)/1.2))
  for eye in eyes:
   pose_eye(eye,a)
   for name in [eye['name'],eye['lower']]:
    for prop in ['location','rotation_quaternion','scale']:rig.pose.bones[name].keyframe_insert(prop,frame=f)
 for fc in action.fcurves:
  for k in fc.keyframe_points:k.interpolation='LINEAR'
# Append only non-facial channels from the latest accepted animation layers.
manifest=json.loads((ROOT/'assets/runtime/models/pip/manifest.json').read_text())
for name,revision in manifest.get('animation_revisions',{}).items():
 if name not in bpy.data.actions:continue
 target=bpy.data.actions[name];source=ROOT/revision['source_master']
 with bpy.data.libraries.load(str(source),link=False) as (available,loaded):loaded.actions=[name]
 incoming=loaded.actions[0]
 for fc in list(target.fcurves):
  if 'pose.bones["blink' not in fc.data_path:target.fcurves.remove(fc)
 for fc in incoming.fcurves:
  if 'pose.bones["blink' in fc.data_path:continue
  out=target.fcurves.new(fc.data_path,index=fc.array_index);out.keyframe_points.add(len(fc.keyframe_points))
  for a,b in zip(fc.keyframe_points,out.keyframe_points):b.co=a.co;b.interpolation=a.interpolation
 bpy.data.actions.remove(incoming)
rig.animation_data.action=None
for p in rig.pose.bones:p.matrix_basis.identity()
scene.render.engine='CYCLES';scene.cycles.samples=20;scene.render.resolution_x=1000;scene.render.resolution_y=650;scene.render.resolution_percentage=100
front=(eyes[0]['n']+eyes[1]['n']).normalized();target=(eyes[0]['c']+eyes[1]['c'])/2
cam=scene.camera;cam.data.ortho_scale=.37
for view,loc in [('front',target+front*2+up*.03),('left',target+eyes[0]['n']*1.5-eyes[0]['t']*1.1+up*.08),('right',target+eyes[1]['n']*1.5+eyes[1]['t']*1.1+up*.08)]:
 cam.location=loc;cam.rotation_euler=(target-loc).to_track_quat('-Z','Y').to_euler()
 for a in [0,.25,.5,.75,1]:
  for eye in eyes:pose_eye(eye,a)
  bpy.context.view_layer.update();scene.render.filepath=str(J/('r%d-%s-%03d.png'%(REV,view,a*100)));bpy.ops.render.render(write_still=True)
rig.animation_data.action=bpy.data.actions['relaxed_idle'];scene.frame_set(1);scene.render.fps=30;scene.render.fps_base=1
bpy.ops.file.pack_all();bpy.ops.wm.save_as_mainfile(filepath=str(J/('pip-blink-r%d.blend'%REV)))
report.update(triangles=sum(len(p.vertices)-2 for p in body.data.polygons),vertices=len(body.data.vertices),bones=[b.name for b in rig.data.bones])
(J/('r%d-report.json'%REV)).write_text(json.dumps(report,indent=2))
(J/'eye-frames.json').write_text(json.dumps([{k:list(v) if isinstance(v,Vector) else v for k,v in e.items()} for e in eyes],indent=2))

import bpy,bmesh,json,math,heapq,sys
import numpy as np
from pathlib import Path
from mathutils import Vector,Matrix
J=Path(__file__).parent;REV=int(sys.argv[sys.argv.index('--')+1])
bpy.ops.wm.open_mainfile(filepath=str(J/'native-source.blend'))
s=bpy.context.scene;r=bpy.data.objects['PipRig'];body=bpy.data.objects['Pip repaired Tripo skin'];r.animation_data.action=None
for track in r.animation_data.nla_tracks:track.mute=True
for p in r.pose.bones:p.matrix_basis.identity()
eyes=json.loads((J/'native-fit.json').read_text());up=Vector((0,0,1));report=[]
for e in eyes:
 for key in ['centre','n','t','pivot']:e[key]=Vector(e[key])
bm=bmesh.new();bm.from_mesh(body.data);bm.verts.ensure_lookup_table()
# Main skin and fixed eyes are separate connected components.
remaining=set(bm.verts);components=[]
while remaining:
 seed=next(iter(remaining));remaining.remove(seed);component={seed};stack=[seed]
 while stack:
  v=stack.pop()
  for edge in v.link_edges:
   other=edge.other_vert(v)
   if other in remaining:remaining.remove(other);component.add(other);stack.append(other)
 components.append(component)
main=max(components,key=len);main_ids={v.index for v in main}
adjacency={v.index:[edge.other_vert(v).index for edge in v.link_edges] for v in main}
boundaries=[v.index for v in main if any(edge.is_boundary for edge in v.link_edges)]
original=[v.co.copy() for v in body.data.vertices]
for ei,e in enumerate(eyes):
 c,n,t=e['centre'],e['n'],e['t'];rx=.035 if ei==0 else .040;rz=.042
 boundary=[i for i in boundaries if (original[i]-c).length<.075 and abs((original[i]-c).dot(n))<.05]
 assert len(boundary)>20,(e['name'],len(boundary))
 # Smooth the actual boundary loop in its optical plane; no replacement cap.
 boundary_set=set(boundary);curve={i:original[i].copy() for i in boundary}
 border_neighbors={v.index:[edge.other_vert(v).index for edge in v.link_edges if edge.is_boundary and edge.other_vert(v).index in boundary_set] for v in main if v.index in boundary_set}
 assert all(len(v)==2 for v in border_neighbors.values())
 for _ in range(8):
  for strength in [.5,-.51]:
   updates={}
   for i in boundary:
    avg=sum((curve[j] for j in border_neighbors[i]),Vector())/2
    delta=(avg-curve[i])*strength;delta-=n*delta.dot(n);updates[i]=curve[i]+delta
   curve.update(updates)
 silhouette_delta={i:curve[i]-original[i] for i in boundary}
 for i in boundary:body.data.vertices[i].co=curve[i]
 def seam(q):return e['pivot'].z+.004*(1-min(1,((q-c).dot(t)/rx)**2))
 distances={i:0. for i in boundary};queue=[(0.,i) for i in boundary];heapq.heapify(queue)
 while queue:
  d,i=heapq.heappop(queue)
  if d>distances[i] or d>.055:continue
  for other in adjacency[i]:
   nd=d+(original[i]-original[other]).length
   if nd<distances.get(other,1e9):distances[other]=nd;heapq.heappush(queue,(nd,other))
 # Fit one shared closure surface; both upper and lower rim land on it.
 points=[]
 for i in boundary:
  q=body.data.vertices[i].co;x=(q-c).dot(t)/rx;dz=q.z-seam(q);w=abs(dz)/(.075 if dz>=0 else .060)
  points.append([x,(q-c).dot(n)+.020*w])
 coeff=np.polyfit(np.array(points)[:,0],np.array(points)[:,1],2)
 coeff[0]=min(-.004,max(-.018,coeff[0]));coeff[1]=0;coeff[2]+=.003
 corrections={}
 for i in boundary:
  q=body.data.vertices[i].co;x=(q-c).dot(t)/rx;dz=q.z-seam(q);w=abs(dz)/(.075 if dz>=0 else .060)
  desired=float(np.polyval(coeff,x))-.020*w;corrections[i]=desired-(q-c).dot(n)
 # Smooth depth adjustments only; keep the original opening silhouette and UVs.
 delta={i:n*corrections[i]+silhouette_delta[i] if i in corrections else Vector() for i in main_ids}
 mobile=[i for i,d in distances.items() if i not in corrections and d<.036 and all((original[i]-c).length<=(original[i]-other['centre']).length for other in eyes)]
 for _ in range(50):
  delta.update({i:sum((delta[j] for j in adjacency[i]),Vector())/len(adjacency[i]) for i in mobile})
 for i in boundary+mobile:body.data.vertices[i].co=original[i]+delta[i]
 upper=body.vertex_groups[e['name']];lower=body.vertex_groups[e['lower_name']];head=body.vertex_groups['head']
 for i,d in distances.items():
  if d>.055 or any((original[i]-other['centre']).length<(original[i]-c).length for other in eyes):continue
  q=body.data.vertices[i].co;dz=q.z-seam(q)
  falloff=math.exp(-(d/.023)**2)
  w=abs(dz)/(.075 if dz>=0 else .060)*falloff
  for group in list(body.data.vertices[i].groups):body.vertex_groups[group.group].remove([i])
  (upper if dz>=0 else lower).add([i],w,'REPLACE');head.add([i],1-w,'REPLACE')
 # Extend the fixed eye island slightly under the smoothed skin aperture.
 for component in components:
  if component is main:continue
  centre=sum((original[v.index] for v in component),Vector())/len(component)
  if (centre-c).length>.065:continue
  edge_ids={v.index for v in component if any(edge.is_boundary for edge in v.link_edges)}
  for i in edge_ids:
   q=body.data.vertices[i].co;offset=q-c;body.data.vertices[i].co=q+t*(offset.dot(t)*.15)+up*(offset.z*.15)-n*.010
 report.append({'eye':e['name'],'boundary':boundary,'closure_surface':list(coeff),'max_depth_correction':max(abs(v) for v in corrections.values()),'geodesic_falloff':.023})
bm.free();body.data.update()
# A connected thin lid margin folds inward over the fixed eye. The upper margin
# extends 0.8 mm past the closure line at closure, eliminating light leaks at the seam.
margin=bpy.data.materials.new('Pip eyelid margin');margin.use_nodes=True
shader=margin.node_tree.nodes.get('Principled BSDF');shader.inputs['Base Color'].default_value=(.030,.013,.006,1);shader.inputs['Roughness'].default_value=.88
new_objects=[];weld_points=[]
for e,data in zip(eyes,report):
 boundary=data['boundary'];ids=set(boundary);mapping={idx:i for i,idx in enumerate(boundary)};count=len(boundary);verts=[];faces=[]
 for level in range(3):
  for idx in boundary:
   q=body.data.vertices[idx].co.copy();x=(q-e['centre']).dot(e['t'])/(.035 if e['name']=='blink.L' else .040)
   seam=e['pivot'].z+.004*(1-min(1,x*x));upper=q.z>=seam
   if level:q+=up*(-.0008 if upper else .0008)+e['n']*((.0010 if upper else .0005) if level==1 else -.002)
   verts.append(q)
 weld_points.extend(verts[:count])
 for edge in body.data.edges:
  a,b=edge.vertices
  if a not in ids or b not in ids:continue
  # Only the original hole boundary, not interior chords between rim vertices.
  linked=sum(a in p.vertices and b in p.vertices for p in body.data.polygons)
  if linked!=1:continue
  a=mapping[a];b=mapping[b]
  for level in range(2):faces.append((level*count+a,level*count+b,(level+1)*count+b,(level+1)*count+a))
 mesh=bpy.data.meshes.new(e['name']+' connected margin');mesh.from_pydata(verts,[],faces);mesh.update();mesh.materials.append(margin)
 obj=bpy.data.objects.new(mesh.name,mesh);s.collection.objects.link(obj)
 groups={g.name:obj.vertex_groups.new(name=g.name) for g in body.vertex_groups}
 for level in range(3):
  for i,idx in enumerate(boundary):
   for group in body.data.vertices[idx].groups:groups[body.vertex_groups[group.group].name].add([level*count+i],group.weight,'REPLACE')
 for face in mesh.polygons:face.use_smooth=True
 new_objects.append(obj)
bpy.ops.object.select_all(action='DESELECT');body.select_set(True)
for obj in new_objects:obj.select_set(True)
bpy.context.view_layer.objects.active=body;bpy.ops.object.join()
bm=bmesh.new();bm.from_mesh(body.data);seams=[v for v in bm.verts if any((v.co-p).length<1e-7 for p in weld_points)]
bmesh.ops.remove_doubles(bm,verts=seams,dist=1e-7);bm.to_mesh(body.data);bm.free();body.data.update()

def pose(e,a):
 for name,dz in [(e['name'],-.075),(e['lower_name'],.060)]:
  b=r.data.bones[name];p=r.pose.bones[name];matrix=b.matrix_local.inverted()@Matrix.Translation(up*(dz*a)+e['n']*(.020*a))@b.matrix_local
  p.location,p.rotation_quaternion,p.scale=matrix.decompose()
s.render.engine='CYCLES';s.cycles.samples=20;s.render.resolution_x=1000;s.render.resolution_y=650;s.render.resolution_percentage=100
cam=s.camera;cam.data.type='ORTHO';cam.data.ortho_scale=.37;target=(eyes[0]['centre']+eyes[1]['centre'])/2;front=(eyes[0]['n']+eyes[1]['n']).normalized()
for view,loc in [('front',target+front*2+up*.03),('left',target+eyes[0]['n']*1.5-eyes[0]['t']*1.1+up*.08),('right',target+eyes[1]['n']*1.5+eyes[1]['t']*1.1+up*.08)]:
 cam.location=loc;cam.rotation_euler=(target-loc).to_track_quat('-Z','Y').to_euler()
 for a in [0,.25,.5,.75,1]:
  for e in eyes:pose(e,a)
  bpy.context.view_layer.update();s.render.filepath=str(J/f'r{REV}-{view}-{int(a*100):03}.png');bpy.ops.render.render(write_still=True)
r.animation_data.action=bpy.data.actions['relaxed_idle'];s.frame_set(1)
bpy.ops.file.pack_all();bpy.ops.wm.save_as_mainfile(filepath=str(J/f'pip-blink-r{REV}.blend'))
(J/f'r{REV}-report.json').write_text(json.dumps(report,indent=2))

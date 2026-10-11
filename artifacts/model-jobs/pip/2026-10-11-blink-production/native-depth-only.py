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
  q=original[i];x=(q-c).dot(t)/rx;dz=q.z-seam(q);w=abs(dz)/(.075 if dz>=0 else .060)
  points.append([x,(q-c).dot(n)+.020*w])
 coeff=np.polyfit(np.array(points)[:,0],np.array(points)[:,1],2)
 coeff[0]=min(-.004,max(-.018,coeff[0]));coeff[1]=0;coeff[2]+=.003
 corrections={}
 for i in boundary:
  q=original[i];x=(q-c).dot(t)/rx;dz=q.z-seam(q);w=abs(dz)/(.075 if dz>=0 else .060)
  desired=float(np.polyval(coeff,x))-.020*w;corrections[i]=desired-(q-c).dot(n)
 # Smooth depth adjustments only; keep the original opening silhouette and UVs.
 delta={i:corrections.get(i,0) for i in main_ids}
 mobile=[i for i,d in distances.items() if i not in corrections and d<.036 and all((original[i]-c).length<=(original[i]-other['centre']).length for other in eyes)]
 for _ in range(50):
  delta.update({i:sum(delta[j] for j in adjacency[i])/len(adjacency[i]) for i in mobile})
 for i in boundary+mobile:body.data.vertices[i].co=original[i]+n*delta[i]
 upper=body.vertex_groups[e['name']];lower=body.vertex_groups[e['lower_name']];head=body.vertex_groups['head']
 for i,d in distances.items():
  if d>.055 or any((original[i]-other['centre']).length<(original[i]-c).length for other in eyes):continue
  q=body.data.vertices[i].co;dz=q.z-seam(q)
  falloff=math.exp(-(d/.023)**2)
  w=abs(dz)/(.075 if dz>=0 else .060)*falloff
  for group in list(body.data.vertices[i].groups):body.vertex_groups[group.group].remove([i])
  (upper if dz>=0 else lower).add([i],w,'REPLACE');head.add([i],1-w,'REPLACE')
 report.append({'eye':e['name'],'boundary':boundary,'closure_surface':list(coeff),'max_depth_correction':max(abs(v) for v in corrections.values()),'geodesic_falloff':.023})
bm.free();body.data.update()
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

import bpy,bmesh,json,sys,math
from pathlib import Path
from mathutils import Vector,Matrix
from mathutils.bvhtree import BVHTree
J=Path(__file__).parent;REV=int(sys.argv[sys.argv.index('--')+1]);exported='--export' in sys.argv;bpy.ops.wm.open_mainfile(filepath=str(J/('clean-reimport.blend' if exported else f'pip-blink-r{REV}.blend')))
s=bpy.context.scene;r=bpy.data.objects['PipRig'];body=bpy.data.objects['Pip repaired Tripo skin'];r.animation_data.action=None
for track in r.animation_data.nla_tracks:track.mute=True
for p in r.pose.bones:p.matrix_basis.identity()
eyes=json.loads((J/'native-fit.json').read_text())
for e in eyes:
 for k in ['centre','n','t']:e[k]=Vector(e[k])
bm=bmesh.new();bm.from_mesh(body.data);bm.verts.ensure_lookup_table();remaining=set(bm.verts);components=[]
aliases={}
for vertex in bm.verts:aliases.setdefault(tuple(round(x,6) for x in vertex.co),[]).append(vertex)
while remaining:
 seed=next(iter(remaining));remaining.remove(seed);group={seed};stack=[seed]
 while stack:
  v=stack.pop()
  for other in [edge.other_vert(v) for edge in v.link_edges]+aliases[tuple(round(x,6) for x in v.co)]:
   if other in remaining:remaining.remove(other);group.add(other);stack.append(other)
 components.append(group)
main=max(components,key=len);eye_ids=[]
for e in eyes:
 eye_group=min((g for g in components if g is not main),key=lambda g:(sum((v.co for v in g),Vector())/len(g)-e['centre']).length)
 eye_ids.append({v.index for v in eye_group})
fixed_faces=[{p.index for p in body.data.polygons if all(i in ids for i in p.vertices)} for ids in eye_ids]
results=[]
for amount in [i/40 for i in range(41)]:
 for e in eyes:
  for name,dz in [(e['name'],-.075),(e['lower_name'],.060)]:
   b=r.data.bones[name];p=r.pose.bones[name];m=b.matrix_local.inverted()@Matrix.Translation(Vector((0,0,dz*amount))+e['n']*(.020*amount))@b.matrix_local;p.location,p.rotation_quaternion,p.scale=m.decompose()
 bpy.context.view_layer.update();evaluated=body.evaluated_get(bpy.context.evaluated_depsgraph_get());mesh=evaluated.to_mesh();coords=[v.co for v in mesh.vertices];tree=BVHTree.FromPolygons(coords,[p.vertices[:] for p in mesh.polygons])
 for ei,e in enumerate(eyes):
  for angle in [-.75,0,.75]:
   direction=(e['n']+e['t']*angle).normalized();visible=[]
   for idx in fixed_faces[ei]:
    point=mesh.polygons[idx].center;hit,normal,first,d=tree.ray_cast(point+direction*2,-direction)
    if first in fixed_faces[ei]:visible.append(idx)
   results.append({'amount':amount,'eye':ei,'angle':angle,'visible_eye_faces':len(visible),'faces':visible if amount==1 else []})
 evaluated.to_mesh_clear()
(J/(f'r{REV}-export-visibility.json' if exported else f'r{REV}-visibility.json')).write_text(json.dumps(results,indent=2));print('CLOSED VISIBILITY',[(x['eye'],x['angle'],x['visible_eye_faces']) for x in results if x['amount']==1])

assert all(x['visible_eye_faces']==0 for x in results if x['amount']==1),'Eye surface exposed at full closure'
assert all(x['visible_eye_faces']>30 for x in results if x['amount']==0),'Eye classification or open pose failed'

for eye in [0,1]:
 for angle in [-.75,0,.75]:
  counts=[x["visible_eye_faces"] for x in results if x["eye"]==eye and x["angle"]==angle]
  assert all(b<=a for a,b in zip(counts,counts[1:])),"Eye visibility reverses during closure"

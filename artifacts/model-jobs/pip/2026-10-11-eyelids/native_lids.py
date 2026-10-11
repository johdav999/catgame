"""Use Pip's original facial polygons as eyelids, retaining UV/coat continuity."""
def construct(ns):
 import bpy,bmesh,math
 from mathutils import Vector
 body,rig,eyes,scene=ns['body'],ns['rig'],ns['eyes'],ns['scene'];surface=ns['surface'];coat=ns['coat'];up=Vector((0,0,1))
 newobjects=[];removed=set();body.data.materials.clear();body.data.materials.append(coat)
 bpy.context.view_layer.objects.active=rig;rig.select_set(True);bpy.ops.object.mode_set(mode='EDIT')
 for side in ['L','R']:
  upper=rig.data.edit_bones['blink.'+side];lower=rig.data.edit_bones.new('blink.lower.'+side)
  lower.head=upper.head;lower.tail=upper.tail;lower.roll=upper.roll;lower.parent=rig.data.edit_bones['head']
 bpy.ops.object.mode_set(mode='OBJECT')
 for eye in eyes:
  n,t,c=eye['n'],eye['t'],eye['centre'];hit=surface(c+n*.2,-n)
  rx,rz=(.035,.042) if eye['name']=='blink.L' else (.040,.043)
  if eye['name']=='blink.R':c=c-up*.004
  eye['front_hit']=hit[0];eye['pivot']=hit[0]-up*.008;eye['angle']=0
  eye['lower_name']=eye['name'].replace('blink.','blink.lower.')
  def seam(p):
   x=max(-1,min(1,(p-c).dot(t)/rx))
   return eye['pivot'].z+.004*(1-x*x)
  def radius(p):
   q=p-c;return math.sqrt((q.dot(t)/rx)**2+(q.z/rz)**2)
  selected={p.index for p in body.data.polygons if radius(p.center)<1 and abs((p.center-c).dot(n))<.07}
  underlay=selected
  removed.update(selected)
  eye_mesh=bpy.data.meshes.new('Pip fixed eye '+eye['name'])
  verts=[]
  for v in body.data.vertices:
   verts.append(v.co.copy())
  eye_mesh.from_pydata(verts,[],[body.data.polygons[i].vertices[:] for i in sorted(underlay)]);eye_mesh.update()
  obj=bpy.data.objects.new('Pip fixed eye '+eye['name'],eye_mesh);scene.collection.objects.link(obj);eye_mesh.materials.append(coat)
  uv=eye_mesh.uv_layers.new()
  for face,oldidx in zip(eye_mesh.polygons,sorted(underlay)):
   for newloop,oldloop in zip(face.loop_indices,body.data.polygons[oldidx].loop_indices):uv.data[newloop].uv=body.data.uv_layers.active.data[oldloop].uv
   face.use_smooth=True
  vg=obj.vertex_groups.new(name='head');vg.add(list(range(len(verts))),1,'REPLACE')
  bm=bmesh.new();bm.from_mesh(eye_mesh);bmesh.ops.delete(bm,geom=[v for v in bm.verts if not v.link_faces],context='VERTS');bm.to_mesh(eye_mesh);bm.free()
  newobjects.append(obj)
  # Original vertices on the opening all reach the same closure plane. Falloff
  # moves the surrounding fur smoothly while leaving the nose/brow anchored.
  edges={}
  for p in body.data.polygons:
   for edge in p.edge_keys:edges.setdefault(tuple(sorted(edge)),[]).append(p.index)
  boundary=set(i for e,fs in edges.items() if len(fs)==2 and sum(f in selected for f in fs)==1 for i in e)
  blink=body.vertex_groups.get(eye['name']);lower=body.vertex_groups.new(name=eye['lower_name']);head=body.vertex_groups['head']
  for v in body.data.vertices:
   q=v.co-c;r=radius(v.co)
   if r>1.55 or abs(q.dot(n))>.08:continue
   w=1 if v.index in boundary or r<1.02 else max(0,min(1,(1.55-r)/.53))
   w=w*w*(3-2*w);d=v.co.z-seam(v.co)
   # A small overlap seals the aperture from oblique views despite the two
   # boundary chains having different vertex spacing and front offsets.
   w*=min(1,(abs(d)+.002)/(.075 if d>=0 else .060))
   for group in list(v.groups):body.vertex_groups[group.group].remove([v.index])
   if w:(blink if d>=0 else lower).add([v.index],w,'REPLACE')
   if w<1:head.add([v.index],1-w,'REPLACE')
  print('NATIVE SOCKET',eye['name'],len(selected),'cut faces',len(boundary),'rim vertices')
 bm=bmesh.new();bm.from_mesh(body.data);bm.faces.ensure_lookup_table();bmesh.ops.delete(bm,geom=[bm.faces[i] for i in removed],context='FACES');bm.to_mesh(body.data);bm.free();body.data.update()
 return newobjects

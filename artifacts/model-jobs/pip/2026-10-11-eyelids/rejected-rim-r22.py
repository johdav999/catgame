"""Use Pip's original facial polygons as eyelids, retaining UV/coat continuity."""
def construct(ns):
 import bpy,bmesh,math
 from mathutils import Vector
 body,rig,eyes,scene=ns['body'],ns['rig'],ns['eyes'],ns['scene'];surface=ns['surface'];coat=ns['coat'];up=Vector((0,0,1))
 newobjects=[];removed=set();body.data.materials.clear();body.data.materials.append(coat)
 margin=bpy.data.materials.new('Pip natural lid margin');margin.use_nodes=True
 ms=margin.node_tree.nodes.get('Principled BSDF');ms.inputs['Base Color'].default_value=(.055,.025,.011,1);ms.inputs['Roughness'].default_value=.82
 mt=margin.node_tree.nodes.new('ShaderNodeTexImage');mt.image=coat.node_tree.nodes.get('Principled BSDF').inputs['Base Color'].links[0].from_node.image
 margin.node_tree.links.new(mt.outputs['Color'],ms.inputs['Base Color'])
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
  underlay={p.index for p in body.data.polygons if radius(p.center)<1.2 and abs((p.center-c).dot(n))<.07}
  removed.update(selected)
  eye_mesh=bpy.data.meshes.new('Pip fixed eye '+eye['name'])
  verts=[]
  for v in body.data.vertices:
   r=radius(v.co);verts.append(v.co-n*(.0015+.035*max(0,min(1,(r-.97)/.23))))
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
  for i in boundary:
   vertex=body.data.vertices[i];q=vertex.co-c;theta=math.atan2(q.z/rz,q.dot(t)/rx)
   target=c+t*(rx*math.cos(theta))+up*(rz*math.sin(theta));sh=surface(target+n*.2,-n)
   if sh:vertex.co=sh[0]
  ordered=sorted(boundary,key=lambda i:math.atan2((body.data.vertices[i].co-c).z/rz,(body.data.vertices[i].co-c).dot(t)/rx))
  rimverts=[body.data.vertices[i].co.copy() for i in ordered]
  rimverts += [p-n*.004 for p in rimverts]
  count=len(ordered);rimfaces=[(i,(i+1)%count,(i+1)%count+count,i+count) for i in range(count)]
  mesh=bpy.data.meshes.new('Pip eyelid thickness '+eye['name']);mesh.from_pydata(rimverts,[],rimfaces);mesh.update()
  obj=bpy.data.objects.new(mesh.name,mesh);scene.collection.objects.link(obj);mesh.materials.append(margin)
  source_uv={loop.vertex_index:body.data.uv_layers.active.data[loop.index].uv.copy() for loop in body.data.loops}
  uv=mesh.uv_layers.new()
  for loop in mesh.loops:uv.data[loop.index].uv=source_uv[ordered[loop.vertex_index%count]]
  for f in mesh.polygons:f.use_smooth=True
  vg=obj.vertex_groups.new(name=eye['name']);lg=obj.vertex_groups.new(name=eye['lower_name']);hg=obj.vertex_groups.new(name='head')
  for i,p in enumerate(rimverts):
   d=p.z-seam(p);w=abs(d)/(.075 if d>=0 else .060);assert w<=1
   (vg if d>=0 else lg).add([i],w,'REPLACE');hg.add([i],1-w,'REPLACE')
  newobjects.append(obj)
  blink=body.vertex_groups.get(eye['name']);lower=body.vertex_groups.new(name=eye['lower_name']);head=body.vertex_groups['head']
  for v in body.data.vertices:
   q=v.co-c;r=radius(v.co)
   if r>1.55 or abs(q.dot(n))>.08:continue
   w=1 if v.index in boundary or r<1.02 else max(0,min(1,(1.55-r)/.53))
   w=w*w*(3-2*w);d=v.co.z-seam(v.co)
   w*=min(1,abs(d)/(.075 if d>=0 else .060))
   for group in list(v.groups):body.vertex_groups[group.group].remove([v.index])
   if w:(blink if d>=0 else lower).add([v.index],w,'REPLACE')
   if w<1:head.add([v.index],1-w,'REPLACE')
  print('NATIVE SOCKET',eye['name'],len(selected),'cut faces',len(boundary),'rim vertices')
 bm=bmesh.new();bm.from_mesh(body.data);bm.faces.ensure_lookup_table();bmesh.ops.delete(bm,geom=[bm.faces[i] for i in removed],context='FACES');bm.to_mesh(body.data);bm.free();body.data.update()
 return newobjects

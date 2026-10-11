"""Replace socket faces with continuous weighted eyelid loops.

The textured eye surface is retained behind the opening, weighted to the head.
The new socket's outer vertices weld to the original face, while the inner rim
is driven by the existing blink bone. No independent cover silhouette remains.
"""
def construct(ns):
 import bpy,bmesh,math
 import numpy as np
 from mathutils import Vector
 body,eyes,scene=ns['body'],ns['eyes'],ns['scene']
 surface,color,coat=ns['surface'],ns['color'],ns['coat']
 up=Vector((0,0,1));newobjects=[];allremoved=set();atlas=np.ones((256,512,4));boundary_updates={}
 body.data.materials.clear();body.data.materials.append(coat)
 mat=bpy.data.materials.new('Pip continuous eyelid skin');mat.use_nodes=True
 shader=mat.node_tree.nodes.get('Principled BSDF');shader.inputs['Roughness'].default_value=.86
 original=[tuple(p.vertices) for p in body.data.polygons]
 for eyeindex,eye in enumerate(eyes):
  n,t,c=eye['n'],eye['t'],eye['centre'];hit=surface(c+n*.2,-n)
  eye['front_hit']=hit[0];eye['pivot']=hit[0]-up*.012;eye['angle']=0
  selected=set()
  for p in body.data.polygons:
   q=p.center-c
   if (q.dot(t)/.057)**2+(q.z/.066)**2<1 and abs(q.dot(n))<.09:selected.add(p.index)
  assert not allremoved.intersection(selected),'Eye sockets overlap'
  allremoved.update(selected)
  edgefaces={}
  for p in body.data.polygons:
   for edge in p.edge_keys:edgefaces.setdefault(tuple(sorted(edge)),[]).append(p.index)
  edges=[edge for edge,faces in edgefaces.items() if len(faces)==2 and sum(f in selected for f in faces)==1]
  neighbors={}
  for a,b in edges:neighbors.setdefault(a,[]).append(b);neighbors.setdefault(b,[]).append(a)
  assert all(len(v)==2 for v in neighbors.values()),'Nonmanifold socket boundary'
  ring=[next(iter(neighbors))];previous=None
  while True:
   a=ring[-1];b=next(v for v in neighbors[a] if v!=previous)
   if b==ring[0]:break
   ring.append(b);previous=a
  assert len(ring)==len(neighbors),'Multiple socket holes'
  # Preserve the original eye as a static inset surface.
  inset=[]
  for vertex in body.data.vertices:
   q=vertex.co-c;r=math.sqrt((q.dot(t)/.057)**2+(q.z/.066)**2)
   depth=.001+.022*max(0,min(1,(r-.65)/.35))
   inset.append(vertex.co-n*depth)
  eye_mesh=bpy.data.meshes.new('Pip original eye '+eye['name']);eye_mesh.from_pydata(inset,[],[original[i] for i in sorted(selected)]);eye_mesh.update()
  obj=bpy.data.objects.new('Pip inset eye '+eye['name'],eye_mesh);scene.collection.objects.link(obj);eye_mesh.materials.append(coat)
  uv=eye_mesh.uv_layers.new()
  for newface,oldidx in zip(eye_mesh.polygons,sorted(selected)):
   oldface=body.data.polygons[oldidx]
   for newloop,oldloop in zip(newface.loop_indices,oldface.loop_indices):uv.data[newloop].uv=body.data.uv_layers.active.data[oldloop].uv
   newface.use_smooth=True
  vg=obj.vertex_groups.new(name='head');vg.add(list(range(len(eye_mesh.vertices))),1,'REPLACE')
  bm=bmesh.new();bm.from_mesh(eye_mesh);bmesh.ops.delete(bm,geom=[v for v in bm.verts if not v.link_faces],context='VERTS');bm.to_mesh(eye_mesh);bm.free()
  newobjects.append(obj)
  # Preserve the actual boundary topology, including small concave sections.
  area=sum((body.data.vertices[ring[k]].co-c).dot(t)*(body.data.vertices[ring[(k+1)%len(ring)]].co-c).z-(body.data.vertices[ring[(k+1)%len(ring)]].co-c).dot(t)*(body.data.vertices[ring[k]].co-c).z for k in range(len(ring)))
  if area<0:ring.reverse()
  start=max(range(len(ring)),key=lambda k:(body.data.vertices[ring[k]].co-c).dot(t))
  ring=ring[start:]+ring[:start]
  outer=[body.data.vertices[i].co.copy() for i in ring]
  lengths=[(outer[(i+1)%len(outer)]-p).length for i,p in enumerate(outer)]
  total=sum(lengths);angles=[0]
  for length in lengths[:-1]:angles.append(angles[-1]+math.tau*length/total)
  sortedpoints=sorted(zip(angles,outer),key=lambda p:p[0])
  def outer_at(theta):
   for k in range(len(sortedpoints)):
    a,pa=sortedpoints[k];b,pb=sortedpoints[(k+1)%len(sortedpoints)]
    if b<a:b+=math.tau
    q=theta if theta>=a else theta+math.tau
    if a<=q<=b:return pa.lerp(pb,(q-a)/(b-a))
   raise RuntimeError('Missing ring angle')
  # Baked coat strip has an exact original-color outer border and clean fur
  # toward the eyelid margin. Texture samples never span unrelated UV islands.
  for y in range(128):
   v=y/127
   for x in range(512):
    theta=x/511*math.tau;p=outer_at(theta)
    q=c+t*(.043*math.cos(theta))+up*(.049*math.sin(theta))
    sample=q.lerp(p,.62+.38*v)
    sh=surface(sample+n*.2,-n);col=color(sh[2]) if sh else np.array([.85,.53,.25])
    atlas[eyeindex*128+y,x,:3]=col
  regular=[]
  for vertex,theta in zip(ring,angles):
   q=c+t*(.060*math.cos(theta))+up*(.071*math.sin(theta));sh=surface(q+n*.2,-n)
   p=sh[0] if sh else q;regular.append(p);boundary_updates[vertex]=p
  outer=regular
  verts=[];uvs=[];faces=[];weights=[]
  fractions=[0,.035,.25,.5,.75,1]
  for v in fractions:
   for theta,pouter in zip(angles,outer):
    q=c+t*(.043*math.cos(theta))+up*(.049*math.sin(theta))
    sh=surface(q+n*.20,-n)
    inner=sh[0]+n*.002 if sh else q
    p=inner.lerp(pouter,v)+n*(.002*math.sin(math.pi*v))
    if v==1:p=pouter
    verts.append(p);uvs.append((theta/math.tau,(eyeindex*128+.5+v*127)/256))
    weights.append((1-v)**1.4)
  count=len(ring)
  for j in range(len(fractions)-1):
   for i in range(count):
    a=j*count+i;b=j*count+(i+1)%count;faces.append((a,b,b+count,a+count))
  mesh=bpy.data.meshes.new('Pip eyelid loops '+eye['name']);mesh.from_pydata(verts,[],faces);mesh.update()
  obj=bpy.data.objects.new('Pip continuous eyelids '+eye['name'],mesh);scene.collection.objects.link(obj);mesh.materials.append(mat)
  uv=mesh.uv_layers.new()
  for face in mesh.polygons:
   wrap=max(uvs[mesh.loops[l].vertex_index][0] for l in face.loop_indices)-min(uvs[mesh.loops[l].vertex_index][0] for l in face.loop_indices)>.5
   for l in face.loop_indices:
    u,v=uvs[mesh.loops[l].vertex_index]
    if wrap and u<.5:u+=1
    uv.data[l].uv=(u,v)
   face.use_smooth=True
  # This constructed band faces out; preserve all existing body normals.
  if mesh.polygons[0].normal.dot(n)<0:
   bm=bmesh.new();bm.from_mesh(mesh);bmesh.ops.reverse_faces(bm,faces=list(bm.faces));bm.to_mesh(mesh);bm.free()
  vg=obj.vertex_groups.new(name=eye['name']);hg=obj.vertex_groups.new(name='head')
  for i,w in enumerate(weights):
   if w:vg.add([i],w,'REPLACE')
   if w<1:hg.add([i],1-w,'REPLACE')
  newobjects.append(obj);print('SOCKET',eye['name'],len(selected),'faces',count,'boundary vertices')
 # Raycasts and UV sampling above used the original topology; delete only now.
 for i,p in boundary_updates.items():body.data.vertices[i].co=p
 bm=bmesh.new();bm.from_mesh(body.data);bm.faces.ensure_lookup_table()
 bmesh.ops.delete(bm,geom=[bm.faces[i] for i in allremoved],context='FACES');bm.to_mesh(body.data);bm.free();body.data.update()
 image=bpy.data.images.new('Pip socket coat derived from source UVs',width=512,height=256);image.pixels=atlas.ravel();image.pack()
 node=mat.node_tree.nodes.new('ShaderNodeTexImage');node.image=image;mat.node_tree.links.new(node.outputs['Color'],shader.inputs['Base Color'])
 return newobjects

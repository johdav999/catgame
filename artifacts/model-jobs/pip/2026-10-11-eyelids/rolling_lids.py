"""Thin, fitted upper/lower lid shells with a modeled leading margin."""
def construct(ns):
 import bpy,math
 import numpy as np
 from mathutils import Vector
 body,eyes,scene=ns['body'],ns['eyes'],ns['scene'];surface,color=ns['surface'],ns['color'];coat=ns['coat'];up=Vector((0,0,1))
 body.data.materials.clear();body.data.materials.append(coat)
 mat=bpy.data.materials.new('Pip lid skin and margin');mat.use_nodes=True;shader=mat.node_tree.nodes.get('Principled BSDF');shader.inputs['Roughness'].default_value=.88
 atlas=np.ones((256,256,4));objects=[]
 for ei,eye in enumerate(eyes):
  n,t,c=eye['n'],eye['t'],eye['centre'];hit=surface(c+n*.2,-n)
  radius=.10;eye['pivot']=hit[0]-n*(radius-.012);eye['angle']=math.radians(48)
  for lower in [False,True]:
   band=ei*2+int(lower);verts=[];uvs=[];faces=[];segments=32;rows=8
   for y in range(64):
    v=y/63
    for x in range(256):
     u=x/255;xpos=(2*u-1)*.052
     q=c+t*xpos+up*((-.058 if lower else .065)+(-.016 if lower else .020)*v)
     sh=surface(q+n*.2,-n);col=color(sh[2]) if sh else np.array([.9,.6,.3])
     if not lower:
      col=col*.35+np.array([.91,.58,.28])*.65
      if y<1:col=np.array([.23,.12,.055])
     atlas[band*64+y,x,:3]=col
   for j in range(rows+1):
    # Extra loop gives the moving edge a submillimetre rounded margin.
    v=([0,.006,.06,.15,.3,.5,.7,.85,1])[j]
    for i in range(segments+1):
     u=i/segments;x=(2*u-1)*.052;r=math.sqrt(radius*radius-x*x)
     edge=math.radians(12+21*math.sin(math.pi*u)**.7)
     if lower:angle=edge-eye['angle']-math.radians(85)*v
     else:angle=edge+math.radians(85)*v
     p=eye['pivot']+t*x+n*(r*math.cos(angle))+up*(r*math.sin(angle))
     if lower:p-=n*.0005
     verts.append(p);uvs.append((u,(band*64+.5+v*63)/256))
   for j in range(rows):
    for i in range(segments):
     a=j*(segments+1)+i;b=a+1;d=a+segments+1;e=d+1
     faces.append((a,b,e,d) if lower else (a,d,e,b))
   mesh=bpy.data.meshes.new('Pip '+('lower' if lower else 'upper')+' eyelid '+eye['name']);mesh.from_pydata(verts,[],faces);mesh.update()
   obj=bpy.data.objects.new(mesh.name,mesh);scene.collection.objects.link(obj);mesh.materials.append(mat);uv=mesh.uv_layers.new()
   for loop in mesh.loops:uv.data[loop.index].uv=uvs[loop.vertex_index]
   for f in mesh.polygons:f.use_smooth=True
   vg=obj.vertex_groups.new(name='head' if lower else eye['name']);vg.add(list(range(len(verts))),1,'REPLACE');objects.append(obj)
 image=bpy.data.images.new('Pip lid coat sampled from original UVs',width=256,height=256);image.pixels=atlas.ravel();image.pack()
 node=mat.node_tree.nodes.new('ShaderNodeTexImage');node.image=image;mat.node_tree.links.new(node.outputs['Color'],shader.inputs['Base Color'])
 return objects

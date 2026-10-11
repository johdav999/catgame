import bpy, bmesh, sys, json
from mathutils import Vector
from pathlib import Path
J=Path(__file__).parent
rev=int(sys.argv[sys.argv.index('--')+1])
bpy.ops.wm.open_mainfile(filepath=str(J/'before.blend'))
body=bpy.data.objects['Pip repaired Tripo skin']
bm=bmesh.new();bm.from_mesh(body.data)
margin_ids={i for i,m in enumerate(body.data.materials) if m.name=='Pip eyelid margin'}
faces=[f for f in bm.faces if f.material_index in margin_ids]
assert len(faces)>100
bmesh.ops.delete(bm,geom=faces,context='FACES')
bm.to_mesh(body.data);bm.free();body.data.update()
for i in sorted(margin_ids,reverse=True):body.data.materials.pop(index=i)
body.data.calc_normals_split()
skin_normals={tuple(v.co):v.normal.copy() for v in body.data.vertices}
if rev >= 17:
    # Hide the sealing lip beneath the facial skin at rest. Mixed upper/lower
    # weights let it unfold at closure without changing any animation channels.
    eyes=json.loads((J/'native-fit.json').read_text())
    topology=bmesh.new();topology.from_mesh(body.data);topology.verts.ensure_lookup_table()
    remaining=set(topology.verts);components=[]
    while remaining:
        seed=next(iter(remaining));remaining.remove(seed);component={seed};stack=[seed]
        while stack:
            v=stack.pop()
            for edge in v.link_edges:
                other=edge.other_vert(v)
                if other in remaining:remaining.remove(other);component.add(other);stack.append(other)
        components.append(component)
    main=max(components,key=len)
    boundaries=[v.index for v in main if any(edge.is_boundary for edge in v.link_edges)]
    boundary_set=set(boundaries)
    outside={v.index:sum((edge.other_vert(v).co-v.co for edge in v.link_edges if edge.other_vert(v).index not in boundary_set),Vector()) for v in main if v.index in boundary_set}
    topology.free()
    margin=bpy.data.materials.new('Pip recessed eyelid seam');margin.use_nodes=True
    shader=margin.node_tree.nodes.get('Principled BSDF')
    shader.inputs['Base Color'].default_value=(.030,.013,.006,1)
    shader.inputs['Roughness'].default_value=.88
    # The skin-to-fold transition inherits the coat UVs. Only the tucked return
    # face is dark; a dark boundary shared with the face would recreate a halo.
    uv_by_vertex={}
    for poly in body.data.polygons:
        for loop_index in poly.loop_indices:
            uv_by_vertex.setdefault(body.data.loops[loop_index].vertex_index,body.data.uv_layers.active.data[loop_index].uv.copy())
    up=Vector((0,0,1));parts=[];weld=[];audit=[]
    for e in eyes:
        c=Vector(e['centre']);n=Vector(e['n']);t=Vector(e['t'])
        boundary=[i for i in boundaries if (body.data.vertices[i].co-c).length<.075 and abs((body.data.vertices[i].co-c).dot(n))<.05]
        ids=set(boundary);count=len(boundary);assert count>20
        mapping={idx:i for i,idx in enumerate(boundary)}
        verts=[];weights=[];faces=[]
        for level in range(3):
            for idx in boundary:
                q=body.data.vertices[idx].co.copy()
                x=(q-c).dot(t)/(.035 if e['name']=='blink.L' else .040)
                upper=q.z>=e['pivot'][2]+.004*(1-min(1,x*x))
                side=1 if upper else -1
                w={body.vertex_groups[g.group].name:g.weight for g in body.data.vertices[idx].groups}
                if level:
                    # The open lip lies on the skin side of the aperture, below
                    # its surface. At full closure it retains r15's clean overlap.
                    direction=outside[idx]-n*outside[idx].dot(n)
                    direction.normalize()
                    rest_z=direction.z*.003;rest_x=direction.dot(t)*.003
                    rest_n=-.0025 if level==1 else -.0035
                    closed_z=-side*.0008
                    closed_n=(.001 if upper else .0005) if level==1 else -.002
                    if rev>=20:
                        # A tucked microfold has no visible open width. The
                        # secondary lid influences unfold it continuously.
                        rest_x=0;rest_z=side*.00002
                        rest_n=-.00003 if level==1 else -.00006
                        closed_n=(.001 if upper else .0005) if level==1 else (.0008 if upper else .0003)
                    q+=up*rest_z+t*rest_x+n*rest_n
                    total=(closed_n-rest_n)/.020
                    wu=(.060*total-(closed_z-rest_z))/.135
                    wl=total-wu
                    w[e['name']]=w.get(e['name'],0)+wu
                    w[e['lower_name']]=w.get(e['lower_name'],0)+wl
                    w['head']-=total
                    assert min(w.values())>=0,(idx,w)
                verts.append(q);weights.append(w)
        weld.extend(verts[:count])
        for edge in body.data.edges:
            a,b=edge.vertices
            if a not in ids or b not in ids:continue
            if sum(a in p.vertices and b in p.vertices for p in body.data.polygons)!=1:continue
            a=mapping[a];b=mapping[b]
            for level in range(2):faces.append((level*count+a,level*count+b,(level+1)*count+b,(level+1)*count+a))
        mesh=bpy.data.meshes.new(e['name']+' recessed seal');mesh.from_pydata(verts,[],faces);mesh.update();mesh.materials.append(body.data.materials[0]);mesh.materials.append(margin)
        uv=mesh.uv_layers.new(name=body.data.uv_layers.active.name)
        for poly in mesh.polygons:
            poly.material_index=1 if rev>=20 else (0 if poly.index%2==0 else 1)
            for loop_index in poly.loop_indices:
                uv.data[loop_index].uv=uv_by_vertex[boundary[mesh.loops[loop_index].vertex_index%count]]
        obj=bpy.data.objects.new(mesh.name,mesh);bpy.context.scene.collection.objects.link(obj)
        groups={g.name:obj.vertex_groups.new(name=g.name) for g in body.vertex_groups}
        for i,w in enumerate(weights):
            for name,weight in w.items():
                if weight>0:groups[name].add([i],weight,'REPLACE')
        for face in mesh.polygons:face.use_smooth=True
        parts.append(obj);audit.append({'eye':e['name'],'boundary_vertices':count,'max_influences':max(len(w) for w in weights)})
    bpy.ops.object.select_all(action='DESELECT');body.select_set(True)
    for obj in parts:obj.select_set(True)
    bpy.context.view_layer.objects.active=body;bpy.ops.object.join()
    bm=bmesh.new();bm.from_mesh(body.data)
    seams=[v for v in bm.verts if any((v.co-p).length<1e-7 for p in weld)]
    bmesh.ops.remove_doubles(bm,verts=seams,dist=1e-7)
    bm.to_mesh(body.data);bm.free();body.data.update()
    if rev>=21:
        # The thin inside fold must not tilt the facial surface's shading normals.
        body.data.use_auto_smooth=True;body.data.calc_normals_split()
        body.data.normals_split_custom_set([skin_normals.get(tuple(body.data.vertices[loop.vertex_index].co),loop.normal).normalized() for loop in body.data.loops])
    (J/'repair-audit.json').write_text(json.dumps(audit,indent=2))
bpy.ops.wm.save_as_mainfile(filepath=str(J/f'pip-blink-r{rev}.blend'))
print('REMOVED_MARGIN_FACES',len(faces))

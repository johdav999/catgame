"""Fit original textured facial skin to canonical translation-only lid controls."""
import bpy, bmesh, heapq, math
import numpy as np
from mathutils import Vector, Matrix
from fit_cats import clusters

def fit_eyelids(body,rig,cat):
    # Add local facial resolution for a smooth deforming rim, without changing
    # the body's topology budget or discarding its source UVs.
    bm=bmesh.new();bm.from_mesh(body.data)
    edges=[e for e in bm.edges if all(.66<v.co.z<.80 and v.co.y<-.25 for v in e.verts)]
    bmesh.ops.subdivide_edges(bm,edges=edges,cuts=1,use_grid_fill=True)
    bmesh.ops.triangulate(bm,faces=list(bm.faces));bm.to_mesh(body.data);bm.free()
    body.data.update();body.data.calc_normals_split()
    original_normals={tuple(body.data.vertices[l.vertex_index].co):l.normal.copy() for l in body.data.loops}
    tex=body.data.materials[0].node_tree.nodes.get('Principled BSDF').inputs['Base Color'].links[0].from_node.image
    pixels=np.array(tex.pixels[:]).reshape(tex.size[1],tex.size[0],4)
    points=[];normals=[]
    for loop in body.data.loops:
        v=body.data.vertices[loop.vertex_index];uv=body.data.uv_layers.active.data[loop.index].uv
        col=pixels[int(uv.y%1*(tex.size[1]-1)),int(uv.x%1*(tex.size[0]-1)),:3]
        if (.696 if cat!='mabel' else .71)<v.co.z<.81 and v.co.y<-.20 and max(col)<.13:
            points.append(v.co[:]);normals.append(v.normal[:])
    centres,assign=clusters(np.array(points),2)
    order=np.argsort(centres[:,0]);eyes=[]
    for side,index in zip(['L','R'],order):
        c=Vector(centres[index]);c.z-=.012
        nn=np.array(normals)[assign==index].mean(0);n=Vector((nn[0],nn[1],0)).normalized()
        t=Vector((-n.y,n.x,0))
        samples=np.array(points)[assign==index]-np.array(c)
        rx=max(.042,min(.050,float(np.max(np.abs(samples@np.array(t))))*1.45))
        rz=.048
        eyes.append({'name':'blink.'+side,'lower':'blink.lower.'+side,'centre':c,'n':n,'t':t,'rx':rx,'rz':rz,'travel':.15,'advance':.09})
    removed=set();created=[];audit=[]
    edge_faces={}
    for p in body.data.polygons:
        for edge in p.edge_keys:edge_faces.setdefault(tuple(sorted(edge)),[]).append(p.index)
    for eye in eyes:
        c,n,t=eye['centre'],eye['n'],eye['t'];rx,rz=eye['rx'],eye['rz']
        def radius(p):q=p-c;return math.sqrt((q.dot(t)/rx)**2+(q.z/rz)**2)
        selected={p.index for p in body.data.polygons if radius(p.center)<1 and abs((p.center-c).dot(n))<.04}
        assert len(selected)>8,(cat,eye,len(selected))
        assert not removed.intersection(selected)
        removed.update(selected)
        boundary={i for edge,fs in edge_faces.items() if len(fs)==2 and sum(f in selected for f in fs)==1 for i in edge}
        # Copy the exact source eye, UVs and colour into an inset stationary island.
        covered=set(selected)
        for _ in range(2):
            ring={f for fs in edge_faces.values() if any(f in covered for f in fs) for f in fs}
            covered.update(ring)
        mesh=bpy.data.meshes.new(cat+' fixed '+eye['name'])
        # Extend the copied eye beyond the skin cut so rim smoothing cannot
        # expose a crack; expansion remains hidden beneath the surrounding coat.
        mesh.from_pydata([v.co-n*.008 for v in body.data.vertices],[],[body.data.polygons[i].vertices[:] for i in sorted(covered)])
        mesh.update();mesh.materials.append(body.data.materials[0]);uv=mesh.uv_layers.new()
        for p,old in zip(mesh.polygons,sorted(covered)):
            for a,b in zip(p.loop_indices,body.data.polygons[old].loop_indices):uv.data[a].uv=body.data.uv_layers.active.data[b].uv
            p.use_smooth=True
        obj=bpy.data.objects.new(mesh.name,mesh);bpy.context.scene.collection.objects.link(obj)
        obj.vertex_groups.new(name='head').add(list(range(len(mesh.vertices))),1,'REPLACE')
        bm=bmesh.new();bm.from_mesh(mesh);bmesh.ops.delete(bm,geom=[v for v in bm.verts if not v.link_faces],context='VERTS');bm.to_mesh(mesh);bm.free()
        created.append(obj)
        # Smooth the opening in its optical plane, retaining the original UVs.
        neighbors={i:[] for i in boundary}
        for edge,fs in edge_faces.items():
            if len(fs)==2 and sum(f in selected for f in fs)==1:
                a,b=edge;neighbors[a].append(b);neighbors[b].append(a)
        assert all(len(ns)==2 for ns in neighbors.values()),'Eye cut must form a closed rim'
        original=[v.co.copy() for v in body.data.vertices];curve={i:original[i].copy() for i in boundary}
        for _ in range(6):
            for strength in [.5,-.51]:
                updates={}
                for i in boundary:
                    delta=(sum((curve[j] for j in neighbors[i]),Vector())/2-curve[i])*strength
                    updates[i]=curve[i]+delta-n*delta.dot(n)
                curve.update(updates)
        # Both rims meet the same curved surface after upper/lower translation.
        def seam(q):return c.z+.004-.008*(1-min(1,((q-c).dot(t)/rx)**2))
        # Two independent lid influences solve vertical closure and forward
        # depth separately. The original open-eye surface no longer needs to
        # be pushed forward into a raised rim to meet a shared closing plane.
        advance=eye['advance'];travel=eye['travel']
        closed_depth=max((curve[i]-c).dot(n)+advance*abs(seam(curve[i])-curve[i].z)/travel for i in boundary)+.001
        for i in boundary:
            q=curve[i];total=(closed_depth-(q-c).dot(n))/advance
            assert abs((seam(q)-q.z)/travel)<=total+1e-6 and total<1,(cat,eye['name'],total)
        adjacency={v.index:[] for v in body.data.vertices}
        for edge in body.data.edges:
            a,b=edge.vertices;adjacency[a].append(b);adjacency[b].append(a)
        distances={i:0. for i in boundary};queue=[(0.,i) for i in boundary];heapq.heapify(queue)
        while queue:
            d,i=heapq.heappop(queue)
            if d>distances[i] or d>.045:continue
            for j in adjacency[i]:
                nd=d+(original[i]-original[j]).length
                if nd<distances.get(j,1e9):distances[j]=nd;heapq.heappush(queue,(nd,j))
        for i in boundary:body.data.vertices[i].co=curve[i]
        for i,d in distances.items():
            if d>.045 or any((original[i]-other['centre']).length<(original[i]-c).length for other in eyes):continue
            q=body.data.vertices[i].co;vertical=(seam(q)-q.z)/travel
            total=max(abs(vertical),(closed_depth-(q-c).dot(n))/advance)
            total=min(.98,total);vertical=max(-total,min(total,vertical))
            falloff=math.exp(-(d/.020)**2)
            upper=(total-vertical)*.5*falloff;lower=(total+vertical)*.5*falloff
            for group in [g.group for g in body.data.vertices[i].groups]:body.vertex_groups[group].remove([i])
            body.vertex_groups[eye['name']].add([i],upper,'REPLACE')
            body.vertex_groups[eye['lower']].add([i],lower,'REPLACE')
            body.vertex_groups['head'].add([i],1-upper-lower,'REPLACE')
        audit.append({'bone':eye['name'],'centre':list(c),'normal':list(n),'tangent':list(t),
                      'radius':[rx,rz],'travel':travel,'advance':advance,'closedDepth':closed_depth,'boundaryVertices':len(boundary),'fixedEyeFaces':len(selected)})
    bm=bmesh.new();bm.from_mesh(body.data);bm.faces.ensure_lookup_table()
    bmesh.ops.delete(bm,geom=[bm.faces[i] for i in removed],context='FACES');bm.to_mesh(body.data);bm.free()
    bpy.ops.object.select_all(action='DESELECT');body.select_set(True)
    for o in created:o.select_set(True)
    bpy.context.view_layer.objects.active=body;bpy.ops.object.join()
    body.data.use_auto_smooth=True;body.data.calc_normals_split()
    body.data.normals_split_custom_set([original_normals.get(tuple(body.data.vertices[l.vertex_index].co),l.normal).normalized() for l in body.data.loops])
    return eyes,audit

def pose_eyes(rig,eyes,amount):
    for eye in eyes:
        for name,dz in [(eye['name'],-eye.get('travel',.075)),(eye['lower'],eye.get('travel',.060))]:
            bone=rig.data.bones[name];p=rig.pose.bones[name]
            m=bone.matrix_local.inverted()@Matrix.Translation(Vector((0,0,dz*amount))+eye['n']*(eye.get('advance',.020)*amount))@bone.matrix_local
            p.location,p.rotation_quaternion,p.scale=m.decompose()

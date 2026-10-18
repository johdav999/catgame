"""Canonical skin preparation. Original Tripo files and Pip authority are read-only."""
import bpy, bmesh, json, math, sys, importlib.util
import numpy as np
from pathlib import Path
from mathutils import Vector, Matrix
from mathutils.kdtree import KDTree

ROOT=Path(__file__).resolve().parents[3]
JOB=ROOT/'artifacts/model-jobs/release1-cats'
AUTHORITY=ROOT/'artifacts/model-jobs/pip/2026-10-11-eye-rim/pip-blink.blend'
SOURCES={'button':'button','mabel':'pepper','pepper':'mabel'}
spec=importlib.util.spec_from_file_location('review',Path(__file__).with_name('inspect_sources.py'))
review=importlib.util.module_from_spec(spec);spec.loader.exec_module(review)

def clusters(points,k):
    centers=[points[0]]
    for _ in range(k-1):
        distances=np.min(np.sum((points[:,None]-np.array(centers)[None,:])**2,axis=2),axis=1)
        centers.append(points[np.argmax(distances)])
    centers=np.array(centers)
    for _ in range(40):
        assignment=np.argmin(np.sum((points[:,None]-centers[None,:])**2,axis=2),axis=1)
        centers=np.array([np.mean(points[assignment==i],axis=0) for i in range(k)])
    return centers,assignment

def smooth(a,b,x):
    t=max(0,min(1,(x-a)/(b-a)));return t*t*(3-2*t)

def material(body,cat,source):
    body.data.materials.clear()
    mat=bpy.data.materials.new(cat+' portable coat');mat.use_nodes=True
    nodes=mat.node_tree.nodes;links=mat.node_tree.links;nodes.clear()
    out=nodes.new('ShaderNodeOutputMaterial');pbr=nodes.new('ShaderNodeBsdfPrincipled')
    links.new(pbr.outputs['BSDF'],out.inputs['Surface'])
    pbr.inputs['Metallic'].default_value=0;pbr.inputs['Roughness'].default_value=.82
    texdir=JOB/cat/'textures';texdir.mkdir(exist_ok=True,parents=True)
    source_dir=next((ROOT/'assets'/source).glob('*.fbm'))
    images={}
    for kind in ['basecolor','roughness','normal']:
        file=next(source_dir.glob('*_'+kind+'.*'))
        im=bpy.data.images.load(str(file),check_existing=False)
        im.name=cat+'-'+kind+'-1024'
        im.colorspace_settings.name='sRGB' if kind=='basecolor' else 'Non-Color'
        im.scale(1024,1024);im.filepath_raw=str(texdir/(kind+'.png'));im.file_format='PNG';im.save();im.pack()
        n=nodes.new('ShaderNodeTexImage');n.image=im;images[kind]=im
        if kind=='basecolor':links.new(n.outputs['Color'],pbr.inputs['Base Color'])
        if kind=='roughness':links.new(n.outputs['Color'],pbr.inputs['Roughness'])
        if kind=='normal':
            nm=nodes.new('ShaderNodeNormalMap');nm.inputs['Strength'].default_value=.35
            links.new(n.outputs['Color'],nm.inputs['Color']);links.new(nm.outputs['Normal'],pbr.inputs['Normal'])
    body.data.materials.append(mat)
    return images

def fit(cat):
    source=SOURCES[cat];folder=JOB/cat;folder.mkdir(exist_ok=True,parents=True)
    # Intake belongs to source folder; final identity assignment is explicit below.
    bpy.ops.wm.open_mainfile(filepath=str(JOB/source/'intake/source-inspected.blend'))
    body=next(o for o in bpy.context.scene.objects if o.type=='MESH' and o.name!='Review ground')
    body.name=cat.title()+' skin'
    for o in list(bpy.context.scene.objects):
        if o!=body:bpy.data.objects.remove(o,do_unlink=True)
    points=np.array([v.co[:] for v in body.data.vertices])
    feet,_=clusters(points[points[:,2]<.065],4)
    front=feet[np.argsort(feet[:,1])[:2]].mean(axis=0)
    rear=feet[np.argsort(feet[:,1])[2:]].mean(axis=0)
    delta=rear-front;angle=math.atan2(delta[0],delta[1])
    rotation=Matrix.Rotation(angle,4,'Z')
    for v in body.data.vertices:v.co=rotation@v.co
    points=np.array([v.co[:] for v in body.data.vertices])
    feet,_=clusters(points[points[:,2]<.065],4)
    front=feet[np.argsort(feet[:,1])[:2]].mean(axis=0);rear=feet[np.argsort(feet[:,1])[2:]].mean(axis=0)
    center=(front+rear)/2
    sx=.29/(np.max(feet[:,0])-np.min(feet[:,0]));sy=.51/(rear[1]-front[1])
    for v in body.data.vertices:
        v.co.x=(v.co.x-center[0])*sx
        v.co.y=(v.co.y-center[1])*sy-.035
        # The Tripo heads sit over the shoulders; the frozen Pip head bind is
        # forward of them. Fit geometry to that bind rather than moving bones.
        h=smooth(.40,.65,v.co.z)*smooth(-.06,-.25,v.co.y)
        v.co.y+=h*({'button':-.11,'mabel':-.12,'pepper':-.07}[cat])
    images=material(body,cat,source)
    with bpy.data.libraries.load(str(AUTHORITY),link=False) as (src,dst):
        dst.objects=['PipRig','Pip repaired Tripo skin'];dst.actions=list(src.actions)
    for action in dst.actions:action.use_fake_user=True
    rig=bpy.data.objects['PipRig'];bpy.context.scene.collection.objects.link(rig)
    donor=bpy.data.objects['Pip repaired Tripo skin'];bpy.context.scene.collection.objects.link(donor)
    rig.animation_data_clear()
    for p in rig.pose.bones:p.matrix_basis.identity()
    # Transfer the accepted anatomical skin through local nearest-neighbour fields.
    # Eyelids are fitted separately: do not transfer Pip's facial opening weights.
    tree=KDTree(len(donor.data.vertices))
    for v in donor.data.vertices:tree.insert(v.co,v.index)
    tree.balance();body.vertex_groups.clear()
    for bone in rig.data.bones:body.vertex_groups.new(name=bone.name)
    for v in body.data.vertices:
        accum={}
        for co,index,distance in tree.find_n(v.co,4):
            fac=1/max(.0001,distance)**3
            for g in donor.data.vertices[index].groups:
                name=donor.vertex_groups[g.group].name
                if name.startswith('blink'):name='head'
                accum[name]=accum.get(name,0)+fac*g.weight
        # Keep face planes rigid while retaining soft neck and shoulder transitions.
        total=sum(accum.values());accum={n:w/total for n,w in accum.items()}
        head=smooth(.46,.60,v.co.z)*smooth(-.20,-.34,v.co.y)
        ear=smooth(.76,.90,v.co.z)*smooth(.085,.16,abs(v.co.x))
        accum={n:w*(1-head) for n,w in accum.items()}
        accum['head']=accum.get('head',0)+head*(1-ear)
        en='ear.L' if v.co.x<0 else 'ear.R';accum[en]=accum.get(en,0)+head*ear
        # Central chest tufts must never inherit a nearby foreleg's weights.
        # Nearest-surface transfer alone confuses a hanging bib with the paw.
        bib=smooth(.24,.36,v.co.z)*smooth(.17,.07,abs(v.co.x))*smooth(.02,-.18,v.co.y)*(1-smooth(.50,.63,v.co.z))
        accum={n:w*(1-bib) for n,w in accum.items()}
        accum['chest']=accum.get('chest',0)+bib
        best=sorted(accum.items(),key=lambda pair:pair[1],reverse=True)[:4];total=sum(w for n,w in best)
        for name,w in best:
            if w>1e-7:body.vertex_groups[name].add([v.index],w/total,'REPLACE')
    # Diffuse sharp nearest-surface weight boundaries through the connected
    # coat, retaining the four strongest normalized influences afterward.
    names=[g.name for g in body.vertex_groups]
    weights=np.zeros((len(body.data.vertices),len(names)))
    neighbors=[[] for _ in body.data.vertices]
    for edge in body.data.edges:
        a,b=edge.vertices;neighbors[a].append(b);neighbors[b].append(a)
    for v in body.data.vertices:
        for g in v.groups:weights[v.index,g.group]=g.weight
    for _ in range(8):
        weights=np.array([weights[i]*.5+weights[ns].mean(0)*.5 if ns else weights[i] for i,ns in enumerate(neighbors)])
    body.vertex_groups.clear()
    for n in names:body.vertex_groups.new(name=n)
    for i,row in enumerate(weights):
        best=np.argsort(row)[-4:];total=row[best].sum()
        for j in best:
            if row[j]>1e-7:body.vertex_groups[names[j]].add([i],float(row[j]/total),'REPLACE')
    bpy.data.objects.remove(donor,do_unlink=True)
    mod=body.modifiers.new('Canonical Paws skin','ARMATURE');mod.object=rig
    body.parent=rig;body.matrix_parent_inverse=Matrix.Identity(4)
    for p in body.data.polygons:p.use_smooth=True
    # Report topology and locate the two eye regions from source texture samples.
    bm=bmesh.new();bm.from_mesh(body.data);bm.verts.ensure_lookup_table()
    remaining=set(bm.verts);components=[]
    while remaining:
        seed=next(iter(remaining));remaining.remove(seed);component={seed};stack=[seed]
        while stack:
            for e in stack.pop().link_edges:
                for other in e.verts:
                    if other in remaining:remaining.remove(other);component.add(other);stack.append(other)
        components.append(component)
    component_report=[{'vertices':len(c),'center':list(sum((v.co for v in c),Vector())/len(c))} for c in sorted(components,key=len,reverse=True)]
    bm.free()
    tex=np.array(images['basecolor'].pixels[:]).reshape(1024,1024,4)
    dark=[]
    for loop in body.data.loops:
        v=body.data.vertices[loop.vertex_index];uv=body.data.uv_layers.active.data[loop.index].uv
        col=tex[int(uv.y%1*1023),int(uv.x%1*1023),:3]
        if .51<v.co.z<.86 and v.co.y<-.30 and max(col)<.13:dark.append(v.co[:])
    report={'cat':cat,'sourceFolder':source,'yawRadians':angle,'scaleXY':[sx,sy],
            'feetBefore':feet.tolist(),'components':component_report,'darkEyeCandidates':dark,
            'sourceAuthority':str(AUTHORITY.relative_to(ROOT)).replace('\\','/')}
    (folder/'fit-report.json').write_text(json.dumps(report,indent=2))
    scene=review.stage((0,0,.5));scene.render.fps=30;scene.render.fps_base=1
    review.render_views(scene,folder/'renders',prefix='fit-r3')
    rig.animation_data_create();rig.animation_data.action=bpy.data.actions['relaxed_idle']
    scene.frame_set(1);bpy.ops.file.pack_all()
    bpy.ops.wm.save_as_mainfile(filepath=str(folder/(cat+'-fit-r3.blend')))
    print('FITTED',cat,'source',source,'scale',sx,sy,'components',component_report[:6],flush=True)

if __name__=='__main__':
    for cat in sys.argv[sys.argv.index('--')+1:] if '--' in sys.argv else SOURCES:fit(cat)

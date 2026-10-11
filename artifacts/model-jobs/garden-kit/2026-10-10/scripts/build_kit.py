"""Paws garden kit. Blender 3.5+, run headless; retained procedural source.
-- --revision N [--no-render]. Source Z-up; glTF conversion alone yields Y-up.
"""
import bpy, math, json, sys, hashlib, random
from pathlib import Path
from mathutils import Vector
from math import sin, cos, pi
JOB=Path(__file__).resolve().parents[1]
ROOT=JOB.parents[3]
args=sys.argv[sys.argv.index('--')+1:] if '--' in sys.argv else []
REV=int(args[args.index('--revision')+1]) if '--revision' in args else 1
OUT=JOB/'exports'/f'r{REV}'; OUT.mkdir(parents=True,exist_ok=True)
MAPS=JOB/'maps'; MAPS.mkdir(exist_ok=True)
bpy.ops.object.select_all(action='SELECT'); bpy.ops.object.delete(use_global=False)
random.seed(47)
scene=bpy.context.scene; scene.unit_settings.system='METRIC'
scene.render.engine='CYCLES'; scene.cycles.samples=24
scene.world.color=(.3,.3,.3)
scene.view_settings.view_transform='Standard'; scene.view_settings.look='Medium High Contrast'
scene.view_settings.exposure=0; scene.view_settings.gamma=1
assets={}; current=[]; mats={}
def srgb(v): return v/12.92 if v<.04045 else ((v+.055)/1.055)**2.4
def color(hex): return tuple(srgb(int(hex[i:i+2],16)/255) for i in (0,2,4))+(1,)
atlas=bpy.data.images.load(str(JOB/'sources/material-atlas.png'))
atlas.pack()
pixels=list(atlas.pixels); aw,ah=atlas.size
families=['stone','terracotta','wood','cloth','wicker','sage_cloth']
rough=[.88,.88,.73,.91,.84,.91]
for k,name in enumerate(families):
    # Exact atlas tile extraction is a retained UV/material preparation step.
    w,h=aw//3,ah//2; ox=(k%3)*w; oy=(1-k//3)*h
    image=bpy.data.images.new(name+' base color',width=w,height=h)
    data=[]
    for y in range(h): data.extend(pixels[((oy+y)*aw+ox)*4:((oy+y)*aw+ox+w)*4])
    image.pixels=data; image.filepath_raw=str(MAPS/(name+'-basecolor.png')); image.file_format='PNG'; image.save(); image.pack()
    m=bpy.data.materials.new(name);m.use_nodes=True;nodes=m.node_tree.nodes;p=nodes.get('Principled BSDF')
    tex=nodes.new('ShaderNodeTexImage');tex.image=image;m.node_tree.links.new(tex.outputs['Color'],p.inputs['Base Color'])
    # Authored linear scalar roughness, independent of base-color luminance.
    rm=bpy.data.images.new(name+' roughness',width=8,height=8);rm.colorspace_settings.name='Non-Color'
    rm.pixels=[rough[k],rough[k],rough[k],1]*64;rm.filepath_raw=str(MAPS/(name+'-roughness.png'));rm.file_format='PNG';rm.save();rm.pack()
    rt=nodes.new('ShaderNodeTexImage');rt.image=rm;m.node_tree.links.new(rt.outputs['Color'],p.inputs['Roughness'])
    # Neutral tangent normal is deliberate: modeled relief, no false color-derived dents.
    nm=bpy.data.images.get('neutral tangent normal')
    if not nm:
        nm=bpy.data.images.new('neutral tangent normal',width=4,height=4);nm.colorspace_settings.name='Non-Color'
        nm.pixels=[.5,.5,1,1]*16;nm.filepath_raw=str(MAPS/'neutral-normal.png');nm.file_format='PNG';nm.save();nm.pack()
    nt=nodes.new('ShaderNodeTexImage');nt.image=nm;n=nodes.new('ShaderNodeNormalMap');m.node_tree.links.new(nt.outputs['Color'],n.inputs['Color']);m.node_tree.links.new(n.outputs['Normal'],p.inputs['Normal'])
    mats[name]=m
for name,hx,r in [('cream','FFF8E9',.85),('coral','D86D55',.72),('teal','28665D',.72),('sky','BFD9DD',.42),('water','71AFB8',.36),('moss','627A57',.92),('sage','91A783',.9),('soil','64513D',1),('ink','253E37',.8),('honey','EBC66B',.8)]:
    m=bpy.data.materials.new(name);m.diffuse_color=color(hx);m.use_nodes=True
    p=m.node_tree.nodes.get('Principled BSDF');p.inputs['Base Color'].default_value=color(hx);p.inputs['Roughness'].default_value=r;mats[name]=m
def finish(o,name,mat,smooth=True):
    o.name=name;o.data.materials.append(mats[mat]);current.append(o)
    if smooth:
        for p in o.data.polygons:p.use_smooth=True
    return o
def bevel(o,width=.02):
    mod=o.modifiers.new('Soft crafted edge','BEVEL');mod.width=width;mod.segments=3
    o.modifiers.new('Weighted corner normals','WEIGHTED_NORMAL')
    return o
def box(name,loc,size,mat='wood',edge=.02):
    bpy.ops.mesh.primitive_cube_add(size=1,location=loc);o=bpy.context.object;o.dimensions=size
    bpy.ops.object.transform_apply(location=False,rotation=False,scale=True)
    finish(o,name,mat,False)
    if edge:bevel(o,edge)
    return o
def ellipsoid(name,loc,scale,mat,segments=24,rings=12):
    bpy.ops.mesh.primitive_uv_sphere_add(segments=segments,ring_count=rings,radius=1,location=loc)
    o=bpy.context.object;o.scale=scale;bpy.ops.object.transform_apply(location=False,rotation=False,scale=True)
    return finish(o,name,mat)
def mesh(name,verts,faces,mat,smooth=True):
    d=bpy.data.meshes.new(name);d.from_pydata(verts,[],faces);d.update();o=bpy.data.objects.new(name,d);scene.collection.objects.link(o);return finish(o,name,mat,smooth)
def lathe(name,profile,mat,n=48):
    v=[(r*cos(i*2*pi/n),r*sin(i*2*pi/n),z) for r,z in profile for i in range(n)]
    f=[]
    for j in range(len(profile)-1):
        for i in range(n):a=j*n+i;b=j*n+(i+1)%n;f.append((a,b,b+n,a+n))
    return mesh(name,v,f,mat)
def tube(name,points,r,mat,sides=8):
    v=[]
    for i,point in enumerate(points):
        p=Vector(point);t=Vector(points[min(i+1,len(points)-1)])-Vector(points[max(0,i-1)])
        t.normalize();a=t.cross(Vector((0,0,1)))
        if a.length<.01:a=t.cross(Vector((0,1,0)))
        a.normalize();b=t.cross(a)
        for j in range(sides):v.append(p+r*(a*cos(j*2*pi/sides)+b*sin(j*2*pi/sides)))
    f=[]
    for i in range(len(points)-1):
        for j in range(sides):f.append((i*sides+j,i*sides+(j+1)%sides,(i+1)*sides+(j+1)%sides,(i+1)*sides+j))
    f.extend([tuple(reversed(range(sides))),tuple((len(points)-1)*sides+j for j in range(sides))]);return mesh(name,v,f,mat)
def extrude(name,points,z0,z1,mat):
    n=len(points);v=[(x,y,z) for z in (z0,z1) for x,y in points]
    f=[tuple(reversed(range(n))),tuple(range(n,2*n))]+[(i,(i+1)%n,(i+1)%n+n,i+n) for i in range(n)]
    return mesh(name,v,f,mat,False)
def outline(kind,scale=1):
    if kind=='ball':return [(scale*.12*cos(i*2*pi/48),scale*.12*sin(i*2*pi/48)) for i in range(48)]
    if kind=='mouse':
        return [(scale*x,scale*y) for x,y in [(0,-.205),(.05,-.17),(.12,-.12),(.12,-.065),(.095,-.03),(.10,.065),(.10,.135),(.10,.23),(.045,.25),(-.03,.185),(-.05,.145),(-.095,.07),(-.095,-.03),(-.12,-.065),(-.12,-.12),(-.05,-.17)]]
    return [(scale*x,scale*y) for x,y in [(-.24,0),(-.15,-.105),(-.01,-.125),(.10,-.06),(.22,-.135),(.22,.135),(.10,.06),(-.01,.125),(-.15,.105)]]
def register(name,footprint,anchors=None):
    global current
    assets[name]={'objects':current,'footprint':footprint,'anchors':anchors or {}}
    current=[]
def planting():
    for i in range(9):
        a=i*2.4;rr=.09*(i%3);x=rr*cos(a);y=rr*sin(a);z=.06+.045*(i%4)
        tube('Plant stem',[(x,y,0),(x*1.6,y*1.6,z+.17)],.012,'moss')
        for side in (-1,1):
            o=ellipsoid('Broad opaque leaf',(x+side*.085,y,z+.08),(.12,.052,.027),'sage' if i%2 else 'moss',12,8)
            o.rotation_euler=(0,side*.5,a)
    for x,y in [(-.14,-.07),(.14,.04)]:
        for i in range(5):ellipsoid('Cream petal',(x+.037*cos(i*2*pi/5),y+.037*sin(i*2*pi/5),.26),(.032,.022,.014),'cream',12,6)
        ellipsoid('Flower centre',(x,y,.27),(.022,.022,.016),'honey',12,6)
# Floor modules: top is ground plane, so props sit at zero.
for ix in range(2):
    for iy in range(2):box('Sandstone slab',((ix-.5)*.5,(iy-.5)*.5,-.06),(.489,.489,.12),'stone',.022)
register('paving',[1,1])
box('Rounded courtyard edge',(0,0,.04),(1,.22,.20),'stone',.045);register('edge',[1,.22])
for x in (-.48,.48):box('Fence post',(x,0,.3),(.075,.105,.6),'wood',.025)
for z in (.18,.4):box('Fence rail',(0,.02,z),(.96,.075,.085),'wood',.015)
for i in range(7):box('Rounded picket',((i-3)*.133,-.025,.28),(.086,.06,.49),'wood',.03)
register('fence',[1,.14]);planting();register('planting',[.72,.65])
# Bench with real gaps and chunky rounded arms.
for x in (-.56,.56):
    for y in (-.23,.23):box('Bench leg',(x,y,.23),(.09,.09,.46),'teal')
    box('Arm support',(x,.18,.63),(.075,.08,.45),'teal')
    box('Bench arm',(x,0,.61),(.12,.62,.07),'wood')
for j in range(4):box('Seat slat',(0,(j-1.5)*.135,.45),(1.27,.12,.07),'wood')
for z in (.62,.78):box('Back slat',(0,.25,z),(1.22,.07,.13),'teal')
register('bench',[1.4,.66],{'perch':[0,.49,0]})
for variant in ('round','fluted'):
    profile=[(0,0),(.20,0),(.215,.035),(.26,.39),(.29,.40),(.29,.445),(.25,.46),(.235,.43),(.23,.38),(.185,.055),(0,.055)]
    pot=lathe('Thrown terracotta pot',profile,'terracotta')
    if variant=='fluted':
        for v in pot.data.vertices:
            a=math.atan2(v.co.y,v.co.x);s=1+.022*cos(12*a);v.co.x*=s;v.co.y*=s
    lathe('Soil inside rim',[(0,.36),(.224,.36)],'soil',32)
    before=len(current);planting()
    for o in current[before:]:o.location.z+=.37
    register('pot_'+variant,[.65,.65],{'floor':[0,0,0],'slide_axis':[1,0,0]})
# One common U-section swept along exact centreline ports.
section=[(-.19,0),(.19,0),(.19,.185),(.155,.2),(.13,.177),(.13,.078),(.11,.06),(-.11,.06),(-.13,.078),(-.13,.177),(-.155,.2),(-.19,.185)]
for curved in (False,True):
    count=25 if curved else 2;v=[]
    for i in range(count):
        t=i/(count-1)
        if curved:a=-pi/2+t*pi/2;x=-.5+.5*cos(a);y=.5+.5*sin(a);nx=cos(a);ny=sin(a)
        else:x=t-.5;y=0;nx=0;ny=1
        for side,z in section:v.append((x+side*nx,y+side*ny,z))
    ns=len(section);f=[]
    for i in range(count-1):
        for j in range(ns):f.append((i*ns+j,i*ns+(j+1)%ns,(i+1)*ns+(j+1)%ns,(i+1)*ns+j))
    f.extend([tuple(reversed(range(ns))),tuple((count-1)*ns+j for j in range(ns))])
    o=mesh('Open wooden trough',v,f,'wood');bevel(o,.008)
    register('channel_curve' if curved else 'channel_straight',[1,1],{'port_a':[-.5,.06,0],'port_b':[0,.06,-.5] if curved else [.5,.06,0],'clear_width':.26,'ball_radius':.105})
ball=ellipsoid('Coral ball',(0,0,.105),(.105,.105,.105),'coral',32,16)
# A cream equatorial band consists of the same sphere faces: no overlapping shell.
ball.data.materials.append(mats['cream'])
for p in ball.data.polygons:
    if abs(p.center.z)<.028:p.material_index=1
register('ball',[.21,.21],{'centre':[0,.105,0]})
ellipsoid('Mouse body',(0,-.005,.065),(.087,.14,.065),'cloth')
ellipsoid('Mouse nose',(0,-.135,.07),(.033,.045,.033),'coral',16,8)
for x in (-.067,.067):
    ellipsoid('Mouse ear',(x,-.075,.13),(.048,.024,.045),'cloth',16,8)
    ellipsoid('Ear inset',(x,-.096,.13),(.030,.006,.026),'coral',12,8)
    ellipsoid('Stitched eye',(x*.48,-.119,.102),(.009,.006,.01),'ink',12,6)
tube('Mouse tail',[(0,.115,.025),(.01,.17,.02),(.05,.2,.025),(.07,.22,.025)],.012,'coral')
register('mouse',[.25,.39])
fish=ellipsoid('Cloth fish body',(-.06,0,.055),(.15,.09,.055),'sky')
tail=extrude('Distinct fish tail',[(.035,0),(.21,-.12),(.19,0),(.21,.12)],.015,.075,'teal');bevel(tail,.019)
for y in (-.08,.08):ellipsoid('Fish stitched eye',(-.145,y,.073),(.013,.008,.013),'ink',12,6)
tube('Fish top seam',[(-.20,0,.056),(-.14,0,.102),(-.03,0,.108),(.07,0,.055)],.006,'cream')
register('fish',[.46,.27])
# Real three-hole tray insert, with individual low floors and clearance.
box('Wicker basket base',(0,0,.027),(1.72,.69,.054),'wicker',.06)
insert=box('Cream fitted insert',(0,0,.105),(1.60,.59,.13),'cloth',0)
socket_centres=[-.53,0,.52]
for kind,x in zip(('ball','mouse','fish'),socket_centres):
    # Mouse opening encloses its ears AND curved tail. No invisible blocker above floor.
    pts=outline(kind,1.15 if kind=='mouse' else 1.07)
    cutter=extrude('Socket cutter',[(u+x,v) for u,v in pts],.058,.3,'cream')
    bpy.context.view_layer.objects.active=insert
    mod=insert.modifiers.new(kind+' clearance','BOOLEAN');mod.operation='DIFFERENCE';mod.solver='EXACT';mod.object=cutter
    bpy.ops.object.modifier_apply(modifier=mod.name);current.remove(cutter);bpy.data.objects.remove(cutter,do_unlink=True)
    tube(kind+' socket piping',[(u+x,v,.172) for u,v in pts]+[(pts[0][0]+x,pts[0][1],.172)],.009,'teal')
bevel(insert,.003)
for y in (-.335,.335):
    box('Basket side',(0,y,.125),(1.7,.04,.2),'wicker',.019)
for x in (-.84,.84):box('Basket end',(x,0,.125),(.04,.68,.2),'wicker',.019)
# Rounded rim woven bands; structural relief is authored geometry.
for z in (.08,.14,.225):
    pts=[(-.78,-.335,z),(.78,-.335,z),(.835,-.28,z),(.835,.28,z),(.78,.335,z),(-.78,.335,z),(-.835,.28,z),(-.835,-.28,z),(-.78,-.335,z)]
    tube('Basket woven rim',pts,.017,'wicker')
register('basket',[1.78,.74],{k:[x,.059,0] for k,x in zip(('ball_socket','mouse_socket','fish_socket'),socket_centres)})
def cushion(name,w,d,h,mat,tuft):
    v=[];f=[];n=48;rings=20
    def sp(x,e):return math.copysign(abs(x)**e,x)
    for j in range(rings+1):
        lat=-pi/2+j*pi/rings
        for i in range(n):
            a=i*2*pi/n;x=w/2*sp(cos(lat),.5)*sp(cos(a),.48);y=d/2*sp(cos(lat),.5)*sp(sin(a),.48);z=h/2+h/2*sp(sin(lat),.65)
            if tuft and z>h/2:z-=.025*sum(math.exp(-((x-u)**2+(y-vv)**2)/.008) for u in (-w*.22,w*.22) for vv in (-d*.22,d*.22))
            v.append((x,y,z))
    for j in range(rings):
        for i in range(n):f.append((j*n+i,j*n+(i+1)%n,(j+1)*n+(i+1)%n,(j+1)*n+i))
    mesh(name,v,f,mat)
    pts=[(w/2*sp(cos(i*2*pi/96),.48),d/2*sp(sin(i*2*pi/96),.48),h*.5) for i in range(97)]
    tube('Tailored cushion piping',pts,.009,'cream' if tuft else 'coral')
    if tuft:
        for x in (-w*.22,w*.22):
            for y in (-d*.22,d*.22):ellipsoid('Covered tuft button',(x,y,h-.025),(.025,.025,.009),'teal',12,6)
cushion('Blocking striped bolster',.72,.49,.22,'cloth',False)
for x in (-.23,.23):tube('Coral blocking-cushion seam',[(x,-.18,.18),(x,-.1,.218),(x,0,.222),(x,.1,.218),(x,.18,.18)],.012,'coral')
register('blocking_cushion',[.76,.53])
cushion('Pip tufted garden cushion',.90,1.14,.18,'sage_cloth',True)
register('cushion',[.94,1.18],{'rest_root':[0,.16,0],'approach':[0,0,1.1]})
# Fixed switch base and moving plate are separate exported actors.
box('Switch plinth',(0,0,.035),(.48,.48,.07),'wood',.035)
register('switch_base',[.5,.5],{'plate_unpressed':[0,.09,0],'plate_pressed':[0,.055,0],'ball_root_pressed':[0,.105,0]})
box('Pressure plate',(0,0,.025),(.39,.39,.05),'teal',.03)
lathe('Ball seat',[(.118,.05),(.132,.065),(.148,.059),(.148,.048)],'cream',32)
register('switch_plate',[.4,.4],{'ball_root':[0,.05,0]})
for x in (-.61,.61):
    box('Gate post',(x,0,.54),(.12,.15,1.08),'wood',.025)
    ellipsoid('Post cap',(x,0,1.08),(.085,.095,.05),'teal',16,8)
register('gate_frame',[1.38,.24],{'hinge':[-.54,.08,0],'passage_width':1.10,'open_angle_degrees':100})
# Leaf local origin is its hinge; never the visual centre.
for x in (.08,.26,.44,.62,.80,.98):box('Gate slat',(x,0,.45),(.125,.065,.88),'teal',.03)
for z in (.19,.66):box('Gate crossbar',(.53,.05,z),(1.06,.07,.09),'wood',.012)
brace=box('Gate diagonal',(.53,.08,.43),(1.02,.05,.07),'wood',.012);brace.rotation_euler.y=-.43
for z in (.15,.7):box('Hinge strap',(.09,-.055,z),(.24,.035,.045),'ink',.01)
register('gate_leaf',[1.06,.18],{'hinge':[0,0,0]})
lathe('Ceramic drinking bowl',[(0,0),(.195,0),(.20,.025),(.16,.065),(.12,.21),(.20,.265),(.245,.29),(.27,.38),(.27,.4),(.245,.408),(.225,.39),(.20,.30),(0,.30)],'sky')
lathe('Cream bowl rim',[(.239,.39),(.245,.408),(.26,.406),(.271,.4),(.267,.386)],'cream')
register('bowl',[.55,.55],{'water':[0,.383,0],'drink_root':[0,0,-.61327],'approach':[0,0,-1.3]})
lathe('Flat opaque water surface',[(0,0),(.220,0)],'water',48)
register('bowl_water',[.44,.44])

def uv_and_normals(objects):
    for o in objects:
        bpy.ops.object.select_all(action='DESELECT');o.select_set(True);bpy.context.view_layer.objects.active=o
        bpy.ops.object.mode_set(mode='EDIT');bpy.ops.mesh.select_all(action='SELECT');bpy.ops.mesh.remove_doubles(threshold=.000001);bpy.ops.mesh.dissolve_degenerate(threshold=.0000001);bpy.ops.mesh.normals_make_consistent(inside=False)
        bpy.ops.uv.smart_project(angle_limit=1.15,island_margin=.015);bpy.ops.object.mode_set(mode='OBJECT')
        if REV>=2 and o.data.materials[0].name in ('wood','wicker'):
            # Follow each member's long axis rather than arbitrary packed islands.
            dims=o.dimensions;long=max(range(3),key=lambda i:dims[i]);uv=o.data.uv_layers.active
            for p in o.data.polygons:
                normal=max(range(3),key=lambda i:abs(p.normal[i]));axes=[i for i in range(3) if i!=normal]
                if long in axes:axes.remove(long);axes.insert(0,long)
                for loop in p.loop_indices:
                    co=o.data.vertices[o.data.loops[loop].vertex_index].co
                    uv.data[loop].uv=(co[axes[0]]*.8+.5,co[axes[1]]*.8+.5)
            if o.name.startswith('Open wooden trough'):
                for loop in o.data.loops:
                    co=o.data.vertices[loop.vertex_index].co
                    if len(o.data.vertices)>50:
                        angle=math.atan2(co.y-.5,co.x+.5);along=(angle+pi/2)/pi;across=math.hypot(co.x+.5,co.y-.5)-.5
                    else:along=co.x;across=co.y
                    uv.data[loop.index].uv=(along+.5,across+co.z*.3+.5)
def bounds(objects):
    pts=[];deps=bpy.context.evaluated_depsgraph_get()
    for o in objects:
        evaluated=o.evaluated_get(deps);me=evaluated.to_mesh();pts.extend(o.matrix_world@v.co for v in me.vertices);evaluated.to_mesh_clear()
    lo=[min(p[i] for p in pts) for i in range(3)];hi=[max(p[i] for p in pts) for i in range(3)]
    return {'min':[lo[0],lo[2],-hi[1]],'max':[hi[0],hi[2],-lo[1]]}
manifest={'schema':1,'id':'garden/kit@1','revision':REV,'units':'metres','axes':'Y up, +Z forward; Blender Z-up -Y forward','source':'garden-kit.blend','assets':[],'materials':families,'assemblies':{
    'gate':[{'asset':'gate_frame','position':[0,0,0]},{'asset':'gate_leaf','position':[-.54,.08,0],'rotation_y_degrees':0}],
    'switch':[{'asset':'switch_base','position':[0,0,0]},{'asset':'switch_plate','position':[0,.09,0]}],
    'bowl':[{'asset':'bowl','position':[0,0,0]},{'asset':'bowl_water','position':[0,.383,0]}]}}
for name,item in assets.items():
    uv_and_normals(item['objects']);bpy.context.view_layer.update()
    bpy.ops.object.select_all(action='DESELECT')
    # One exported mesh per asset with shared material slots: preserve editable
    # construction pieces in the master without paying one runtime draw per slat.
    deps=bpy.context.evaluated_depsgraph_get();copies=[]
    for o in item['objects']:
        data=bpy.data.meshes.new_from_object(o.evaluated_get(deps),depsgraph=deps)
        copy=bpy.data.objects.new(name+' export',data);scene.collection.objects.link(copy);copy.matrix_world=o.matrix_world.copy();copy.select_set(True);copies.append(copy)
    bpy.context.view_layer.objects.active=copies[0];bpy.ops.object.join();export=bpy.context.object;export.name=name
    path=OUT/(name+'.glb')
    bpy.ops.export_scene.gltf(filepath=str(path),export_format='GLB',use_selection=True,export_apply=True,export_yup=True,export_animations=False,export_extras=False)
    bpy.data.objects.remove(export,do_unlink=True)
    deps=bpy.context.evaluated_depsgraph_get();tri=0
    for o in item['objects']:
        me=o.evaluated_get(deps).to_mesh();me.calc_loop_triangles();tri+=len(me.loop_triangles);o.evaluated_get(deps).to_mesh_clear()
    manifest['assets'].append({'id':'garden/'+name+'@1','file':name+'.glb','sha256':hashlib.sha256(path.read_bytes()).hexdigest(),'bytes':path.stat().st_size,'triangles':tri,'bounds':bounds(item['objects']),'footprint':item['footprint'],'pivot':'hinge' if name=='gate_leaf' else 'floor/contact centre','anchors':item['anchors'],'materials':sorted(set(m.name for o in item['objects'] for m in o.data.materials))})
(OUT/'asset-manifest.json').write_text(json.dumps(manifest,indent=2))
# Stable review arrangement, maintained identically in the Paws preview manifest.
layout=[('paving',0,0,0),('edge',-2,2.6,0),('fence',-2,2,0),('planting',-2.4,1.2,0),('bench',0,2,0),('pot_round',-2,0,0),('pot_fluted',-2,-1,0),('channel_straight',-.5,0,0),('channel_curve',.5,0,0),('ball',-.72,0,.06),('basket',0,-1.3,0),('mouse',-.1,-2,.0),('fish',.6,-2,0),('blocking_cushion',-1.5,-2,0),('cushion',1.7,-1.2,0),('switch_base',1.6,.2,0),('switch_plate',1.6,.2,.09),('gate_frame',1.6,1.6,0),('gate_leaf',1.06,1.6,.08),('bowl',2.45,-.25,0),('bowl_water',2.45,-.25,.383)]
manifest['review_instances']=[{'asset':name,'position':[x,z,-y]} for name,x,y,z in layout]
(OUT/'asset-manifest.json').write_text(json.dumps(manifest,indent=2))
for name,x,y,z in layout:
    for o in assets[name]['objects']:o.location+=Vector((x,y,z))
# Low-contrast review surface independent from delivered paving.
current=[]
for x in range(-3,3):
    for y in range(-3,3):
        box('Review paving',(x+.5,y+.5,-.067),(.988,.988,.12),'stone',.025)
for o in current:uv_and_normals([o])
def camera(name,loc,target,scale):
    bpy.ops.object.camera_add(location=loc);c=bpy.context.object;c.name=name;c.rotation_euler=(Vector(target)-c.location).to_track_quat('-Z','Y').to_euler();c.data.type='ORTHO';c.data.ortho_scale=scale;return c
c=camera('Review three-quarter',(7,-10,10),(0,0,.2),7.3);scene.camera=c
for loc,power,size in [((-3,-4,8),850,5),((4,2,6),650,5)]:
    bpy.ops.object.light_add(type='AREA',location=loc);l=bpy.context.object;l.data.energy=power;l.data.shape='DISK';l.data.size=size;l.rotation_euler=(Vector((0,0,0))-l.location).to_track_quat('-Z','Y').to_euler()
bpy.ops.wm.save_as_mainfile(filepath=str(JOB/f'garden-kit-r{REV}.blend'))
if '--no-render' not in args:
    scene.render.resolution_x=1400;scene.render.resolution_y=1100;scene.render.resolution_percentage=100
    scene.render.filepath=str(JOB/'renders'/f'r{REV}-kit.png');bpy.ops.render.render(write_still=True)
    c.location=(0,-.5,10);c.rotation_euler=(Vector((0,-.5,0))-c.location).to_track_quat('-Z','Y').to_euler();c.data.ortho_scale=6.5
    scene.render.filepath=str(JOB/'renders'/f'r{REV}-top.png');bpy.ops.render.render(write_still=True)
print('KIT_COMPLETE',len(assets),'assets',sum(a['triangles'] for a in manifest['assets']),'triangles')

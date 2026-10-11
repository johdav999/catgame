"""Pip canonical authoring source. Blender 3.5+, metres, front -Y, up Z.
Run with -- --revision 1|2|3. Retains editable union construction and actions.
"""
import bpy, math, json, sys, argparse, hashlib
from pathlib import Path
from mathutils import Vector, Quaternion, Matrix
import numpy as np

JOB = Path(__file__).resolve().parents[1]
args = argparse.ArgumentParser()
args.add_argument('--revision', type=int, default=1)
args.add_argument('--final', action='store_true')
opt = args.parse_args(sys.argv[sys.argv.index('--')+1:] if '--' in sys.argv else [])
REV = opt.revision
bpy.ops.object.select_all(action='SELECT'); bpy.ops.object.delete(use_global=False)
for a in list(bpy.data.actions): bpy.data.actions.remove(a)
scene = bpy.context.scene
scene.unit_settings.system='METRIC'; scene.unit_settings.scale_length=1
scene.render.engine='CYCLES'; scene.cycles.samples=32
scene.render.resolution_x=960; scene.render.resolution_y=960; scene.render.resolution_percentage=100
scene.view_settings.view_transform='Standard'; scene.view_settings.look='Medium High Contrast'
scene.view_settings.exposure=0; scene.view_settings.gamma=1
scene.world.color=(.34,.34,.34)
scene.render.image_settings.file_format='PNG'; scene.render.film_transparent=False
scene.render.fps=30
geo=bpy.data.collections.new('Pip geometry'); scene.collection.children.link(geo)
construction=bpy.data.collections.new('Editable sculpt construction'); scene.collection.children.link(construction)
construction.hide_render=True; construction.hide_viewport=True

def activate(o):
    bpy.ops.object.select_all(action='DESELECT'); o.select_set(True); bpy.context.view_layer.objects.active=o
def move_collection(o,c):
    for old in list(o.users_collection): old.objects.unlink(o)
    c.objects.link(o)
def ellipsoid(name,loc,scale,collection=geo):
    bpy.ops.mesh.primitive_uv_sphere_add(segments=24,ring_count=16,location=loc)
    o=bpy.context.object; o.name=name; o.scale=scale
    bpy.ops.object.transform_apply(location=False,rotation=False,scale=True)
    move_collection(o,collection)
    for p in o.data.polygons: p.use_smooth=True
    return o
parts=[]
def mass(name,loc,scale):
    o=ellipsoid(name,loc,scale); parts.append(o); return o
mass('Ribcage',(0,.015,.415),(.187,.335,.218))
mass('Haunch',(0,.255,.39),(.197,.185,.224))
mass('Shoulders',(0,-.245,.45),(.176,.16,.205))
mass('Neck',(0,-.34,.53),(.15,.14,.20))
mass('Head',(0,-.445,.68),(.218,.174,.192))
for s in [-1,1]:
    mass('Cheek',(.155*s,-.47,.615),(.091 if REV>=2 else .101,.119,.109))
    mass('Muzzle',(.063*s,-.596,.619),(.083,.049,.06))
mass('Chin',(0,-.573,.568),(.104,.055,.04))
for side in [-1,1]:
    for front,y in [(True,-.245),(False,.26)]:
        x=side*(.137 if front else .155)
        mass('Upper limb',(x,y,.292),(.074,.087,.156))
        mass('Lower limb',(x,y-.023,.135),(.053,.062,.116))
        mass('Broad paw',(x,y-.065,.059),(.076,.102,.056))
        if REV>=2:
            for k in [-1,0,1]: mass('Soft toe',(x+k*.036,y-.125,.045),(.025,.03,.027))
# A continuous thick hook tail. Joined by remeshing, not separate beads.
tailpoints=[(0,.355,.46),(.01,.455,.55),(.025,.495,.68),(.035,.515,.82),(.04,.51,.95),(.04,.465,1.035),(.04,.375,1.055),(.04,.316,1.025)]
for i in range(len(tailpoints)-1):
    a,b=Vector(tailpoints[i]),Vector(tailpoints[i+1])
    for j in range(5):
        p=a.lerp(b,j/4); t=(i+j/4)/(len(tailpoints)-1)
        r=.051*(1-.48*t) if REV==1 else .061*(1-.52*t)
        mass('Tail sculpt',p,(r,r,r))
# Subdivision ears with rounded triangular outline and a broad attached base.
for s in [-1,1]:
    verts=[]; faces=[]
    rings=[(.77,.102,.065,.132),(.835,.077,.045,.155),(.918,.039,.028,.18),(.97,.006,.008,.187)]
    for z,rx,ry,cx in rings:
        for i in range(16):
            t=2*math.pi*i/16; verts.append((s*cx+rx*math.cos(t),-.421+ry*math.sin(t),z))
    for j in range(3):
        for i in range(16): faces.append((j*16+i,j*16+(i+1)%16,(j+1)*16+(i+1)%16,(j+1)*16+i))
    faces += [tuple(reversed(range(16))),tuple(range(48,64))]
    m=bpy.data.meshes.new('Ear volume'); m.from_pydata(verts,[],faces); m.update()
    o=bpy.data.objects.new('Ear sculpt',m); geo.objects.link(o); parts.append(o)
# Retain each editable source volume in a hidden collection.
for o in parts:
    copy=o.copy(); copy.data=o.data.copy(); construction.objects.link(copy)
bpy.ops.object.select_all(action='DESELECT')
for o in parts:o.select_set(True)
bpy.context.view_layer.objects.active=parts[0]; bpy.ops.object.join()
body=bpy.context.object; body.name='Pip continuous coat'
bpy.ops.object.transform_apply(location=True,rotation=True,scale=True)
rem=body.modifiers.new('Union sculpt volumes','REMESH'); rem.mode='VOXEL'; rem.voxel_size=.0065
bpy.ops.object.modifier_apply(modifier=rem.name)
smooth=body.modifiers.new('Soften sculpt joins','SMOOTH'); smooth.factor=1.1; smooth.iterations=5
bpy.ops.object.modifier_apply(modifier=smooth.name)
dec=body.modifiers.new('Runtime topology','DECIMATE'); dec.ratio=min(1,(11500 if REV>=5 else 26400)/max(1,sum(len(p.vertices)-2 for p in body.data.polygons)))
bpy.ops.object.modifier_apply(modifier=dec.name)
for p in body.data.polygons:p.use_smooth=True

def material(name,color,rough=.82):
    m=bpy.data.materials.new(name); m.use_nodes=True
    p=m.node_tree.nodes.get('Principled BSDF'); p.inputs['Base Color'].default_value=(*color,1)
    p.inputs['Roughness'].default_value=rough
    return m
coat=material('Pip baked coat',(1,.5,.16))
body.data.materials.append(coat)
src=bpy.data.images.load(str(JOB/'sources/ginger-color-imagegen.png')); src.pack()
srcarr=np.array(src.pixels[:],dtype=np.float32).reshape(src.size[1],src.size[0],4)
attr=body.data.color_attributes.new(name='CoatColor',type='FLOAT_COLOR',domain='CORNER')
def smoothstep(a,b,x):
    t=max(0,min(1,(x-a)/(b-a))); return t*t*(3-2*t)
def coat_color(p):
    x,y,z=p; ax=abs(x)
    pix=srcarr[int((z*2.1%1)*(src.size[1]-1)),int(((y+x*.25)*2.1%1)*(src.size[0]-1)),:3]
    if REV>=2:pix=np.array([.64,.335,.115])*.9+pix*.1
    # Input samples are linear, deliberate markings are also linear authoring values.
    base=pix*np.array([1.05,.94,.83]); base=np.maximum(base,[.25,.105,.025])
    stripe=0
    if y>-.28 and z>.23:
        wave=math.sin(y*42+z*6+math.sin(z*18)*.4)
        stripe=smoothstep(.55,.84,wave)*smoothstep(.025,.11,ax)
    if y<-.34 and z>.66:
        # Symmetric forehead M and cheek ticks, deliberately away from muzzle/eyes.
        wave=math.sin(x*76+abs(z-.80)*32)
        stripe=max(stripe,smoothstep(.45,.8,wave)*smoothstep(.68,.77,z))
    if z<.29:
        stripe=max(stripe,smoothstep(.55,.85,math.sin(z*70+y*3))*.6)
    if y>.36 and z>.52:
        stripe=smoothstep(.35,.75,math.sin(z*43+y*5))
    base*=1-stripe*(.43 if REV<3 else .35)
    # Cream bib follows the front of the throat, muzzle and toes, never a UV rectangle.
    muzzle=smoothstep(-.556,-.592,y)*(1-smoothstep(.636,.661,z))
    bib=(1-smoothstep(.07,.13,ax))*smoothstep(-.33,-.41,y)*(1-smoothstep(.54,.58,z))*smoothstep(.20,.29,z)
    toes=(1-smoothstep(.067,.095,z))*.8
    cream=max(muzzle,bib,toes)
    return (*((1-cream)*base+cream*np.array([.93,.835,.65])),1)
for loop in body.data.loops:attr.data[loop.index].color=coat_color(body.data.vertices[loop.vertex_index].co)
nodes=coat.node_tree.nodes; links=coat.node_tree.links
color=nodes.new('ShaderNodeVertexColor'); color.layer_name='CoatColor'
links.new(color.outputs['Color'],nodes.get('Principled BSDF').inputs['Base Color'])

# Face details are attached meshes sharing a single atlas material after baking.
details=[]
def feature(name,loc,scale,col,group='head'):
    o=ellipsoid(name,loc,scale)
    if REV>=4:
        d=o.modifiers.new('Small feature budget','DECIMATE');d.ratio=.22;activate(o);bpy.ops.object.modifier_apply(modifier=d.name)
    o.data.materials.append(material(name,col)); details.append((o,group)); return o
eye_scale=.050 if REV==1 else .043
for s,label in [(-1,'L'),(1,'R')]:
    x=s*.107; y=-.587; z=.703
    feature('Eye surround '+label,(x,y+.004,z),(eye_scale*1.13,.018,eye_scale*1.12),(.17,.064,.023),'blink.'+label)
    feature('Golden iris '+label,(x,y-.008,z),(eye_scale,.010,eye_scale*.97),(.48,.30,.07),'blink.'+label)
    feature('Pupil '+label,(x,y-.016,z),(.018,.005,.032),(.012,.018,.011),'blink.'+label)
    feature('Eye glint '+label,(x-.009,y-.021,z+.014),(.007,.003,.009),(.95,.98,.85),'blink.'+label)
    # Ear interior inset, tucked into the outer cartilage silhouette.
    ear=feature('Inner ear '+label,(s*.164,-.457,.864),(.038,.014,.067),(.61,.26,.20),'ear.'+label)
    ear.rotation_euler[1]=s*.21
    # Small muzzle freckles, opaque geometry, no unsupported alpha cards.
    if REV>=2:
        for j in range(3):feature('Whisker freckle',(s*(.067+j*.022),-.64+j*.008,.618+(j%2)*.016),(.003,.002,.003),(.23,.11,.047))
nose=feature('Rose nose',(0,-.647,.643),(.026,.016,.017),(.64,.24,.20))
# Tapered nose triangle and tiny mouth lines.
for v in nose.data.vertices: v.co.x*=.55+.45*smoothstep(-.015,.01,v.co.z)
feature('Philtrum',(0,-.64,.616),(.003,.003,.017),(.25,.10,.047))
for s in [-1,1]:
    line=feature('Mouth smile',(s*.015,-.633,.601),(.018,.003,.004),(.25,.10,.047)); line.rotation_euler[1]=s*-.22

# One stable deformation rig. Rest bones all align with world +Z for simple tools.
armdata=bpy.data.armatures.new('Paws cat rig v1'); rig=bpy.data.objects.new('PipRig',armdata); scene.collection.objects.link(rig)
activate(rig); bpy.ops.object.mode_set(mode='EDIT')
bones={}
def bone(name,point,parent=None):
    b=armdata.edit_bones.new(name); b.head=point; b.tail=Vector(point)+Vector((0,0,.06))
    if parent:b.parent=armdata.edit_bones[parent]
    bones[name]=tuple(point)
bone('root',(0,0,0)); bone('pelvis',(0,.24,.4),'root'); bone('spine',(0,0,.43),'pelvis')
bone('chest',(0,-.25,.46),'spine'); bone('neck',(0,-.35,.54),'chest'); bone('head',(0,-.43,.65),'neck')
for s,label in [(-1,'L'),(1,'R')]:
    bone('ear.'+label,(s*.145,-.42,.79),'head'); bone('blink.'+label,(s*.107,-.587,.703),'head')
    for front,y,xx in [('front',-.245,.137),('hind',.26,.155)]:
        bone(front+'_upper.'+label,(s*xx,y,.37 if front=='front' else .39),'chest' if front=='front' else 'pelvis')
        bone(front+'_lower.'+label,(s*xx,y,.21),front+'_upper.'+label)
        bone(front+'_paw.'+label,(s*xx,y-.04,.065),front+'_lower.'+label)
for i,p in enumerate(tailpoints[:-1]):bone('tail.%02d'%i,p,'pelvis' if i==0 else 'tail.%02d'%(i-1))
bpy.ops.object.mode_set(mode='OBJECT')

def legacy_weights(p):
    x,y,z=p; s='R' if x>0 else 'L'
    if y>.365 and z>.49:
        candidates=['tail.%02d'%i for i in range(7)]
    elif z>.8 and y<-.30:candidates=['ear.'+s,'head']
    elif y<-.36 and z>.55:candidates=['head','neck']
    elif z<.31 and abs(x)>.07:
        limb='front' if y<0 else 'hind'; candidates=[limb+'_upper.'+s,limb+'_lower.'+s,limb+'_paw.'+s]
    else:candidates=['pelvis','spine','chest','neck','head']
    ranked=sorted([(max(.018,(Vector(p)-Vector(bones[n])).length),n) for n in candidates])[:3]
    raw=[(n,1/d**5) for d,n in ranked]; total=sum(w for n,w in raw)
    return [(n,w/total) for n,w in raw]
def weights(p):
    if REV<4:return legacy_weights(p)
    p=Vector(p)
    # Continuous distance fields around anatomical segments, with no hard
    # height/region switch at shoulders, ankles or the curved tail tip.
    segments={
        'pelvis':((0,.12,.40),(0,.31,.40)),
        'spine':((0,-.12,.43),(0,.10,.43)),
        'chest':((0,-.27,.46),(0,-.15,.46)),
        'neck':((0,-.35,.54),(0,-.32,.49)),
        'head':((0,-.46,.65),(0,-.45,.76))}
    for s,label in [(-1,'L'),(1,'R')]:
        segments['ear.'+label]=((s*.16,-.42,.83),(s*.187,-.42,.95))
        for limb,y,x in [('front',-.245,.137),('hind',.26,.155)]:
            segments[limb+'_upper.'+label]=((s*x,y,.34),(s*x,y,.245))
            segments[limb+'_lower.'+label]=((s*x,y,.19),(s*x,y-.015,.10))
            segments[limb+'_paw.'+label]=((s*x,y-.04,.056),(s*x,y-.115,.05))
    for i in range(7):segments['tail.%02d'%i]=(tailpoints[i],tailpoints[i+1])
    scores=[]
    for n,(a,b) in segments.items():
        a,b=Vector(a),Vector(b);v=b-a;t=max(0,min(1,(p-a).dot(v)/v.length_squared));dist=(p-(a+t*v)).length
        radius=.085 if n.startswith('tail') else .10
        score=math.exp(-min(100,(dist/radius)**2*2.0))
        scores.append((score,n))
    ranked=sorted(scores,reverse=True)[:4];total=sum(w for w,n in ranked)
    normalized={n:w/total for w,n in ranked}
    if REV>=6:
        # Face and its attached details must have the same rigid head transform.
        h=smoothstep(.47,.585,p.z)*smoothstep(-.26,-.38,p.y)
        normalized={n:w*(1-h) for n,w in normalized.items()}
        e=smoothstep(.79,.91,p.z)*smoothstep(.09,.16,abs(p.x))
        normalized['head']=normalized.get('head',0)+h*(1-e)
        ear='ear.R' if p.x>0 else 'ear.L';normalized[ear]=normalized.get(ear,0)+h*e
    best=sorted(normalized.items(),key=lambda it:it[1],reverse=True)[:4];total=sum(w for n,w in best)
    return [(n,w/total) for n,w in best if w>1e-7]
for o,group in [(body,None)]+details:
    activate(o); bpy.ops.object.transform_apply(location=True,rotation=True,scale=True)
    for n in bones:o.vertex_groups.new(name=n)
    for v in o.data.vertices:
        for n,w in ([(group,1)] if group else weights(v.co)):o.vertex_groups[n].add([v.index],w,'REPLACE')
    mod=o.modifiers.new('Canonical skin','ARMATURE'); mod.object=rig; o.parent=rig

def rot(name,xyz):
    p=rig.pose.bones[name]; rest=p.bone.matrix_local.to_quaternion()
    from mathutils import Euler
    p.rotation_mode='QUATERNION'; p.rotation_quaternion=rest.inverted()@Euler(tuple(math.radians(v) for v in xyz),'XYZ').to_quaternion()@rest
def translate(name,xyz):
    p=rig.pose.bones[name]; p.location=p.bone.matrix_local.to_quaternion().inverted()@Vector(xyz)
def reset_pose():
    for p in rig.pose.bones:p.location=(0,0,0); p.rotation_mode='QUATERNION';p.rotation_quaternion=(1,0,0,0);p.scale=(1,1,1)
def blink(amount):
    for n in ['blink.L','blink.R']:
        # bone local Y corresponds to world Z.
        rig.pose.bones[n].scale=(1,max(.045,1-amount),1)
def resting(t):
    translate('pelvis',(0,0,-.19*t)); rot('spine',(0,0,18*t)); rot('neck',(-12*t,0,16*t));rot('head',(16*t,0,20*t))
    for s in ['L','R']:
        rot('front_upper.'+s,(55*t,0,0));rot('front_lower.'+s,(-95*t,0,0));rot('front_paw.'+s,(40*t,0,0))
        rot('hind_upper.'+s,(-60*t,0,0));rot('hind_lower.'+s,(110*t,0,0));rot('hind_paw.'+s,(-50*t,0,0))
        if REV>=8:
            rot('front_lower.'+s,(-65*t,0,0));rot('front_paw.'+s,(10*t,0,0))
            rot('hind_lower.'+s,(90*t,0,0));rot('hind_paw.'+s,(-30*t,0,0))
    rot('tail.00',(80*t,40*t,15*t))
    for i in range(1,7):rot('tail.%02d'%i,(0,0,10*t))
    if REV>=5:
        rot('spine',(0,0,42*t));rot('chest',(0,0,10*t));rot('neck',(4*t,0,28*t));rot('head',(12*t,0,25*t))
        translate('neck',(0,0,-.075*t));rot('tail.00',(93*t,58*t,-25*t))
    if REV>=6:
        translate('pelvis',(0,0,-.215*t));rot('spine',(0,0,28*t));rot('chest',(0,0,5*t));rot('neck',(4*t,0,-10*t));rot('head',(8*t,0,-25*t))
        translate('neck',(.035*t,.06*t,-.145*t))
        if REV>=9:
            translate('neck',(.035*t,.045*t,-.23*t));rot('head',(32*t,0,-25*t))
        # Author the tail around the haunch at cushion height, instead of
        # rotating an upright hook into the body. Blend from the canonical pose.
        desired=[(0,.355,.245),(.09,.445,.205),(.21,.47,.175),(.30,.405,.145),(.34,.27,.125),(.32,.16,.13),(.26,.08,.145),(.19,.07,.155)]
        bpy.context.view_layer.update()
        for i in range(7):
            name='tail.%02d'%i;b=rig.data.bones[name];p=rig.pose.bones[name]
            original=Vector(tailpoints[i+1])-Vector(tailpoints[i]);target=Vector(desired[i+1])-Vector(desired[i])
            rotation=original.rotation_difference(target)
            m=Matrix.Translation(Vector(desired[i]))@rotation.to_matrix().to_4x4()@Matrix.Translation(-Vector(tailpoints[i]))@b.matrix_local
            # matrix is in armature space; convert to local pose for stable keys.
            local=b.convert_local_to_pose(m,b.matrix_local,parent_matrix=p.parent.matrix,parent_matrix_local=b.parent.matrix_local,invert=True)
            loc,q,scale=local.decompose();p.location=loc*t;p.rotation_quaternion=Quaternion().slerp(q,t);p.scale=Vector((1,1,1)).lerp(scale,t)
            bpy.context.view_layer.update()
    blink(.93*t)
def drinking(t):
    rot('chest',(12*t,0,0)); rot('neck',(25*t,0,0)); rot('head',(18*t,0,0))
    rot('tail.00',(-12*t,0,0))
def pose(name,t,duration):
    reset_pose(); phase=t/duration; w=math.sin(phase*math.tau)
    if name=='relaxed_idle':
        rot('head',(0,w*1.5,w*1.0)); rot('tail.02',(0,w*5,0)); rot('ear.L',(0,0,w*3))
        blink(max(0,1-abs(t-duration*.64)/.28))
    elif name=='attentive_request':
        k=math.sin(math.pi*phase)**2; rot('head',(-5*k,-13*k,3*k));rot('ear.R',(-10*k,0,9*k));rot('tail.03',(0,k*8,0))
    elif name=='happy_pounce':
        k=math.sin(math.pi*phase)**2; translate('pelvis',(0,-.03*k,.105*k)); rot('head',(-7*k,0,0))
        for s in ['L','R']:rot('front_upper.'+s,(-24*k,0,0));rot('front_lower.'+s,(32*k,0,0))
        rot('tail.02',(0,10*w,0))
    elif name=='garden_walk':
        for side,offset in [('L',0),('R',math.pi)]:
            for limb,shift in [('front',0),('hind',math.pi)]:
                v=math.sin(phase*math.tau+offset+shift); rot(limb+'_upper.'+side,(20*v,0,0));rot(limb+'_lower.'+side,(-12*max(0,v),0,0));rot(limb+'_paw.'+side,(-10*v,0,0))
        translate('pelvis',(0,0,.008*(1-math.cos(phase*math.tau*2))));rot('tail.02',(0,w*5,0))
    elif name=='settle_cushion':resting(smoothstep(0,1,phase))
    elif name=='curled_rest':resting(1);rot('ear.R',(0,w*2,0))
    elif name=='approach_bowl':drinking(smoothstep(0,1,phase))
    elif name=='drink':drinking(1);rot('head',(18+2*math.sin(phase*math.tau*4),0,0))
    elif name=='return_idle':drinking(1-smoothstep(0,1,phase))
clips=[('relaxed_idle',4,True),('attentive_request',2.4,False),('happy_pounce',1.2,False),('garden_walk',1.2,True),('settle_cushion',2.4,False),('curled_rest',4,True),('approach_bowl',1.8,False),('drink',2,True),('return_idle',1.8,False)]
# Bake floor-contact correction into pelvis translation, never the locomotion root.
# This is authoring-time skin evaluation, not a physics-driven runtime animation.
rest_positions=np.array([(*v.co,1) for v in body.data.vertices])
skin_weights=np.zeros((len(body.data.vertices),len(bones)))
bone_names=list(bones)
for v in body.data.vertices:
    for g in v.groups:skin_weights[v.index,bone_names.index(body.vertex_groups[g.group].name)]=g.weight
inverse_binds={n:rig.data.bones[n].matrix_local.inverted() for n in bones}
def correct_contact():
    bpy.context.view_layer.update();z=np.zeros(len(rest_positions))
    for i,n in enumerate(bone_names):
        row=np.array((rig.pose.bones[n].matrix@inverse_binds[n])[2]);z+=(rest_positions@row)*skin_weights[:,i]
    lift=max(0,.003-float(z.min()))
    rig.pose.bones['pelvis'].location+=rig.data.bones['pelvis'].matrix_local.to_quaternion().inverted()@Vector((0,0,lift))
rig.animation_data_create()
for name,duration,loop in clips:
    action=bpy.data.actions.new(name); action.use_fake_user=True; rig.animation_data.action=action
    for f in range(round(duration*30)+1):
        pose(name,f/30,duration)
        if REV>=7:correct_contact()
        for p in rig.pose.bones:
            p.keyframe_insert('location',frame=f+1);p.keyframe_insert('rotation_quaternion',frame=f+1);p.keyframe_insert('scale',frame=f+1)
    for fc in action.fcurves:
        for key in fc.keyframe_points:key.interpolation='LINEAR'
rig.animation_data.action=bpy.data.actions['relaxed_idle']; scene.frame_set(1)

# Stable review rig. Neutral ground, warm key and cool fill.
floor=material('Review sandstone',(.57,.62,.52),.93)
bpy.ops.mesh.primitive_plane_add(size=200); plane=bpy.context.object;plane.name='Review ground';plane.data.materials.append(floor);plane.location.z=-.005
def area(name,loc,energy,size):
    d=bpy.data.lights.new(name,'AREA');o=bpy.data.objects.new(name,d);scene.collection.objects.link(o);o.location=loc;d.energy=energy;d.size=size;o.rotation_euler=(Vector((0,0,.45))-o.location).to_track_quat('-Z','Y').to_euler()
area('Soft warm key',(-3,-4,6),430,4); area('Cool fill',(4,-1,3),180,5);area('Rim',(0,4,5),260,3)
bpy.ops.object.camera_add(location=(1.45,-2.25,1.28));camera=bpy.context.object;camera.name='Review camera';scene.camera=camera
camera.data.type='ORTHO';camera.data.ortho_scale=1.47
def view(loc,target=(0,0,.51),scale=1.47):
    camera.location=loc;camera.rotation_euler=(Vector(target)-camera.location).to_track_quat('-Z','Y').to_euler();camera.data.ortho_scale=scale
def render(name,loc,target=(0,0,.51),scale=1.47):
    view(loc,target,scale);scene.render.filepath=str(JOB/'renders'/name);bpy.ops.render.render(write_still=True)

if opt.final:
    # Unique UV atlas; bake unlit coat + flat feature colors to one portable material.
    meshes=[body]+[o for o,g in details]
    bpy.ops.object.select_all(action='DESELECT')
    for o in meshes:o.select_set(True)
    bpy.context.view_layer.objects.active=body;bpy.ops.object.join();body.name='Pip'
    activate(body);bpy.ops.object.mode_set(mode='EDIT');bpy.ops.mesh.select_all(action='SELECT');bpy.ops.uv.smart_project(angle_limit=1.15,island_margin=.015);bpy.ops.object.mode_set(mode='OBJECT')
    atlas=bpy.data.images.new('Pip coat albedo',width=1024,height=1024,alpha=False)
    for mat in body.data.materials:
        n=mat.node_tree.nodes.new('ShaderNodeTexImage');n.image=atlas;mat.node_tree.nodes.active=n
    # Evaluate material in object space at atlas texels, avoiding vertex-color
    # interpolation artifacts on deliberate stripe and cream boundaries.
    def ss(a,b,x):
        t=np.clip((x-a)/(b-a),0,1);return t*t*(3-2*t)
    def shade(points):
        x,y,z=points.T;ax=np.abs(x)
        pix=srcarr[((z*2.1%1)*(src.size[1]-1)).astype(int),((((y+x*.25)*2.1)%1)*(src.size[0]-1)).astype(int),:3]
        base=(np.array([.64,.335,.115])*.85+pix*.15)*np.array([1.05,.94,.83])
        stripe=ss(.55,.84,np.sin(y*42+z*6+np.sin(z*18)*.4))*ss(.025,.11,ax)*(y>-.28)*(z>.23)
        forehead=ss(.45,.8,np.sin(x*76+np.abs(z-.80)*32))*ss(.68,.77,z)*(y<-.34)
        stripe=np.maximum(stripe,forehead)
        stripe=np.maximum(stripe,ss(.55,.85,np.sin(z*70+y*3))*.6*(z<.29))
        stripe=np.where((y>.36)&(z>.52),ss(.35,.75,np.sin(z*43+y*5)),stripe)
        base*=1-stripe[:,None]*.35
        muzzle=ss(-.556,-.592,y)*(1-ss(.636,.661,z))
        bib=(1-ss(.07,.13,ax))*ss(-.33,-.41,y)*(1-ss(.54,.58,z))*ss(.20,.29,z)
        cream=np.maximum(np.maximum(muzzle,bib),(1-ss(.067,.095,z))*.8)[:,None]
        return base*(1-cream)+np.array([.93,.835,.65])*cream
    rgba=np.zeros((1024,1024,4),np.float32);rgba[:,:,:3]=[.64,.335,.115];covered=np.zeros((1024,1024),bool)
    body.data.calc_loop_triangles();uv=body.data.uv_layers.active.data
    for tri in body.data.loop_triangles:
        tc=np.array([uv[i].uv[:] for i in tri.loops])*1023
        pc=np.array([body.data.vertices[i].co[:] for i in tri.vertices])
        lo=np.maximum(0,np.floor(tc.min(axis=0)).astype(int));hi=np.minimum(1023,np.ceil(tc.max(axis=0)).astype(int))
        if np.any(hi<lo):continue
        xx,yy=np.meshgrid(np.arange(lo[0],hi[0]+1),np.arange(lo[1],hi[1]+1));q=np.stack([xx.ravel(),yy.ravel()],axis=1)
        ab=tc[1]-tc[0];ac=tc[2]-tc[0];den=ab[0]*ac[1]-ab[1]*ac[0]
        if abs(den)<1e-7:continue
        p=q-tc[0];u=(p[:,0]*ac[1]-p[:,1]*ac[0])/den;v=(ab[0]*p[:,1]-ab[1]*p[:,0])/den
        inside=(u>=-.005)&(v>=-.005)&(u+v<=1.005);q=q[inside];u=u[inside];v=v[inside]
        positions=pc[0]+u[:,None]*(pc[1]-pc[0])+v[:,None]*(pc[2]-pc[0])
        mat=body.data.materials[tri.material_index]
        rgb=shade(positions) if mat==coat else np.array(mat.node_tree.nodes.get('Principled BSDF').inputs['Base Color'].default_value[:3])
        rgba[q[:,1],q[:,0],:3]=rgb;rgba[q[:,1],q[:,0],3]=1;covered[q[:,1],q[:,0]]=True
    # Deterministic eight-pixel edge dilation for filtered texture sampling.
    for _ in range(8):
        nxt=covered.copy()
        for axis,shift in [(0,1),(0,-1),(1,1),(1,-1)]:
            valid=np.roll(covered,shift,axis)&~nxt;rgba[valid]=np.roll(rgba,shift,axis)[valid];nxt|=valid
        covered=nxt
    # Blender's float buffer for this generated color image stores sRGB values.
    # Convert linear material colors for parity with Principled base color.
    rgb=rgba[:,:,:3];rgba[:,:,:3]=np.where(rgb<=.0031308,rgb*12.92,1.055*np.power(np.maximum(rgb,0),1/2.4)-.055)
    rgba[:,:,3]=1;atlas.pixels.foreach_set(rgba.ravel());atlas.update()
    atlas.filepath_raw=str(JOB/'textures/pip-basecolor.png');atlas.file_format='PNG';atlas.save();atlas.pack()
    final=material('Pip portable atlas',(.8,.5,.2),.86);n=final.node_tree.nodes.new('ShaderNodeTexImage');n.image=atlas;final.node_tree.links.new(n.outputs['Color'],final.node_tree.nodes.get('Principled BSDF').inputs['Base Color'])
    # Independent material-property maps: no conversion from albedo luminance.
    for name,rgba in [('pip-roughness',(1,.86,0,1)),('pip-normal',(.5,.5,1,1))]:
        im=bpy.data.images.new(name,width=32,height=32,alpha=False);im.colorspace_settings.name='Non-Color';im.pixels=list(rgba)*1024;im.filepath_raw=str(JOB/('textures/'+name+'.png'));im.file_format='PNG';im.save();im.pack()
        node=final.node_tree.nodes.new('ShaderNodeTexImage');node.image=im
        if name=='pip-normal':
            normal=final.node_tree.nodes.new('ShaderNodeNormalMap');final.node_tree.links.new(node.outputs['Color'],normal.inputs['Color']);final.node_tree.links.new(normal.outputs['Normal'],final.node_tree.nodes.get('Principled BSDF').inputs['Normal'])
        else:
            sep=final.node_tree.nodes.new('ShaderNodeSeparateRGB');final.node_tree.links.new(node.outputs['Color'],sep.inputs[0]);final.node_tree.links.new(sep.outputs['G'],final.node_tree.nodes.get('Principled BSDF').inputs['Roughness'])
    body.data.materials.clear();body.data.materials.append(final)
    for p in body.data.polygons:p.material_index=0
    # Drop authoring-only vertex colors so GLB does not multiply the baked coat.
    for c in list(body.data.color_attributes):body.data.color_attributes.remove(c)
    scene.frame_set(1);rig.animation_data.action=None;reset_pose()
    activate(body);rig.select_set(True)
    bpy.ops.export_scene.gltf(filepath=str(JOB/'exports/pip.glb'),export_format='GLB',use_selection=True,export_animations=True,export_nla_strips=True,export_force_sampling=True,export_frame_range=False,export_skins=True,export_morph=False,export_yup=True)
    rig.animation_data.action=bpy.data.actions['relaxed_idle'];scene.frame_set(1)
    tris=sum(len(p.vertices)-2 for p in body.data.polygons)
    points=[body.matrix_world@v.co for v in body.data.vertices]
    report={'triangles':tris,'vertices':len(body.data.vertices),'materials':len(body.data.materials),'bones':list(bones),'bounds_blender':[[min(p[i] for p in points) for i in range(3)],[max(p[i] for p in points) for i in range(3)]],'clips':[{'name':n,'seconds':d,'loop':l} for n,d,l in clips]}
    (JOB/'exports/measurements.json').write_text(json.dumps(report,indent=2))
    bpy.ops.wm.save_as_mainfile(filepath=str(JOB/'pip-master.blend'))
else:bpy.ops.wm.save_as_mainfile(filepath=str(JOB/('pip-r%d.blend'%REV)))
render('r%d-three-quarter.png'%REV,(1.45,-2.25,1.28))
render('r%d-side.png'%REV,(2.5,0,1.0))
render('r%d-face.png'%REV,(.12,-2,.83),(0,-.43,.73),.66)
if opt.final:
    render('r%d-rear.png'%REV,(1.4,2.1,1.25))
    render('r%d-puzzle-distance.png'%REV,(2.4,-3.5,3.5),(0,0,.4),3.8)
    for name,frame in [('attentive_request',37),('happy_pounce',19),('garden_walk',10),('settle_cushion',73),('curled_rest',30),('approach_bowl',55),('drink',16),('return_idle',55),('relaxed_idle',78)]:
        rig.animation_data.action=bpy.data.actions[name];scene.frame_set(frame)
        render('pose-'+name+'.png',(1.45,-2.25,1.28))
    scene.render.film_transparent=True;plane.hide_render=True
    for name,action,frame in [('dialogue','attentive_request',37),('shop','relaxed_idle',1),('ending','curled_rest',30)]:
        rig.animation_data.action=bpy.data.actions[action];scene.frame_set(frame)
        render('portrait-'+name+'.png',(.65,-2,1.05),(0,-.4,.69) if name!='ending' else (0,0,.25),.68 if name!='ending' else 1.2)
print('PIP AUTHORING COMPLETE',REV)

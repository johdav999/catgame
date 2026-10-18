"""Mesh-adapted in-place actions on the frozen Pip bind rig, 30 fps."""
import bpy, json, math, sys, runpy
import numpy as np
from pathlib import Path
from mathutils import Vector, Matrix, Euler
sys.path.insert(0,str(Path(__file__).parent))
from fit_cats import ROOT,JOB,AUTHORITY,review,smooth
from eyelids import fit_eyelids,pose_eyes

REV=24
PROBE='--probe' in sys.argv
DURATIONS={'relaxed_idle':4.,'attentive_request':2.4,'happy_pounce':1.2,
 'garden_walk':1.2,'settle_cushion':2.4,'curled_rest':4.,'approach_bowl':1.8,
 'drink':2.,'return_idle':1.8,'scratch':3.,'perch_step':2.4,'perch_settle':2.4,
 'playhouse_enter':1.8,'playhouse_lookout':3.,'picnic_sit':2.4,'personality_reaction':3.}
LOOPS={'relaxed_idle','garden_walk','curled_rest','drink','scratch','playhouse_lookout'}

def ease(t):t=max(0,min(1,t));return t*t*t*(t*(6*t-15)+10)
def blink(t):
    if t<2.4:return 0
    if t<2.5:return ease((t-2.4)/.1)
    if t<2.566666667:return 1
    return 1-ease((t-2.566666667)/.2)

def build(cat):
    folder=JOB/cat;folder.mkdir(exist_ok=True,parents=True)
    bpy.ops.wm.open_mainfile(filepath=str(AUTHORITY if cat=='pip' else folder/(cat+'-fit-r3.blend')))
    s=bpy.context.scene;r=bpy.data.objects['PipRig']
    body=next(o for o in s.objects if o.type=='MESH' and o.find_armature()==r)
    r.animation_data.action=None
    for track in r.animation_data.nla_tracks:track.mute=True
    for p in r.pose.bones:p.matrix_basis.identity()
    bpy.context.view_layer.update()
    if cat=='pip':
        eyes=[]
        for e in json.loads((AUTHORITY.parent/'native-fit.json').read_text()):
            eyes.append({'name':e['name'],'lower':e['lower_name'],'n':Vector(e['n']),'centre':Vector(e['centre'])})
        eye_audit={'reused':'accepted Pip fitted eye topology and controls'}
        # Correct only the hanging central bib's mistaken foreleg influences.
        # Preserve accepted face, eyelids, feet and tail weights.
        for v in body.data.vertices:
            a=smooth(.24,.36,v.co.z)*smooth(.17,.07,abs(v.co.x))*smooth(.02,-.18,v.co.y)*(1-smooth(.50,.63,v.co.z))
            if a<=0:continue
            weights={body.vertex_groups[g.group].name:g.weight*(1-a) for g in v.groups}
            weights['chest']=weights.get('chest',0)+a
            for group in [g.group for g in v.groups]:body.vertex_groups[group].remove([v.index])
            best=sorted(weights.items(),key=lambda p:p[1],reverse=True)[:4];total=sum(w for n,w in best)
            for n,w in best:
                if w>0:body.vertex_groups[n].add([v.index],w/total,'REPLACE')
    else:eyes,eye_audit=fit_eyelids(body,r,cat)
    # Tail weights must not drag the lower haunch with the tail arc.
    for v in body.data.vertices:
        tail=smooth(.38,.53,v.co.z)*smooth(.25,.38,v.co.y)
        transferred=0
        for g in list(v.groups):
            group=body.vertex_groups[g.group]
            if group.name.startswith('tail.'):
                transferred+=g.weight*(1-tail);group.add([v.index],g.weight*tail,'REPLACE')
        if transferred:body.vertex_groups['pelvis'].add([v.index],transferred,'ADD')
    # Protect the back and shoulders from limb-weight spill. Torso vertices
    # blend along the spine; knees and paws remain on the original leg fields.
    for v in body.data.vertices:
        head=sum(g.weight for g in v.groups if body.vertex_groups[g.group].name in ['head','neck','ear.L','ear.R'] or body.vertex_groups[g.group].name.startswith(('blink','tail.')))
        a=smooth(.28,.42,v.co.z)*smooth(.25,.16,abs(v.co.x))*(1-head)
        if a<.001:continue
        weights={body.vertex_groups[g.group].name:g.weight*(1-a) for g in v.groups}
        torso={n:math.exp(-((v.co.y-y)/.15)**2) for n,y in [('pelvis',.24),('spine',0),('chest',-.25)]}
        total=sum(torso.values())
        for n,w in torso.items():weights[n]=weights.get(n,0)+a*w/total
        for group in [g.group for g in v.groups]:body.vertex_groups[group].remove([v.index])
        best=sorted(weights.items(),key=lambda p:p[1],reverse=True)[:4];total=sum(w for n,w in best)
        for n,w in best:
            if w>0:body.vertex_groups[n].add([v.index],w/total,'REPLACE')
    # Bake and export the identical four-influence skin (including newly
    # interpolated eyelid vertices); do not rely on exporter truncation.
    for v in body.data.vertices:
        best=sorted([(g.group,g.weight) for g in v.groups if g.weight>1e-8],key=lambda p:p[1],reverse=True)[:4]
        total=sum(w for _,w in best);assert total>0
        for group in [g.group for g in v.groups]:body.vertex_groups[group].remove([v.index])
        for group,weight in best:body.vertex_groups[group].add([v.index],weight/total,'REPLACE')
    (folder/'eyelid-fit.json').write_text(json.dumps(eye_audit,indent=2))
    bpy.ops.wm.save_as_mainfile(filepath=str(folder/(cat+'-skin.blend')))
    names=[b.name for b in r.data.bones]
    positions=np.array([(*v.co,1) for v in body.data.vertices])
    weights=np.zeros((len(positions),len(names)))
    for v in body.data.vertices:
        for g in v.groups:weights[v.index,names.index(body.vertex_groups[g.group].name)]=g.weight
    assert np.max(np.abs(weights.sum(axis=1)-1))<1e-6
    assert max(np.count_nonzero(row) for row in weights)<=4
    inverse={n:r.data.bones[n].matrix_local.inverted() for n in names}
    active=[(i,n,np.flatnonzero(weights[:,i]>0)) for i,n in enumerate(names) if weights[:,i].max()>0]
    footmasks={}
    for kind in ['front','hind']:
        for side in ['L','R']:
            mask=(positions[:,2]<.10)&((positions[:,1]<0) if kind=='front' else (positions[:,1]>.1))&((positions[:,0]<0) if side=='L' else (positions[:,0]>0))
            assert mask.sum()>5,(cat,kind,side)
            footmasks[kind+'.'+side]=mask
    def points():
        bpy.context.view_layer.update();pts=np.zeros((len(positions),3))
        for i,n,ids in active:
            matrix=np.array(r.pose.bones[n].matrix@inverse[n])
            pts[ids]+=np.einsum('ij,kj->ki',matrix[:3],positions[ids])*weights[ids,i,None]
        return pts
    def rotate(n,xyz):
        p=r.pose.bones[n];q=p.bone.matrix_local.to_quaternion()
        p.rotation_quaternion=q.inverted()@Euler(tuple(math.radians(v) for v in xyz)).to_quaternion()@q
        bpy.context.view_layer.update()
    def world_rotate(n,axis,angle):
        p=r.pose.bones[n];m=p.matrix.copy();o=m.translation.copy()
        p.matrix=Matrix.Translation(o)@Matrix.Rotation(math.radians(angle),4,axis)@Matrix.Translation(-o)@m
        bpy.context.view_layer.update()
    def shift_pelvis(delta):
        p=r.pose.bones['pelvis'];m=p.matrix.copy();m.translation+=Vector(delta);p.matrix=m
        bpy.context.view_layer.update()
    def align(n,child,target):
        p=r.pose.bones[n];m=p.matrix.copy();origin=m.translation.copy()
        direction=r.pose.bones[child].matrix.translation-origin
        q=direction.rotation_difference(target-origin)
        p.matrix=Matrix.Translation(origin)@q.to_matrix().to_4x4()@Matrix.Translation(-origin)@m
        bpy.context.view_layer.update()
    def limb(kind,side,target,pole_override=None):
        a,b,c=[kind+'_'+part+'.'+side for part in ['upper','lower','paw']]
        shoulder=r.pose.bones[a].matrix.translation.copy()
        l1=(r.data.bones[b].head_local-r.data.bones[a].head_local).length
        l2=(r.data.bones[c].head_local-r.data.bones[b].head_local).length
        delta=target-shoulder;d=max(.001,min(delta.length,l1+l2-.00001));axis=delta.normalized()
        along=(l1*l1-l2*l2+d*d)/(2*d);height=math.sqrt(max(0,l1*l1-along*along))
        pole=Vector(pole_override if pole_override else (0,1 if kind=='front' else -1,0));pole=(pole-axis*pole.dot(axis)).normalized()
        align(a,b,shoulder+axis*along+pole*height);align(b,c,target)
        m=r.data.bones[c].matrix_local.copy();m.translation=r.pose.bones[c].matrix.translation
        r.pose.bones[c].matrix=m;bpy.context.view_layer.update()
    # Copy sampled accepted actions before authoring. Reading the latest master
    # preserves accepted walk timing and all unchanged common action channels.
    source={}
    for name in list(DURATIONS)[:9]:
        a=bpy.data.actions[name];r.animation_data.action=a;frames=[]
        for f in range(1,round(DURATIONS[name]*30)+2):
            s.frame_set(f);frames.append({n:r.pose.bones[n].matrix_basis.copy() for n in names})
        source[name]=frames
    initial=source['relaxed_idle'][0]
    # Match the actual existing bowl's 0.383 m water plane with a measured
    # lower-muzzle marker. The new meshes have different head proportions.
    tex=body.data.materials[0].node_tree.nodes.get('Principled BSDF').inputs['Base Color'].links[0].from_node.image
    pixels=np.array(tex.pixels[:]).reshape(tex.size[1],tex.size[0],4);nose=[]
    for loop in body.data.loops:
        v=body.data.vertices[loop.vertex_index];uv=body.data.uv_layers.active.data[loop.index].uv
        red,green,blue=pixels[int(uv.y%1*(tex.size[1]-1)),int(uv.x%1*(tex.size[0]-1)),:3]
        if .52<v.co.z<.72 and v.co.y<-.3 and red>.25 and red>green*1.4 and blue>green*.65:nose.append(v.index)
    nose=np.unique(nose);assert len(nose)>2
    centre=positions[nose,:3].mean(0);front=sum((e['n'] for e in eyes),Vector()).normalized()
    near=np.flatnonzero((np.abs(positions[:,2]-(centre[2]-.032))<.018)&(np.linalg.norm(positions[:,:2]-centre[:2],axis=1)<.065))
    depth=positions[near,:3]@np.array(front);mouth=near[depth>np.percentile(depth,85)]
    def drink_adjust(angle):
        heading=r.pose.bones['head'].matrix.to_3x3()@inverse['head'].to_3x3()@front
        axis=Vector((0,0,1)).cross(heading).normalized()
        world_rotate('neck',axis,angle*.65);world_rotate('head',axis,angle*.35)
    r.animation_data.action=None
    low,high=0.,85.
    for _ in range(18):
        angle=(low+high)/2
        for n,m in source['drink'][0].items():r.pose.bones[n].matrix_basis=m
        bpy.context.view_layer.update();drink_adjust(angle)
        sample=points()
        if sample[mouth,2].mean()+.003-sample[:,2].min()>.392:low=angle
        else:high=angle
    bowl_angle=(low+high)/2
    report={'cat':cat,'revision':REV,'fps':30,'actions':{},'boneCount':len(names),
            'bowlFit':{'waterY':.383,'muzzleTargetY':.392,'additionalPitchDegrees':bowl_angle}}
    for name,duration in DURATIONS.items():
        action=bpy.data.actions.get(name) or bpy.data.actions.new(name)
        action.fcurves.clear();action.use_fake_user=True;r.animation_data.action=action
        measurements=[];count=round(duration*30)
        for f in ([0,count//2,count] if PROBE else range(count+1)):
            s.frame_set(f+1);t=f/30;u=t/duration
            base=source[name][f] if name in source else initial
            for n,m in base.items():r.pose.bones[n].matrix_basis=m
            bpy.context.view_layer.update()
            if name in ['settle_cushion','curled_rest'] and cat in ['pip','button']:
                # Lower onto a flank, tuck the legs, then turn head and tail into
                # one sleeping arc. Joint lengths/bind transforms stay fixed.
                amount=ease(t/2.4) if name=='settle_cushion' else 1
                for n,m in initial.items():r.pose.bones[n].matrix_basis=m
                rotate('pelvis',(0,25*amount,0))
                world_rotate('spine','Z',55*amount);world_rotate('chest','Z',30*amount)
                world_rotate('neck','Z',20*amount)
                heading=r.pose.bones['head'].matrix.to_3x3()@inverse['head'].to_3x3()@sum((e['n'] for e in eyes),Vector()).normalized()
                pitch=Vector((0,0,1)).cross(heading).normalized()
                world_rotate('neck',pitch,65*amount);world_rotate('head',pitch,-15*amount)
                shift_pelvis((0,0,-.285*amount))
                for side in ['L','R']:
                    for kind in ['front','hind']:
                        target=r.data.bones[kind+'_paw.'+side].head_local.copy()
                        tucked=Vector((.32 if kind=='front' else .16,.08 if kind=='front' else .18,.12))
                        tucked.y+=(-.035 if side=='L' else .035)
                        limb(kind,side,target.lerp(tucked,amount),(-amount,1-amount,0) if kind=='front' else None)
                # Connected tail follows the outside of the curled silhouette.
                for i in range(6):
                    bone='tail.%02d'%i;child='tail.%02d'%(i+1)
                    origin=r.pose.bones[bone].matrix.translation.copy()
                    angle=math.radians(20-i*30)
                    direction=Vector((math.cos(angle),math.sin(angle),-.10 if i==0 else 0))
                    current=r.pose.bones[child].matrix.translation-origin
                    direction=current.normalized().lerp(direction.normalized(),amount)
                    align(bone,child,origin+direction)
            if name in ['picnic_sit','perch_settle']:
                a=ease(u);shift_pelvis((0,.045*a,-.15*a));rotate('pelvis',(-17*a,0,0));rotate('spine',(12*a,0,0));rotate('neck',(-5*a,0,0))
                for side in ['L','R']:
                    for kind in ['front','hind']:
                        target=r.data.bones[kind+'_paw.'+side].head_local.copy()
                        target.y+=(-.10 if kind=='hind' else 0)*a
                        limb(kind,side,target)
                world_rotate('tail.00','Y',65*a);world_rotate('tail.00','Z',-35*a)
            if name=='scratch':
                # Supported hind legs and paws pressing an authored post face.
                rotate('pelvis',(-26,0,0));rotate('spine',(-18,0,0));rotate('neck',(20,0,0))
                shift_pelvis((0,.025,.02))
                for side,phase in [('L',0),('R',math.pi)]:
                    limb('hind',side,r.data.bones['hind_paw.'+side].head_local.copy())
                    target=Vector((-.13 if side=='L' else .13,-.39,.65+.028*math.sin(2*math.pi*u*3+phase)))
                    limb('front',side,target)
                    world_rotate('front_paw.'+side,'X',-60)
                world_rotate('tail.00','Y',20)
            if name=='perch_step':
                # In-place step gesture; Prompt 6 owns translation between
                # approach/step/top anchors and supports the actor throughout.
                a=math.sin(math.pi*u)**2
                rotate('spine',(-5*a,0,0));rotate('neck',(5*a,0,0))
                for side,offset in [('L',0),('R',.25)]:
                    for kind,delay in [('front',0),('hind',.45)]:
                        phase=(u-offset-delay)%1;lift=max(0,math.sin(math.pi*min(1,phase/.3))) if phase<.3 else 0
                        target=r.data.bones[kind+'_paw.'+side].head_local.copy();target.z+=.12*lift*a;target.y-=.055*lift*a
                        limb(kind,side,target)
            if name=='playhouse_enter':
                a=math.sin(math.pi*u)**2;rotate('neck',(15*a,0,0));rotate('head',(10*a,0,0));shift_pelvis((0,0,-.025*a))
                for side in ['L','R']:
                    for kind in ['front','hind']:limb(kind,side,r.data.bones[kind+'_paw.'+side].head_local.copy())
            if name=='playhouse_lookout':
                rotate('neck',(0,0,8*math.sin(2*math.pi*u)));rotate('head',(0,5*math.sin(2*math.pi*u),0))
                rotate('ear.L',(0,0,5*math.sin(2*math.pi*u)));rotate('ear.R',(0,0,-5*math.sin(2*math.pi*u)))
            if name=='personality_reaction':
                a=math.sin(math.pi*u)**2
                rotate('head',((7 if cat=='button' else -3)*a,(12 if cat=='pepper' else 4)*a,(-6 if cat=='mabel' else 9)*a))
                rotate('ear.L',(0,0,(12 if cat=='button' else -8)*a));rotate('ear.R',(0,0,-5*a))
                for i in range(3,7):world_rotate('tail.%02d'%i,'X',(5 if cat=='pepper' else 2)*math.sin(math.tau*u)*a)
            if name in ['approach_bowl','drink','return_idle']:
                blend=ease(u) if name=='approach_bowl' else 1-ease(u) if name=='return_idle' else 1
                drink_adjust(bowl_angle*blend)
            # Adapt canonical blink timing to each mesh's fitted skin sockets.
            amount=blink(t) if name=='relaxed_idle' else 0
            if name=='curled_rest':amount=1
            if name=='settle_cushion':amount=ease((t-1.2)/1.2)
            if name=='personality_reaction' and cat=='mabel':amount=ease(u/.35)*(1-ease((u-.6)/.35))
            pose_eyes(r,eyes,amount)
            pts=points()
            # Measured ground correction acts on pelvis only, leaving the root
            # in place. It cannot detach any child joint or shorten a limb.
            lift=.003-float(pts[:,2].min())
            if name=='happy_pounce':lift=max(0,lift)
            if lift:shift_pelvis((0,0,lift));pts=points()
            contacts={k:{'minZ':float(pts[mask,2].min()),'center':pts[mask].mean(0).tolist()} for k,mask in footmasks.items()}
            measurements.append({'time':t,'min':pts.min(0).tolist(),'max':pts.max(0).tolist(),'groundLift':lift,'paws':contacts})
            for p in r.pose.bones:
                for prop in ['location','rotation_quaternion','scale']:p.keyframe_insert(prop,frame=f+1)
        for fc in action.fcurves:
            for key in fc.keyframe_points:key.interpolation='LINEAR'
        report['actions'][name]={'duration':duration,'loop':name in LOOPS,'samples':measurements}
        print('ACTION',cat,name,flush=True)
    bpy.ops.file.pack_all();bpy.ops.wm.save_as_mainfile(filepath=str(folder/f'{cat}-r{REV}.blend'))
    # Retain useful stills as an actual second review cycle.
    s.render.engine='BLENDER_EEVEE';s.render.resolution_x=s.render.resolution_y=640;s.render.resolution_percentage=100
    s.camera.data.type='ORTHO';s.camera.data.ortho_scale=1.65
    for action,time in [('relaxed_idle',0),('garden_walk',.3),('curled_rest',0),('scratch',.5),('perch_step',.6),('picnic_sit',2.4),('drink',.5)]:
        r.animation_data.action=bpy.data.actions[action];s.frame_set(round(time*30)+1)
        for view,loc in [('three-quarter',(1.7,-2.5,1.4)),('side',(3,0,.85))]+([('top',(0,0,3))] if action=='curled_rest' else []):
            s.camera.location=loc;s.camera.rotation_euler=(Vector((0,0,.46))-s.camera.location).to_track_quat('-Z','Y').to_euler()
            s.render.filepath=str(folder/'renders'/f'r{REV}-{action}-{view}.png');bpy.ops.render.render(write_still=True)
    # Face closeups inspect both continuous closure and open identity.
    target=sum((e['centre'] for e in eyes),Vector())/2;front=sum((e['n'] for e in eyes),Vector()).normalized()
    s.camera.location=target+front*2+Vector((0,0,.03));s.camera.rotation_euler=(target-s.camera.location).to_track_quat('-Z','Y').to_euler();s.camera.data.ortho_scale=.42
    r.animation_data.action=bpy.data.actions['relaxed_idle']
    for label,time in [('open',0),('half',2.45),('closed',2.533333)]:
        frame=time*30+1;s.frame_set(int(frame),subframe=frame%1)
        if PROBE:
            r.animation_data.action=None
            for n,m in initial.items():r.pose.bones[n].matrix_basis=m
            pose_eyes(r,eyes,{'open':0,'half':.5,'closed':1}[label]);bpy.context.view_layer.update()
        s.render.filepath=str(folder/'renders'/f'r{REV}-blink-{label}.png');bpy.ops.render.render(write_still=True)
    r.animation_data.action=bpy.data.actions['relaxed_idle'];s.frame_set(1)
    s.render.fps=30;s.render.fps_base=1
    if PROBE:
        bpy.ops.wm.save_as_mainfile(filepath=str(folder/f'{cat}-probe-r{REV}.blend'))
        return
    (folder/'animation-measurements.json').write_text(json.dumps(report,indent=2))
    bpy.ops.file.pack_all();bpy.ops.wm.save_as_mainfile(filepath=str(folder/f'{cat}-r{REV}.blend'))
    (folder/'exports').mkdir(exist_ok=True)
    bpy.ops.object.select_all(action='DESELECT');r.select_set(True);body.select_set(True);bpy.context.view_layer.objects.active=r
    out=folder/'exports'/f'{cat}-r{REV}.glb'
    bpy.ops.export_scene.gltf(filepath=str(out),export_format='GLB',use_selection=True,export_animations=True,export_force_sampling=True,export_frame_range=False,export_nla_strips=True)
    runpy.run_path(str(ROOT/'artifacts/model-jobs/pip/2026-10-10-tripo-repair/normalize_glb.py'))['normalize'](out)
    print('ANIMATED',cat,len(body.data.vertices),len(r.data.bones),len(DURATIONS),flush=True)

if __name__=='__main__':
    for cat in [x for x in sys.argv[sys.argv.index('--')+1:] if x!='--probe'] if '--' in sys.argv else ['button','mabel','pepper','pip']:build(cat)

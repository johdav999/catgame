"""Focused rest/settle correction on the accepted canonical Pip; source preserved."""
import bpy, math, json, sys, argparse
import numpy as np
from pathlib import Path
from mathutils import Matrix, Vector

job = Path(__file__).resolve().parent
root = job.parents[3]
source = job.parent / '2026-10-10-tripo-repair/pip-repaired-r8.blend'
parser = argparse.ArgumentParser()
parser.add_argument('--roll', type=float, default=65)
parser.add_argument('--bend', type=float, default=35)
parser.add_argument('--revision', type=int, default=1)
parser.add_argument('--source', type=str)
args = parser.parse_args(sys.argv[sys.argv.index('--')+1:] if '--' in sys.argv else [])
if args.source: source = Path(args.source).resolve()
bpy.ops.wm.open_mainfile(filepath=str(source))
scene = bpy.context.scene
rig = next(o for o in scene.objects if o.type == 'ARMATURE')
body = next(o for o in scene.objects if o.type == 'MESH' and any(m.type == 'ARMATURE' for m in o.modifiers))
names = [b.name for b in rig.data.bones]
positions = np.array([(*v.co, 1) for v in body.data.vertices])
weights = np.zeros((len(positions), len(names)))
for v in body.data.vertices:
    for g in v.groups:
        name = body.vertex_groups[g.group].name
        if name in names: weights[v.index,names.index(name)] = g.weight
inverse = {n:rig.data.bones[n].matrix_local.inverted() for n in names}
def points():
    bpy.context.view_layer.update()
    result = np.zeros((len(positions),3))
    for i,n in enumerate(names):
        result += (positions @ np.array(rig.pose.bones[n].matrix @ inverse[n]).T)[:,:3] * weights[:,i,None]
    return result
def rotate_world(name, axis, degrees):
    p = rig.pose.bones[name]
    world = p.matrix.copy(); pivot = world.translation.copy()
    world = Matrix.Translation(pivot) @ Matrix.Rotation(math.radians(degrees),4,axis) @ Matrix.Translation(-pivot) @ world
    p.matrix = world
    bpy.context.view_layer.update()
def key_pose(frame):
    for p in rig.pose.bones:
        p.keyframe_insert('location',frame=frame)
        p.keyframe_insert('rotation_quaternion',frame=frame)
        p.keyframe_insert('scale',frame=frame)
measurements = {}
for name in ['settle_cushion','curled_rest']:
    action = bpy.data.actions[name];rig.animation_data.action=action
    # Retain a complete copy to evaluate baseline samples while authoring the correction.
    baseline=action.copy();baseline.name=name+'.baseline';baseline.use_fake_user=True
    end=round(action.frame_range[1])
    for f in range(1,end+1):
        rig.animation_data.action=baseline;scene.frame_set(f);bpy.context.view_layer.update()
        baseline_tail=[rig.pose.bones['tail.%02d'%i].matrix.copy() for i in range(7)]
        baseline_tail_local=[rig.pose.bones['tail.%02d'%i].matrix_basis.copy() for i in range(7)]
        amount=1 if name=='curled_rest' else (f-1)/(end-1)
        amount=amount*amount*(3-2*amount)
        rotate_world('pelvis','Y',args.roll*amount)
        rotate_world('spine','Z',args.bend*.35*amount)
        rotate_world('chest','Z',args.bend*.45*amount)
        rotate_world('neck','Z',args.bend*.65*amount)
        rotate_world('head','Z',args.bend*.45*amount)
        if args.revision>=11:
            rotate_world('neck','X',(-25 if args.revision>=12 else 25)*amount)
            rotate_world('head','X',(-40 if args.revision>=12 else 40)*amount)
        # Lower the spine and tuck all four chains into a common sleeping plane;
        # the original crouch otherwise leaves the upper-side paws in the air.
        if args.revision in (3,4):
            for bone_name,z in [('pelvis',.24),('spine',.25),('chest',.24),('neck',.20),('head',.17)]:
                p=rig.pose.bones[bone_name];world=p.matrix.copy();world.translation.z+=(z-world.translation.z)*amount;p.matrix=world;bpy.context.view_layer.update()
            for side in ['L','R']:
                for limb,z in [('front_upper.',.24),('front_lower.',.13),('front_paw.',.085),('hind_upper.',.24),('hind_lower.',.14),('hind_paw.',.085)]:
                    p=rig.pose.bones[limb+side];world=p.matrix.copy();world.translation.z+=(z-world.translation.z)*amount;p.matrix=world;bpy.context.view_layer.update()
        pts=points();p=rig.pose.bones['pelvis'];world=p.matrix.copy()
        # Keep the grounded pivot and centre the compact silhouette on its cushion.
        body_mask=weights[:,names.index('tail.00'):names.index('tail.06')+1].sum(axis=1)<.5
        world.translation.z += .003-float(pts[body_mask,2].min())
        world.translation.x -= float((pts[body_mask,0].min()+pts[body_mask,0].max())/2)*amount
        world.translation.y -= float((pts[body_mask,1].min()+pts[body_mask,1].max())/2)*amount
        p.matrix=world;bpy.context.view_layer.update()
        if amount and 6<=args.revision<=10:
            # Keep the approved tail's local segment lengths and planar contact,
            # bending its real joints instead of translating a synthetic arc.
            anchor=rig.pose.bones['tail.00'].matrix.translation.copy()
            world=baseline_tail[0].copy();world.translation=anchor
            rig.pose.bones['tail.00'].matrix=world;bpy.context.view_layer.update()
            for i in range(1,7):
                rig.pose.bones['tail.%02d'%i].matrix_basis=baseline_tail_local[i]
                bpy.context.view_layer.update();rotate_world('tail.%02d'%i,'Z',-18*amount)
            pts=points();lift=max(0,.003-float(pts[:,2].min()))
            if lift:
                p=rig.pose.bones['pelvis'];world=p.matrix.copy();world.translation.z+=lift;p.matrix=world;bpy.context.view_layer.update()
        if amount and args.revision<6:
            # Curl the tail on the contact plane without letting its tip lift the
            # entire body. Preserve a connected joint chain and original lengths.
            anchor=rig.pose.bones['tail.00'].matrix.translation.copy()
            old=[rig.pose.bones['tail.%02d'%i].matrix.copy() for i in range(7)]
            tail_points=[]
            for i in range(7):
                angle=math.radians(180+30*i)
                radius=.30
                target=Vector((anchor.x+radius*(math.cos(angle)+1),anchor.y+radius*math.sin(angle),anchor.z+(max(.075,anchor.z-.10*i)-anchor.z)*amount))
                tail_points.append(old[i].translation.lerp(target,amount))
            for i in range(7):
                b=rig.pose.bones['tail.%02d'%i];world=old[i].copy()
                direction=tail_points[min(i+1,6)]-tail_points[max(i-1,0)]
                if direction.length>1e-5:
                    old_direction=world.to_quaternion()@Vector((0,1,0))
                    world=old_direction.rotation_difference(direction.normalized()).to_matrix().to_4x4()@world
                world.translation=tail_points[i];b.matrix=world;bpy.context.view_layer.update()
            pts=points();lift=max(0,.003-float(pts[:,2].min()))
            if lift:
                p=rig.pose.bones['pelvis'];world=p.matrix.copy();world.translation.z+=lift;p.matrix=world;bpy.context.view_layer.update()
        pose={n:rig.pose.bones[n].matrix_basis.copy() for n in names}
        rig.animation_data.action=action
        for n,m in pose.items():rig.pose.bones[n].matrix_basis=m
        key_pose(f)
    for curve in action.fcurves:
        for k in curve.keyframe_points:k.interpolation='LINEAR'
    scene.frame_set(end);pts=points()
    measurements[name]={'minimum':pts.min(axis=0).tolist(),'maximum':pts.max(axis=0).tolist()}
rig.animation_data.action=bpy.data.actions['curled_rest'];scene.frame_set(1)
scene.render.engine='CYCLES';scene.cycles.samples=24
scene.render.resolution_x=scene.render.resolution_y=768;scene.render.resolution_percentage=100
scene.render.film_transparent=False
camera=scene.camera
for view,loc in [('front',(1.4,-2.4,1.2)),('top',(0,-.1,3)),('side',(2.4,0,1))]:
    camera.location=loc;camera.rotation_euler=(Vector((0,0,.2))-camera.location).to_track_quat('-Z','Y').to_euler();camera.data.ortho_scale=1.65
    scene.render.filepath=str(job/('r%d-%s.png'%(args.revision,view)));bpy.ops.render.render(write_still=True)
for action in list(bpy.data.actions):
    if action.name.endswith('.baseline'):bpy.data.actions.remove(action)
    else:action.use_fake_user=True
rig.animation_data.action=bpy.data.actions['relaxed_idle'];scene.frame_set(1)
bpy.ops.file.pack_all();bpy.ops.wm.save_as_mainfile(filepath=str(job/('pip-cushion-r%d.blend'%args.revision)))
for o in scene.objects:o.select_set(False)
rig.select_set(True);body.select_set(True);bpy.context.view_layer.objects.active=rig
bpy.ops.export_scene.gltf(filepath=str(job/('pip-cushion-r%d.glb'%args.revision)),export_format='GLB',use_selection=True,
    export_animations=True,export_force_sampling=True,export_frame_range=False,export_nla_strips=False)
(job/('r%d-measurements.json'%args.revision)).write_text(json.dumps(measurements,indent=2)+'\n')

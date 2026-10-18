"""Clean-process GLB reimport, measured anchors, portraits and review renders."""
import bpy,json,sys,math
import numpy as np
from pathlib import Path
from mathutils import Vector
sys.path.insert(0,str(Path(__file__).parent))
from fit_cats import ROOT,JOB,AUTHORITY,review

def runtime(p):return [float(p[0]),float(p[2]),float(-p[1])]
def run(cat,revision):
    folder=JOB/cat;export=folder/'exports'/f'{cat}-r{revision}.glb'
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.ops.import_scene.gltf(filepath=str(export))
    rig=next(o for o in bpy.context.scene.objects if o.type=='ARMATURE')
    body=next(o for o in bpy.context.scene.objects if o.type=='MESH' and o.find_armature()==rig)
    actions={}
    required=json.loads((JOB/'rig-contract.json').read_text())
    for name in required['commonActions']+required['extraActions']+['personality_reaction']:
        matches=[a for a in bpy.data.actions if a.name==name or a.name.startswith(name+'_')]
        assert len(matches)==1,(name,[a.name for a in bpy.data.actions]);actions[name]=matches[0]
    for track in rig.animation_data.nla_tracks:track.mute=True
    for a in actions.values():a.use_fake_user=True
    s=review.stage((0,0,.45));s.render.fps=30;s.render.fps_base=1
    # Retain the exact imported geometry for contact measurements.
    rig.animation_data.action=None
    for p in rig.pose.bones:p.matrix_basis.identity()
    bpy.context.view_layer.update()
    rest=np.array([v.co[:] for v in body.data.vertices])
    feet={kind+'.'+side:np.flatnonzero((rest[:,2]<.10)&((rest[:,1]<0) if kind=='front' else (rest[:,1]>.1))&((rest[:,0]<0) if side=='L' else (rest[:,0]>0))) for kind in ['front','hind'] for side in ['L','R']}
    texture=body.data.materials[0].node_tree.nodes.get('Principled BSDF').inputs['Base Color'].links[0].from_node.image
    pixels=np.array(texture.pixels[:]).reshape(texture.size[1],texture.size[0],4)
    nose=[]
    for l in body.data.loops:
        v=body.data.vertices[l.vertex_index];uv=body.data.uv_layers.active.data[l.index].uv
        color=pixels[int(uv.y%1*(texture.size[1]-1)),int(uv.x%1*(texture.size[0]-1)),:3]
        red,green,blue=color
        if .52<v.co.z<.72 and v.co.y<-.3 and red>.25 and red>green*1.4 and blue>green*.65:nose.append(v.index)
    nose=np.unique(nose);assert len(nose)>2,(cat,'nose marker not found')
    eyes=json.loads((folder/'eyelid-fit.json').read_text())
    if cat=='pip':eyes=json.loads((AUTHORITY.parent/'native-fit.json').read_text())
    front=sum((Vector(e.get('normal',e.get('n'))) for e in eyes),Vector()).normalized()
    centre=np.mean(rest[nose],0)
    near=np.flatnonzero((np.abs(rest[:,2]-(centre[2]-.032))<.018)&(np.linalg.norm(rest[:,:2]-centre[:2],axis=1)<.065))
    assert len(near)>3
    depth=rest[near]@np.array(front);mouth=near[depth>np.percentile(depth,85)]
    report={'cat':cat,'export':export.relative_to(ROOT).as_posix(),'units':'metres','up':'+Y','forward':'+Z',
            'jointCount':len(rig.data.bones),'actions':list(actions),'restBounds':{'min':runtime([rest[:,0].min(),rest[:,1].max(),rest[:,2].min()]),'max':runtime([rest[:,0].max(),rest[:,1].min(),rest[:,2].max()])},
            'contactMethod':'Means of original foot regions, textured nose vertices and lowest-front muzzle region, evaluated after clean GLB import. Joint origins are recorded separately; they are not surface contacts.',
            'samples':{}}
    for name,a in actions.items():
        rig.animation_data.action=a
        values=[]
        for t in [0,(a.frame_range.y-a.frame_range.x)/60,(a.frame_range.y-a.frame_range.x)/30]:
            frame=a.frame_range.x+t*30;s.frame_set(int(frame),subframe=frame%1)
            evaluated=body.evaluated_get(bpy.context.evaluated_depsgraph_get());mesh=evaluated.to_mesh()
            pts=np.array([(evaluated.matrix_world@v.co)[:] for v in mesh.vertices])
            values.append({'seconds':t,'bounds':{'min':runtime([pts[:,0].min(),pts[:,1].max(),pts[:,2].min()]),'max':runtime([pts[:,0].max(),pts[:,1].min(),pts[:,2].max()])},
                'nose_tip':runtime(pts[nose].mean(0)),'muzzle_contact':runtime(pts[mouth].mean(0)),
                'paws':{key:{'centre':runtime(pts[ids].mean(0)),'soleY':float(pts[ids,2].min()),
                            'solePoint':runtime(pts[ids[np.argmin(pts[ids,2])]]),
                            'forwardSurface':runtime(pts[ids[np.argmin(pts[ids,1])]]),
                            'bounds':{'min':runtime([pts[ids,0].min(),pts[ids,1].max(),pts[ids,2].min()]),
                                      'max':runtime([pts[ids,0].max(),pts[ids,1].min(),pts[ids,2].max()])}} for key,ids in feet.items()},
                'joints':{n:runtime(rig.matrix_world@rig.pose.bones[n].matrix.translation) for n in ['root','pelvis','head','front_paw.L','front_paw.R','hind_paw.L','hind_paw.R']}})
            evaluated.to_mesh_clear()
        report['samples'][name]=values
    (folder/'contacts.json').write_text(json.dumps(report,indent=2)+'\n')
    rig.animation_data.action=actions['relaxed_idle'];s.frame_set(int(actions['relaxed_idle'].frame_range.x))
    review.render_views(s,folder/'renders',prefix='reimport')
    for name,t in [('curled_rest',0),('scratch',.5),('drink',.5),('perch_step',.6),('picnic_sit',2.4)]:
        rig.animation_data.action=actions[name];s.frame_set(round(t*30+actions[name].frame_range.x))
        s.camera.location=(1.7,-2.5,1.4);s.camera.rotation_euler=(Vector((0,0,.4))-s.camera.location).to_track_quat('-Z','Y').to_euler()
        s.render.filepath=str(folder/'renders'/f'reimport-{name}.png');bpy.ops.render.render(write_still=True)
    rig.animation_data.action=actions['relaxed_idle'];s.frame_set(int(actions['relaxed_idle'].frame_range.x))
    target=sum((Vector(e['centre']) for e in eyes),Vector())/2+Vector((0,0,-.06))
    s.camera.location=target+front*2+Vector((0,0,.03));s.camera.rotation_euler=(target-s.camera.location).to_track_quat('-Z','Y').to_euler()
    s.camera.data.ortho_scale=.69;s.render.resolution_x=s.render.resolution_y=512
    s.render.filepath=str(folder/'renders'/'portrait.png');bpy.ops.render.render(write_still=True)
    bpy.ops.file.pack_all();bpy.ops.wm.save_as_mainfile(filepath=str(folder/f'{cat}-clean-reimport.blend'))
    print('CLEAN_REIMPORT',cat,len(rig.data.bones),len(actions),len(nose),len(mouth),flush=True)

if __name__=='__main__':
    args=sys.argv[sys.argv.index('--')+1:]
    for cat in args[1:]:run(cat,int(args[0]))

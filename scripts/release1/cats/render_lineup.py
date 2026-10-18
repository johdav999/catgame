"""Render the four final GLBs together for identity/scale comparison."""
import bpy,json,math,sys
from pathlib import Path
from mathutils import Vector
sys.path.insert(0,str(Path(__file__).parent))
from fit_cats import JOB,AUTHORITY,review
revision=int(sys.argv[sys.argv.index('--')+1])
bpy.ops.wm.read_factory_settings(use_empty=True)
for index,cat in enumerate(['pip','mabel','button','pepper']):
    before=set(bpy.data.objects);actions=set(bpy.data.actions)
    bpy.ops.import_scene.gltf(filepath=str(JOB/cat/'exports'/f'{cat}-r{revision}.glb'))
    rig=next(o for o in set(bpy.data.objects)-before if o.type=='ARMATURE')
    rig.animation_data.action=next(a for a in set(bpy.data.actions)-actions if a.name.startswith('relaxed_idle'))
    for track in rig.animation_data.nla_tracks:track.mute=True
    eye_path=AUTHORITY.parent/'native-fit.json' if cat=='pip' else JOB/cat/'eyelid-fit.json'
    eyes=json.loads(eye_path.read_text());front=sum((Vector(e.get('normal',e.get('n'))) for e in eyes),Vector()).normalized()
    rig.rotation_mode="XYZ";rig.rotation_euler.z=-math.atan2(front.x,-front.y);rig.location.x=(index-1.5)*1.05
s=review.stage((0,0,.5));s.frame_set(0)
s.render.resolution_x=1600;s.render.resolution_y=640
s.camera.data.ortho_scale=4.65;s.camera.location=(0,-5,2);s.camera.rotation_euler=(Vector((0,0,.48))-s.camera.location).to_track_quat('-Z','Y').to_euler()
s.render.filepath=str(JOB/'lineup.png');bpy.ops.render.render(write_still=True)

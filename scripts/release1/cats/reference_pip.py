"""Views of the frozen accepted Pip master, never the new production variant."""
import bpy,json,hashlib,sys
from pathlib import Path
sys.path.insert(0,str(Path(__file__).parent))
from fit_cats import ROOT,JOB,AUTHORITY,review
bpy.ops.wm.open_mainfile(filepath=str(AUTHORITY))
rig=bpy.data.objects['PipRig']
body=next(o for o in bpy.context.scene.objects if o.type=='MESH' and o.find_armature()==rig)
for o in list(bpy.context.scene.objects):
    if o not in [body,rig]:bpy.data.objects.remove(o,do_unlink=True)
rig.animation_data.action=bpy.data.actions['relaxed_idle']
for track in rig.animation_data.nla_tracks:track.mute=True
bpy.context.scene.frame_set(1)
s=review.stage((0,0,.5));folder=JOB/'pip/references';folder.mkdir(parents=True,exist_ok=True)
review.render_views(s,folder,prefix='accepted-pip')
record={'authority':AUTHORITY.relative_to(ROOT).as_posix(),'sha256':hashlib.sha256(AUTHORITY.read_bytes()).hexdigest(),
        'action':'relaxed_idle','seconds':0,'geometryModified':False,'lighting':'Neutral intake stage',
        'views':[p.relative_to(ROOT).as_posix() for p in folder.glob('accepted-pip-*.png')]}
(folder/'authority.json').write_text(json.dumps(record,indent=2)+'\n')

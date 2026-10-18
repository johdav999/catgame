import bpy,bmesh,json,sys
from pathlib import Path
from mathutils import Vector
sys.path.insert(0,str(Path(__file__).parent))
from fit_cats import JOB
from eyelids import pose_eyes
cat='button';bpy.ops.wm.open_mainfile(filepath=str(JOB/cat/(cat+'-skin.blend')))
r=bpy.data.objects['PipRig'];r.animation_data_clear()
for p in r.pose.bones:p.matrix_basis.identity()
body=next(o for o in bpy.context.scene.objects if o.type=='MESH' and o.find_armature()==r)
eyes=[dict(name=e['bone'],lower=e['bone'].replace('blink.','blink.lower.'),n=Vector(e['normal'])) for e in json.loads((JOB/cat/'eyelid-fit.json').read_text())]
pose_eyes(r,eyes,1);bpy.context.view_layer.update()
bm=bmesh.new();bm.from_mesh(body.data);bm.verts.ensure_lookup_table()
boundary=[v.index for v in bm.verts if any(e.is_boundary for e in v.link_edges)];bm.free()
report=[]
for e in json.loads((JOB/cat/'eyelid-fit.json').read_text()):
    c=Vector(e['centre']);ns=[]
    for i in boundary:
        v=body.data.vertices[i]
        if (v.co-c).length>.065:continue
        weights={body.vertex_groups[g.group].name:g.weight for g in v.groups}
        pos=sum(((r.pose.bones[n].matrix@r.data.bones[n].matrix_local.inverted()@v.co)*w for n,w in weights.items()),Vector())
        ns.append({'original':list(v.co),'posed':list(pos),'weights':weights})
    report.append({'eye':e,'boundary':ns})
(JOB/cat/'debug-eyes.json').write_text(json.dumps(report,indent=2))
s=bpy.context.scene;s.camera.data.ortho_scale=.42
c=sum((Vector(e['centre']) for e in json.loads((JOB/cat/'eyelid-fit.json').read_text())),Vector())/2
n=sum((e['n'] for e in eyes),Vector()).normalized();s.camera.location=c+n*2;s.camera.rotation_euler=(c-s.camera.location).to_track_quat('-Z','Y').to_euler()
s.render.filepath=str(JOB/cat/'renders/debug-closed.png');bpy.ops.render.render(write_still=True)

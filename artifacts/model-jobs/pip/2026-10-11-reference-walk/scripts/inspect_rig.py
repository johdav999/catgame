import bpy,json
from pathlib import Path
from mathutils import Matrix
job=Path(__file__).resolve().parents[1]
bpy.ops.wm.open_mainfile(filepath=str(job.parent/'2026-10-10-tripo-repair/pip-repaired.blend'))
rig=next(o for o in bpy.context.scene.objects if o.type=='ARMATURE')
rig.animation_data.action=None
for p in rig.pose.bones:p.matrix_basis=Matrix.Identity(4)
bpy.context.view_layer.update()
body=next(o for o in bpy.context.scene.objects if o.type=='MESH' and o.find_armature()==rig)
report={'rig':rig.name,'body':body.name,'bones':{b.name:{'head':list(b.head_local),'tail':list(b.tail_local)} for b in rig.data.bones},'feet':{}}
for limb in ['front','hind']:
 for side in ['L','R']:
  points=[v.co for v in body.data.vertices if v.co.z<.085 and ((v.co.x<0)==(side=='L')) and ((v.co.y<0)==(limb=='front'))]
  report['feet'][limb+'.'+side]={'min':[min(p[i] for p in points) for i in range(3)],'max':[max(p[i] for p in points) for i in range(3)],'count':len(points)}
(job/'references/rig-inspection.json').write_text(json.dumps(report,indent=2))
print(json.dumps(report))

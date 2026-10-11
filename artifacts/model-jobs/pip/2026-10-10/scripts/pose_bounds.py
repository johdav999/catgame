import bpy,json
from pathlib import Path
JOB=Path(__file__).resolve().parents[1]
bpy.ops.wm.open_mainfile(filepath=str(JOB/'pip-master.blend'))
rig=bpy.data.objects['PipRig'];mesh=bpy.data.objects['Pip'];report={}
for action in bpy.data.actions:
    rig.animation_data.action=action; samples=[]
    end=int(action.frame_range[1])
    for f in sorted(set([1,end]+list(range(1,end+1,6)))):
        bpy.context.scene.frame_set(f);evaluated=mesh.evaluated_get(bpy.context.evaluated_depsgraph_get());coords=[v.co for v in evaluated.data.vertices]
        samples.append({'frame':f,'min_z':min(v.z for v in coords),'max_z':max(v.z for v in coords)})
    report[action.name]=samples
(JOB/'exports/pose-bounds.json').write_text(json.dumps(report,indent=2))
print({name:min(s['min_z'] for s in samples) for name,samples in report.items()})

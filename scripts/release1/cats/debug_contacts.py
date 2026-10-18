import bpy,json
from pathlib import Path
bpy.ops.wm.open_mainfile(filepath=str(Path('artifacts/model-jobs/release1-cats/button/button-r19.blend').resolve()))
r=bpy.data.objects['PipRig'];r.animation_data.action=bpy.data.actions['curled_rest'];bpy.context.scene.frame_set(1)
b=next(o for o in bpy.context.scene.objects if o.type=='MESH' and o.find_armature()==r)
ev=b.evaluated_get(bpy.context.evaluated_depsgraph_get());m=ev.to_mesh()
print('MATRIX',b.matrix_world,r.matrix_world)
used={i for p in m.polygons for i in p.vertices}
print('USED',len(used),'TOTAL',len(m.vertices))
print('FLOOR',bpy.data.objects['Review ground'].location[:])
for v in sorted(m.vertices,key=lambda v:v.co.z)[:8]:print('LOW',v.index,v.index in used,list(v.co),[(b.vertex_groups[g.group].name,g.weight) for g in b.data.vertices[v.index].groups])
for n in ['pelvis','spine','chest','head']+['tail.%02d'%i for i in range(7)]:print('BONE',n,list(r.pose.bones[n].matrix.translation))

import bpy,json
from pathlib import Path
from mathutils import Vector
j=Path('C:/Users/Johan/source/repos/CatGame/artifacts/model-jobs/pip/2026-10-11-eyelids')
bpy.ops.wm.open_mainfile(filepath=str(j/'pip-eyelids-r9.blend'))
b=bpy.data.objects['Pip repaired Tripo skin'];b.data.update()
for e in json.load(open(j/'fit.json')):
 c=Vector(e['centre']);n=Vector(e['n']);ps=[p for p in b.data.polygons if p.material_index==1 and (p.center-c).length<.10]
 print(e['name'],'faces',len(ps),'negative',sum(p.normal.dot(n)<0 for p in ps),'min',min(p.normal.dot(n) for p in ps))
 print('bad',[(list(p.center),p.normal.dot(n)) for p in ps if p.normal.dot(n)<0][:12])
print('BAD VERTICES',[[list(b.data.vertices[i].co) for i in p.vertices] for p in b.data.polygons if p.material_index==1 and p.normal.x < -.8][:3])
print('material',[(i,m.name) for i,m in enumerate(b.data.materials)])

import bpy,bmesh,json
from pathlib import Path
J=Path(__file__).parent
bpy.ops.wm.open_mainfile(filepath=str(J/'pip-blink-r6.blend'))
b=bpy.data.objects['Pip repaired Tripo skin'];bm=bmesh.new();bm.from_mesh(b.data)
mixed=[e for e in bm.edges if len({f.material_index for f in e.link_faces})==2]
print('SEAMS',len(mixed),'boundary skin',sum(e.is_boundary and any(f.material_index==1 for f in e.link_faces) for e in bm.edges))
(J/'bands.json').write_text(json.dumps({'vertices':[list(v.co) for v in b.data.vertices], 'faces':[list(p.vertices) for p in b.data.polygons if p.material_index==1]}))

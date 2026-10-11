import bpy, json, sys
from pathlib import Path
bpy.ops.wm.read_factory_settings(use_empty=True)
source=Path(sys.argv[sys.argv.index('--')+1])
bpy.ops.import_scene.fbx(filepath=str(source))
report={'source':str(source),'armatures':[],'meshes':[],'actions':[]}
for o in bpy.data.objects:
    if o.type=='ARMATURE':
        report['armatures'].append({'name':o.name,'bones':[{'name':b.name,'parent':b.parent.name if b.parent else None,'head':list(b.head_local),'tail':list(b.tail_local)} for b in o.data.bones]})
    if o.type=='MESH':
        weights=[sum(g.weight for g in v.groups) for v in o.data.vertices]
        report['meshes'].append({'name':o.name,'vertices':len(o.data.vertices),'triangles':sum(len(p.vertices)-2 for p in o.data.polygons),'dimensions':list(o.dimensions),'groups':[g.name for g in o.vertex_groups],'unweighted':sum(w<1e-6 for w in weights),'max_influences':max((sum(g.weight>1e-6 for g in v.groups) for v in o.data.vertices),default=0),'modifiers':[{'type':m.type,'target':m.object.name if m.type=='ARMATURE' and m.object else None} for m in o.modifiers],'materials':[m.name for m in o.data.materials]})
for a in bpy.data.actions:
    report['actions'].append({'name':a.name,'frame_range':list(a.frame_range),'curves':len(a.fcurves)})
out=Path(__file__).parent/'blender-inspection.json'
out.write_text(json.dumps(report,indent=2))
print(json.dumps(report))

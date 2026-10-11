import bpy
from pathlib import Path
j=Path('C:/Users/Johan/source/repos/CatGame/artifacts/model-jobs/pip/2026-10-11-eyelids')
bpy.ops.wm.open_mainfile(filepath=str(j/'pip-eyelids-r5.blend'))
b=bpy.data.objects['Pip repaired Tripo skin'];r=bpy.data.objects['PipRig'];r.animation_data.action=None
for p in r.pose.bones:p.matrix_basis.identity()
for m in b.data.materials:
 m.use_nodes=True;s=m.node_tree.nodes.get('Principled BSDF')
 for inp in ['Base Color','Normal','Roughness','Metallic']:
  for l in list(s.inputs[inp].links):m.node_tree.links.remove(l)
 s.inputs['Base Color'].default_value=(.5,.5,.5,1);s.inputs['Roughness'].default_value=.9;s.inputs['Metallic'].default_value=0
bpy.context.scene.render.filepath=str(j/'geometry-r5.png');bpy.ops.render.render(write_still=True)
print('counts',len(b.data.vertices),sum(len(p.vertices)-2 for p in b.data.polygons))

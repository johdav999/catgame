import bpy
from mathutils import Vector
from pathlib import Path
J=Path(__file__).parent
bpy.ops.wm.open_mainfile(filepath=str(J/'pip-blink-r4.blend'))
s=bpy.context.scene;r=bpy.data.objects['PipRig'];r.animation_data.action=None
for p in r.pose.bones:p.matrix_basis.identity()
b=bpy.data.objects['Pip repaired Tripo skin'];bpy.context.view_layer.objects.active=b
bpy.ops.mesh.customdata_custom_splitnormals_clear();b.data.use_auto_smooth=False
m=bpy.data.materials.new('debug');m.diffuse_color=(.5,.5,.5,1);m.use_nodes=True
b.data.materials.clear();b.data.materials.append(m)
for p in b.data.polygons:p.material_index=0;p.use_smooth=True
s.camera.location=(1.3,-2,.82);s.camera.rotation_euler=(Vector((.08,-.54,.75))-s.camera.location).to_track_quat('-Z','Y').to_euler();s.cycles.samples=8;s.render.filepath=str(J/'clay.png');bpy.ops.render.render(write_still=True)

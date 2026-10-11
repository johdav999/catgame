import bpy
from mathutils import Vector
from pathlib import Path
J=Path(__file__).parent
bpy.ops.wm.open_mainfile(filepath=str(J/'source.blend'))
s=bpy.context.scene;r=bpy.data.objects['PipRig'];r.animation_data.action=None
for p in r.pose.bones:p.matrix_basis.identity()
b=bpy.data.objects['Pip repaired Tripo skin'];bpy.context.view_layer.objects.active=b
import bmesh
bm=bmesh.new();bm.from_mesh(b.data);bmesh.ops.triangulate(bm,faces=list(bm.faces));bm.to_mesh(b.data);bm.free()
m=bpy.data.materials.new('debug');m.diffuse_color=(.5,.5,.5,1);m.use_nodes=True
#keep
#keep
s.camera.location=(1.3,-2,.82);s.camera.rotation_euler=(Vector((.08,-.54,.75))-s.camera.location).to_track_quat('-Z','Y').to_euler();s.cycles.samples=8;s.render.filepath=str(J/'triangulated-source.png');bpy.ops.render.render(write_still=True)

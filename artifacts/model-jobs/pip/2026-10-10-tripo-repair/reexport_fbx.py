import bpy
from pathlib import Path
from mathutils import Matrix
JOB=Path(__file__).parent
bpy.ops.wm.open_mainfile(filepath=str(JOB/'pip-repaired-r6.blend'))
rig=bpy.data.objects['PipRig'];rig.animation_data.action=None
for p in rig.pose.bones:p.matrix_basis=Matrix.Identity(4)
bpy.context.view_layer.update()
bpy.ops.object.select_all(action='DESELECT');rig.select_set(True)
body=bpy.data.objects['Pip repaired Tripo skin'];body.select_set(True);bpy.context.view_layer.objects.active=body
bpy.ops.export_scene.fbx(filepath=str(JOB/'exports/pip-repaired-r7.fbx'),use_selection=True,object_types={'MESH','ARMATURE'},add_leaf_bones=False,apply_unit_scale=True,apply_scale_options='FBX_SCALE_ALL',axis_forward='-Z',axis_up='Y',bake_anim=True,bake_anim_use_all_actions=True,bake_anim_use_nla_strips=False,bake_anim_simplify_factor=0,path_mode='COPY',embed_textures=True)

import bpy,json
from pathlib import Path
from mathutils import Matrix
J=Path(__file__).parent
bpy.ops.wm.open_mainfile(filepath=str(J/'pip-blink.blend'))
r=bpy.data.objects['PipRig'];body=bpy.data.objects['Pip repaired Tripo skin'];r.animation_data.action=None
for p in r.pose.bones:p.matrix_basis.identity()
bpy.ops.object.select_all(action='DESELECT');r.select_set(True);body.select_set(True);bpy.context.view_layer.objects.active=body
bpy.ops.export_scene.fbx(filepath=str(J/'pip-blink-raw.fbx'),use_selection=True,object_types={'MESH','ARMATURE'},add_leaf_bones=False,apply_unit_scale=True,apply_scale_options='FBX_SCALE_ALL',axis_forward='-Z',axis_up='Y',bake_anim=True,bake_anim_use_all_actions=True,bake_anim_use_nla_strips=False,bake_anim_simplify_factor=0,path_mode='COPY',embed_textures=True)
from io_scene_fbx import parse_fbx,encode_bin
root,version=parse_fbx.parse(str(J/'pip-blink-raw.fbx'))
methods={'C':'add_bool','Y':'add_int16','I':'add_int32','L':'add_int64','F':'add_float32','D':'add_float64','S':'add_string','R':'add_bytes','f':'add_float32_array','d':'add_float64_array','i':'add_int32_array','l':'add_int64_array','b':'add_bool_array','c':'add_byte_array'}
def clone(node):
 out=encode_bin.FBXElem(node.id)
 for index,(typ,value) in enumerate(zip(node.props_type,node.props)):
  if node.id==b'AnimationStack' and index==1 and typ==ord('S'):
   name,suffix=value.split(b'\x00\x01',1);value=name.split(b'|')[-1]+b'\x00\x01'+suffix
  getattr(out,methods[chr(typ)])(value)
 out.elems=[clone(c) for c in node.elems];return out
encode_bin.write(str(J/'pip-blink.fbx'),clone(root),version)

"""Normalize Blender FBX animation stack names without altering transforms."""
import sys
from pathlib import Path
from io_scene_fbx import parse_fbx,encode_bin
JOB=Path(__file__).parent
revision=int(sys.argv[sys.argv.index('--')+1])
source=JOB/'exports'/('pip-repaired-r%d.fbx'%revision)
root,version=parse_fbx.parse(str(source))
methods={'C':'add_bool','Y':'add_int16','I':'add_int32','L':'add_int64','F':'add_float32','D':'add_float64','S':'add_string','R':'add_bytes','f':'add_float32_array','d':'add_float64_array','i':'add_int32_array','l':'add_int64_array','b':'add_bool_array','c':'add_byte_array'}
renamed=[]
def clone(node):
 out=encode_bin.FBXElem(node.id)
 for index,(typ,value) in enumerate(zip(node.props_type,node.props)):
  if node.id==b'AnimationStack' and index==1 and typ==ord('S'):
   old=value;name,suffix=value.split(b'\x00\x01',1);name=name.split(b'|')[-1];value=name+b'\x00\x01'+suffix;renamed.append((str(old),str(value)))
  getattr(out,methods[chr(typ)])(value)
 out.elems=[clone(child) for child in node.elems]
 return out
dest=source.with_name('pip-repaired.fbx')
encode_bin.write(str(dest),clone(root),version)
print(renamed)


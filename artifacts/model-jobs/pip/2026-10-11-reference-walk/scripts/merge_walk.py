"""Replace only garden_walk in a current GLB, preserving concurrent asset work.
python merge_walk.py base.glb walk.glb output.glb
"""
import json,struct,copy,sys,hashlib
from pathlib import Path
def read(path):
 raw=Path(path).read_bytes();size,kind=struct.unpack_from('<II',raw,12);doc=json.loads(raw[20:20+size]);offset=20+size
 size,kind=struct.unpack_from('<II',raw,offset);assert kind==0x004e4942
 return doc,bytearray(raw[offset+8:offset+8+size])
def merge(base,walk,out):
 doc,binary=read(base);src,data=read(walk)
 before=copy.deepcopy(doc);original_binary=bytes(binary)
 nodes={n['name']:i for i,n in enumerate(doc['nodes']) if 'name' in n}
 animation=copy.deepcopy(next(a for a in src['animations'] if a['name']=='garden_walk'))
 original=next(a for a in doc['animations'] if a['name']=='garden_walk')
 # Facial geometry may have been revised since the walk authoring source.
 # Retain its matching eyelid controls, including optional lower-lid joints.
 animation['channels']=[c for c in animation['channels'] if not src['nodes'][c['target']['node']]['name'].startswith('blink.')]
 accessors={};views={}
 def accessor(index):
  if index in accessors:return accessors[index]
  a=copy.deepcopy(src['accessors'][index]);assert 'sparse' not in a
  vi=a['bufferView']
  if vi not in views:
   v=copy.deepcopy(src['bufferViews'][vi]);start=v.get('byteOffset',0);length=v['byteLength']
   binary.extend(b'\0'*((-len(binary))%4));v['byteOffset']=len(binary);v['buffer']=0
   binary.extend(data[start:start+length]);views[vi]=len(doc['bufferViews']);doc['bufferViews'].append(v)
  a['bufferView']=views[vi];accessors[index]=len(doc['accessors']);doc['accessors'].append(a);return accessors[index]
 for sampler in animation['samplers']:
  for field in ['input','output']:sampler[field]=accessor(sampler[field])
 for channel in animation['channels']:
  name=src['nodes'][channel['target']['node']]['name'];assert name in nodes
  channel['target']['node']=nodes[name]
 for channel in original['channels']:
  if doc['nodes'][channel['target']['node']]['name'].startswith('blink.'):
   preserved_channel=copy.deepcopy(channel);preserved_channel['sampler']=len(animation['samplers'])
   animation['samplers'].append(copy.deepcopy(original['samplers'][channel['sampler']]))
   animation['channels'].append(preserved_channel)
 i=next(i for i,a in enumerate(doc['animations']) if a['name']=='garden_walk');doc['animations'][i]=animation
 doc['buffers'][0]['byteLength']=len(binary)
 assert binary[:len(original_binary)]==original_binary
 for key in ['nodes','meshes','skins','materials','textures','images']:assert doc.get(key)==before.get(key),key
 assert [a for a in doc['animations'] if a['name']!='garden_walk']==[a for a in before['animations'] if a['name']!='garden_walk']
 encoded=json.dumps(doc,separators=(',',':')).encode();encoded+=b' '*((-len(encoded))%4);binary.extend(b'\0'*((-len(binary))%4))
 body=struct.pack('<II',len(encoded),0x4e4f534a)+encoded+struct.pack('<II',len(binary),0x004e4942)+binary
 Path(out).write_bytes(struct.pack('<III',0x46546c67,2,12+len(body))+body)
 print(json.dumps({'base_sha256':hashlib.sha256(Path(base).read_bytes()).hexdigest(),'output_sha256':hashlib.sha256(Path(out).read_bytes()).hexdigest(),'preserved':'geometry, materials, rig and every non-walk action','output':str(out)},indent=2))
if __name__=='__main__':merge(*sys.argv[1:])

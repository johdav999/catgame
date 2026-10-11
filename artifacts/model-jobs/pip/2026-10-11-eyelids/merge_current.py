"""Keep every current non-eyelid animation channel byte-for-byte.

Usage: python merge_current.py current.glb eyelids.glb output.glb
Geometry, skin and four eyelid channels come from the eyelid candidate. Every
other animation channel comes from the current runtime, matched by node name.
"""
import json,struct,copy,sys,hashlib
from pathlib import Path
FACIAL={'blink.L','blink.R','blink.lower.L','blink.lower.R'}
def read(path):
 raw=Path(path).read_bytes();size,kind=struct.unpack_from('<II',raw,12);doc=json.loads(raw[20:20+size]);offset=20+size
 size,kind=struct.unpack_from('<II',raw,offset);assert kind==0x004e4942
 return doc,bytearray(raw[offset+8:offset+8+size])
def write(path,doc,data):
 doc['buffers'][0]['byteLength']=len(data);encoded=json.dumps(doc,separators=(',',':')).encode();encoded+=b' '*(-len(encoded)%4);data.extend(b'\0'*(-len(data)%4))
 body=struct.pack('<II',len(encoded),0x4e4f534a)+encoded+struct.pack('<II',len(data),0x004e4942)+data
 Path(path).write_bytes(struct.pack('<III',0x46546c67,2,12+len(body))+body)
def merge(current,candidate,output):
 src,data=read(current);doc,binary=read(candidate);nodes={n['name']:i for i,n in enumerate(doc['nodes']) if 'name' in n}
 before={k:copy.deepcopy(doc.get(k)) for k in ['meshes','skins','materials','nodes','images','textures']}
 views={};accessors={};preserved=[]
 def accessor(index):
  if index in accessors:return accessors[index]
  a=copy.deepcopy(src['accessors'][index]);assert 'sparse' not in a;vi=a['bufferView']
  if vi not in views:
   v=copy.deepcopy(src['bufferViews'][vi]);start=v.get('byteOffset',0);length=v['byteLength'];binary.extend(b'\0'*(-len(binary)%4));v['byteOffset']=len(binary);v['buffer']=0
   binary.extend(data[start:start+length]);views[vi]=len(doc['bufferViews']);doc['bufferViews'].append(v)
   assert binary[v['byteOffset']:v['byteOffset']+length]==data[start:start+length]
  a['bufferView']=views[vi];accessors[index]=len(doc['accessors']);doc['accessors'].append(a);return accessors[index]
 for animation in doc['animations']:
  old=next(a for a in src['animations'] if a['name']==animation['name'])
  animation['channels']=[c for c in animation['channels'] if doc['nodes'][c['target']['node']].get('name') in FACIAL]
  count=0
  for channel in old['channels']:
   name=src['nodes'][channel['target']['node']].get('name')
   if name in FACIAL:continue
   # Blender's renamed mesh wrapper has no authored animation; bone channels
   # require an exact name match rather than an assumed joint index.
   assert name in nodes,(name,animation['name'])
   sampler=copy.deepcopy(old['samplers'][channel['sampler']])
   for field in ['input','output']:sampler[field]=accessor(sampler[field])
   out=copy.deepcopy(channel);out['target']['node']=nodes[name];out['sampler']=len(animation['samplers']);animation['samplers'].append(sampler);animation['channels'].append(out);count+=1
  preserved.append({'clip':animation['name'],'unchanged_non_facial_channels':count})
 for k,v in before.items():assert doc.get(k)==v,k
 write(output,doc,binary)
 report={'runtime_base_sha256':hashlib.sha256(Path(current).read_bytes()).hexdigest(),'candidate_sha256':hashlib.sha256(Path(candidate).read_bytes()).hexdigest(),'preserved':preserved,'output_sha256':hashlib.sha256(Path(output).read_bytes()).hexdigest()}
 Path(output).with_suffix('.merge.json').write_text(json.dumps(report,indent=2));print(json.dumps(report))
if __name__=='__main__':merge(*sys.argv[1:])

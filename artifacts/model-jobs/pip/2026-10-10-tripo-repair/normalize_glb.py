"""Shift glTF clip input times to zero; preserve all poses and other payloads."""
import json,struct,sys
from pathlib import Path
def normalize(path):
 path=Path(path);raw=path.read_bytes();magic,version,total=struct.unpack_from('<III',raw)
 assert magic==0x46546c67 and version==2 and total==len(raw)
 chunks=[];offset=12
 while offset<len(raw):
  size,kind=struct.unpack_from('<II',raw,offset);chunks.append([kind,bytearray(raw[offset+8:offset+8+size])]);offset+=8+size
 doc=json.loads(chunks[0][1]);binary=next(c[1] for c in chunks if c[0]==0x004e4942)
 inputs={s['input'] for a in doc.get('animations',[]) for s in a['samplers']}
 for index in inputs:
  acc=doc['accessors'][index];assert acc['componentType']==5126 and acc['type']=='SCALAR' and 'sparse' not in acc
  view=doc['bufferViews'][acc['bufferView']];assert view.get('buffer',0)==0
  start=view.get('byteOffset',0)+acc.get('byteOffset',0);stride=view.get('byteStride',4)
  times=[struct.unpack_from('<f',binary,start+i*stride)[0] for i in range(acc['count'])]
  first=min(times);times=[0.0 if abs(t-first)<1e-7 else t-first for t in times]
  for i,t in enumerate(times):struct.pack_into('<f',binary,start+i*stride,t)
  acc['min']=[min(times)];acc['max']=[max(times)]
 encoded=json.dumps(doc,separators=(',',':')).encode();encoded+=b' '*((-len(encoded))%4);chunks[0][1]=encoded
 body=b''.join(struct.pack('<II',len(data),kind)+bytes(data) for kind,data in chunks)
 path.write_bytes(struct.pack('<III',magic,version,12+len(body))+body)
 print('Normalized zero-based clip times:',path)
if __name__=='__main__':normalize(sys.argv[1])


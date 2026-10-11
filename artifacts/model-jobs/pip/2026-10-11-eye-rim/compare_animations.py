import json,struct,math
from pathlib import Path
J=Path(__file__).resolve().parent
ROOT=J.parents[3]
def read(p):
 b=p.read_bytes();size=struct.unpack_from('<I',b,12)[0];d=json.loads(b[20:20+size]);off=20+size;binary=b[off+8:]
 def values(i):
  a=d['accessors'][i];v=d['bufferViews'][a['bufferView']];n={'SCALAR':1,'VEC3':3,'VEC4':4}[a['type']];start=v.get('byteOffset',0)+a.get('byteOffset',0);stride=v.get('byteStride',n*4);return [struct.unpack_from('<'+'f'*n,binary,start+j*stride) for j in range(a['count'])]
 result={}
 for clip in d['animations']:
  for channel in clip['channels']:
   target=channel['target'];name=d['nodes'][target['node']]['name']
   sampler=clip['samplers'][channel['sampler']];result[(clip['name'],name,target['path'])]=([t[0] for t in values(sampler['input'])],values(sampler['output']))
 return result
def sample(track,t):
 ts,vs=track
 if t<=ts[0]:return vs[0]
 for i in range(1,len(ts)):
  if ts[i]>=t:
   a=(t-ts[i-1])/(ts[i]-ts[i-1]);return tuple(x+(y-x)*a for x,y in zip(vs[i-1],vs[i]))
 return vs[-1]
a=read(J/'before.glb');b=read(J/'pip-blink.glb');assert a.keys()==b.keys(),(len(a),len(b));maximum=0
for key in a:
 for step in range(121):
  t=a[key][0][-1]*step/120;x=sample(a[key],t);y=sample(b[key],t);diff=max(abs(i-j) for i,j in zip(x,y))
  if key[2]=='rotation':diff=min(diff,max(abs(i+j) for i,j in zip(x,y)))
  maximum=max(maximum,diff)
assert maximum<.00001,maximum
report={'all_channels_including_blink':len(a),'samples_per_channel':121,'max_component_error':maximum}
(J/'animation-preservation.json').write_text(json.dumps(report,indent=2));print(report)

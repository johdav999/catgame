"""Binary export gate independent of Blender and the native importer."""
import argparse,hashlib,json,math,struct
from pathlib import Path
ROOT=Path(__file__).resolve().parents[3]

def read_glb(path):
    raw=path.read_bytes();magic,version,length=struct.unpack_from('<III',raw)
    assert magic==0x46546c67 and version==2 and length==len(raw),'Invalid GLB header'
    chunks={};at=12
    while at<len(raw):
        size,kind=struct.unpack_from('<II',raw,at);chunks[kind]=raw[at+8:at+8+size];at+=8+size
    return json.loads(chunks[0x4e4f534a]),chunks[0x004e4942],raw

def accessor(doc,binary,index):
    a=doc['accessors'][index];v=doc['bufferViews'][a['bufferView']]
    assert 'sparse' not in a and v.get('buffer',0)==0
    code={5126:'f',5125:'I',5123:'H',5121:'B'}[a['componentType']]
    width={'SCALAR':1,'VEC2':2,'VEC3':3,'VEC4':4,'MAT4':16}[a['type']]
    fmt='<'+code*width;stride=v.get('byteStride',struct.calcsize(fmt))
    start=v.get('byteOffset',0)+a.get('byteOffset',0)
    return [struct.unpack_from(fmt,binary,start+i*stride) for i in range(a['count'])]

def validate(path):
    doc,binary,raw=read_glb(path);bind=json.loads((ROOT/'docs/release1/canonical-rig.json').read_text())
    assert len(doc.get('skins',[]))==1,'Exactly one canonical skin required'
    skin=doc['skins'][0];nodes=doc['nodes'];parent={c:i for i,n in enumerate(nodes) for c in n.get('children',[])}
    joints={nodes[i]['name']:(i,ib) for i,ib in zip(skin['joints'],accessor(doc,binary,skin['inverseBindMatrices']))}
    assert set(joints)=={j['name'] for j in bind['joints']},'Canonical bone names differ'
    def near(a,b):return len(a)==len(b) and all(math.isfinite(x) and abs(x-y)<=bind['tolerance'] for x,y in zip(a,b))
    for ref in bind['joints']:
        i,ib=joints[ref['name']];node=nodes[i]
        assert nodes[parent[i]]['name']==ref['parent'],'Canonical hierarchy differs'
        for key,default in [('translation',[0,0,0]),('rotation',[0,0,0,1]),('scale',[1,1,1])]:
            assert near(node.get(key,default),ref[key]),'Canonical '+key+' differs: '+ref['name']
        assert near(ib,ref['inverseBindMatrix']),'Canonical inverse bind differs: '+ref['name']
    contract=json.loads((ROOT/'docs/release1/contract.json').read_text())
    required=contract['rig']['commonActions']+contract['rig']['extraActions']+['personality_reaction']
    assert set(required)=={a['name'] for a in doc['animations']},'Action set differs'
    for anim in doc['animations']:
        for channel in anim['channels']:
            assert channel['target']['path'] in ['translation','rotation','scale'],'Unsupported morph animation'
            assert channel['target']['node'] in skin['joints'],'Animation targets a non-bone node'
        for sampler in anim['samplers']:
            times=[x[0] for x in accessor(doc,binary,sampler['input'])]
            assert abs(times[0])<1e-7 and all(a<b for a,b in zip(times,times[1:])),'Invalid sampler times'
            assert all(all(math.isfinite(v) for v in values) for values in accessor(doc,binary,sampler['output'])),'Nonfinite animation'
    assert not doc.get('extensionsRequired'),'Unsupported required extension'
    assert all(i.get('mimeType') in ['image/png','image/jpeg'] and 'bufferView' in i for i in doc.get('images',[])),'Textures must be embedded PNG/JPEG'
    triangles=0
    for mesh in doc['meshes']:
        for p in mesh['primitives']:
            assert p.get('mode',4)==4 and not p.get('targets'),'Only triangle skeletal meshes supported'
            assert {'POSITION','NORMAL','TEXCOORD_0','JOINTS_0','WEIGHTS_0'}<=set(p['attributes']),'Missing vertex stream'
            assert 'JOINTS_1' not in p['attributes'] and 'WEIGHTS_1' not in p['attributes'],'More than four influences'
            triangles+=doc['accessors'][p['indices']]['count']//3
            for w in accessor(doc,binary,p['attributes']['WEIGHTS_0']):assert all(v>=0 for v in w) and abs(sum(w)-1)<1e-5,'Invalid weights'
    assert triangles<=28000,'Triangle budget'
    return {'file':str(path),'bytes':len(raw),'sha256':hashlib.sha256(raw).hexdigest(),'triangles':triangles,
            'materials':len(doc.get('materials',[])),'embeddedImages':len(doc.get('images',[])),
            'bones':len(joints),'actions':required,'canonicalBind':'passed','zeroBasedSamplers':'passed'}

if __name__=='__main__':
    parser=argparse.ArgumentParser();parser.add_argument('files',nargs='+',type=Path);parser.add_argument('--report',type=Path)
    args=parser.parse_args();report=[validate(p) for p in args.files]
    if args.report:args.report.parent.mkdir(parents=True,exist_ok=True);args.report.write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps(report,indent=2))

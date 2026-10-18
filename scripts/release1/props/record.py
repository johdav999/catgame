"""Record measured payload/material evidence and verify the namespaced handoff."""
from pathlib import Path
import hashlib,json,struct,io,shutil
from PIL import Image,ImageDraw
ROOT=Path(__file__).resolve().parents[3];JOB=ROOT/'artifacts/model-jobs/release1-props/2026-10-11'
f=json.loads((ROOT/'docs/release1/fragments/props.json').read_text());contract=json.loads((ROOT/'docs/release1/contract.json').read_text())
digest=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
ids={a['id'] for a in f['assets']+f['reuse']};rows=[];textures=[]
for a in f['assets']:
    p=ROOT/'assets/runtime'/a['url'][7:];raw=p.read_bytes();assert len(raw)==a['bytes'] and digest(p)==a['sha256']
    size=struct.unpack_from('<I',raw,12)[0];g=json.loads(raw[20:20+size]);binary=raw[28+size:]
    images=[]
    for im in g.get('images',[]):
        bv=g['bufferViews'][im['bufferView']];data=binary[bv.get('byteOffset',0):bv.get('byteOffset',0)+bv['byteLength']]
        image=Image.open(io.BytesIO(data));assert image.width<=512 and image.height<=512
        images.append({'name':im.get('name',''),'width':image.width,'height':image.height,'encodedBytes':len(data),'sha256':hashlib.sha256(data).hexdigest()})
    a['images']=images;textures+=images
    assert a['triangles']<=12000,a['name']
    rows.append(f"| {a['name']} | {a['triangles']:,} | {a['vertices']:,} | {a['materialCount']} | {len(images)} | {a['bytes']:,} |")
for group,parts in f['assemblies'].items():
    for part in parts:
        assert part['asset'] in ids,(group,part['asset'])
        assert all(len(part[k])==3 for k in ('position','rotationDegrees','scale'))
for q in contract['quests'][3:]:
    for role in q['roles']:assert role in f['roleBindings'],role
assert [(i['id'],i['price']) for i in f['catalog']]==[(i['id'],i['price']) for i in contract['items']]
thumb=[]
for item in f['catalog']:
    p=ROOT/'assets/runtime'/item['thumbnail'][7:];im=Image.open(p);assert im.size==(512,512)
    thumb.append({'id':item['id'],'url':item['thumbnail'],'sha256':digest(p),'bytes':p.stat().st_size,'dimensions':[512,512]})
sources=[{'path':str(p.relative_to(ROOT)).replace('\\','/'),'sha256':digest(p),'bytes':p.stat().st_size} for folder in ['maps','sources','scripts'] for p in sorted((JOB/folder).iterdir()) if p.is_file()]
browser=json.loads((ROOT/'build/evidence/release1/prompt-03/report.json').read_text());assert not browser.get('failure') and not browser['errors']
for a in browser['assets']:
    runtime=ROOT/'assets/runtime'/a['url'][7:];assert a['sha256']==digest(runtime)
report={'schema':'paws-prop-evidence/1','revision':f['revision'],'enginePin':contract['enginePin'],'newAssets':len(f['assets']),'reusedAssets':len(f['reuse']),
 'newTriangles':sum(a['triangles'] for a in f['assets']),'newGlbBytes':sum(a['bytes'] for a in f['assets']),
 'embeddedImages':len(textures),'uniqueImageIdentities':len({a['sha256'] for a in textures}),
 'decodedRgbaBytesBeforeCrossFileSharing':sum(i['width']*i['height']*4 for i in textures),
 'master':{'path':f['provenance']['master'],'sha256':digest(ROOT/f['provenance']['master'])},'sources':sources,'thumbnails':thumb,
 'wasmSha256':digest(ROOT/'build/web-release1-p03-pinned-release/site/release1_preview.wasm'),
 'fragmentSha256':digest(ROOT/'docs/release1/fragments/props.json'),'browser':browser,'pending':f['pending']}
(JOB/'validation.json').write_text(json.dumps(report,indent=2)+'\n');(JOB/'material-inventory.json').write_text(json.dumps([{'asset':a['id'],'materials':a['materials'],'images':a['images']} for a in f['assets']],indent=2)+'\n')
(ROOT/'assets/runtime/models/release1/props/thumbnails/manifest.json').write_text(json.dumps(thumb,indent=2)+'\n')
(JOB/'BUDGETS.md').write_text('# Measured Prompt 3 asset budgets\n\n'+f"Revision {f['revision']}: {report['newAssets']} new GLBs, {report['newTriangles']:,} triangles, {report['newGlbBytes']:,} bytes. Existing 21 assets retain their original identities.\n\n"+
 '| Asset | Triangles | Vertices | Materials | Embedded images | GLB bytes |\n| --- | ---: | ---: | ---: | ---: | ---: |\n'+'\n'.join(rows)+'\n\n'+
 f"All color maps are at most 512 square. {len(textures)} embedded images / {report['uniqueImageIdentities']} distinct encoded identities. Unshared RGBA decoding estimate: {report['decodedRgbaBytesBeforeCrossFileSharing']:,} bytes, excluding meshes, mip levels and staging. GLB image duplication is measured; cross-file GPU sharing is not assumed. See material-inventory.json and validation.json for texture dimensions and runtime counters.\n\n"+
 'Every individual mesh remains below 12,000 triangles. Bells and barrel use four material slots; the hammock uses five for wood, painted frame, cloth, piping and ties. These deliberate exceptions to a three-material starting target retain readable surface differences and are measured above. The entire workshop is an inspection scene, not a representative simultaneous gameplay load or physical-device performance acceptance.\n')
evidence=ROOT/'build/evidence/release1/prompt-03';shutil.copytree(evidence,JOB/'browser-evidence',dirs_exist_ok=True)
names=[a['name'] for a in f['assets']]
for start in range(0,len(names),15):
    canvas=Image.new('RGB',(1500,1050),'#fff8e9');draw=ImageDraw.Draw(canvas)
    for j,name in enumerate(names[start:start+15]):
        im=Image.open(evidence/(name+'.png'));im.thumbnail((500,185));x=j%3*500;y=j//3*210;canvas.paste(im,(x,y+20));draw.text((x+8,y+2),name,fill='#253e37')
    canvas.save(JOB/'browser-evidence'/f'contact-sheet-{start}.png')
print(json.dumps({k:report[k] for k in ['revision','newAssets','newTriangles','newGlbBytes','embeddedImages','uniqueImageIdentities','decodedRgbaBytesBeforeCrossFileSharing']}))

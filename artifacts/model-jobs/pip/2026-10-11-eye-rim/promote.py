import json,hashlib,shutil,struct
from pathlib import Path
J=Path(__file__).resolve().parent;ROOT=J.parents[3];R=ROOT/'assets/runtime/models/pip'
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
assert sha(R/'pip.glb')==sha(J/'before.glb'),'Runtime changed during repair'
assert json.loads((J/'animation-preservation.json').read_text())['max_component_error']==0
closure=json.loads((J/'r21-export-visibility.json').read_text())
assert all(x['visible_eye_faces']==0 for x in closure if x['amount']==1)
assert all(x['reduction']>.9 for x in json.loads((J/'rim-comparison.json').read_text()))
backup=J/'previous-runtime';backup.mkdir(exist_ok=True)
for p in R.iterdir():
 if p.is_file():shutil.copy2(p,backup/p.name)
fbx=ROOT/'assets/Pip/pip-repaired.fbx';shutil.copy2(fbx,backup/fbx.name)
shutil.copy2(J/'pip-blink.glb',R/'pip.glb');shutil.copy2(J/'pip-blink.fbx',fbx)
for p in (J/'portraits').glob('portrait-*.png'):shutil.copy2(p,R/p.name)
m=json.loads((J/'before-manifest.json').read_text());measure=json.loads((J/'measurements.json').read_text())
binary=(R/'pip.glb').read_bytes();size=struct.unpack_from('<I',binary,12)[0];gltf=json.loads(binary[20:20+size])
vertices=sum(gltf['accessors'][p['attributes']['POSITION']]['count'] for mesh in gltf['meshes'] for p in mesh['primitives'])
m['previous_facial_revision']=json.loads(json.dumps(m['facial_revision']))
m.update(revision='pip-fitted-blink-r21',source_master=(J/'pip-blink.blend').relative_to(ROOT).as_posix(),triangles=measure['triangles'],source_vertices=measure['vertices'],engine_vertices=vertices,materials=measure['materials'])
m['facial_revision'].update(revision='recessed-eyelid-seam-r21',job=J.relative_to(ROOT).as_posix(),upstream_master=(J/'before.blend').relative_to(ROOT).as_posix(),upstream_master_sha256=sha(J/'before.blend'),deformation='Original fitted facial skin and blink channels preserved; tucked microfold opens continuously with upper/lower lid influences; original facial shading normals isolated from fold; same clean overlapping closure',preserved_all_animation_channels=837,preservation_max_component_error=0)
m['validation']={'job':J.relative_to(ROOT).as_posix(),'native_glb':'validation-native.txt','native_fbx':'validation-fbx.txt','rig_glb':'rig-glb.txt','rig_fbx':'rig-fbx.txt','clean_reimport':'clean-reimport.json','closure':'r21-export-visibility.json','closure_positions':41,'closure_eye_view_combinations':6,'exposed_eye_faces_at_closed':0,'open_rim_comparison':'rim-comparison.json','all_animation_channels':'animation-preservation.json','browser':'browser/review.json','browser_video':'browser/blink-three-views-4x-slow.webm','physical_mobile':'not tested'}
paths=[J/'pip-blink.blend',fbx,R/'pip.glb',*sorted(R.glob('portrait-*.png'))]
m['files']=[{'path':p.relative_to(ROOT).as_posix(),'bytes':p.stat().st_size,'sha256':sha(p)} for p in paths]
for p in [R/'manifest.json',J/'asset-manifest.json']:p.write_text(json.dumps(m,indent=2)+'\n')
notice=R/'NOTICE.md';notice.write_text(notice.read_text()+'\nThe open-eye rim correction retains all supplied coat maps and accepted animations. Source, comparison and export evidence: artifacts/model-jobs/pip/2026-10-11-eye-rim/PROVENANCE.md.\n')
print(json.dumps({'revision':m['revision'],'sha256':sha(R/'pip.glb'),'vertices':vertices,'bytes':len(binary)}))

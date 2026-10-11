import json,hashlib,shutil
from pathlib import Path
J=Path(__file__).resolve().parent;ROOT=J.parents[3];R=ROOT/'assets/runtime/models/pip';sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
assert sha(R/'pip.glb')==sha(J/'concurrent.glb'),'Runtime changed since preservation check'
m=json.loads((R/'manifest.json').read_text());measure=json.loads((J/'measurements.json').read_text());backup=J/'previous-runtime';backup.mkdir(exist_ok=True)
for p in R.iterdir():
 if p.is_file() and not (backup/p.name).exists():shutil.copy2(p,backup/p.name)
fbx=ROOT/'assets/Pip/pip-repaired.fbx'
if not (backup/fbx.name).exists():shutil.copy2(fbx,backup/fbx.name)
shutil.copy2(J/'pip-blink.glb',R/'pip.glb');shutil.copy2(J/'pip-blink.fbx',fbx)
for p in (J/'portraits').glob('portrait-*.png'):shutil.copy2(p,R/p.name)
m['previous_facial_revision']=m.get('facial_revision');m.update(revision='pip-fitted-blink-r15',source_master=(J/'pip-blink.blend').relative_to(ROOT).as_posix(),triangles=measure['triangles'],source_vertices=measure['vertices'],engine_vertices=12240,materials=measure['materials'],bones=measure['bones'])
m['facial_revision']={'revision':'fitted-native-skin-r15','job':J.relative_to(ROOT).as_posix(),'upstream_master':(J/'native-source.blend').relative_to(ROOT).as_posix(),'upstream_master_sha256':sha(J/'native-source.blend'),'upper_controls':['blink.L','blink.R'],'lower_controls':['blink.lower.L','blink.lower.R'],'parent':'head','deformation':'smoothed fitted skin boundaries; geodesic weight falloff into brow and cheek; shared closure surface; connected overlapping lid margin; stationary eye islands; unit scales','preserved_non_facial_channels':729,'preservation_max_component_error':0,'idle_blink_seconds':{'start':2.4,'close':.1,'hold':2/30,'open':.2}}
m['validation']={'job':J.relative_to(ROOT).as_posix(),'native_glb':'validation-native.txt','native_fbx':'validation-fbx.txt','rig_glb':'rig-glb.txt','rig_fbx':'rig-fbx.txt','clean_reimport':'clean-reimport.json','closure':'r15-export-visibility.json','closure_positions':41,'closure_eye_view_combinations':6,'exposed_eye_faces_at_closed':0,'browser':'browser/review.json','browser_video':'browser/blink-three-views-4x-slow.webm','physical_mobile':'not tested'}
paths=[J/'pip-blink.blend',fbx,R/'pip.glb',*sorted(R.glob('portrait-*.png'))];m['files']=[{'path':p.relative_to(ROOT).as_posix(),'bytes':p.stat().st_size,'sha256':sha(p)} for p in paths]
for p in [R/'manifest.json',J/'asset-manifest.json']:p.write_text(json.dumps(m,indent=2)+'\n')
notice=R/'NOTICE.md';notice.write_text(notice.read_text()+'\nThe fitted blink revision and its original-skin/UV provenance are recorded in artifacts/model-jobs/pip/2026-10-11-blink-production/PROVENANCE.md. The supplied source and previous runtime exports are preserved.\n')
print(json.dumps({'revision':m['revision'],'sha256':sha(R/'pip.glb'),'triangles':m['triangles'],'vertices':m['source_vertices']}))

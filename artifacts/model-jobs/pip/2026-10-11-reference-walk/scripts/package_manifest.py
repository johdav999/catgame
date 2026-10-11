import json,hashlib,subprocess
from pathlib import Path
JOB=Path(__file__).resolve().parents[1];ROOT=JOB.parents[3]
def info(path):
 p=Path(path);return {'path':p.relative_to(ROOT).as_posix(),'bytes':p.stat().st_size,'sha256':hashlib.sha256(p.read_bytes()).hexdigest()}
spec=json.loads((JOB/'references/walk-spec.json').read_text())
manifest={'schema':'paws-animation-revision/1','asset_id':'cats/pip@1','revision':'reference-walk-r1','created':'2026-10-11',
 'engine_pin':subprocess.check_output(['git','-C',str(ROOT/'external/WebEngine'),'rev-parse','HEAD'],text=True).strip(),
 'clip':'garden_walk','root_policy':'in-place, application-owned translation','duration_seconds':1.2,'fps':30,'stride_metres':.36,'speed_metres_per_second':.30,
 'source_axes':{'up':'+Z','forward':'-Y'},'runtime_axes':{'up':'+Y','forward':'+Z'},'units':'metres',
 'rig':'original 29 deform bones, unchanged bind/hierarchy; 31 engine joints','triangles':17854,'materials':2,'texture_policy':'existing packed maps reused unchanged',
 'contact_phase':spec['contact_phase'],'stance_fraction':spec['stance_fraction'],
 'validation':{'walk_native':'logs/native-walk.txt','scene_native':'logs/native-scenes.txt','clean_reimport':'exports/clean-reimport.json','browser':'browser-evidence/walk-report.json','game':'game-evidence-final/report.json'},
 'limits':['Interpreted reference reconstruction, not motion capture','Straight constant-speed contacts; no world-space foot IK through waypoint turns or blends','Concurrent new settle/rest assertions fail unchanged r8 cushion clips','Physical mobile not tested'],
 'files':[info(JOB/p) for p in ['pip-walk-r1.blend','exports/runtime-before.glb','exports/pip.glb','references/muybridge-716.jpg','references/walk-spec.json','walk-review.mp4','browser-walk.mp4','garden-walk.mp4']]}
(JOB/'asset-manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
runtime=ROOT/'assets/runtime/models/pip/pip.glb';catalogue=ROOT/'assets/runtime/models/pip/manifest.json';data=json.loads(catalogue.read_text())
# Read current metadata, preserve other revisions and modify only walk ownership/hash.
data.setdefault('animation_revisions',{})['garden_walk']={
 'revision':'reference-walk-r1','source_master':info(JOB/'pip-walk-r1.blend')['path'],'job':JOB.relative_to(ROOT).as_posix(),
 'duration_seconds':1.2,'stride_metres':.36,'speed_metres_per_second':.30,'provenance':(JOB/'PROVENANCE.md').relative_to(ROOT).as_posix()}
if data.get('revision')=='pip-tripo-repaired-r8':data['revision']='pip-tripo-r8-reference-walk-r1'
for f in data['files']:
 if f['path']=='assets/runtime/models/pip/pip.glb':f.update(info(runtime))
catalogue.write_text(json.dumps(data,indent=2)+'\n')
print(json.dumps({'runtime':info(runtime),'manifest':str(JOB/'asset-manifest.json')},indent=2))

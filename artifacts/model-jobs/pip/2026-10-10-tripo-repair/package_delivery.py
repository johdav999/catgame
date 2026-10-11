"""Package the verified revision; retain original exports and prior runtime."""
from pathlib import Path
import json,hashlib,shutil,struct
JOB=Path(__file__).parent;ROOT=JOB.parents[3];RUNTIME=ROOT/'assets/runtime/models/pip'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
shutil.copy2(JOB/'pip-repaired-r8.blend',JOB/'pip-repaired.blend')
shutil.copy2(JOB/'exports/pip-repaired-r8.glb',JOB/'exports/pip-repaired.glb')
data=json.loads((JOB/'exports/measurements-r8.json').read_text())
manifest={'schema':'paws-authored-asset/1','asset_id':'cats/pip@1','revision':'pip-tripo-repaired-r8','created':'2026-10-10','engine_pin':'3fe7d1dc1dc610c703944803cbb717cd65024ec9','runtime_url':'assets/models/pip/pip.glb','units':'metres','source_axes':{'up':'+Z','forward':'-Y'},'runtime_axes':{'up':'+Y','forward':'+Z'},'pivot':'ground-centred canonical root below torso','root_policy':'in-place root; movement remains application-owned','rig_identity':'exact Paws cat rig v1 bone names, hierarchy, metre-scale bind pose','bones':data['bones'],'clips':data['clips'],'triangles':data['triangles'],'source_vertices':data['vertices'],'engine_vertices':10407,'materials':2,'max_influences':4,'unweighted_vertices':0,'source_fbx':{'path':'assets/Pip/pip.fbx','sha256':sha(ROOT/'assets/Pip/pip.fbx')},'source_transform':data['source_transform'],'textures':data['textures'],'texture_decoded_budget_estimate':{'glb_bytes':3*1024*1024*4+64*64*4,'fbx_bytes_including_generated_mr':5*1024*1024*4+64*64*4,'default_fbx_limit_bytes':128*1024*1024,'excludes_mips_and_gpu_copies':True},'rest_bounds_runtime':{'minimum':[-.199805886,0,-.618297637],'maximum':[.199805886,1.05300915,.635533571]},'source_master':str((JOB/'pip-repaired.blend').relative_to(ROOT)).replace('\\','/'),'validation':{'native_fbx':'validation-fbx.txt','native_glb':'validation-glb.txt','rig_fbx':'rig-fbx.txt','rig_glb':'rig-glb.txt','clean_reimport':'exports/clean-reimport.json','browser':'browser-evidence/pip-smoke.json','physical_mobile':'not tested','furnishing_fit':'not tested'},'files':[]}
for p in [JOB/'pip-repaired.blend',ROOT/'assets/Pip/pip-repaired.fbx',RUNTIME/'pip.glb']+[RUNTIME/('portrait-'+n+'.png') for n in ['dialogue','shop','ending']]:
 manifest['files'].append({'path':str(p.relative_to(ROOT)).replace('\\','/'),'bytes':p.stat().st_size,'sha256':sha(p)})
(JOB/'asset-manifest.json').write_text(json.dumps(manifest,indent=2))
(RUNTIME/'manifest.json').write_text(json.dumps(manifest,indent=2))
evidence=JOB/'browser-evidence';evidence.mkdir(exist_ok=True)
for name in ['pip-smoke.json','desktop.png','static-0.png','static-5.png','playing-3.png','shadow-free-close.png','shadow-free-puzzle.png']:
 source=ROOT/'build/evidence/pip-tripo-repair'/name
 if source.exists():shutil.copy2(source,evidence/name)
print(json.dumps(manifest,indent=2))



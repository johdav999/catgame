"""Promote a validated eyelid export only if its animation base is still current."""
import json,hashlib,shutil
from pathlib import Path
job=Path(__file__).resolve().parent;root=job.parents[3]
runtime=root/'assets/runtime/models/pip';exports=job/'exports'
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
merge=json.loads((exports/'pip-merged.merge.json').read_text())
assert sha(runtime/'pip.glb')==merge['runtime_base_sha256'],'Runtime changed; remerge and validate before promotion'
backup=job/'previous-runtime';backup.mkdir(exist_ok=True)
for p in runtime.iterdir():
 if p.is_file() and not (backup/p.name).exists():shutil.copy2(p,backup/p.name)
fbx=root/'assets/Pip/pip-repaired.fbx'
if not (backup/fbx.name).exists():shutil.copy2(fbx,backup/fbx.name)
manifest=json.loads((runtime/'manifest.json').read_text());measure=json.loads((exports/'measurements.json').read_text())
shutil.copy2(exports/'pip-merged.glb',runtime/'pip.glb');shutil.copy2(exports/'pip-eyelids.fbx',fbx)
for p in (job/'portraits').glob('*.png'):shutil.copy2(p,runtime/p.name)
manifest.update(revision='pip-eyelids-r24-with-current-actions',source_master=(exports/'pip-eyelids.blend').relative_to(root).as_posix(),rig_identity='Paws cat rig v1: all 29 original names, hierarchy and bind transforms unchanged; two optional lower-eyelid controls',bones=measure['bones'],triangles=measure['triangles'],source_vertices=measure['vertices'],engine_vertices=11766,materials=measure['materials'])
manifest['facial_revision']={'revision':'native-skin-eyelids-r24','job':job.relative_to(root).as_posix(),'upper_controls':['blink.L','blink.R'],'lower_controls':['blink.lower.L','blink.lower.R'],'parent':'head','deformation':'translation, unit scale, original coat UVs and stationary eye islands','idle_blink_seconds':{'start':2.4,'close':0.1,'hold':2/30,'open':0.2},'preserved_non_facial_channels':729}
manifest['textures']=[t for t in manifest['textures'] if t['size']==[1024,1024]]
manifest['texture_decoded_budget_estimate'].update(glb_bytes=12582912,fbx_bytes_including_generated_mr=20971520)
manifest['validation']={'job':job.relative_to(root).as_posix(),'native_fbx':'validation-fbx.txt','native_glb':'validation-glb.txt','rig_fbx':'rig-fbx.txt','rig_glb':'rig-glb.txt','clean_reimport':'exports/clean-reimport.json','blink_browser':'browser-evidence/blink-review.json','browser':'browser-evidence/pip-smoke.json','physical_mobile':'not tested'}
paths=[exports/'pip-eyelids.blend',fbx,runtime/'pip.glb',*sorted(runtime.glob('portrait-*.png'))]
manifest['files']=[{'path':p.relative_to(root).as_posix(),'bytes':p.stat().st_size,'sha256':sha(p)} for p in paths]
for path in [runtime/'manifest.json',job/'asset-manifest.json']:path.write_text(json.dumps(manifest,indent=2)+'\n')
print(json.dumps({'revision':manifest['revision'],'runtime_sha256':sha(runtime/'pip.glb')}))

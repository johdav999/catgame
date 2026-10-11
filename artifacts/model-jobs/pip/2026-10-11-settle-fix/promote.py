"""Publish the exact reviewed GLB only if the concurrent source is unchanged."""
from pathlib import Path
import hashlib,json,shutil
J=Path(__file__).resolve().parent;R=J.parents[3]
runtime=R/'assets/runtime/models/pip/pip.glb'
reviewed=R/'build/web-settle-review/site/assets/models/pip/pip.glb'
digest=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
base='ec05b1df52bc916f738fca152eb0a951f41dd0478b8be499bb6d76eb3cbdd85a'
expected='83192482aae477fdaebf848436b63abd024ffca1110e4bfca02d3a6b99cbcdc2'
assert digest(runtime)==base,'Runtime changed; merge and validate again before promotion'
assert digest(reviewed)==expected,'Reviewed candidate changed'
shutil.copy2(runtime,J/'before-promotion.glb');shutil.copy2(reviewed,runtime)
manifest_path=runtime.with_name('manifest.json');manifest=json.loads(manifest_path.read_text())
layer={'revision':'connected-settle-r9','source_master':str((J/'pip-settle.blend').relative_to(R)).replace('\\','/'),'job':str(J.relative_to(R)).replace('\\','/'),'pose':'supported low rest; relaxed forelegs and tail alongside haunch; not a tight sleeping curl','preserves':'canonical bind, mesh, materials, current blink channels and every other action','validation':'tests/pip_settle_tests.cpp'}
for name in ['settle_cushion','curled_rest']:manifest.setdefault('animation_revisions',{})[name]=layer
for record in manifest['files']:
 if record['path']=='assets/runtime/models/pip/pip.glb':record.update(bytes=runtime.stat().st_size,sha256=expected)
manifest_path.write_text(json.dumps(manifest,indent=2)+'\n')
staged=[]
for site in (R/'build').glob('*/site'):
 dest=site/'assets/models/pip/pip.glb'
 if dest.exists() and digest(dest) in [base,expected,digest(J/'before.glb')]:
  if dest.resolve()!=reviewed.resolve():shutil.copy2(runtime,dest)
  shutil.copy2(manifest_path,dest.with_name('manifest.json'));staged.append(str(site.relative_to(R)))
report={'runtime_sha256':expected,'base_sha256':base,'staged_sites':staged,'triangles':17854,'deform_bones':29,'materials':2,'max_influences':4,'runtime_bytes':runtime.stat().st_size,'physical_mobile':'not tested','animation_layer':layer,'files':[]}
for path in [J/'pip-settle.blend',J/'pip-settle.glb',runtime]:report['files'].append({'path':str(path.relative_to(R)).replace('\\','/'),'sha256':digest(path),'bytes':path.stat().st_size})
(J/'asset-manifest.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))

"""Record this task's actual payload, inputs and final evidence (no stale pass)."""
from pathlib import Path
import hashlib,json,subprocess
ROOT=Path(__file__).resolve().parents[3]
SITE=ROOT/'build/web-release1-p06-pinned-release/site'
EVIDENCE=ROOT/'build/evidence/release1/prompt-06'
digest=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
report=json.loads((EVIDENCE/'browser-final/report.json').read_text())
assert not report.get('failure') and not report['errors'], 'Final browser evidence failed'
assert report['wasmSha256']==digest(SITE/'release1_preview.wasm'), 'Browser evidence is for a different payload'
files=['index.html','style.css','host.js','saves.js','engine-storage.js','release1_preview.js','release1_preview.wasm']
identity={'enginePin':subprocess.check_output(['git','-C',str(ROOT/'external/WebEngine'),'rev-parse','HEAD'],text=True).strip(),
          'site':str(SITE),'browser':report['browser'],'files':{f:{'sha256':digest(SITE/f),'bytes':(SITE/f).stat().st_size} for f in files},
          'inputs':json.loads((EVIDENCE/'inputs.json').read_text()),'checks':report['checks'],'gates':report['gates'],
          'nativeEvidenceSha256':digest(EVIDENCE/'native-final.txt'),'baselineEvidenceSha256':digest(EVIDENCE/'baseline-scenes.txt'),
          'artAcceptance':'Pip + final props only; final release-one cats/contact playback pending',
          'registration':'Isolated harness; production composition belongs to Prompt 9'}
(EVIDENCE/'identity.json').write_text(json.dumps(identity,indent=2)+'\n')
print(json.dumps({'wasmSha256':identity['files']['release1_preview.wasm']['sha256'],'browser':identity['browser'],'uniqueGLBs':report['uniqueAssets'],'checks':len(report['checks'])}))

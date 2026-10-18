"""Run the pinned native importer/player and bind its evidence to exact GLBs."""
import hashlib,json,subprocess,sys
from pathlib import Path
from validate_exports import ROOT,validate

evidence=ROOT/'build/evidence/release1/prompt-02';evidence.mkdir(parents=True,exist_ok=True)
site=ROOT/'build/web-release1-cats/site/candidates'
files={cat:validate(site/f'{cat}.glb') for cat in ['pip','mabel','button','pepper']}
exe=ROOT/'build/native-release1-cats/release1_track_tests.exe'
result=subprocess.run([str(exe),str(site)],capture_output=True,text=True)
(evidence/'native-tests.log').write_text(result.stdout+result.stderr)
report={'passed':result.returncode==0,'returnCode':result.returncode,'files':files,
        'executableSha256':hashlib.sha256(exe.read_bytes()).hexdigest(),
        'enginePin':'3fe7d1dc1dc610c703944803cbb717cd65024ec9'}
(evidence/'native-report.json').write_text(json.dumps(report,indent=2)+'\n')
print(result.stdout+result.stderr);sys.exit(result.returncode)

"""Generate only the 49 authorized missing effects through the ElevenLabs skill."""
import argparse
import hashlib
import json
from pathlib import Path
import sys

ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(Path.home()/'.codex/skills/elevenlabs/scripts'))
from elevenlabs import generate, repo_root

if __name__=='__main__':
    root=repo_root(str(ROOT))
    jobs=json.loads((ROOT/'artifacts/audio-production/remaining-effects-jobs.json').read_text())['jobs']
    assert len(jobs)==49 and len({j['id'] for j in jobs})==49
    for n,job in enumerate(jobs,1):
        output=ROOT/'artifacts/audio-production/sources/remaining-effects'/(job['id']+'.mp3')
        if output.exists():
            metadata=json.loads(output.with_suffix('.mp3.provenance.json').read_text())
            assert hashlib.sha256(output.read_bytes()).hexdigest()==metadata['sha256'], 'Changed original: '+job['id']
            print('Preserving '+job['id'],flush=True)
            continue
        print(f"Generating {n}/49: {job['id']}",flush=True)
        prompt=job['prompt']+' No human speech, no music or background ambience; natural restrained sound for a cosy garden game.'
        generate(root,argparse.Namespace(prompt=prompt,duration=job['duration'],loop=job.get('loop',False),
                 prompt_influence=.3,output=str(output)))

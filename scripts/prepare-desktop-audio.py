"""Stage bounded WAV delivery from the same reviewed browser audio catalogue."""
import argparse
import hashlib
import json
from pathlib import Path
import shutil
import subprocess

p = argparse.ArgumentParser()
p.add_argument('--source', type=Path, required=True)
p.add_argument('--header', type=Path, required=True)
p.add_argument('--output', type=Path)
a = p.parse_args()
catalog = json.loads((a.source / 'catalog.json').read_text())
paths = []
for clip in catalog['clips']:
    playable = clip.get('accepted') or (clip.get('playable') and clip.get('review', {}).get('technicalVerified'))
    if not playable:
        paths.append('""')
        continue
    assert clip.get('source', {}).get('rightsRecord'), clip['id']
    assert not clip.get('accepted') or clip.get('review', {}).get('sourceAuditioned'), clip['id']
    source = (a.source / clip['file']).resolve()
    assert source.is_relative_to(a.source.resolve()), clip['id']
    assert hashlib.sha256(source.read_bytes()).hexdigest() == clip['sha256'], clip['id']
    paths.append(json.dumps('audio/native/' + clip['id'] + '.wav'))
    if a.output:
        a.output.mkdir(parents=True, exist_ok=True)
        target = a.output / (clip['id'] + '.wav')
        stamp = target.with_suffix('.source-sha256')
        if target.exists() and stamp.exists() and stamp.read_text() == clip['sha256']:
            continue
        if source.suffix.lower() == '.wav':
            shutil.copy2(source, target)
        else:
            subprocess.run(['ffmpeg', '-v', 'error', '-y', '-i', str(source), '-c:a', 'pcm_s16le', str(target)], check=True)
        stamp.write_text(clip['sha256'])
a.header.parent.mkdir(parents=True, exist_ok=True)
text = '// Generated from the reviewed Paws audio catalogue.\nstatic constexpr const char* nativeAudioPaths[] = {\n' + ',\n'.join(paths) + '\n};\n'
if not a.header.exists() or a.header.read_text() != text:
    a.header.write_text(text)

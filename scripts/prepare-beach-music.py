"""Prepare the supplied Beachside Puzzle master for the existing music player."""
import hashlib
import json
import re
import subprocess
import wave
from pathlib import Path
import numpy as np

ROOT = Path(__file__).resolve().parents[1]
RUNTIME = ROOT / 'assets/runtime/audio'
CLIP = 'music_beachside_puzzle_01'


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    original = RUNTIME / 'beach/Beachside Puzzle.wav'
    with wave.open(str(original)) as stream:
        assert stream.getsampwidth() == 2 and stream.getnchannels() == 2
        duration = stream.getnframes() / stream.getframerate()
        samples = np.frombuffer(stream.readframes(stream.getnframes()), dtype='<i2')
        peak = float(np.max(np.abs(samples.astype(np.float32) / 32768)))
    assert 0 < duration <= 300 and peak > 0
    records = ROOT / 'artifacts/audio-production/sources/beachside-puzzle'
    records.mkdir(parents=True, exist_ok=True)
    rights = records / 'rights-status.txt'
    rights.write_text('The user supplied Beachside Puzzle.wav and explicitly requested its use as beach adventure music in Paws.\n'
                      'Original provider, generation settings and license were not supplied. Local integration is authorized;\n'
                      'independent ownership/license verification and final listening approval are not asserted.\n', encoding='utf-8')
    destination = RUNTIME / f'beach/{CLIP}.mp3'
    processing = f'volume={min(1., .7/peak):.9f},afade=t=in:d=0.1,afade=t=out:st={duration-.1:.9f}:d=0.1'
    subprocess.run(['ffmpeg', '-v', 'error', '-y', '-i', str(original), '-map_metadata', '-1',
                    '-af', processing, '-ar', '48000', '-ac', '2', '-codec:a', 'libmp3lame',
                    '-b:a', '192k', str(destination)], check=True)
    decoded = subprocess.run(['ffmpeg', '-v', 'error', '-i', str(destination), '-f', 'f32le',
                              '-acodec', 'pcm_f32le', 'pipe:1'], capture_output=True, check=True).stdout
    pcm = np.frombuffer(decoded, dtype='<f4')
    peak = float(np.max(np.abs(pcm)))
    assert np.isfinite(pcm).all() and 0 < peak < 1 and pcm.nbytes <= 96*1024*1024
    assert abs(pcm.size/2/48000-duration) < .1
    record = dict(id=CLIP, title='Beachside Puzzle', family='music_beachside_puzzle',
                  file=f'beach/{destination.name}', provider='User supplied', bus=0, gain=.18,
                  cooldown=0, loop=True, accepted=False, playable=True,
                  source=dict(original=original.relative_to(ROOT).as_posix(), originalSha256=sha(original),
                              rightsRecord=rights.relative_to(ROOT).as_posix(), rightsSha256=sha(rights),
                              processing=['Full-length stereo 192 kbps MP3', processing]),
                  review=dict(technicalVerified=True, sourceAuditioned=False, journeyAuditioned=False,
                              approved=False, notes='Length, stereo, finite PCM, headroom and hashes verified; no human listening claim.'),
                  pcm=dict(sampleRate=48000, channels=2, frames=pcm.size//2, duration=pcm.size/2/48000,
                           peakDbfs=float(20*np.log10(peak)), pcmBytesAtDeviceRate=pcm.nbytes),
                  sha256=sha(destination), bytes=destination.stat().st_size, loopFrames=None)
    catalog_path = RUNTIME / 'catalog.json'
    catalog = json.loads(catalog_path.read_text(encoding='utf-8'))
    previous = next((i for i, c in enumerate(catalog['clips']) if c['id'] == CLIP), None)
    if previous is None:
        catalog['clips'].append(record)
    else:
        catalog['clips'][previous] = record
    catalog['playlists'] = [p for p in catalog.get('playlists', []) if p['id'] != 'beachside-puzzle'] + [
        dict(id='beachside-puzzle', repeat=True, tracks=[CLIP])]
    catalog_path.write_text(json.dumps(catalog, indent=2, ensure_ascii=False)+'\n', encoding='utf-8')
    header_path = ROOT / 'src/audio_catalog.hpp'
    header = header_path.read_text(encoding='utf-8')
    header = re.sub(r'^    \{"music_beachside_puzzle_01".*\n', '', header, flags=re.MULTILINE)
    header = re.sub(r'AudioClipSpec,\d+>', f'AudioClipSpec,{len(catalog["clips"])}>', header)
    header_path.write_text(header.replace('}};', f'    {{"{CLIP}","music_beachside_puzzle",0,true,0.18f,0}},\n'+'}};'), encoding='utf-8')
    (records / 'preparation.json').write_text(json.dumps(record, indent=2)+'\n', encoding='utf-8')
    print(json.dumps(dict(id=CLIP, pcm=record['pcm'], bytes=record['bytes']), indent=2))


if __name__ == '__main__':
    main()

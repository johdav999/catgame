"""Prepare the user's two full-length stereo tracks for the game. Requires ffmpeg and numpy."""
import hashlib
import json
import subprocess
import wave
from pathlib import Path
import numpy as np

ROOT = Path(__file__).resolve().parents[1]
RUNTIME = ROOT / 'assets/runtime/audio'
TRACKS = [('Golden Afternoon Garden 1.wav', 'music_garden_afternoon_01'),
          ('Golden Afternoon Garden2 .wav', 'music_garden_puzzles_01')]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    catalog_path = RUNTIME / 'catalog.json'
    catalog = json.loads(catalog_path.read_text(encoding='utf-8'))
    records = ROOT / 'artifacts/audio-production/sources/golden-afternoon'
    records.mkdir(parents=True, exist_ok=True)
    rights = records / 'rights-status.txt'
    rights.write_text('The user supplied these tracks, stated that they created them, and requested playback in Paws.\n'
                      'Originals are retained in assets/music. No provider, account plan, or third-party license is inferred.\n'
                      'Local game integration is authorized; distribution entitlement has not been independently verified.\n', encoding='utf-8')
    report = []
    for number, (filename, clip_id) in enumerate(TRACKS, 1):
        original = ROOT / 'assets/music' / filename
        with wave.open(str(original), 'rb') as wav:
            assert wav.getsampwidth() == 2 and wav.getnchannels() == 2
            rate, frames = wav.getframerate(), wav.getnframes()
            samples = np.frombuffer(wav.readframes(frames), dtype='<i2').astype(np.float32) / 32768
        duration = frames / rate
        assert 0 < duration <= 300 and np.max(np.abs(samples)) > 0
        # Leave room for lossy-codec overshoot, with gentle starts and ends.
        gain = min(1., .7 / float(np.max(np.abs(samples))))
        destination = RUNTIME / (clip_id + '.mp3')
        processing = f'volume={gain:.9f},afade=t=in:d=0.1,afade=t=out:st={duration-.1:.9f}:d=0.1'
        subprocess.run(['ffmpeg', '-v', 'error', '-y', '-i', str(original), '-map_metadata', '-1',
                        '-af', processing, '-ar', '48000', '-ac', '2', '-codec:a', 'libmp3lame',
                        '-b:a', '192k', str(destination)], check=True)
        decoded = subprocess.run(['ffmpeg', '-v', 'error', '-i', str(destination), '-f', 'f32le',
                                  '-acodec', 'pcm_f32le', 'pipe:1'], check=True, capture_output=True).stdout
        pcm = np.frombuffer(decoded, dtype='<f4')
        peak = float(np.max(np.abs(pcm)))
        assert np.isfinite(pcm).all() and 0 < peak < 1
        metrics = dict(sampleRate=48000, channels=2, frames=pcm.size//2, duration=pcm.size/2/48000,
                       peakDbfs=float(20*np.log10(peak)), pcmBytesAtDeviceRate=pcm.nbytes)
        assert pcm.nbytes <= 96*1024*1024 and abs(metrics['duration']-duration) < .1
        source = dict(provider='User supplied', original=original.relative_to(ROOT).as_posix(),
                      originalSha256=sha(original), rightsRecord=rights.relative_to(ROOT).as_posix(),
                      processing=['Full-length stereo 192 kbps MP3', processing],
                      licenseNotice='User-created music supplied for Paws; see recorded rights status.')
        review = dict(technicalVerified=True, sourceAuditioned=False, approved=False, journeyAuditioned=False,
                      notes='Length, stereo, finite PCM, headroom and content hashes checked. No human listening claimed.')
        clip = next(c for c in catalog['clips'] if c['id'] == clip_id)
        clip.update(title=f'Golden Afternoon Garden {number}', provider='User supplied',
                    file=destination.name, accepted=False, playable=True, source=source, review=review,
                    sha256=sha(destination), bytes=destination.stat().st_size, pcm=metrics)
        # Clip loop flags describe legacy single-track playback. The playlist plays each once.
        (RUNTIME / (clip_id + '.notice.txt')).write_text(source['licenseNotice']+'\n', encoding='utf-8')
        report.append(dict(id=clip_id, title=clip['title'], source=source, review=review, pcm=metrics,
                           sha256=clip['sha256'], bytes=clip['bytes']))
    catalog['playlists'] = [p for p in catalog.get('playlists', []) if p['id'] != 'golden-afternoon-garden'] + [
        dict(id='golden-afternoon-garden', repeat=True, tracks=[clip_id for _, clip_id in TRACKS])]
    catalog_path.write_text(json.dumps(catalog, indent=2, ensure_ascii=False)+'\n', encoding='utf-8')
    (records/'preparation.json').write_text(json.dumps(report, indent=2)+'\n', encoding='utf-8')
    print(json.dumps([dict(title=r['title'], bytes=r['bytes'], pcm=r['pcm']) for r in report], indent=2))


if __name__ == '__main__':
    main()

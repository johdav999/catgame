"""Prepare real Pip downloads as runtime WAVs; retain source and review provenance."""
import hashlib
import json
from pathlib import Path
import subprocess
import wave
import numpy as np

ROOT = Path(__file__).resolve().parents[1]
SOURCES = ROOT/'artifacts/audio-production/sources/pip-sounds'
RUNTIME = ROOT/'assets/runtime/audio'
IDS = ['pip_request_01', 'pip_request_02', 'pip_happy_01', 'pip_happy_02',
       'pip_purr_01', 'pip_paws_stone_01', 'pip_paws_stone_02', 'pip_paws_stone_03',
       'pip_settle_01', 'pip_drink_01']

def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def main():
    catalog = json.loads((RUNTIME/'catalog.json').read_text(encoding='utf-8'))
    for name in IDS:
        assert not (RUNTIME/(name+'.wav')).exists(), 'Preserve existing runtime: '+name
        assert (SOURCES/(name+'.mp3')).is_file(), 'Missing provider source: '+name
    rights = SOURCES/'rights-status.txt'
    rights.write_text('Generated through the user-configured ElevenLabs API for Paws.\n'
        'The user authorized these ten Pip sounds and local game integration.\n'
        'Account plan, distribution entitlement and attribution remain unverified.\n'
        'Provider terms: https://elevenlabs.io/terms-of-use\n'
        'No universal commercial license or human listening approval is asserted.\n', encoding='utf-8')
    report = []
    for name in IDS:
        original = SOURCES/(name+'.mp3')
        provenance = json.loads(original.with_suffix('.mp3.provenance.json').read_text(encoding='utf-8'))
        assert sha(original) == provenance['sha256']
        rate, channels = 44100, 1
        raw = subprocess.run(['ffmpeg', '-v', 'error', '-i', str(original), '-f', 'f32le',
            '-acodec', 'pcm_f32le', '-ar', str(rate), '-ac', '1', 'pipe:1'], check=True, capture_output=True).stdout
        pcm = np.frombuffer(raw, dtype='<f4').astype(np.float64)
        assert np.isfinite(pcm).all() and np.max(np.abs(pcm)) > 0
        pcm -= np.mean(pcm)
        edits = ['Decoded to mono 44100 Hz PCM; removed DC offset']
        loop = name == 'pip_purr_01'
        if loop:
            overlap = int(rate*.25)
            blend = np.linspace(0, 1, overlap)
            pcm = np.concatenate([pcm[overlap:-overlap], pcm[-overlap:]*(1-blend)+pcm[:overlap]*blend])
            edits.append('Circular 250 ms end/start crossfade, rotated complete buffer')
        else:
            if name.startswith('pip_paws_'):
                # Provider minimum is 500 ms. Export one 230 ms contact window
                # centred around its highest-energy transient, not leading silence.
                width = int(rate*.23)
                energy = np.convolve(pcm*pcm, np.ones(int(rate*.012))/int(rate*.012), mode='same')
                start = max(0, min(len(pcm)-width, int(np.argmax(energy))-int(rate*.065)))
                pcm = pcm[start:start+width]
                edits.append(f'Energy-centred single 230 ms paw contact; original start frame {start}')
            fade = int(rate*.015)
            pcm[:fade] *= np.linspace(0, 1, fade)
            pcm[-fade:] *= np.linspace(1, 0, fade)
            edits.append('15 ms entry/exit fades')
        pcm -= np.mean(pcm)
        pcm *= 10**(-6/20)/np.max(np.abs(pcm))
        samples = np.rint(pcm*32767).astype('<i2')
        destination = RUNTIME/(name+'.wav')
        with wave.open(str(destination), 'wb') as stream:
            stream.setnchannels(channels); stream.setsampwidth(2); stream.setframerate(rate)
            stream.writeframes(samples.tobytes())
        measured = samples.astype(np.float64)/32768
        metrics = dict(sampleRate=rate, channels=channels, frames=len(pcm), duration=len(pcm)/rate,
            peakDbfs=float(20*np.log10(np.max(np.abs(measured)))),
            rmsDbfs=float(20*np.log10(np.sqrt(np.mean(measured**2)))), dc=[float(np.mean(measured))],
            pcmBytesAtDeviceRate=int(np.ceil(len(pcm)/rate*48000))*4,
            seamStep=float(abs(measured[-1]-measured[0])))
        assert metrics['peakDbfs'] <= -2 and abs(metrics['dc'][0]) < .01
        assert metrics['rmsDbfs'] > -60 and metrics['pcmBytesAtDeviceRate'] < 3*1024*1024
        assert not loop or (8 <= metrics['duration'] <= 12 and metrics['seamStep'] < .02)
        source = dict(provider='ElevenLabs', assetId=provenance.get('request_id') or 'sha256:'+sha(original),
            url=provenance['endpoint'], generatedAt=provenance['generated_at'], prompt=provenance['request']['text'],
            settings=provenance['request'], original=original.relative_to(ROOT).as_posix(), originalSha256=sha(original),
            rightsRecord=rights.relative_to(ROOT).as_posix(), rightsSha256=sha(rights), processing=edits+['Peak -6 dBFS; PCM16 WAV'],
            licenseNotice='ElevenLabs-generated Pip sound. Account distribution rights and attribution remain to be verified.')
        review = dict(technicalVerified=True, approved=False, sourceAuditioned=False, loopAuditioned=False,
            offlineRepeats=0, gameLoopRepeats=0, journeyAuditioned=False, reviewer='Codex technical validation',
            notes='Provider original hash, non-silent PCM, duration, headroom, DC and seam checked. No human or physical-device listening claimed.')
        record = next(c for c in catalog['clips'] if c['id'] == name)
        record.update(playable=True, accepted=False, source=source, review=review, pcm=metrics,
            sha256=sha(destination), bytes=destination.stat().st_size,
            loopFrames=dict(startFrame=0, endFrame=len(pcm)) if loop else None)
        (RUNTIME/(name+'.notice.txt')).write_text(source['licenseNotice']+'\n', encoding='utf-8')
        (RUNTIME/(name+'.source-rights.txt')).write_bytes(rights.read_bytes())
        report.append(dict(id=name, sha256=record['sha256'], pcm=metrics, source=source, review=review))
    (RUNTIME/'catalog.json').write_text(json.dumps(catalog, indent=2, ensure_ascii=False)+'\n', encoding='utf-8')
    output = ROOT/'artifacts/audio-production/reviews/pip-sounds'
    output.mkdir(parents=True, exist_ok=True)
    (output/'technical-report.json').write_text(json.dumps(report, indent=2)+'\n', encoding='utf-8')
    print(json.dumps([dict(id=r['id'], **r['pcm']) for r in report], indent=2))

if __name__ == '__main__':
    main()

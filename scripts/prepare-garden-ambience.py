"""Prepare four real ElevenLabs downloads for playback without inventing listening approval.

Requires ffmpeg on PATH and numpy. Does not call ElevenLabs or spend credits.
"""
import hashlib
import json
from pathlib import Path
import subprocess
import wave
import numpy as np

ROOT = Path(__file__).resolve().parents[1]
SOURCES = ROOT/'artifacts/audio-production/sources/garden-ambience'
RUNTIME = ROOT/'assets/runtime/audio'
IDS = ['amb_garden_air_01'] + [f'amb_garden_bird_{n:02}' for n in range(1,4)]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    catalog = json.loads((RUNTIME/'catalog.json').read_text(encoding='utf-8'))
    for name in IDS:
        if (RUNTIME/(name+'.wav')).exists():
            raise ValueError('Runtime file already exists; preserve it: '+name)
        if not (SOURCES/(name+'.mp3')).is_file():
            raise ValueError('Missing ElevenLabs original: '+name)
    rights = SOURCES/'rights-status.txt'
    rights.write_text('Generated through the user-configured ElevenLabs API for Paws on 2026-10-11.\n'
                      'The user authorized generation and local game integration.\n'
                      'Account plan, distribution entitlement and attribution requirements remain unverified.\n'
                      'Provider terms: https://elevenlabs.io/terms-of-use\n'
                      'No third-party recording or universal commercial license is asserted.\n', encoding='utf-8')
    report=[]
    for name in IDS:
        original=SOURCES/(name+'.mp3')
        provenance=json.loads(original.with_suffix('.mp3.provenance.json').read_text(encoding='utf-8'))
        assert sha(original)==provenance['sha256'], 'Changed provider original'
        rate=44100
        loop=name=='amb_garden_air_01'
        channels=2 if loop else 1
        command=['ffmpeg','-v','error','-i',str(original),'-f','f32le','-acodec','pcm_f32le',
                 '-ar',str(rate),'-ac',str(channels),'pipe:1']
        raw=subprocess.run(command,check=True,capture_output=True).stdout
        pcm=np.frombuffer(raw,dtype='<f4').reshape(-1,channels).astype(np.float64)
        assert np.isfinite(pcm).all() and np.max(np.abs(pcm))>0
        pcm-=np.mean(pcm,axis=0)
        edits=['ffmpeg decoded to 44100 Hz '+str(channels)+' channel float PCM', 'Removed per-channel DC offset']
        if loop:
            overlap=int(rate*.25)
            blend=np.linspace(0,1,overlap)[:,None]
            pcm=np.concatenate([pcm[overlap:-overlap],pcm[-overlap:]*(1-blend)+pcm[:overlap]*blend])
            edits.append('Circular 250 ms end/start overlap, rotated complete buffer; no runtime crossfade needed')
        else:
            fade=int(rate*.02)
            pcm[:fade]*=np.linspace(0,1,fade)[:,None]
            pcm[-fade:]*=np.linspace(1,0,fade)[:,None]
            edits.append('20 ms entry/exit fades')
        pcm-=np.mean(pcm,axis=0)
        target_peak=10**(-6/20)
        pcm*=target_peak/np.max(np.abs(pcm))
        edits.append('Peak normalized to -6 dBFS; PCM16 WAV export')
        samples=np.rint(pcm*32767).astype('<i2')
        destination=RUNTIME/(name+'.wav')
        with wave.open(str(destination),'wb') as stream:
            stream.setnchannels(channels);stream.setsampwidth(2);stream.setframerate(rate);stream.writeframes(samples.tobytes())
        measured=samples.astype(np.float64)/32768
        metrics=dict(sampleRate=rate,channels=channels,frames=len(pcm),duration=len(pcm)/rate,
                     peakDbfs=float(20*np.log10(np.max(np.abs(measured)))),
                     rmsDbfs=float(20*np.log10(np.sqrt(np.mean(measured**2)))),
                     dc=np.mean(measured,axis=0).tolist(),
                     pcmBytesAtDeviceRate=int(np.ceil(len(pcm)/rate*48000))*channels*4,
                     seamStep=float(np.max(np.abs(measured[-1]-measured[0]))))
        assert metrics['peakDbfs']<=-2 and max(map(abs,metrics['dc']))<.01
        assert metrics['pcmBytesAtDeviceRate']<=24*1024*1024
        assert (20<=metrics['duration']<=30) if loop else (1<=metrics['duration']<=3)
        record=next(c for c in catalog['clips'] if c['id']==name)
        source=dict(provider='ElevenLabs',assetId=provenance.get('request_id') or 'sha256:'+sha(original),
                    url=provenance['endpoint'],generatedAt=provenance['generated_at'],
                    prompt=provenance['request']['text'],settings=provenance['request'],
                    original=original.relative_to(ROOT).as_posix(),originalSha256=sha(original),
                    rightsRecord=rights.relative_to(ROOT).as_posix(),rightsSha256=sha(rights),
                    processing=edits,licenseNotice='ElevenLabs-generated Paws garden ambience. Account distribution rights and attribution remain to be verified.')
        review=dict(technicalVerified=True,approved=False,sourceAuditioned=False,loopAuditioned=False,
                    offlineRepeats=0,gameLoopRepeats=0,journeyAuditioned=False,reviewer='Codex technical validation',
                    notes='Real provider bytes, PCM/headroom/DC/seam metrics verified. No human or physical-device listening claimed.')
        record.update(playable=True,accepted=False,source=source,review=review,pcm=metrics,
                      sha256=sha(destination),bytes=destination.stat().st_size,
                      loopFrames=dict(startFrame=0,endFrame=len(pcm)) if loop else None)
        (RUNTIME/(name+'.notice.txt')).write_text(source['licenseNotice']+'\n',encoding='utf-8')
        report.append(dict(id=name,sha256=record['sha256'],bytes=record['bytes'],pcm=metrics,source=source,review=review))
    (RUNTIME/'catalog.json').write_text(json.dumps(catalog,indent=2,ensure_ascii=False)+'\n',encoding='utf-8')
    output=ROOT/'artifacts/audio-production/reviews/garden-ambience'
    output.mkdir(parents=True,exist_ok=True)
    (output/'technical-report.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print(json.dumps([dict(id=r['id'],**r['pcm']) for r in report],indent=2))


if __name__=='__main__':
    main()

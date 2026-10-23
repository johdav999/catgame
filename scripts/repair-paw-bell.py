"""Derive the high paw note from the retained, ringing leaf recording.

No provider generation: preserve the source recording's provenance and rights.
The former paw source was accelerated 2.61x and had virtually no audible tail.
"""
import copy
import hashlib
import json
from pathlib import Path
import wave

import numpy as np

ROOT = Path(__file__).resolve().parents[1]
AUDIO = ROOT / 'assets/runtime/audio'


def read(path):
    with wave.open(str(path), 'rb') as stream:
        assert stream.getnchannels() == 1 and stream.getsampwidth() == 2
        return stream.getframerate(), np.frombuffer(stream.readframes(stream.getnframes()), '<i2').astype(float) / 32768


def main():
    catalog = json.loads((AUDIO / 'catalog.json').read_text(encoding='utf-8'))
    leaf = next(c for c in catalog['clips'] if c['id'] == 'bell_leaf_01')
    paw = next(c for c in catalog['clips'] if c['id'] == 'bell_paw_01')
    source = AUDIO / leaf['file']
    assert hashlib.sha256(source.read_bytes()).hexdigest() == leaf['sha256']
    rate, recording = read(source)
    ratio = 1046.502 / 659.255
    samples = np.interp(np.arange(int(len(recording) / ratio)) * ratio, np.arange(len(recording)), recording)
    samples -= np.mean(samples)
    fade = int(rate * .015)
    samples[:fade] *= np.linspace(0, 1, fade)
    samples[-fade:] *= np.linspace(1, 0, fade)
    samples *= 10 ** (-6 / 20) / np.max(np.abs(samples))
    pcm = np.rint(samples * 32767).astype('<i2')
    destination = AUDIO / paw['file']
    with wave.open(str(destination), 'wb') as stream:
        stream.setnchannels(1)
        stream.setsampwidth(2)
        stream.setframerate(rate)
        stream.writeframes(pcm.tobytes())
    _, measured = read(destination)
    rms = lambda a: float(20 * np.log10(np.sqrt(np.mean(a * a))))
    # Catch the former near-silent tail, not merely a nonzero initial transient.
    tail_rms = rms(measured[int(rate * .15):int(rate * .45)])
    assert .7 <= len(measured) / rate <= 1.2
    assert tail_rms > -26 and rms(measured) > -23
    metrics = dict(sampleRate=rate, channels=1, frames=len(measured), duration=len(measured)/rate,
                   peakDbfs=float(20*np.log10(np.max(np.abs(measured)))), rmsDbfs=rms(measured),
                   dc=[float(np.mean(measured))], pcmBytesAtDeviceRate=int(np.ceil(len(measured)/rate*48000))*4,
                   seamStep=float(abs(measured[-1]-measured[0])))
    provenance = copy.deepcopy(leaf['source'])
    provenance['derivedFrom'] = dict(file=source.relative_to(ROOT).as_posix(), sha256=leaf['sha256'])
    provenance['processing'] += ['Derived paw note from processed leaf bell: 659.255 Hz to 1046.502 Hz by resampling',
                                '15 ms boundary fades; peak -6 dBFS; PCM16 WAV; verified sustained 150-450 ms ring']
    review = copy.deepcopy(leaf['review'])
    review['notes'] = 'Derived from existing provider bell. Hash, distinct pitch, 0.88 s duration and sustained audible PCM verified; no human or physical-device listening claimed.'
    paw.update(source=provenance, review=review, pcm=metrics, bytes=destination.stat().st_size,
               sha256=hashlib.sha256(destination.read_bytes()).hexdigest())
    (AUDIO / 'catalog.json').write_text(json.dumps(catalog, indent=2, ensure_ascii=False)+'\n', encoding='utf-8')
    destination.with_suffix('.notice.txt').write_text(provenance['licenseNotice']+'\nDerived paw note from bell_leaf_01; see catalog.json for source and processing hashes.\n', encoding='utf-8')
    report = dict(id=paw['id'], sha256=paw['sha256'], pcm=metrics, tailRmsDbfs=tail_rms, source=provenance)
    output = ROOT / 'artifacts/audio-production/reviews/paw-bell-repair.json'
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(report, indent=2)+'\n', encoding='utf-8')
    print(json.dumps(dict(duration=metrics['duration'], rmsDbfs=metrics['rmsDbfs'], tailRmsDbfs=tail_rms)))


if __name__ == '__main__':
    main()

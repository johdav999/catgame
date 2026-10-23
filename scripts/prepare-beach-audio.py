"""Remove the opening impact from supplied waves; preserve immutable source copies."""
import hashlib
import json
import math
from pathlib import Path
import re
import struct
import shutil
import wave

ROOT = Path(__file__).resolve().parents[1]
RUNTIME = ROOT / 'assets/runtime/audio'


def main():
    rights = RUNTIME / 'beach/PROVENANCE.md'
    catalog_path = RUNTIME / 'catalog.json'
    catalog = json.loads(catalog_path.read_text(encoding='utf-8'))
    records = []
    sources = ROOT / 'artifacts/audio-production/sources/beach-waves-original'
    sources.mkdir(parents=True, exist_ok=True)
    for variant in range(1, 5):
        path = RUNTIME / f'beach/waves{variant}.wav'
        original = sources / path.name
        if not original.exists():
            shutil.copyfile(path, original)
        with wave.open(str(original)) as stream:
            assert stream.getsampwidth() == 2 and stream.getcomptype() == 'NONE'
            rate, channels, frames = stream.getframerate(), stream.getnchannels(), stream.getnframes()
            source_samples = struct.unpack('<' + 'h' * (frames * channels), stream.readframes(frames))
        assert channels in (1, 2) and 0 < frames / rate < 90
        # The supplied variants share a sharp, decaying impact in the first 300 ms.
        # Keep the 2.55 s timing for foam synchronization; fade into the quiet surf.
        samples = list(source_samples)
        for frame in range(round(.5 * rate)):
            t = max(0., min(1., (frame / rate - .3) / .2))
            gain = .5 - .5 * math.cos(math.pi * t)
            for channel in range(channels):
                i = frame * channels + channel
                samples[i] = round(source_samples[i] * gain)
        assert not any(samples[:round(.3 * rate) * channels])
        assert samples[round(.5 * rate) * channels:] == list(source_samples[round(.5 * rate) * channels:])
        with wave.open(str(path), 'wb') as output:
            output.setparams((channels, 2, rate, frames, 'NONE', 'not compressed'))
            output.writeframes(struct.pack('<' + 'h' * len(samples), *samples))
        peak = max(abs(sample) for sample in samples) / 32768
        rms = math.sqrt(sum((sample / 32768) ** 2 for sample in samples) / len(samples))
        assert peak > 0 and rms > 0
        records.append(dict(
            id=f'beach_wave_{variant:02}', family='beach_wave', file=f'beach/waves{variant}.wav',
            provider='User supplied', bus=2, gain=.12, cooldown=0, loop=False,
            accepted=False, playable=True,
            source=dict(original=original.relative_to(ROOT).as_posix(),
                        originalSha256=hashlib.sha256(original.read_bytes()).hexdigest(),
                        rightsRecord=rights.relative_to(ROOT).as_posix(),
                        rightsSha256=hashlib.sha256(rights.read_bytes()).hexdigest(),
                        processing=['Opening 300 ms impact removed; 200 ms raised-cosine fade into surf; samples after 500 ms unchanged; original duration retained; runtime gain 0.12; no looping']),
            review=dict(technicalVerified=True, sourceAuditioned=False, journeyAuditioned=False,
                        notes='Opening transient removed, fade continuity and unchanged remaining samples checked. Original source retained. Human listening and independent rights review not asserted.'),
            pcm=dict(sampleRate=rate, channels=channels, frames=frames, duration=frames/rate,
                     peakDbfs=20*math.log10(peak), rmsDbfs=20*math.log10(rms),
                     pcmBytesAtDeviceRate=math.ceil(frames/rate*48000)*channels*4),
            sha256=hashlib.sha256(path.read_bytes()).hexdigest(), bytes=path.stat().st_size,
            loopFrames=None))
    ids = {record['id'] for record in records}
    catalog['clips'] = [record for record in catalog['clips'] if record['id'] not in ids] + records
    catalog_path.write_text(json.dumps(catalog, indent=2, ensure_ascii=False)+'\n', encoding='utf-8')
    # Append stable indices; retain all existing authored cue policies verbatim.
    header_path = ROOT / 'src/audio_catalog.hpp'
    header = header_path.read_text(encoding='utf-8')
    header = re.sub(r'^    \{"beach_wave_.*\n', '', header, flags=re.MULTILINE)
    header = re.sub(r'AudioClipSpec,\d+>', f'AudioClipSpec,{len(catalog["clips"])}>', header)
    extra = ''.join(f'    {{"{record["id"]}","beach_wave",2,false,0.12f,0}},\n' for record in records)
    header_path.write_text(header.replace('}};', extra+'}};'), encoding='utf-8')
    print(json.dumps(records, indent=2))


if __name__ == '__main__':
    main()

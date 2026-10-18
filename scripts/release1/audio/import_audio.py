"""Import reviewed release-one WAVs; audit real bytes, never infer listening/rights."""
import argparse
import hashlib
import json
import math
import re
import struct
import wave
from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]

def sha(path): return hashlib.sha256(path.read_bytes()).hexdigest()
def require(value, message):
    if not value: raise ValueError(message)
def dump(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, indent=2)+'\n', encoding='utf-8')
def paths(root):
    return root/'artifacts/audio-jobs/release1', root/'assets/runtime/audio/release1'
def local(job, value):
    p=(job/value).resolve()
    require(p.is_relative_to(job.resolve()) and p.is_file(), 'Source must be a file within the audio job')
    return p
def measure(path, device_rate=48000):
    require(path.stat().st_size<=24*1024*1024, 'Encoded budget exceeded')
    with wave.open(str(path), 'rb') as w:
        channels, rate, frames, width=w.getnchannels(),w.getframerate(),w.getnframes(),w.getsampwidth()
        require(width==2 and channels in (1,2) and 8000<=rate<=96000 and frames>0, 'Require PCM16 mono/stereo WAV')
        require(frames/rate<=90 and frames/rate*channels*device_rate*4<=24*1024*1024, 'Decoded clip budget exceeded')
        raw=w.readframes(frames)
        require(len(raw)==frames*channels*2, 'Truncated PCM')
    samples=struct.unpack('<'+'h'*(frames*channels), raw)
    peak=max(abs(x) for x in samples)/32768
    require(0<peak<=10**(-2/20), 'Silent or peak above -2 dBFS')
    dc=[sum(samples[ch::channels])/(frames*32768) for ch in range(channels)]
    require(max(map(abs, dc))<.01, 'Remove DC offset from every channel')
    rms=math.sqrt(sum((x/32768)**2 for x in samples)/len(samples))
    return dict(sampleRate=rate, channels=channels, frames=frames, duration=frames/rate,
                peakDbfs=20*math.log10(peak), rmsDbfs=20*math.log10(rms), dc=dc,
                pcmBytesAtDeviceRate=math.ceil(frames/rate*device_rate)*channels*4,
                measuredDeviceRate=device_rate)
def import_clip(root, metadata):
    job, runtime=paths(root)
    catalog=json.loads((job/'catalog.json').read_text())
    m=metadata
    c=next((x for x in catalog['clips'] if x['id']==m['id']),None)
    require(c is not None, 'Unknown clip')
    require(not c['accepted'], 'Accepted audio is immutable; version it deliberately before replacement')
    require(re.fullmatch('[a-z0-9_]+',c['id']) and c['file']==c['id']+'.wav', 'Invalid output name')
    source=m['source'];review=m['review']
    require(source['provider']==c['provider'], 'Selected provider mismatch')
    for key in ('assetId','url','generatedAt','prompt','settings','original','rightsRecord','processing','licenseNotice'):
        require(bool(source.get(key)) and not any(t in str(source[key]) for t in ('REPLACE_','RECORD_','COPY_THE_')), 'Unfilled source field: '+key)
    require(review.get('approved') is True and review.get('sourceAuditioned') is True and review.get('reviewer'), 'Human source review required')
    original=local(job,source['original']);rights=local(job,source['rightsRecord']);edited=local(job,m['edited'])
    require(rights.stat().st_size>0, 'Empty rights evidence')
    pcm=measure(edited)
    low,high=c['durationRange']
    require(low<=pcm['duration']<=high, 'Outside production duration brief')
    if c['loop']:
        require(m.get('loop')==dict(startFrame=0,endFrame=pcm['frames']), 'Loop must cover complete trimmed buffer')
        require(review.get('loopAuditioned') is True and review.get('offlineRepeats',0)>=5, 'Five offline loop repeats required')
    c.update(accepted=True, source=dict(source,originalSha256=sha(original),rightsSha256=sha(rights)),
             review=review, pcm=pcm, sha256=sha(edited), bytes=edited.stat().st_size,loopFrames=m.get('loop'))
    baseline=json.loads((root/'assets/runtime/audio/catalog.json').read_text())['clips']
    delivered=[x for x in baseline+catalog['clips'] if x['accepted']]
    def retained(x): return math.ceil(x['pcm']['duration']*48000)*x['pcm']['channels']*4
    effects=sum(retained(x) for x in delivered if not x['family'].startswith('music_'))
    music=max([retained(x) for x in delivered if x['family'].startswith('music_')]+[0])
    require(effects+music<=48*1024*1024, 'Aggregate retained budget exceeded')
    # Validate everything before any publication. Preserve originals and account evidence outside runtime.
    runtime.mkdir(parents=True,exist_ok=True)
    require(not (runtime/c['file']).exists(), 'Refuse to overwrite existing runtime audio')
    (runtime/c['file']).write_bytes(edited.read_bytes())
    (runtime/(c['id']+'.notice.txt')).write_text(source['licenseNotice']+'\n',encoding='utf-8')
    dump(job/'reviews'/(c['id']+'.json'),c)
    dump(job/'catalog.json',catalog)
    refresh_fragment(root)
    return c
def refresh_fragment(root):
    job,runtime=paths(root)
    clips=json.loads((job/'catalog.json').read_text())['clips']
    assets=[]
    for c in clips:
        if not c['accepted']: continue
        p=runtime/c['file'];rights=local(job,c['source']['rightsRecord'])
        require(sha(p)==c['sha256'] and sha(rights)==c['source']['rightsSha256'], 'Imported content/rights changed')
        assets.append(dict(id='release1/audio/'+c['id']+'@1',url='assets/audio/release1/'+c['file'],
            sha256=sha(p),bytes=p.stat().st_size,bounds=None,footprint=None,anchors={},
            rightsRecord=rights.relative_to(root).as_posix(),cueFamily=c['family'],bus='Effects',
            duration=c['pcm']['duration'],loop=c['loop'],pcmEstimate=c['pcm']['pcmBytesAtDeviceRate'],
            listening=c['review']))
    base=json.loads((root/'assets/runtime/audio/catalog.json').read_text())
    fragment=dict(schema='paws-release1-fragment/1',task=8,namespace='release1/audio',assets=assets,
        assemblies=[],provenance=[a['rightsRecord'] for a in assets],reuses=[c['id'] for c in base['clips']],
        aliases={'picnic_complete':'music_pip_at_home'},status='preparation' if not assets else 'partial; actual-game listening gate separate')
    dump(root/'docs/release1/fragments/audio.json',fragment)
def audit(root, rate=48000):
    job,runtime=paths(root)
    new=json.loads((job/'catalog.json').read_text())['clips']
    old=json.loads((root/'assets/runtime/audio/catalog.json').read_text())['clips']
    missing=[]; measured=[]
    for c in old+new:
        if not c['accepted']: missing.append(c['id']);continue
        p=(runtime if c in new else root/'assets/runtime/audio')/c['file']
        require(p.is_file() and sha(p)==c['sha256'], 'Missing/changed accepted bytes: '+c['id'])
        pcm=measure(p, rate)
        measured.append(dict(id=c['id'],family=c['family'],**pcm))
    effects=sum(c['pcmBytesAtDeviceRate'] for c in measured if not c['family'].startswith('music_'))
    music=max([c['pcmBytesAtDeviceRate'] for c in measured if c['family'].startswith('music_')]+[0])
    require(effects+music<=48*1024*1024,'Retained effects plus largest music exceed cache budget')
    return dict(imported=len(measured),required=len(old+new),missing=missing,measurements=measured,
        measuredSubsetPcmBytes=effects+music,finalCatalogPcmBytes=None if missing else effects+music,
        deviceRate=rate,finalListeningAccepted=not missing and all(c.get('review',{}).get('journeyAuditioned') and
            (not c['loop'] or c['review'].get('gameLoopRepeats',0)>=5) for c in old+new))
def main():
    p=argparse.ArgumentParser(__doc__);p.add_argument('metadata',nargs='?',type=Path)
    p.add_argument('--check',action='store_true');p.add_argument('--require-complete',action='store_true')
    p.add_argument('--device-rate',type=int,default=48000);a=p.parse_args()
    require(8000<=a.device_rate<=192000,'Invalid device rate')
    if a.metadata: import_clip(ROOT,json.loads(a.metadata.read_text()))
    refresh_fragment(ROOT)
    report=audit(ROOT,a.device_rate);print(json.dumps(report,indent=2))
    if a.require_complete and (report['missing'] or not report['finalListeningAccepted']): raise SystemExit(2)
if __name__=='__main__': main()

"""Validate reviewed PCM16 sources and stage one real Prompt 9 delivery."""
import argparse, hashlib, json, math, shutil, struct, wave
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]
CATALOG=ROOT/'assets/runtime/audio/catalog.json'
def sha(path): return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
    parser=argparse.ArgumentParser(__doc__)
    parser.add_argument('metadata', nargs='?', type=Path)
    parser.add_argument('--check', action='store_true')
    parser.add_argument('--require-complete', action='store_true')
    args=parser.parse_args()
    catalog=json.loads(CATALOG.read_text(encoding='utf-8'))
    if args.check:
        missing=[]
        for c in catalog['clips']:
            if not c['accepted']: missing.append(c['id']);continue
            p=CATALOG.parent/c['file']
            assert p.is_file() and sha(p)==c['sha256'], 'Missing or changed accepted file: '+c['id']
        print(json.dumps(dict(imported=47-len(missing),missing=missing,audiblyVerified=sum(bool(c.get('review',{} ) and c['review'].get('journeyAuditioned')) for c in catalog['clips'])),indent=2))
        if args.require_complete and missing: raise SystemExit(2)
        return
    assert args.metadata, 'Pass metadata.json or --check'
    m=json.loads(args.metadata.read_text(encoding='utf-8'))
    c=next(c for c in catalog['clips'] if c['id']==m['id'])
    source=m['source']; review=m['review']
    assert source['provider']==c['provider']
    for field in ('assetId','url','generatedAt','prompt','settings','rightsRecord','processing','licenseNotice'):
        assert source.get(field), 'Required source field: '+field
        assert 'REPLACE_' not in str(source[field]) and 'RECORD_' not in str(source[field]) and 'COPY_THE_' not in str(source[field]), 'Unfilled metadata: '+field
    def local(value):
        p=(ROOT/value).resolve();p.relative_to(ROOT/'artifacts/audio-production');assert p.is_file();return p
    original=local(source['original']);rights=local(source['rightsRecord']);edited=local(m['edited'])
    assert review['sourceAuditioned'] is True and review['approved'] is True
    with wave.open(str(edited),'rb') as w:
        channels,rate,frames,width=w.getnchannels(),w.getframerate(),w.getnframes(),w.getsampwidth()
        assert width==2 and channels in (1,2) and 8000<=rate<=96000 and frames>0, 'Use PCM16 mono/stereo WAV'
        samples=list(struct.unpack('<'+'h'*(frames*channels),w.readframes(frames)))
    duration=frames/rate
    assert duration<=90 and duration*channels*48000*4<=24*1024*1024, 'Clip decode budget at 48 kHz exceeded; shorten or use mono'
    peak=max(abs(x) for x in samples)/32768
    dc=sum(samples)/len(samples)/32768
    assert 0<peak<=10**(-2/20), 'Silent or > -2 dBFS peak'
    assert abs(dc)<.01, 'Remove DC offset'
    if c['loop']:
        assert m['loop']==dict(startFrame=0,endFrame=frames), 'Trim loop to complete buffer'
        assert review['loopAuditioned'] is True and review['offlineRepeats']>=5, 'Audition five repeats before import'
    destination=CATALOG.parent/c['file'];shutil.copyfile(edited,destination)
    (CATALOG.parent/(c['id']+'.notice.txt')).write_text(source['licenseNotice']+'\n',encoding='utf-8')
    c.update(accepted=True,sha256=sha(destination),source=source,review=review,
        pcm=dict(sampleRate=rate,channels=channels,frames=frames,duration=duration,peakDbfs=20*math.log10(peak),dc=dc),loopFrames=m.get('loop'))
    c['source']['originalSha256']=sha(original);c['source']['rightsSha256']=sha(rights)
    audit=ROOT/'artifacts/audio-production/reviews';audit.mkdir(parents=True,exist_ok=True)
    # Retained compact waveform: min/max envelope per 10 ms, no fabricated listening verdict.
    stride=max(channels,int(rate*.01)*channels)
    envelope=[[min(samples[i:i+stride])/32768,max(samples[i:i+stride])/32768] for i in range(0,len(samples),stride)]
    (audit/(c['id']+'.json')).write_text(json.dumps(dict(metadata=c,waveformIntervalSeconds=.01,waveform=envelope),indent=2)+'\n')
    CATALOG.write_text(json.dumps(catalog,indent=2,ensure_ascii=False)+'\n',encoding='utf-8')
    print('Imported '+c['id']+'; game journey listening remains '+str(review.get('journeyAuditioned',False)))
if __name__=='__main__': main()

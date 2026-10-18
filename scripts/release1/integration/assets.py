"""Merge accepted catalogues, verify immutable bytes, and audit staged production.

Does not promote in-progress model jobs or accept unreviewed audio sources.
"""
import argparse, hashlib, json, math, shutil
from contacts import compose
from pathlib import Path
ROOT=Path(__file__).resolve().parents[3]
def read(p): return json.loads(p.read_text(encoding='utf-8'))
def sha(p): return hashlib.sha256(p.read_bytes()).hexdigest()
def write_text(p,value):
    p.parent.mkdir(parents=True,exist_ok=True)
    if not p.exists() or p.read_text(encoding='utf-8')!=value:p.write_text(value,encoding='utf-8')
def write(p,v): write_text(p,json.dumps(v,indent=2)+'\n')
def merge(site=None):
    runtime=ROOT/'assets/runtime';assets=[]
    def append(record):
        url=record['url'];assert url.startswith('assets/') and '..' not in Path(url).parts and not url.startswith('/'),url
        path=runtime/url[7:];assert path.is_file(),url
        assert sha(path)==record['sha256'] and path.stat().st_size==record['bytes'],f'Changed accepted bytes: {url}'
        assets.append(record)
    garden=read(runtime/'models/garden/asset-manifest.json')
    for a in garden['assets']:append(dict(a,url='assets/models/garden/'+a['file'],rightsRecord='assets/models/garden/NOTICE.md'))
    marker=runtime/'models/garden/quest-marker-manifest.json'
    if marker.exists():
        for a in read(marker)['assets']:append(a)
    rewards=runtime/'models/rewards/manifest.json'
    if rewards.exists():
        for a in read(rewards)['assets']:append(a)
    pip=read(runtime/'models/pip/manifest.json')
    for a in pip['files']:
        if a['path'].startswith('assets/runtime/'):
            url='assets/'+a['path'][len('assets/runtime/'):]
            append(dict(a,id='cats/pip@1' if url.endswith('.glb') else 'pip/'+Path(url).stem,url=url,rightsRecord='assets/models/pip/NOTICE.md'))
    props=read(ROOT/'docs/release1/fragments/props.json')
    for a in props['assets']:append(dict(a,rightsRecord='assets/models/release1/props/PROVENANCE.md'))
    for a in read(runtime/'models/release1/props/thumbnails/manifest.json'):append(dict(a,id='shop/'+a['id'],rightsRecord='assets/models/release1/props/PROVENANCE.md'))
    for a in read(runtime/'fonts/manifest.json'):append(dict(a,id='fonts/'+a['id'],url='assets/fonts/'+a['file'],rightsRecord='assets/fonts/'+a['license']))
    for a in read(runtime/'textures/garden/manifest.json'):append(a)
    missing=[]
    cats=ROOT/'docs/release1/fragments/cats.json'
    catNames=['pip','mabel','button','pepper'];catRecords=[dict(cat='pip',id='cats/pip@1',url='assets/models/pip/pip.glb',sha256=sha(runtime/'models/pip/pip.glb'))]+[dict(cat=c,id='',url='',sha256='') for c in catNames[1:]]
    catPortraits=[]; measuredContacts=[]
    if cats.exists() and (ROOT/'docs/release1/prompt-02-handoff.md').is_file():
        for a in read(cats)['assets']:
            notice=runtime/'models/release1/cats/NOTICE.md'
            notice.parent.mkdir(parents=True,exist_ok=True)
            shutil.copyfile(ROOT/a['rightsRecord'],notice)
            append(dict(a,rightsRecord='assets/models/release1/cats/NOTICE.md'))
            name=a.get('cat',a.get('name'))
            if name in catNames and a['url'].endswith('.glb'):catRecords[catNames.index(name)]=dict(a,cat=name)
            for kind in ['portrait','contacts']:
                nested=a[kind]; url='assets/'+nested['path'][len('assets/runtime/'):]
                record=dict(nested,id=a['id']+'/'+kind,url=url,rightsRecord='assets/models/release1/cats/NOTICE.md')
                append(record)
                if kind=='portrait':catPortraits.append(dict(record,cat=name))
            measuredContacts+=compose(a,read(ROOT/a['contacts']['path']))
    else:missing.append('Prompt 2 accepted cat fragment/handoff and measured final furnishing contacts')
    lines=['// Generated from verified accepted manifest records, never model job exports.','static const CatInput catInputs[] = {']
    for a in catRecords:lines.append('{'+','.join(json.dumps(a[k]) for k in ['cat','id','url'])+'},')
    lines+=['};','static const PortraitInput catPortraitInputs[] = {']
    for a in catPortraits:lines.append('{'+json.dumps(a['cat'])+','+json.dumps(a['url'])+'},')
    lines+=['{nullptr,nullptr}};','void acceptedContacts(paws::release1::garden::Assets& assets) {']
    contacts=measuredContacts+read(ROOT/'scripts/release1/garden/contacts.json')
    def vec(v):
        assert len(v)==3 and all(math.isfinite(float(x)) for x in v)
        return '{'+','.join(str(float(x))+'f' for x in v)+'}'
    for row in contacts:
        if not row.get('verified'):continue
        i=catNames.index(row['cat']);assert row['meshSha256']==catRecords[i]['sha256'],'Contact targets a different accepted mesh'
        assert (ROOT/row['evidence']).is_file(),'Missing measured contact evidence'
        c=row['contact'];fields=[vec(c['approach']),vec(c['root']),str(float(c['yaw']))+'f']+[json.dumps(c[k]) for k in ['enter','loop','leave']]
        fields += [str(float(c[k])) for k in ['enterSeconds','leaveSeconds','still']]
        fields += ['{'+','.join(vec(v) for v in c.get('supportedRoots',[]))+'}','true']
        lines.append('assets.cats['+str(i)+'].contacts['+json.dumps(row['item'])+']={'+','.join(fields)+'};')
        if row.get('naturalCurl'):lines.append('assets.cats['+str(i)+'].naturalCurl=true;')
    lines.append('}')
    write_text(ROOT/'src/release1/integration/accepted.inc','\n'.join(lines)+'\n')
    write(ROOT/'build/evidence/release1/prompt-09/contact-inputs.json',dict(contacts=contacts,meshHashes={a['cat']:a['sha256'] for a in catRecords}))
    base=read(runtime/'audio/catalog.json');new=read(ROOT/'artifacts/audio-jobs/release1/catalog.json')
    # Preserve delivered runtime imports when refreshing the authored slot list.
    # Replacing the 19 release-one records erased their generated sound metadata.
    delivered={c['id']:c for c in base['clips']}
    additions=[]
    for c in new['clips']:
        record=dict(c,file='release1/'+c['file'])
        previous=delivered.get(c['id'],{})
        if previous.get('source') and previous.get('file')==record['file']:
            for key in ['accepted','playable','source','review','pcm','sha256','bytes','loopFrames']:
                if key in previous:record[key]=previous[key]
        additions.append(record)
    authoredIds={c['id'] for c in base['clips'][:47]}|{c['id'] for c in additions}
    extras=[c for c in base['clips'] if c['id'] not in authoredIds]
    base['clips']=base['clips'][:47]+additions+extras
    assert len(base['clips'])==66+len(extras)
    for c in base['clips']:
        playable=c['accepted'] or (c.get('playable') and c.get('review',{}).get('technicalVerified'))
        if playable:
            source=c['source'];rights=ROOT/source['rightsRecord'];assert rights.is_file(),c['id']
            if source.get('rightsSha256'):assert sha(rights)==source['rightsSha256'],c['id']
            notice=runtime/'audio'/(c['id']+'.source-rights.txt');shutil.copyfile(rights,notice)
            append(dict(id='audio/'+c['id'],url='assets/audio/'+c['file'],sha256=c['sha256'],bytes=c['bytes'],
                accepted=c['accepted'],rightsRecord='assets/audio/'+notice.name))
    absent=sum(not (c['accepted'] or (c.get('playable') and c.get('review',{}).get('technicalVerified'))) for c in base['clips'])
    pending=sum(not c['accepted'] for c in base['clips'])
    if pending:missing.append(f'{absent} clips have no playable delivery; final rights/listening pending for {pending} clips')
    write(runtime/'audio/catalog.json',base)
    manifest=dict(schemaVersion=3,contentId='paws-release1/1',quests=[q['contentId'] for q in read(ROOT/'docs/release1/contract.json')['quests']],
        assetCatalogues=['models/pip/manifest.json','models/garden/asset-manifest.json','models/garden/quest-marker-manifest.json','models/release1/props/manifest.json','audio/catalog.json'],
        assets=assets,assemblies=props['assemblies'],catalog=props['catalog'],missingAcceptance=missing,
        notice='Accepted runtime content only; source masters and development previews are excluded.')
    write(runtime/'manifest.json',manifest)
    for notice in ['models/pip/NOTICE.md','models/garden/NOTICE.md','models/release1/props/PROVENANCE.md','fonts/fraunces-OFL.txt','fonts/nunitosans-OFL.txt']:
        assert (runtime/notice).is_file(),notice
    if site:
        for a in assets:
            p=site/a['url'];assert p.is_file() and p.stat().st_size==a['bytes'] and sha(p)==a['sha256'],a['url']
            if a.get('rightsRecord'):assert (site/a['rightsRecord']).is_file(),a['rightsRecord']
        for forbidden in ['release1','pip-preview.html','garden-preview.html','pip_preview.js','garden_preview.js']:
            assert not (site/forbidden).exists(),f'Development file staged: {forbidden}'
        assert len(read(site/'assets/audio/catalog.json')['clips'])==len(base['clips'])
        audioUrls={a['url'] for a in assets if a['id'].startswith('audio/')}
        for p in (site/'assets/audio').rglob('*'):
            if p.suffix.lower() in {'.wav','.mp3','.ogg','.flac','.m4a'}:assert p.relative_to(site).as_posix() in audioUrls,'Unaccepted sound staged: '+str(p)
        assert read(site/'build-info.json')['enginePin']=='3fe7d1dc1dc610c703944803cbb717cd65024ec9'
    return dict(acceptedAssets=len(assets),acceptedGlbs=sum(a['url'].endswith('.glb') for a in assets),missingAcceptance=missing,site=str(site) if site else None)
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--site',type=Path);p.add_argument('--report',type=Path);args=p.parse_args()
    result=merge(args.site)
    if args.report:write(args.report,result)
    print(json.dumps(result,indent=2))

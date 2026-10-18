"""Bind final production evidence to immutable site and current source identities."""
import argparse,hashlib,json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[3]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf-8-sig'))
def main(site):
    site=site.resolve();evidence=ROOT/'build/evidence/release1/prompt-09';build=read(site/'build-info.json')
    assert sha(site/'paws.wasm')==build['wasmSha256']
    reports={}
    for name in ['browser-final','faults','uses-final']:
        p=evidence/name/'report.json';r=read(p);assert r.get('success'),name
        assert r['build']==build and r['loadedBuild']==build,name+' tested another payload'
        reports[name]=dict(path=p.relative_to(ROOT).as_posix(),sha256=sha(p),checks=r['checks'])
    manifest=read(site/'assets/manifest.json')
    for a in manifest['assets']:
        p=site/a['url'];assert p.stat().st_size==a['bytes'] and sha(p)==a['sha256'],a['url']
        if a.get('rightsRecord'):assert (site/a['rightsRecord']).is_file()
    actual={r['url'].split('/afternoon/')[1]:r for r in read(evidence/'browser-final/report.json')['loadedGlbs']}
    for a in manifest['assets']:
        if a['url'].endswith('.glb'):assert actual[a['url']]['sha256']==a['sha256'] and actual[a['url']]['bytes']==a['bytes']
    native=(evidence/'native-final.txt').read_text(encoding='utf-8-sig');assert '100% tests passed, 0 tests failed out of 21' in native
    paths=[p for folder in ['src','cmake','scripts/release1/integration'] for p in (ROOT/folder).rglob('*') if p.is_file() and p.suffix in {'.cpp','.hpp','.inc','.cmake','.py','.ps1','.cjs','.json'} and '__pycache__' not in p.parts]
    paths += [ROOT/'CMakeLists.txt',ROOT/'docs/release1/fragments/cats.json',ROOT/'docs/release1/fragments/props.json',ROOT/'docs/release1/contract.json']
    paths += [ROOT/'web'/n for n in ['index.html','host.js','saves.js','paws-audio.js','style.css']]
    sources={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(set(paths))}
    result=dict(build=build,site=str(site),reports=reports,native=dict(suites=21,sha256=sha(evidence/'native-final.txt')),
        loadedGlbs=len(actual),assets=len(manifest['assets']),missingAcceptance=manifest['missingAcceptance'],
        sourceHashes=sources,payloadHashes={p.relative_to(site).as_posix():sha(p) for p in sorted(site.rglob('*')) if p.is_file()})
    (evidence/'identity.json').write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8')
    print(json.dumps({k:v for k,v in result.items() if k not in ['sourceHashes','payloadHashes','reports']},indent=2))
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--site',type=Path,default=ROOT/'build/web-release1-p09-pinned-release/site');main(p.parse_args().site)

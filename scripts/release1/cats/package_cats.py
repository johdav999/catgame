"""Package reviewed exports only after the exact bytes pass all automated gates."""
import argparse,hashlib,json,re,shutil
from pathlib import Path
from validate_exports import ROOT,validate,read_glb

JOB=ROOT/'artifacts/model-jobs/release1-cats'
EVIDENCE=ROOT/'build/evidence/release1/prompt-02'
CATS=['pip','mabel','button','pepper']
def identity(path):
    data=path.read_bytes();return {'path':path.relative_to(ROOT).as_posix(),'bytes':len(data),'sha256':hashlib.sha256(data).hexdigest()}
def main(revision):
    intake=json.loads((JOB/'intake.json').read_text())
    assert identity(ROOT/intake['authority']['glb']['path'])==intake['authority']['glb'],'Accepted Pip GLB changed'
    assert identity(ROOT/intake['authority']['master']['path'])==intake['authority']['master'],'Accepted Pip master changed'
    browser=json.loads((EVIDENCE/'browser/report.json').read_text());assert browser.get('passed') and not browser['errors']
    native=(EVIDENCE/'native-tests.log').read_text()
    assert all(cat+': canonical bind,' in native for cat in CATS),'Native tests incomplete'
    native_report=json.loads((EVIDENCE/'native-report.json').read_text());assert native_report['passed']
    fragment={'schema':'paws-release1-fragment/1','task':2,'namespace':'release1/cats','assets':[],
              'assemblies':[],'provenance':['artifacts/model-jobs/release1-cats/PROVENANCE.md'],
              'reuses':['cats/pip@1'],'authority':intake['authority'],'enginePin':'3fe7d1dc1dc610c703944803cbb717cd65024ec9',
              'evidence':{'rawGLB':'build/evidence/release1/prompt-02/glb-validation.json','native':'build/evidence/release1/prompt-02/native-tests.log',
                          'browser':'build/evidence/release1/prompt-02/browser/report.json'},
              'integrationOwner':'Prompt 9; application movement and furnishing choreography belong to Prompt 6'}
    for cat in CATS:
        folder=JOB/cat;source=folder/'exports'/f'{cat}-r{revision}.glb';gate=validate(source)
        seen=next(x for x in browser['cats'] if x['cat']==cat)
        assert seen['glbSha256']==gate['sha256'],'Browser evidence does not match export'
        assert native_report['files'][cat]['sha256']==gate['sha256'],'Native evidence does not match export'
        contact=json.loads((folder/'contacts.json').read_text())
        assert contact['export']==source.relative_to(ROOT).as_posix(),'Reimport/contact revision differs'
        motion=json.loads((folder/'animation-measurements.json').read_text());assert motion['revision']==revision
        runtime_motion=json.loads(json.dumps(motion))
        runtime_motion.update(schema='paws-cat-motion-measurements/1',units='metres',up='+Y',forward='+Z')
        for action in runtime_motion['actions'].values():
            for sample in action['samples']:
                lo,hi=sample['min'],sample['max'];sample['min']=[lo[0],lo[2],-hi[1]];sample['max']=[hi[0],hi[2],-lo[1]]
                sample['supportAdjustmentY']=sample.pop('groundLift')
                for paw in sample['paws'].values():
                    x,y,z=paw['center'];paw['center']=[x,z,-y];paw['soleY']=paw.pop('minZ')
        dense=folder/'animation-measurements-runtime.json';dense.write_text(json.dumps(runtime_motion,indent=2)+'\n')
        bounds=contact['restBounds'];footprint=[bounds['max'][i]-bounds['min'][i] for i in [0,2]]
        assert footprint[0]<.6 and bounds['max'][1]<1.1,'Playhouse entrance contract exceeded'
        drink=contact['samples']['drink'][0]['muzzle_contact']
        assert abs(drink[1]-.392)<.012,'Muzzle does not meet the existing bowl water plane'
        rest=contact['samples']['curled_rest'][0]['bounds']
        rest_centre=[(rest['min'][i]+rest['max'][i])/2 if i!=1 else 0 for i in range(3)]
        if cat in ['pip','button']:
            assert rest['max'][0]-rest['min'][0]<.94 and rest['max'][2]-rest['min'][2]<1.18,'Cushion footprint exceeded'
        target=ROOT/'assets/runtime/models/release1/cats'/cat;target.mkdir(parents=True,exist_ok=True)
        for src,dst in [(source,target/f'{cat}.glb'),(folder/'renders/portrait.png',target/'portrait.png'),(folder/'contacts.json',target/'contacts.json')]:shutil.copy2(src,dst)
        # Retain editable separate map files as well as the GLB's embedded maps.
        doc,binary,_=read_glb(source);maps=[];textures=folder/'textures/final';textures.mkdir(parents=True,exist_ok=True)
        for index,img in enumerate(doc['images']):
            view=doc['bufferViews'][img['bufferView']];start=view.get('byteOffset',0)
            name=re.sub('[^A-Za-z0-9_-]+','-',img.get('name','image')).strip('-')
            path=textures/f'{index}-{name}.png';path.write_bytes(binary[start:start+view['byteLength']]);maps.append(identity(path))
        asset={'id':f'release1/cats/{cat}@1','cat':cat,'version':1,'url':f'assets/models/release1/cats/{cat}/{cat}.glb',
               **{k:gate[k] for k in ['sha256','bytes','triangles','materials','embeddedImages','bones']},
               'bounds':bounds,'footprint':footprint,'pivot':'root on support plane; metres, Y up, +Z forward',
               'portrait':identity(target/'portrait.png'),'master':identity(folder/f'{cat}-r{revision}.blend'),
               'cleanReimport':identity(folder/f'{cat}-clean-reimport.blend'),'materialMaps':maps,
               'rightsRecord':'artifacts/model-jobs/release1-cats/PROVENANCE.md',
               'sourceIdentity':intake['cats'][cat] if cat!='pip' else intake['authority'],
               'rigAuthority':'docs/release1/canonical-rig.json','actions':[{k:v for k,v in data.items() if k!='samples'}|{'name':name} for name,data in motion['actions'].items()],
               'contacts':identity(target/'contacts.json'),'denseMeasurements':identity(dense),
               'anchors':{'root_origin':{'position':[0,0,0],'rotationDegrees':[0,0,0]},
                          'drink_muzzle':{'position':drink,'rotationDegrees':[0,0,0],'action':'drink','seconds':0},
                          'drink_root_from_water':{'position':[-drink[0],0,-drink[2]],'rotationDegrees':[0,0,0],'waterY':.383,'muzzleClearance':drink[1]-.383},
                          'rest_root':{'position':rest_centre,'rotationDegrees':[0,0,0],'action':'curled_rest','supportPlaneY':0},
                          'rest_root_from_support':{'position':[-x for x in rest_centre],'rotationDegrees':[0,0,0],'action':'curled_rest','supportPlaneY':0}},
               'walk':{'duration':1.2,'strideMetres':.36,'speedMetresPerSecond':.30,'rootMotion':False},
               'choreography':'Use measured per-action contacts. Approach/step/top/exit actor motion and prop support are application-owned; no animation event commits rules or rewards.'}
        (target/'manifest.json').write_text(json.dumps(asset,indent=2)+'\n');fragment['assets'].append(asset)
    path=ROOT/'docs/release1/fragments/cats.json';path.parent.mkdir(parents=True,exist_ok=True);path.write_text(json.dumps(fragment,indent=2)+'\n')
    (ROOT/'assets/runtime/models/release1/cats/manifest.json').write_text(json.dumps(fragment,indent=2)+'\n')
    (JOB/'asset-manifest.json').write_text(json.dumps(fragment,indent=2)+'\n')
    print('Packaged four verified cats, portraits, contact contracts and namespaced fragment')

if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('revision',type=int);main(p.parse_args().revision)

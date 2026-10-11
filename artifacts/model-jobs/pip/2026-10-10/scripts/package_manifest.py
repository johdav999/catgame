"""Record the inspected asset package; does not generate or approve artwork."""
import hashlib,json,struct
from pathlib import Path
JOB=Path(__file__).resolve().parents[1];REPO=JOB.parents[3]
def file_record(path):
    path=Path(path);data=path.read_bytes();r={'path':path.relative_to(REPO).as_posix(),'bytes':len(data),'sha256':hashlib.sha256(data).hexdigest()}
    if path.suffix=='.png':r['dimensions']=list(struct.unpack_from('>II',data,16))
    return r
measurement=json.loads((JOB/'exports/measurements.json').read_text())
clean=json.loads((JOB/'exports/clean-reimport.json').read_text())
runtime=REPO/'assets/runtime/models/pip'
record={
 'schema':'paws-authored-asset/1','asset_id':'cats/pip@1','revision':'pip-r9','created':'2026-10-10',
 'engine_pin':'3fe7d1dc1dc610c703944803cbb717cd65024ec9',
 'runtime_url':'assets/models/pip/pip.glb','units':'metres',
 'source_axes':{'up':'+Z','forward':'-Y'},'runtime_axes':{'up':'+Y','forward':'+Z'},
 'pivot':'ground-centred root below torso','root_policy':'in-place root; authored pelvis contact correction; caller owns world movement',
 'mesh':measurement,'clean_reimport':clean,
 'rig':{'id':'paws-cat-rig-v1','deform_bones':29,'engine_joints_including_wrappers':31,'max_influences':4,'sample_rate':30,'rest_pose':'standing quadruped, upright hook tail','morphs':False},
 'anchors':[
   {'name':'root_contact','space':'mesh bind, runtime axes','position':[0,0,0]},
   {'name':'muzzle','space':'mesh bind, runtime axes','position':[0,.643,.647],'bone':'head','usage':'transform bind point through head skin matrix; fit final bowl lip in phase 7'},
   {'name':'front_paw.L','space':'mesh bind, runtime axes','position':[-.137,.055,.310]},
   {'name':'front_paw.R','space':'mesh bind, runtime axes','position':[.137,.055,.310]}],
 'material':{'count':1,'opaque':True,'albedo':'1024x1024 unique UV atlas, sRGB','source':'built-in OpenAI ImageGen colour variation + authored object-space tabby/cream masks','roughness':.86,'metallic':0,'normal':'+Y neutral tangent normal; geometric coat masses provide relief','normal_source':'independently authored','roughness_source':'independently authored, not inferred from colour'},
 'source_master':file_record(JOB/'pip-master.blend'),
 'source_files':[file_record(p) for p in sorted((JOB/'scripts').glob('*.py'))]+[file_record(JOB/'sources/ginger-color-imagegen.png')],
 'portable_maps':[file_record(p) for p in sorted((JOB/'textures').glob('*.png'))],
 'runtime_files':[file_record(p) for p in sorted(runtime.iterdir()) if p.suffix in ['.glb','.png','.md']],
 'portraits':{'dialogue':'assets/models/pip/portrait-dialogue.png','shop':'assets/models/pip/portrait-shop.png','ending':'assets/models/pip/portrait-ending.png','source':'same packed final model; portrait-only lowered tail for head framing'},
 'validation':{'blender':'3.5.1','clean_process':True,'portable_tests':'build/native-pip; paws_pip_asset and paws_starter_scene','browser_report':'build/evidence/pip/pip-smoke.json','evaluation':'artifacts/model-jobs/pip/2026-10-10/EVALUATION.md'},
 'limits':['Final garden/cushion/bowl composition is phase 5/7 work','Physical mobile not tested','Browser PBR lighting is flatter than Cycles review lighting','Chirrup audio is phase 9 work'],
 'rights_record':'PROVENANCE.md; original task-authored geometry/actions and built-in ImageGen colour source; no fabricated provider license document'
}
(JOB/'asset-manifest.json').write_text(json.dumps(record,indent=2)+'\n')
runtime_record={k:record[k] for k in ['schema','asset_id','revision','engine_pin','runtime_url','units','runtime_axes','root_policy','rig','anchors','material','portraits','limits']}
runtime_record['clips']=measurement['clips']
runtime_record['files']=[dict(r,path=Path(r['path']).name) for r in record['runtime_files']]
(runtime/'manifest.json').write_text(json.dumps(runtime_record,indent=2)+'\n')
print('Recorded',record['asset_id'],record['source_master']['sha256'])

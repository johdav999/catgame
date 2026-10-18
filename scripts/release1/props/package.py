"""Publish only Prompt 3 assets and compile its isolated preview contract."""
import json,hashlib,shutil,struct,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[3]
JOB=ROOT/'artifacts/model-jobs/release1-props/2026-10-11'
revision=int(sys.argv[1]) if len(sys.argv)>1 else 6
source=JOB/'exports'/f'r{revision}'
dest=ROOT/'assets/runtime/models/release1/props'
measure=json.loads((source/'measurements.json').read_text())
for water in ('water_curve','water_straight'):measure['layout'][water][1]=.02
legacy=json.loads((ROOT/'assets/runtime/models/garden/asset-manifest.json').read_text())
rights='assets/runtime/models/release1/props/PROVENANCE.md'
fragment={'schema':'paws-release1-fragment/1','task':3,'namespace':'release1/props','revision':revision,
 'axes':{'units':'metres','up':'+Y','forward':'+Z','right':'+X'},'assets':[],'assemblies':{},
 'provenance':{'rightsRecord':rights,'master':str((JOB/f'props-r{revision}.blend').relative_to(ROOT)).replace('\\','/'),'materialSource':'artifacts/model-jobs/garden-kit/2026-10-10/PROVENANCE.md'},
 'reuse':[], 'roleBindings':{}, 'catalog':[], 'pending':['Final cushion board/socket positions from Prompt 4','Per-cat contacts against final Mabel/Button/Pepper meshes in Prompt 6']}
def identity(name):return 'release1/props/'+name+'@1'
def part(name,p=(0,0,0),r=(0,0,0),legacy=False):
 return {'asset':('garden/'+name+'@1') if legacy else identity(name),'position':list(p),'rotationDegrees':list(r),'scale':[1,1,1]}
for a in measure['assets']:
 name=a['name'];file=source/(name+'.glb');shutil.copy2(file,dest/file.name)
 raw=file.read_bytes();n=struct.unpack_from('<I',raw,12)[0];g=json.loads(raw[20:20+n]);prim=[p for m in g['meshes'] for p in m['primitives']]
 a.update(id=identity(name),url='assets/models/release1/props/'+file.name,rightsRecord=rights,
          triangles=sum(g['accessors'][p['indices']]['count']//3 for p in prim),vertices=sum(g['accessors'][p['attributes']['POSITION']]['count'] for p in prim),
          materialCount=len(g.get('materials',[])),textureCount=len(g.get('images',[])),pivot='hang' if name.startswith('bell_') and name!='bell_support' else 'hinge' if name=='playhouse_door' else 'local footprint origin' if name.startswith(('cushion_','socket_')) else 'floor/contact centre')
 fragment['assets'].append(a)
for a in legacy['assets']:
 name=a['file'][:-4]
 fragment['reuse'].append({'id':a['id'],'url':'assets/models/garden/'+a['file'],'sha256':a['sha256'],'bytes':a['bytes'],'role':'unchanged accepted kit','rightsRecord':'assets/runtime/models/garden/NOTICE.md'})
fragment['assemblies']={
 'bells':[part('bell_support')]+[part('bell_'+k,(x,1.30,0)) for k,x in [('leaf',-.5),('star',0),('paw',.5)]],
 'cushion_basket':[part('socket_square'),part('socket_bar',(.66,0,0)),part('socket_elbow',(.66,0,.42))],
 'cushion_basket_fitted':[part('socket_square'),part('cushion_square',(0,.036,0)),part('socket_bar',(.66,0,0)),part('cushion_bar',(.66,.036,0)),part('socket_elbow',(.66,0,.42)),part('cushion_elbow',(.66,.036,.42))],
 'water_route':[part('water_support',(x,0,0)) for x in (-1,0,1)]+[part('rain_barrel',(-1,.4,0)),part('channel_straight',(0,.4,0),legacy=True),part('channel_curve',(1,.4,0),legacy=True),part('water_straight',(0,.472,0)),part('water_curve',(1,.472,0)),part('water_drop',(1,0,-.5)),part('bowl',(1,0,-.90),legacy=True),part('bowl_water',(1,.383,-.90),legacy=True)],
 'playhouse':[part('playhouse_frame'),part('playhouse_door',(-.36,.07,.73)),part('switch_base',(-.85,0,1.25),legacy=True),part('switch_plate',(-.85,.09,1.25),legacy=True),part('switch_base',(.85,0,1.25),legacy=True),part('switch_plate',(.85,.09,1.25),legacy=True)],
 'retrieval':[part('bench',legacy=True),part('mouse',(0,0,0),legacy=True),part('crate',(-1.1,0,.5)),part('crate',(1.1,0,.5))],
 'plank_route':[part('platform',(-1.5,0,0)),part('plank_2m'),part('platform',(1.5,0,0)),part('plank_socket',(-1,0,0)),part('plank_socket',(1,0,0))],
 'picnic':[part('low_picnic_table'),part('picnic_mat',(2,0,0)),part('cushion',(-1.2,0,0),legacy=True),part('bowl',(0,0,1.1),legacy=True),part('bowl_water',(0,.383,1.1),legacy=True),part('shade_clue',(-1.2,0,1.1)),part('adjacency_clue',(1.2,0,1.1)),part('quiet_clue',(-1.2,0,-1))]
}
fragment['assemblyNotes']={'cushion_basket':'Modular inserts; authored review arrangement only. Apply Prompt 4 socket origins/rotations before integration.', 'water_route':'Unmodified gutter ports retain local y=.06; supported route assembly raises them .40 m. Pouring ribbon clears the existing .408 m bowl rim and ends at .383 m water plane.', 'playhouse':'Door open +100 degrees about local Y. Switch plate released y=.09, pressed y=.055. Both independently addressable.', 'bells':'Hang pivots. Swing each bell about local X; no shared geometry mutation.'}
roles=fragment['roleBindings']
for a in fragment['assets']:roles[a['name']]=a['id']
roles['plank']=identity('plank_1m')
roles.update(bench='garden/bench@1',mouse='garden/mouse@1',gutter='garden/channel_straight@1',gutter_curve='garden/channel_curve@1',drinking_bowl='garden/bowl@1',water_surface='garden/bowl_water@1',picnic_bed='garden/cushion@1',picnic_bowl='garden/bowl@1',switch_pip='assembly:legacy_switch',switch_pepper='assembly:legacy_switch',cushion_basket='assembly:cushion_basket',reach_socket='anchor:bench/reach_socket')
fragment['anchorOverrides']={'garden/bench@1':{'reach_socket':{'position':[0,.02,.32],'rotationDegrees':[0,0,0]}},'garden/bowl@1':{'drink_root':{'position':[0,0,-.61327],'rotationDegrees':[0,0,0]},'water_surface':{'position':[0,.383,0],'rotationDegrees':[0,0,0]},'muzzle_contact':{'position':[0,.383,0],'rotationDegrees':[0,0,0]}},'garden/switch_base@1':{k:{'position':v,'rotationDegrees':[0,0,0]} for k,v in {'plate_released':[0,.09,0],'plate_pressed':[0,.055,0],'paw_contact':[0,.105,0]}.items()}}
fragment['assemblies']['legacy_switch']=[part('switch_base',legacy=True),part('switch_plate',(0,.09,0),legacy=True)]
fragment['anchorOverrides']['garden/bowl_water@1']={'fill_origin':{'position':[0,0,0],'rotationDegrees':[0,0,0]}}
contract=json.loads((ROOT/'docs/release1/contract.json').read_text())
for item in contract['items']:
 key=item['id'];asset='garden/cushion@1' if key=='garden_cushion' else 'garden/bowl@1' if key=='drinking_bowl' else 'assembly:playhouse_furnishing' if key=='playhouse' else identity(key)
 fragment['catalog'].append({**item,'asset':asset,'thumbnail':'assets/models/release1/props/thumbnails/'+key+'.png','contactStatus':'authored support/approach anchors; final per-cat contact acceptance belongs to Prompt 6'})
fragment['assemblies']['playhouse_furnishing']=[part('playhouse_frame'),part('playhouse_door',(-.36,.07,.73),(0,100,0))]
fragment['shapeGrid']={**contract['modelContract']['shapes'],'origin':'lower-left footprint boundary; +X/+Z cell axes','cushionInsetMetres':.016,'socketFloorY':.036,'rotation':'quarter turns about footprint_origin, then normalize occupied offsets; transform socket and cushion together'}
fragment['assemblyAnchors']={'cushion_basket':{k:{'position':p,'rotationDegrees':[0,0,0]} for k,p in {'socket_square':[0,.036,0],'socket_bar':[.66,.036,0],'socket_elbow':[.66,.036,.42]}.items()}}
fragment['contactEnvelopes']={
 'sunny_perch':{'deckMetres':[.78,.86],'surfaceY':.46,'stepY':.20,'approachReservedBeyondFootprint':True},
 'hammock':{'clothMetres':[.8,1.18],'centreY':.33,'surface':'y=.33+.30*(z/.59)^2+.035*(x/.4)^2; final curl needs matching soft-contact offsets'},
 'playhouse':{'entryClearMetres':[.76,1.13],'floorY':.07,'doorOpenDegrees':100,'doorSweep':'reserves space beyond placement footprint; do not collide with neighbouring furniture'},
 'bench':{'retrievalClearMetres':[.6,.25],'catTraversal':False},
 'picnic_mat':{'surfaceY':.04,'sitAnchors':'allocation guides; final per-cat body spacing requires Prompt 6 reservation and contact review'}}
text=json.dumps(fragment,indent=2)+'\n';(ROOT/'docs/release1/fragments/props.json').write_text(text);(dest/'manifest.json').write_text(text);(JOB/'asset-manifest.json').write_text(text)

# All geometry loaded through the pinned engine importer. No GLB-node actor assumption.
def vec(v):return '{'+','.join(f'{x:.8f}f' for x in v)+'}'
out=['// Generated by scripts/release1/props/package.py.','static const paws::GardenAsset assetSpecs[] = {']
for a in fragment['assets']:
 out.append('{'+','.join([json.dumps(a['name']),json.dumps(a['id']),json.dumps(a['url']),vec(a['bounds']['min']),vec(a['bounds']['max']),str(a['triangles'])])+'},')
out+=['};','static const PropAnchor anchorSpecs[] = {']
for a in fragment['assets']:
 for name,an in a['anchors'].items():out.append('{'+','.join([json.dumps(a['name']),json.dumps(name),vec(an['position'])])+'},')
out+=['};','static const PropInstance instanceSpecs[] = {']
for name,p in measure['layout'].items():out.append('{'+','.join([json.dumps(name),vec(p),vec([0,0,0]),'0'])+'},')
groups=[]
for group,(key,parts) in enumerate(fragment['assemblies'].items(),1):
 origin=[(group-1)%4*5-7.5,0,16+(group-1)//4*5];groups.append((key,origin))
 for p in parts:
  name=p['asset'].split('/')[-1].split('@')[0];world=[x+y for x,y in zip(origin,p['position'])]
  out.append('{'+','.join([json.dumps(name),vec(world),vec(p['rotationDegrees']),str(group)])+'},')
out+=['};','static const PropView viewSpecs[] = {','{"Whole new kit",{0,.2f,0},18,18},']
for a in fragment['assets']:
 p=measure['layout'][a['name']];b=a['bounds'];centre=[p[i]+(b['min'][i]+b['max'][i])/2 for i in range(3)];size=max(b['max'][i]-b['min'][i] for i in range(3))
 out.append('{'+json.dumps(a['name'])+','+vec(centre)+f',{max(1.15,size*1.9):.6f}f,{max(.85,size*1.5):.6f}f'+'},')
for name,p in groups:out.append('{'+json.dumps(name)+','+vec([p[0],.45,p[2]])+',4.6f,3.5f},')
out+=['};'];(ROOT/'src/release1/props/manifest.inc').write_text('\n'.join(out)+'\n')
(ROOT/'web/release1/props/views.json').write_text(json.dumps(['Whole new kit']+[a['name'] for a in fragment['assets']]+[g[0] for g in groups]))
print(json.dumps({'assets':len(fragment['assets']),'triangles':sum(a['triangles'] for a in fragment['assets']),'bytes':sum(a['bytes'] for a in fragment['assets'])}))

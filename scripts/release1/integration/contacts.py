"""Compose furnishing contacts from immutable cat surface measurements.

This validates contact geometry; browser inspection is separately recorded in
Prompt 9 evidence. It never accepts an absent model or substitutes bone origins.
"""
import math,json
from pathlib import Path

def compose(record, measurements):
    if measurements.get('rig')=='toon-cat/1':return compose_toon(record,measurements)
    anchors=record['anchors'];samples=measurements['samples'];rows=[]
    def add(item,approach,root,yaw,enter,loop,leave,seconds=2.4,leave_seconds=2.4,still=0,path=(),curl=False):
        assert all(math.isfinite(n) for v in [approach,root,*path] for n in v)
        assert all(name in samples for name in [enter,loop,leave])
        rows.append(dict(cat=record['cat'],item=item,meshSha256=record['sha256'],verified=True,
            evidence=record['contacts']['path'],naturalCurl=curl,
            contact=dict(approach=approach,root=root,yaw=yaw,enter=enter,loop=loop,leave=leave,
                         enterSeconds=seconds,leaveSeconds=leave_seconds,still=still,supportedRoots=list(path))))
    rest=anchors['rest_root_from_support']['position']
    bounds=samples['curled_rest'][0]['bounds'];width=bounds['max'][0]-bounds['min'][0];depth=bounds['max'][2]-bounds['min'][2]
    compact=record['cat'] in ('pip','button') and width<=.82 and depth<=.82
    add('garden_cushion',[rest[0],0,rest[2]+.85],[rest[0],.16,rest[2]],0,'settle_cushion','curled_rest','settle_cushion',curl=compact)
    drink=anchors['drink_root_from_water']['position'];clearance=anchors['drink_root_from_water']['muzzleClearance']
    assert 0<=clearance<=.025, 'Muzzle must meet the actual water plane'
    add('drinking_bowl',[drink[0],0,-1.1],drink,0,'approach_bowl','drink','return_idle',1.8,1.8)
    if record['cat']=='pip':
        paws=samples['scratch'][0]['paws'];front=[sum(paws[k]['centre'][n] for k in ('front.L','front.R'))/2 for n in range(3)]
        add('scratching_post',[0,0,1.1],[front[0],0,.13+front[2]],math.pi,'scratch','scratch','return_idle',3,1.8,.5)
    if record['cat']=='mabel':
        paws=samples['perch_settle'][-1]['paws'];centre=[sum(p['centre'][n] for p in paws.values())/4 for n in range(3)]
        root=[centre[0],.46,centre[2]]
        add('sunny_perch',[0,0,1.25],root,math.pi,'perch_step','perch_settle','perch_step',2.4,2.4,2.4,
            [[0,0,1.0],[0,.2,.575],root])
    if record['cat']=='button':
        # Full-loop skin/cloth measurement is bound to this immutable export.
        # A bounding rectangle overestimates the lift and visibly floats the cat.
        measured=json.loads((Path(__file__).with_name('contact-measurements.json')).read_text())['buttonHammock']
        assert record['sha256']==measured['meshSha256'],'New cat needs a fresh cloth-contact measurement'
        root=[rest[0],measured['rootY'],rest[2]]
        add('hammock',[.95,0,0],root,0,'settle_cushion','curled_rest','settle_cushion',2.4,2.4,0,[[.85,0,0],root],compact)
    if record['cat'] in ('pip','pepper'):
        add('playhouse',[0,0,1.6],[0,.07,0],math.pi,'playhouse_enter','playhouse_lookout','playhouse_enter',1.8,1.8,0,
            [[0,0,1.6],[0,.07,.75],[0,.07,0]])
    i=['pip','mabel','button','pepper'].index(record['cat']);yaw=[math.pi/2,0,-math.pi/2,math.pi][i]
    table=[[-.85,0,0],[0,0,-.95],[.85,0,0],[0,0,.95]][i]
    add('low_picnic_table',[n*1.5 for n in table],table,yaw,'picnic_sit','picnic_sit','return_idle',2.4,1.8,2.4)
    mat=[[-1.4,0,0],[0,0,-1.4],[1.4,0,0],[0,.04,.1]][i]
    add('picnic_mat',[mat[0],0,mat[2]+(-.4 if i==1 else .8)],mat,yaw,'picnic_sit','picnic_sit','return_idle',2.4,1.8,2.4)
    return rows

def compose_toon(record,measurements):
    """Contacts measured from the replacement rig, never the retired cat skin."""
    rows=[];samples=measurements['samples'];cat=record['cat']
    rest=samples['curled_rest'][0]['bounds']
    centre=[-(rest['min'][i]+rest['max'][i])/2 for i in range(3)]
    floor=min(s['bounds']['min'][1] for s in samples['curled_rest'])
    def add(item,approach,root,yaw,enter,loop,leave,seconds=2.4,leave_seconds=2.4,still=0,path=(),resting=False):
        for clip in (enter,loop,leave):assert clip in samples
        rows.append(dict(cat=cat,item=item,meshSha256=record['sha256'],verified=True,evidence=record['contacts']['path'],naturalCurl=resting,
            contact=dict(approach=approach,root=root,yaw=yaw,enter=enter,loop=loop,leave=leave,enterSeconds=seconds,leaveSeconds=leave_seconds,still=still,supportedRoots=list(path))))
    restroot=[centre[0],.16-floor,centre[2]]
    add('garden_cushion',[0,0,.85],restroot,0,'settle_cushion','curled_rest','rest_leave',resting=True)
    nose=samples['drink'][0]['muzzle'];assert .383<=nose[1]<=.408
    drink=[-nose[0],0,-nose[2]]
    add('drinking_bowl',[drink[0],0,-1.1],drink,0,'approach_bowl','drink','return_idle',1.8,1.8)
    if cat=='pip':
        # Use the elevated-paw part of the authored sharpening cycle.
        s=max(samples['scratch'],key=lambda s:sum(s['paws'][k]['centre'][1] for k in ('front.L','front.R')))
        front=[sum(s['paws'][k]['centre'][i] for k in ('front.L','front.R'))/2 for i in range(3)]
        add('scratching_post',[0,0,1.1],[front[0],0,.13+front[2]],math.pi,'scratch','scratch','return_idle',3,1.8,s['seconds'])
    if cat=='mabel':
        root=[0,.46,0]
        add('sunny_perch',[0,0,1.25],root,math.pi,'perch_step','sit_idle','perch_step',2.4,2.4,0,[[0,0,1],[0,.2,.575],root])
    if cat=='button':
        root=[centre[0],measurements['hammockRootY'],centre[2]]
        add('hammock',[.95,0,0],root,0,'settle_cushion','curled_rest','rest_leave',2.4,2.4,0,[[.85,0,0],root],True)
    if cat in ('pip','pepper'):
        add('playhouse',[0,0,1.6],[0,.07,0],math.pi,'playhouse_enter','playhouse_lookout','playhouse_enter',1.8,1.8,0,[[0,0,1.6],[0,.07,.75],[0,.07,0]])
    i=['pip','mabel','button','pepper'].index(cat);yaw=[math.pi/2,0,-math.pi/2,math.pi][i]
    table=[[-.85,0,0],[0,0,-.95],[.85,0,0],[0,0,.95]][i]
    add('low_picnic_table',[n*1.5 for n in table],table,yaw,'picnic_sit','sit_idle','sit_leave',2.4,1.8)
    mat=[[-1.4,0,0],[0,0,-1.4],[1.4,0,0],[0,.04,.1]][i]
    add('picnic_mat',[mat[0],0,mat[2]+(-.4 if i==1 else .8)],mat,yaw,'picnic_sit','sit_idle','sit_leave',2.4,1.8)
    return rows

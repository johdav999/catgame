import bpy,json,sys
from pathlib import Path
from mathutils import Vector,Matrix
J=Path(__file__).parent
stage=sys.argv[sys.argv.index('--')+1]
bpy.ops.wm.open_mainfile(filepath=str(J/('clean-reimport.blend' if stage=='export' else 'pip-blink.blend')))
s=bpy.context.scene;r=bpy.data.objects['PipRig'];body=bpy.data.objects['Pip repaired Tripo skin']
r.animation_data.action=None
for track in r.animation_data.nla_tracks:track.mute=True
for p in r.pose.bones:p.matrix_basis.identity()
eyes=json.loads((J/'native-fit.json').read_text())
for e in eyes:
 for k in ['centre','n','t']:e[k]=Vector(e[k])
up=Vector((0,0,1));cam=s.camera
target=(eyes[0]['centre']+eyes[1]['centre'])/2;front=(eyes[0]['n']+eyes[1]['n']).normalized()
s.render.engine='CYCLES';s.cycles.samples=24
s.render.resolution_x=900;s.render.resolution_y=600;s.render.resolution_percentage=100
cam.data.type='ORTHO';cam.data.ortho_scale=.37
for view,location in [('front',target+front*2+up*.03),('left',target+eyes[0]['n']*1.5-eyes[0]['t']*1.1+up*.08),('right',target+eyes[1]['n']*1.5+eyes[1]['t']*1.1+up*.08)]:
 cam.location=location;cam.rotation_euler=(target-location).to_track_quat('-Z','Y').to_euler()
 for amount in [0,.5,1]:
  for e in eyes:
   for name,dz in [(e['name'],-.075),(e['lower_name'],.060)]:
    b=r.data.bones[name];p=r.pose.bones[name]
    m=b.matrix_local.inverted()@Matrix.Translation(up*(dz*amount)+e['n']*(.020*amount))@b.matrix_local
    p.location,p.rotation_quaternion,p.scale=m.decompose()
  bpy.context.view_layer.update();s.render.filepath=str(J/'renders'/f'{stage}-{view}-{int(amount*100):03}.png');bpy.ops.render.render(write_still=True)
if stage=='source':
 measured={'vertices':len(body.data.vertices),'triangles':sum(len(p.vertices)-2 for p in body.data.polygons),'materials':len(body.data.materials),'bones':[b.name for b in r.data.bones],'max_influences':max(len(v.groups) for v in body.data.vertices),'dimensions_metres':list(body.dimensions)}
 (J/'measurements.json').write_text(json.dumps(measured,indent=2))

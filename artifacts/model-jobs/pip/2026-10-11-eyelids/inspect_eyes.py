import bpy,bmesh,json
import numpy as np
from pathlib import Path
from mathutils import Vector
JOB=Path(__file__).parent
bpy.ops.wm.open_mainfile(filepath=str(JOB.parent/'2026-10-10-tripo-repair/pip-repaired.blend'))
rig=bpy.data.objects['PipRig'];body=bpy.data.objects['Pip repaired Tripo skin']
rig.animation_data.action=None
for p in rig.pose.bones:p.matrix_basis.identity()
# Remove only the rejected cap islands.
bm=bmesh.new();bm.from_mesh(body.data)
bmesh.ops.delete(bm,geom=[f for f in bm.faces if f.material_index==1],context='FACES')
bm.to_mesh(body.data);bm.free();body.data.update()
image=next(n.image for n in body.data.materials[0].node_tree.nodes if n.type=='TEX_IMAGE' and 'Base' in n.label) if False else None
shader=body.data.materials[0].node_tree.nodes.get('Principled BSDF')
image=shader.inputs['Base Color'].links[0].from_node.image
pixels=np.array(image.pixels[:]).reshape(image.size[1],image.size[0],4)
data=[]
for loop in body.data.loops:
 v=body.data.vertices[loop.vertex_index];uv=body.data.uv_layers.active.data[loop.index].uv
 if .59<v.co.z<.80 and v.co.y<-.27:
  col=pixels[int((uv.y%1)*(image.size[1]-1)),int((uv.x%1)*(image.size[0]-1)),:3]
  data.append({'p':list(v.co),'n':list(v.normal),'uv':list(uv),'c':list(col)})
(JOB/'eye-samples.json').write_text(json.dumps(data))
dark=np.array([d['p'] for d in data if sum(d['c'])<.35])
for side in [-1,1]:
 d=dark[dark[:,0]*side>0];print('EYE',side,'min',d.min(axis=0),'max',d.max(axis=0),'median',np.median(d,axis=0))
scene=bpy.context.scene;scene.render.resolution_x=1000;scene.render.resolution_y=650
scene.camera.location=(.08,-2,.77);target=Vector((0,-.48,.72))
scene.camera.rotation_euler=(target-scene.camera.location).to_track_quat('-Z','Y').to_euler();scene.camera.data.ortho_scale=.44
scene.render.filepath=str(JOB/'baseline-eyes.png');bpy.ops.render.render(write_still=True)
bpy.ops.wm.save_as_mainfile(filepath=str(JOB/'eyes-source.blend'))

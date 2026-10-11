import bpy
from pathlib import Path
from mathutils import Vector
job=Path(__file__).parent
bpy.ops.wm.open_mainfile(filepath=str(job/'pip-blink.blend'))
scene=bpy.context.scene;rig=bpy.data.objects['PipRig'];cam=scene.camera
scene.render.resolution_x=scene.render.resolution_y=512
scene.render.resolution_percentage=100;scene.render.film_transparent=True
scene.render.image_settings.file_format='PNG';scene.render.image_settings.color_mode='RGBA'
for obj in scene.objects:
 if obj.type=='MESH' and obj.name!='Pip repaired Tripo skin':obj.hide_render=True
out=job/'portraits';out.mkdir(exist_ok=True)
for label,action,frame,location,target,scale in [
 ('dialogue','attentive_request',37,(.55,-2,.95),(0,-.37,.70),.78),
 ('shop','relaxed_idle',1,(.55,-2,.95),(0,-.37,.70),.78),
 ('ending','curled_rest',30,(1.45,-2.25,1.15),(0,0,.3),1.4)]:
 rig.animation_data.action=bpy.data.actions[action];scene.frame_set(frame)
 cam.location=location;cam.rotation_euler=(Vector(target)-cam.location).to_track_quat('-Z','Y').to_euler();cam.data.ortho_scale=scale
 scene.render.filepath=str(out/('portrait-'+label+'.png'));bpy.ops.render.render(write_still=True)

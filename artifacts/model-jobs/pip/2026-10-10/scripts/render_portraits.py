"""512px presentation portraits rendered from the accepted packed rig/atlas."""
import bpy,math
from mathutils import Vector,Euler
from pathlib import Path
JOB=Path(__file__).resolve().parents[1]
bpy.ops.wm.open_mainfile(filepath=str(JOB/'pip-master.blend'))
scene=bpy.context.scene;rig=bpy.data.objects['PipRig'];camera=scene.camera
scene.render.resolution_x=512;scene.render.resolution_y=512;scene.render.resolution_percentage=100
scene.render.film_transparent=True;bpy.data.objects['Review ground'].hide_render=True
scene.render.image_settings.color_mode='RGBA'
for name,action,frame in [('dialogue','attentive_request',37),('shop','relaxed_idle',1),('ending','curled_rest',30)]:
    rig.animation_data.action=bpy.data.actions[action];scene.frame_set(frame)
    if name!='ending':
        # Portrait-only tail pose: lower it behind the shoulders so a clipped
        # upright tail does not create a thin stray shape beside the face.
        p=rig.pose.bones['tail.00'];q=p.bone.matrix_local.to_quaternion()
        p.rotation_quaternion=q.inverted()@Euler((math.radians(-95),0,0)).to_quaternion()@q
        p.keyframe_insert('rotation_quaternion',frame=frame)
    camera.location=(.45,-2,.92) if name!='ending' else (1.45,-2.25,1.28)
    target=Vector((0,-.46,.71) if name!='ending' else (0,0,.24))
    camera.rotation_euler=(target-camera.location).to_track_quat('-Z','Y').to_euler();camera.data.ortho_scale=.64 if name!='ending' else 1.2
    bpy.context.view_layer.update();scene.render.filepath=str(JOB/('renders/portrait-'+name+'.png'));bpy.ops.render.render(write_still=True)

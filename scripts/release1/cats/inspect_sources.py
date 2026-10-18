"""Read-only source intake; run with Blender --background --python-exit-code 1."""
import bpy, json, hashlib
from pathlib import Path
from mathutils import Vector, Matrix

ROOT = Path(__file__).resolve().parents[3]
JOB = ROOT / 'artifacts/model-jobs/release1-cats'

def identity(path):
    return {'path': path.relative_to(ROOT).as_posix(), 'bytes': path.stat().st_size,
            'sha256': hashlib.sha256(path.read_bytes()).hexdigest()}

def stage(target, size=1.6):
    s = bpy.context.scene
    s.render.engine = 'BLENDER_EEVEE'
    s.eevee.use_gtao = True
    s.eevee.gtao_distance = .08
    s.render.resolution_x = s.render.resolution_y = 640
    s.render.resolution_percentage = 100
    s.view_settings.view_transform = 'Standard'
    s.view_settings.look = 'Medium High Contrast'
    s.world = bpy.data.worlds.new('Neutral review world')
    s.world.color = (.35, .35, .35)
    bpy.ops.mesh.primitive_plane_add(size=200, location=(0,0,-.008))
    floor = bpy.context.object
    floor.name = 'Review ground'
    mat = bpy.data.materials.new('Review sage')
    mat.diffuse_color = (.5,.57,.49,1)
    floor.data.materials.append(mat)
    for name, loc, energy in [('Key',(-3,-4,6),450),('Fill',(4,-1,3),300),('Rim',(0,4,5),300)]:
        d=bpy.data.lights.new(name,'AREA'); o=bpy.data.objects.new(name,d)
        s.collection.objects.link(o); o.location=loc; d.energy=energy; d.size=4
        o.rotation_euler=(Vector(target)-o.location).to_track_quat('-Z','Y').to_euler()
    bpy.ops.object.camera_add()
    s.camera=bpy.context.object; s.camera.data.type='ORTHO'; s.camera.data.ortho_scale=size
    return s

def render_views(s, folder, target=(0,0,.5), prefix='source'):
    folder.mkdir(parents=True, exist_ok=True)
    for name,loc in [('front',(0,-3,.8)),('side',(3,0,.8)),('rear',(0,3,.8)),('three-quarter',(1.7,-2.5,1.4)),('top',(0,0,4))]:
        s.camera.location=loc
        s.camera.rotation_euler=(Vector(target)-s.camera.location).to_track_quat('-Z','Y').to_euler()
        s.render.filepath=str(folder/f'{prefix}-{name}.png')
        bpy.ops.render.render(write_still=True)

def main():
    for cat in ['button','mabel','pepper']:
        folder=JOB/cat; folder.mkdir(parents=True,exist_ok=True)
        for sub in ['intake','references','renders','exports','textures','logs']:
            (folder/sub).mkdir(exist_ok=True)
        src=ROOT/'assets'/cat
        files=[identity(p) for p in sorted(src.rglob('*')) if p.is_file()]
        bpy.ops.wm.read_factory_settings(use_empty=True)
        bpy.ops.import_scene.fbx(filepath=str(src/f'{cat}.fbx'))
        report={'cat':cat,'sourceFiles':files,'provider':'Tripo3D (user supplied)',
                'rightsStatus':'User supplied export; account plan and generation rights record not supplied',
                'objects':[],'armatures':[]}
        meshes=[o for o in bpy.context.scene.objects if o.type=='MESH']
        for o in bpy.context.scene.objects:
            report['objects'].append({'name':o.name,'type':o.type,'matrixWorld':[list(r) for r in o.matrix_world]})
            if o.type=='ARMATURE':
                report['armatures'].append({'name':o.name,'bones':[{'name':b.name,'head':list(b.head_local),'tail':list(b.tail_local)} for b in o.data.bones]})
        coords=[o.matrix_world@v.co for o in meshes for v in o.data.vertices]
        lo=Vector(tuple(min(p[i] for p in coords) for i in range(3)))
        hi=Vector(tuple(max(p[i] for p in coords) for i in range(3)))
        report['sourceBounds']={'min':list(lo),'max':list(hi)}
        factor=1.05/(hi.z-lo.z)
        center=Vector(((lo.x+hi.x)/2,(lo.y+hi.y)/2,lo.z))
        for o in meshes:
            transform=o.matrix_world.copy()
            for mod in list(o.modifiers):o.modifiers.remove(mod)
            o.parent=None; o.matrix_world=Matrix.Identity(4)
            for v in o.data.vertices:v.co=(transform@v.co-center)*factor
            for p in o.data.polygons:p.use_smooth=True
            report.setdefault('meshes',[]).append({'name':o.name,'vertices':len(o.data.vertices),
                'triangles':sum(len(p.vertices)-2 for p in o.data.polygons),'materials':len(o.data.materials),
                'uvLayers':len(o.data.uv_layers),'groups':[g.name for g in o.vertex_groups]})
        for o in list(bpy.context.scene.objects):
            if o.type!='MESH':bpy.data.objects.remove(o,do_unlink=True)
        report['inspectionTransform']={'uniformScale':factor,'subtractWorldOrigin':list(center)}
        (folder/'intake/source-inventory.json').write_text(json.dumps(report,indent=2))
        s=stage((0,0,.5));render_views(s,folder/'renders')
        bpy.ops.file.pack_all()
        bpy.ops.wm.save_as_mainfile(filepath=str(folder/'intake/source-inspected.blend'))
        print('INTAKE',cat,json.dumps(report['meshes']),flush=True)

if __name__=='__main__':main()

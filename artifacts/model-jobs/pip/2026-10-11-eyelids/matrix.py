import bpy
bpy.ops.wm.open_mainfile(filepath='C:/Users/Johan/source/repos/CatGame/artifacts/model-jobs/pip/2026-10-11-eyelids/eyes-source.blend')
for n in ['PipRig','Pip repaired Tripo skin']:
 o=bpy.data.objects[n];print(n,'MATRIX',list(map(list,o.matrix_world)),'local',list(map(list,o.matrix_local)))

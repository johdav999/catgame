# Pip FBX compatibility check — 10 October 2026

## Verdict

The file is not ready for animated use in Paws. WebEngine can decode its geometry and skeleton when companion texture loading is disabled, and preserves the bind pose accurately. The source skin weights are incomplete. Default fully textured import also fails its memory budget.

Source: assets/Pip/pip.fbx
SHA-256: 60D739A7F5EDDFF7A40D561D434425D541384D0FEB152A819B5AC7B3985C4617
Pinned WebEngine: 3fe7d1dc1dc610c703944803cbb717cd65024ec9
The pinned submodule was clean. CMake confirmed the audit's native-pip libraries were current (ninja: no work to do). The input FBX, companion images and existing production Pip were not changed.

## Actual checks

- Blender 3.5.1 fresh-scene FBX import: one armature, 33 bones, one mesh, 9,892 source vertices, 19,780 triangles, no animation actions.
- Only bone_7, bone_12, bone_18, bone_19 and bone_28 have vertex groups/positive skin influences. 3,984 source vertices (40.3%) are unweighted. Maximum nonzero source influences: 3.
- WebEngine FbxImporter::loadFile with default settings: FAIL, "FBX textures exceed the total decoded size limit".
- All five companion maps are 4096 x 4096 (64 MiB each decoded RGBA). The importer requests base color, normal, roughness and metallic, plus creates a packed roughness/metallic map; its default total decoded budget is 128 MiB. It does not use the supplied rm image in this import path.
- Diagnostic import with an explicit resolver returning no texture bytes: 11,404 seam-split/optimized vertices, 19,780 triangles, one material, 36 hierarchy joints, 35 palette bindings, zero clips. Expected missing-texture warnings are from this deliberate diagnostic mode.
- Skeleton validation and SkeletalMeshComponent construction pass. Maximum bind-pose position error is 0.000000206477 metres.
- WebEngine assigns the unweighted geometry a rigid mesh-node fallback binding: 4,520 imported vertices. They have normalized palette weights, but are not bound to the cat bones.
- A direct SkeletonComponent pose test translating bone_0 by 0.1 m leaves 4,520 vertices stationary while 6,884 move. Therefore animation of the skeleton would leave substantial geometry behind.
- Paws validatePip rejects the imported asset at the first missing action: relaxed_idle. Other required actions are also absent. The generic bone_0...bone_32 names/hierarchy differ from Paws' existing canonical rig, so existing clips cannot be assumed compatible.
- Imported Y-up bounds: (-0.489593, approximately 0, -0.276367) to (0.489624, 0.842407, 0.276367) metres. Triangle count is 1,780 above the current provisional 18,000 allocation.

## Next steps

1. Correct/re-export skinning so the complete mesh is weighted to the intended anatomical bones, including leg chains, head and tail. Check deformation rather than just the presence of a skeleton.
2. Choose whether to adapt this mesh to the existing canonical Paws rig or deliberately replace that contract and retarget/re-author the actions. Bone renaming alone does not solve differing rest poses, hierarchy or missing weights.
3. Supply the nine required clips and validate their contact/loop/transition behavior. Render portraits from the same final model.
4. Reduce texture resolution to the game's intended budget (typically 1024; 2048 where justified), keeping materials and maps correct. Do not solve this by indiscriminately increasing mobile memory limits.
5. Re-run native import/deformation checks, then actual browser WebGPU playback.

## Evidence and scope

- inspect_fbx.py and blender-inspection.json: independent source rig inspection.
- engine_audit.cpp and engine-audit.txt: actual pinned engine import/pose diagnostics.
- default-import.txt: default import error.
- texture-dimensions.json: measured map sizes.
- engine_audit.exe: local diagnostic executable, requires the matching MinGW runtime on PATH.

Native CPU skin-palette tests were run. Browser GPU rendering, visual animation quality and physical mobile compatibility were not tested; the existing import and skinning failures already prevent acceptance. No fixes were applied to the supplied asset.


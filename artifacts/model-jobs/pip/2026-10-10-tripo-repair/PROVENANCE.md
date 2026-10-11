# Provenance

- Mesh and four original coat maps: user-supplied assets/Pip/pip.fbx and its accompanying .fbm directory. The original FBX SHA-256 is 60d739a7f5eddff7a40d561d434425d541384d0feb152a819b5ac7b3985c4617. The directory/file names identify a Tripo export; provider account/license terms were not supplied. No additional ownership or third-party license is asserted.
- Character design: design.md and the original generated Pip reference from the preceding request. The repair preserves the supplied mesh's coat identity.
- Canonical rig and initial clips: repository-authored Paws cat rig v1 from artifacts/model-jobs/pip/2026-10-10/pip-master.blend. The prior job and source scripts are preserved.
- Modifications: orient to -Y in Blender, scale to Paws metres, reduce to the triangle budget, discard old incomplete weights, bind every vertex to anatomical canonical bones, adapt contact/tail/blink channels, normalize FBX clip names and export unit handling.
- Texture changes: Blender downsampling of existing accepted source maps from 4096 to 1024; no regenerated coat pattern. This focused repair reuses the user-provided surface inputs rather than replacing its identity with new ImageGen output.
- Eyelids and 64-square closed-eye colour image: original procedural geometry/map authored in repair_pip.py. Bone scale/translation drives the opaque lids without morphs or transparency.
- Portraits: Blender renders of the same final repaired mesh and rig, never independently generated faces.
- No engine demo assets, stock skeleton, stock clips or external research imagery were imported.
- Content hashes and delivery sizes are in asset-manifest.json. Original maps/FBX and the previous runtime package are retained.


# Pip production asset — phase 3

Canonical revision: `pip@1`, authored 10 October 2026. The accepted package is the
revision-9 continuous mesh and rig, with 14,660 triangles, one atlas material,
29 deform bones and nine actions. WebEngine retains 31 joints including GLB
wrapper nodes. No engine source or gitlink change is required.

- Editable packed master: [pip-master.blend](pip-master.blend).
- Construction, UV/material authoring, skin weights and clips: [build_pip.py](scripts/build_pip.py).
- Final runtime asset: [pip.glb](exports/pip.glb); promoted to
  `assets/runtime/models/pip/pip.glb`.
- Same-model portraits: [dialogue](renders/portrait-dialogue.png),
  [shop](renders/portrait-shop.png), [ending](renders/portrait-ending.png).
- [Rig and runtime contract](../../../../docs/pip-asset.md),
  [manifest](asset-manifest.json), [provenance](PROVENANCE.md),
  [iterations](ITERATIONS.md), [evaluation](EVALUATION.md).

The source contains the hidden `Editable sculpt construction` collection, the
continuous skinned coat, attached facial details, UV atlas, nine editable actions,
and a stable review lighting/camera setup. Independent source masks/roughness/normal
maps and the original generated colour input are retained beside the master.

Rebuild with Blender 3.5.1 or a compatible Blender version:

```powershell
& $blender --background --python-exit-code 1 --python artifacts/model-jobs/pip/2026-10-10/scripts/build_pip.py -- --revision 9 --final
& $blender --background --python-exit-code 1 --python artifacts/model-jobs/pip/2026-10-10/scripts/verify_export.py
& $blender --background --python-exit-code 1 --python artifacts/model-jobs/pip/2026-10-10/scripts/pose_bounds.py
& $blender --background --python-exit-code 1 --python artifacts/model-jobs/pip/2026-10-10/scripts/render_portraits.py
python artifacts/model-jobs/pip/2026-10-10/scripts/package_manifest.py
```

Inspect regenerated renders before promoting files. The manifest script records
hashes; it does not approve new artwork. `artifacts/` is never deployed. Only the
GLB, three portraits, runtime manifest and notice belong in the runtime folder.

Preview the actual pinned Paws renderer at `/pip-preview.html` in the generated
site. Its HTML controls are a development inspection harness; production UI and
quest/furnishing composition remain the later phase contracts.

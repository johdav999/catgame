# Repaired Tripo Pip — 10 October 2026

Pip's supplied mesh now uses the exact Paws canonical rig, complete normalized skin weights, reduced textures and all nine named actions. Both FBX and GLB pass the pinned WebEngine importer and animation checks. Actual Chrome WebGPU playback was exercised through the generated Paws preview.

- [Repaired FBX](../../../../assets/Pip/pip-repaired.fbx): embedded 1024-square maps and normalized clip names.
- [Runtime GLB](../../../../assets/runtime/models/pip/pip.glb): the current Paws asset.
- [Editable packed master](pip-repaired.blend): all nine actions and packed maps.
- [Self-contained GLB export](exports/pip-repaired.glb).
- [Manifest](asset-manifest.json), [evaluation](EVALUATION.md), [provenance](PROVENANCE.md), [iterations](ITERATIONS.md).
- [Actual WebGPU preview](browser-evidence/desktop.png) and [browser test record](browser-evidence/pip-smoke.json).
- Original input remains at assets/Pip/pip.fbx. The prior runtime package is retained in previous-runtime/.

The final mesh has 17,854 triangles, two material slots, 29 canonical deform bones and a maximum of four normalized influences. Every imported vertex follows the skeleton root. No static mesh-node fallback weights remain. GLB and FBX have 31 and 32 hierarchy joints respectively because their import wrappers differ.

The original ginger markings and UVs are reused. Four 4096-square source maps were reduced to 1024-square maps. A small 64-square opaque eyelid map supports actual blink motion. Estimated decoded texture storage is about 12.02 MiB for GLB and 20.02 MiB for FBX including the importer's generated metallic-roughness map, excluding mip/GPU copies. No importer limits were increased.

The nine actions use the canonical durations at 30 fps, keep the root in place and preserve tested loop/transition endpoints. Contact, resting tail and eyelids were adapted to the supplied geometry. The resting pose is currently a compact crouch with tail laid aside; exact cushion curl, bowl lip alignment and physical mobile acceptance remain further asset/gameplay acceptance work.

## Reproduce

Run in Blender 3.5.1 (or a compatible version):

```powershell
& $blender --background --python-exit-code 1 --python artifacts/model-jobs/pip/2026-10-10-tripo-repair/inspect_source.py
& $blender --background --python-exit-code 1 --python artifacts/model-jobs/pip/2026-10-10-tripo-repair/repair_pip.py -- --revision 8
& $blender --background --python-exit-code 1 --python artifacts/model-jobs/pip/2026-10-10-tripo-repair/normalize_fbx.py -- 8
& $blender --background --python-exit-code 1 --python artifacts/model-jobs/pip/2026-10-10-tripo-repair/verify_export.py -- 8
```

The repair script reads the prior canonical master in ../2026-10-10/, not a machine-specific engine path. normalize_fbx.py rewrites only animation stack names using Blender's binary FBX parser/encoder. FBX_SCALE_ALL preserves metre-valued local bind translations rather than introducing a 100x armature wrapper. The repair calls normalize_glb.py to shift clip input times to zero without changing pose payloads.

engine_validation.cpp extends the repository Pip check to both formats. rig_validation.cpp checks anatomical bindings, the exact canonical hierarchy/local bind transforms and a 10 cm root translation over every vertex. The retained executables require the matching native compiler runtime on PATH.

Launch the generated site at /pip-preview.html. The final package is staged by the normal Paws asset staging path; only assets/runtime/ is deployed. Sources, revisions and original raw input remain outside the deployed tree.



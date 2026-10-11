# Pip eyelids — accepted revision 24

Replaces the raised orange caps with skinned original facial polygons. Coat UVs,
original eye appearance and open-pose normals remain continuous. Static eye
islands are head-weighted; upper/lower skin moves over them with a small closure
overlap. No morph support, custom shader, runtime mesh replacement or new texture
is required. The desktop WebGPU renderer plays the exported skeletal clips.

- Master: `exports/pip-eyelids.blend` (packed).
- Runtime: `exports/pip-merged.glb`, promoted to `assets/runtime/models/pip/pip.glb`.
- FBX: `exports/pip-eyelids.fbx`, promoted to `assets/Pip/pip-repaired.fbx`.
- Rig: 29 original bones unchanged plus `blink.lower.L/R` under `head`.
- Geometry: 20,298 triangles, one material, 31 deform bones, at most four
  normalized influences. Metres; Blender Z-up/-Y-forward, runtime Y-up/+Z-forward.
- Maps: original approved coat maps at 1024 square; default 128 MiB import budget.
- Timing: 100 ms close, 67 ms hold, 200 ms release at 2.4 s in the four-second idle.
  Settle gradually closes; rest holds closed; other actions remain open.

All eyelid scales are exactly one. The previous near-zero scales caused jagged
normal shading in WebGPU despite looking acceptable in Blender. Translation and
skin weights now control closure. Runtime tests enforce unit scales in every clip.

`merge_current.py` retains 729 non-facial channels byte-for-byte from the current
runtime (including reference walk and connected settle). `export_candidate.py`
also merges their accepted source actions into the editable master and FBX.
The walk merge script now preserves all `blink.*` channels on later reapplication.

Rebuild: `inspect_eyes.py`, `build_eyelids.py -- 24`, `export_candidate.py -- 24`
and `verify_exports.py` run in Blender 3.5; then `merge_current.py` takes current
runtime, candidate GLB and output GLB paths. Run `render_portraits.py` in Blender.
Validate before `promote.py`; promotion refuses a runtime changed since merging.
The retained script logs show the exact invocation/order used for this delivery.

See [evaluation](EVALUATION.md), [iterations](ITERATIONS.md),
[provenance](PROVENANCE.md) and [manifest](asset-manifest.json).
Only revision 24 is accepted. Earlier numbered masters, scripts and renders are
retained experiments, not alternative production assets.

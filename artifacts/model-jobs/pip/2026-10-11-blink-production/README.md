# Pip fitted blink repair

Accepted delivery: r15, 11 October 2026. Runtime revision `pip-fitted-blink-r15`.

The old implementation hid and expanded detached spherical caps over a painted eye. Its geometry could not produce a continuous facial blink. The first native-skin replacement also had irregular opening boundaries, different upper/lower closure depths and an area of skin that collapsed into the seam.

The final repair keeps the original facial skin and coat UVs. It smooths each opening in the eye plane, fits upper/lower edges to one shared closure surface, blends skin weights into the cheek and brow using distance along the mesh, and adds a welded thin inward-folding lid margin. Fixed eye islands extend under the opening. Four head-parented controls use translation and unit scale. No runtime timing workaround or overlay cap is involved.

## Deliverables

- `pip-blink.blend`: packed editable source containing all nine current actions.
- `pip-blink.glb`, `pip-blink.fbx`: validated matching exports.
- `portraits/`: matching dialogue, shop and ending renders.
- `asset-manifest.json`: runtime provenance, measurements and checksums.
- `browser/blink-three-views-4x-slow.webm`: actual WebGPU capture, slowed four times for inspection.
- `browser/view-*-*.png`: exported-asset front and side frames, including reopening.

The generated preview has Blink inspection controls: play, complete blink scrubber, front, both sides and puzzle distance. The game on port 8080 uses the same exported GLB.

## Reproduce

Run Blender 3.5 in background with `--python-exit-code 1 --python refine_native.py -- 15`, then `export.py -- 15`, `export_fbx.py` and `render_portraits.py`. Paths resolve relative to this job. `native-source.blend` and `native-fit.json` are immutable copies of the prior native-skin source, identified in the manifest. `validate_closure.py -- 15 --export` checks the clean GLB reimport. The retained `build_blink.py` and revisions r1-r11 are rejected experiments, not the delivery pipeline.

Run the repository native Pip, walk, settle and scene tests against the final GLB. `compare_animations.py` compares all non-facial channels against the runtime before promotion. `promote.py` refuses to replace a runtime that differs from `concurrent.glb`; restage and revalidate if the base changes. The previous runtime and FBX remain in `previous-runtime/`.

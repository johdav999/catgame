# Pip reference walk — 11 October 2026

`garden_walk` is a four-beat, contact-controlled 1.2-second walk. The runtime actor
travels 0.36 m per cycle (0.30 m/s). The skeletal root remains in-place.

- [Editable animation master](pip-walk-r1.blend)
- [Merged runtime GLB](exports/pip.glb)
- [Side and three-quarter animation](walk-review.mp4)
- [Moving WebGPU playback](browser-walk.mp4)
- [Actual garden playback](garden-walk.mp4)
- [Five-view export review](renders/review-sheet.jpg)
- [Five-view runtime review](browser-evidence/review-sheet.jpg)
- [Reference photograph](references/muybridge-716.jpg) and [editable pose specification](references/walk-spec.json)
- [Evaluation](EVALUATION.md), [iterations](ITERATIONS.md), [provenance](PROVENANCE.md)

The photograph supplies lift, passing and reach silhouettes from walking frames
1–6. The later gallop is excluded. Timing, the opposite half-cycle, body weight
transfer and retargeting to Pip are authored interpretations, not motion capture.
Contact order is LH, LF, RH, RF, a quarter-cycle apart; stance occupies two thirds
of each paw's cycle. Cubic swing targets pass through explicit lift, passing and
landing poses. Two-bone IK is baked at 30 fps. Sole corrections account for the
actual blended skin rather than only bone endpoints.

The r8 mesh, bind rig, textures and eight other actions are preserved in the merged
export. `scripts/merge_walk.py` replaces only walk channels and asserts that other
actions, geometry, materials and rig remain unchanged. Use it to reapply the walk
to a subsequently accepted eyelid/cushion delivery. The master is the r8-based walk
authoring source, not a consolidation of concurrent jobs. Historical FBX is
untouched; this delivery's runtime format is GLB.

## Reproduce

Run Blender 3.5.1 with `--background --python-exit-code 1 --python` and the retained
`scripts/build_walk.py`, followed by `-- --revision 1 --render`. `review_export.py`
clean-imports the merged GLB and renders every frame from five cameras.
`package_review.py` packages contact sheets and timestamp-preserving captures.

Build `build/web-walk-pinned-release` against `external/WebEngine`, serve its actual
`site` folder and open `pip-preview.html`. Walk inspection offers a cycle scrubber,
five views, in-place playback and a translating lane. The lane resets after five
cycles. Run repository `scripts/pip-walk-smoke.cjs` to capture all 36 samples from
all five views. `scripts/phase7-smoke.cjs` accepts `WALK_EVIDENCE` and
`PAWS_TEST_PORT` for garden capture and isolated ports.

Native `paws_pip_walk_tests` measures exported paw skin at 120 samples per cycle
across two cycles: fore/aft and lateral drift, floor contact and swing clearance.
`paws_phase7_scene` checks that actor speed matches the authored stride.

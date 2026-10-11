# Verification

- Final GLB: 20,762 triangles, 12,240 imported vertices, two materials, 31 deform bones / 33 engine joints, nine clips. FBX adds one importer wrapper joint.
- Original 29-bone hierarchy and bind transforms preserved. Every imported vertex follows a 10 cm root translation. See `rig-glb.txt` and `rig-fbx.txt`.
- Native GLB and FBX contracts pass: weights, root, loops, transitions, floor/height, static poses and lifecycle. Walk contacts, settle/rest continuity and portable scenes pass (`validation-native.txt`, `validation-fbx.txt`).
- All 729 non-facial channels match the previous runtime with zero component error over 121 samples per channel (`animation-preservation.json`).
- Clean GLB reimport: `clean-reimport.blend` and `clean-reimport.json`.
- Reimported geometry checked at 41 closure positions, two eyes and three optical directions per eye. Visibility decreases monotonically and reaches zero exposed fixed-eye faces at full closure in all six combinations. `r15-export-visibility.json` records the samples; `validate_closure.py` fails on incomplete closure.
- Source close-ups and actual Chrome WebGPU screenshots cover open, closing, closed and reopening from front and both sides. `browser/review.json` records zero browser errors. The WebM movie is the rendered browser canvas, with the exported clip sampled at quarter speed.
- Matching portraits were regenerated from the packed final model. Tests concern desktop browser rendering; physical mobile acceptance was not performed.

The final served-site smoke also passes all nine visible action controls, reduced motion, suspension, rapid switching, desktop/shadow-free rendering, responsive layouts, portrait loading and recovery from an injected HTTP 503. See `browser-smoke/pip-smoke.json`. `live-site.json` confirms the port-8080 asset checksum.

# Scoped evaluation

| Check | Evidence/result |
| --- | --- |
| Root cause isolation | Removing only r15's margin eliminates the circular outline; `no-margin-browser/` retains this comparison. |
| Open-eye outline | Front and both sides: no detached circle. Compared with the no-margin reference, excess dark pixels fall from 1034/954/982 to 35/14/8, a 96.6/98.5/99.2% reduction. This measures the margin contribution, not all natural eye shading. See `rim-comparison.json`. |
| Accepted blink preserved | All 837 exported channels, including blink, compared at 121 samples each: maximum component error zero. `animation-preservation.json`. |
| Clean closure | Clean reimport sampled at 41 closure positions for two eyes and three view directions each. Eye visibility decreases monotonically and reaches zero in all six closed views. `r21-export-visibility.json`. |
| Visual animation review | Actual Paws WebGPU: open, quarter, half, three-quarter, closed, reopening and open-again from front/left/right. `browser/`. |
| Editable/export parity | Packed master reopened and GLB reimported; nine source and nine export renders in `renders/`; matching opening/closure inspected. `clean-reimport.json`. |
| Rig and importer | Existing native GLB/FBX validators pass; canonical binds and root-follow preserved. `validation-native.txt`, `validation-fbx.txt`, `rig-glb.txt`, `rig-fbx.txt`. |
| Other actions | Existing Pip, walk-contact and settle/rest regression tests pass. `pip-tests.txt`, `walk-tests.txt`, `settle-tests.txt`. |
| Live site | Port 8080 served SHA-256 `da50380685ce63cd7ea42d7ed3f49580ed607e4fc3a7ce9093f377533be241e2`, matching the delivered GLB. Actual three-view video captured from this response; `browser/video.json`. |
| Browser regression | All nine action controls, animated playback, reduced motion, suspension, shadow-free/puzzle views, responsive layouts, rapid switching and HTTP-failure retry pass with no console errors. `browser-smoke/pip-smoke.json`. |
| User browser | Existing in-app tab reloaded and saved game continued; corrected asset loads in the garden. |

Original iris/sclera and coat texture details are intentionally retained. This
revision corrects the added circular margin, without redesigning the eyes or the
already accepted blink. It makes no new physical-mobile performance claim.

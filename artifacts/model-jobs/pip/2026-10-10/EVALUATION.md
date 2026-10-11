# Final evaluation — Pip revision 9

Evaluated 10 October 2026 against `design.md`, the local style reference, prompt 3,
source renders, clean-process GLB reimport and the actual pinned Paws renderer.

| Area | Evidence and result |
| --- | --- |
| Identity and silhouette | `renders/r9-three-quarter.png`, `r9-side.png`, `r9-rear.png`, `r9-face.png`: ginger tabby, cream muzzle/bib/toes, joined broad paws, short muzzle, substantial hooked tail. Coat is a continuous mesh, not disconnected primitive body parts. |
| Material | UV-aligned 1024-square base colour consumes the retained 1254-square ImageGen original. Cream and stripe masks remain coherent across the visible body. One matte nonmetal material, independent uniform roughness and neutral tangent normal. |
| Deformation | Source `pose-*.png`, four `reimport-*.png` views and nine browser static/playing captures inspected. Shoulder/tail spikes and head/ear weighting defects corrected; final rest is a tucked pose with tail curled around the haunch. Sculpted folds remain deliberately simple. |
| Contact | `exports/pose-bounds.json`: sampled minimum source height is approximately 0.003 m for lowering/rest clips. Native tests also skin actual imported vertices and reject ground penetration or excessive height. |
| Clip continuity | Root fixed at bind, all looping endpoints match, settle/rest and approach/drink/return/idle boundaries match. Checked per joint by `paws_pip_tests`. |
| Editable delivery | `pip-master.blend` reopens in a clean process; all consumed maps are packed. Source volumes, UV mesh, rig, actions, review cameras and retained scripts are present. |
| GLB | Clean import: 14,660 triangles, 10,458 vertices after UV splitting, one material, 29 deform bones, nine actions, four weights maximum. Engine includes two wrapper joints. GLB is 2,097,156 bytes. |
| Portraits | Three inspected transparent 512-square PNGs from the same final model. Dialogue/shop lower the tail behind the shoulders only for portrait framing. |
| Runtime | Chrome 154.0.8037.99 on Windows, pinned Release/Emscripten 6.0.11, actual `/pip-preview.html`. All asset HTTP requests return 200; no console errors on the normal run. Nine actions, rendered movement, reduced motion, rapid selection, suspension boundary, failed HTTP fetch and Retry pass. |
| Views | 1440x1000 desktop, 390x844 portrait and 844x390 short landscape. Close and ordinary puzzle distance captured with desktop shadows and shadow maps off. Shadow-free preview uses a quiet procedural contact cue. These are desktop viewports, not physical phones. |

Browser evidence: `build/evidence/pip/pip-smoke.json` and sibling screenshots.
Native evidence: `native-tests.log` (2/2 selected tests pass). Build logs:
`browser-build.log`, `browser-build-final.log`, `browser-build-preview-final.log`.
Source/import records: `r1.log` through `r9.log`, `reimport.log`, `portraits.log`.

The measured walking sample has one bone upload (3,968 bytes) per frame.
At that desktop viewport, the final sample reports 0.1 ms p95 application update,
0.9 ms p95 measured CPU work, 3.34 ms p95 GPU work and 20.2 ms p95 frame interval.
These counters describe this short isolated preview; CPU/GPU times must not be
added, and this is not a sustained physical-mobile or whole-garden performance
claim. Shadow passes double submitted triangle work for the cat and floor.

Build identity `paws-pip-r9-pinned-release-20261010`:

- Engine pin: `3fe7d1dc1dc610c703944803cbb717cd65024ec9` (working tree unchanged).
- GLB SHA-256: `55d0767f59a7464f6e4bebddc53cee02900691ddaf17f3b4a56d9802fd76e544`.
- Preview Wasm SHA-256: `f34cb0d9116a37a835e75e54a141b669ef19201d6cc99952b356449a42306089`.
- Preview JS SHA-256: `3c48d8f5b8791809e4cd041f315441636a6d767c5eb141d758ca70052c0f24bb`.
- Full source/runtime file hashes: `asset-manifest.json`.

Scoped limits: browser PBR appears flatter/more muted than Cycles review lighting.
The style is smooth sculpted coat masses, not the board's fine strand fur. Garden
movement, final cushion/bowl fit, production GUI portrait wiring and chirrup audio
belong to phases 5, 7 and 9. No physical-mobile claim or final furnished-scene
acceptance is made. There is no unresolved tool blocker for this asset phase.

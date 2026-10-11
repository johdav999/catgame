# Garden model and assembly contract

Prompt 4 delivers `garden/kit@1` revision 7: 21 GLBs, 45,588 triangles, 49,216
exported vertices and 7,750,336 bytes of GLBs. Full measurements, exact bounds,
hashes, footprints and named anchors are in
[`asset-manifest.json`](../assets/runtime/models/garden/asset-manifest.json).
Editable delivery: [garden model job](../artifacts/model-jobs/garden-kit/2026-10-10/README.md).

All game transforms use metres, Y up, +Z forward, +X right. Blender uses Z up and
-Y forward; the glTF exporter performs the single conversion. Most pivots are at
floor/contact centre; paving tops are y=0 and gate leaf origin is the hinge.
The 0.20 m edge piece extends 0.06 m below the ground plane intentionally.

| Interface | Contract |
| --- | --- |
| Routes | 1 m grid, open trough width 0.26 m, floor y=0.06 m. Straight ports (-0.5,0.06,0)/(0.5,0.06,0); curve ports (-0.5,0.06,0)/(0,0.06,-0.5). Rotate about asset origin in 90-degree steps. |
| Ball | Radius 0.105 m; floor pivot, centre y=0.105. Set its root at route-floor height. Same mesh in basket and switch. |
| Pots | Shared 0.65 × 0.65 slide footprint, round/fluted variants. Planting is decorative geometry within the prop; rules own occupied cells. |
| Basket | Three cut recesses, floor/root placements at y=0.059, x=-0.53/0/0.52 for ball/mouse/fish. Same toy meshes pass actual vertex/surface clearance tests. |
| Switch | Separate base/plate. Plate y=0.09 released, y=0.055 pressed. Ball root y=0.105 when pressed. |
| Gate | Frame and leaf separate. Leaf hinge at (-0.54,0.08,0) relative to frame; open +100 degrees around Y. More than a Pip-wide passage remains clear. |
| Cushion | Sage tufted shop cushion, 0.94 × 1.18 placement footprint; rest root y=0.16 for slight soft compression. Cream/coral rectangular blocking cushion is distinct. |
| Bowl | Sky ceramic raised bowl with real interior, separate opaque water at y=0.383. Drink root (0,0,-0.61327); rim y=0.408. Raised design fits the current canonical drinking pose without modifying Pip. |

`scripts/generate-garden-contract.py` compiles manifest IDs, bounds, review
transforms, anchors and gate angle to `src/garden_manifest.inc`. Run `--check`
after any asset update. The sidecar is **not** parsed by WebEngine; Paws explicitly
spawns the actors and projects their transforms. No reliance on Blender empties,
GLB node hierarchy, physics or clip completion for puzzle correctness.

`paws_garden` exposes immutable resource validation and reusable scene composition.
`garden_preview` loads all dependencies through the engine asset manager and only
publishes a complete scene. Retry replaces request interests. Application owns
clocks/publication; browser events stay in `garden-preview.js`. Scene-scoped
handles resolve moving parts. Borrowed renderer/controller references die first.
The preview's HTML controls are inspection tooling, not production game GUI.

```powershell
./scripts/build.ps1 -Pinned -Configuration Release -EmsdkPath <installed-emsdk> -BuildDirectory build/web-garden-pinned-release
./scripts/serve.ps1 -SiteDirectory build/web-garden-pinned-release/site -Port 19484
# http://127.0.0.1:19484/garden-preview.html
node scripts/garden-smoke.cjs
python scripts/generate-garden-contract.py --check
ctest --test-dir build/native-pip -R 'paws_garden_kit|paws_starter_scene' --output-on-failure
```

Configure native CMake normally first; `native-pip` is the reused local pinned
test tree. Put the native compiler runtime on PATH on Windows. Browser test
overrides: `PAWS_URL`, `BROWSER_EXECUTABLE`, `EVIDENCE_DIR`.
Final evidence is under `build/evidence/garden/`. Physical mobile is untested.
Phase 5 composes these assets with gameplay; phase 7 owns purchases and cat use.

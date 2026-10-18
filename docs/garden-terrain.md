# Garden terrain

With a terrain-capable WebEngine, `addGardenEnclosure` replaces its four exterior
lawn slabs with a real `TerrainAsset` and 64 `TerrainComponent` chunks. The
game-owned height policy and assembly are in `src/garden_terrain.cpp`; meshes,
normals, seam-preserving LODs, culling and layered rendering come from WebEngine.
There is no imported/generated 3D model or custom renderer.

The heightfield covers 768 by 768 metres at three-metre sample spacing, with
three engine LOD candidates. Its origin follows the enclosure/house footprint.
The garden, house, gate exit and planted apron remain at the previous lawn's
top elevation of -0.08 metres. A source-cell guard keeps interpolated triangles
flat throughout that footprint; a smooth 16-metre transition joins the hills.
Existing courtyard paving and prop/cat contact coordinates remain authored at
their existing elevations. Terrain is decorative, has no selection/collision
components, and carries no saved gameplay state.

The browser preloads the two original ImageGen albedos before scene publication.
Meadow grass repeats every three metres; limestone earth every four metres.
A generated linear RGBA weight map introduces restrained earth on steeper and
higher slopes. All chunks share the immutable source and material per enclosure.
Terrain receives shadows but does not cast them. The three chapter enclosures
retain their existing enable/disable ownership, so exactly one terrain is active.

Five loose groves add twenty shared apple/pear tree instances on the hills to
either side and behind the house. Each group contains four trees with varied
scale and yaw, separated by open meadow. Trunk bases sample the actual terrain
triangles with a four-centimetre root overlap. They follow the enclosure parent
through chapter and quest transitions and introduce no collision or selection.
These distant instances do not cast shadows. The six existing garden trees and
the pinned engine's flat-lawn composition retain their established placement.

Production garden and quest cameras use a 180-metre far plane in terrain builds.
The heightfield extends beyond their complete near/far frustum footprints at
desktop, portrait and short-landscape sizes, including orbit and pulled-back
chapter views. Sky remains independently owned by the existing daytime policy.

`PAWS_TERRAIN` is detected from the selected engine's `terrain.hpp`. The configured
adjacent development engine and the pinned `0426d93` revision both supply it.
Build the terrain version with the pinned engine selection described in README:

```powershell
./scripts/build.ps1 -Pinned -Configuration Release -EmsdkPath C:/Users/Johan/source/repos/WebEngine/.tools/emsdk
./scripts/serve.ps1 -SiteDirectory build/web-pinned-release/site -Port 8115
```

`tests/enclosure_tests.cpp` checks the interpolated flat apron and complete house
footprint for three garden sizes, hills, chunk sharing/LOD policy, lazy physics,
and all frustum corners at multiple aspect ratios, elevations and orbit headings.
`scripts/garden-terrain-smoke.cjs` exercises the generated site, verifies active
terrain/no slabs, full frustum coverage, exact HTTP texture hashes, mouse moves,
undo/restart/completion/return, resize and saved reload. Browser diagnostics are
available through `Module._paws_terrain_snapshot()` / `window.pawsTerrain`.

## Verification on 15 October 2026

The local Release browser build passed 12 checks on WebGPU and 12 on actual
WebGL 2 fallback (the latter disables WebGPU adapter acquisition in the test).
Both reported 64 active terrain chunks, zero lawn slabs, two matching texture
hashes, full frustum coverage, successful scene transitions/reload and no browser
errors. Screenshots/reports are in `build/evidence/garden-terrain/` and
`build/evidence/garden-terrain-webgl2/`. Desktop horizon and portrait screenshots
were visually reviewed. Fallback uses its existing simpler lighting policy.

Five native suites passed: `paws_starter_scene`, `paws_phase5_scene`,
`paws_phase6_scene`, `paws_garden_enclosure` and `paws_release1_scene`.
The pinned Release browser target also builds with the compatibility gate off.
The tested terrain Wasm SHA-256 is
`ed914d21d166dbafa66a1eee54cf9839866e61d94403dd98679a80b09cd88e98`;
its engine comes from the explicitly configured adjacent development checkout,
despite the existing build-info enginePin field recording the game's gitlink.
This is local desktop Chrome verification, not physical mobile acceptance.

For fallback verification, set `RENDER_BACKEND=webgl2` as well as `PAWS_URL` and
an independent `EVIDENCE_DIR` before running the smoke script.

## Hillside grove verification on 18 October 2026

The local Release browser build passed all twelve terrain smoke checks on both
WebGPU and WebGL2, including puzzle interaction, undo, restart, completion,
garden/quest transitions, resizing and saved reload, with no browser errors.
Reports and screenshots are under `build/evidence/hillside-trees-final-webgpu/`
and `build/evidence/hillside-trees-webgl2/`. The native `paws_garden_enclosure`
test passes and checks all twenty hillside trunk contacts at three garden sizes.

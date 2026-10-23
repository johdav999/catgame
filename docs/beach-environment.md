# Beach environment: first visual pass

## Ball-quest ground contact (2026-10-19)

Pip's beach ball and driftwood channels use the imported kit's floor-contact
pivots rather than the legacy greybox's 0.42/0.40 m elevations. The ball starts
on sand, rolls at the authored channel port height, and finishes at the cushion's
rest anchor. Scene projection and browser roll animation share `ballPosition`,
so slides, Undo, restart, reload and reduced motion preserve these contacts.
Covered channels remain hidden until their basket moves away.

Validation: native `paws_starter_scene` and `paws_release1_scene` passed.
The first-quest browser acceptance passed on WebGL2 and WebGPU with no browser
errors, including measured ball heights during rolling, Undo, board reload,
portrait interaction and completed revisit. Low-angle captures were inspected.
Reports and captures: `build/evidence/beach-ground-contact/{webgl2,webgpu}/`.
Browser checks used the existing isolated beach engine `077df1e`; the adjacent
development engine currently has an unrelated GUI API mismatch. The verified
Wasm SHA256 is `cd2538a59ba3ebf749b1af3bbe8f571c1e42495d33f4b835f9190e0eec880c77`,
also fetched from the running localhost:8080 preview.

## Softer beach shadows (2026-10-19)

Beach scenes use a 24 m wide directional shadow volume, centred one metre
seaward, with 36 m depth. Automatic and High quality use one 512-square shadow
map and nine-sample PCF. Relative to the former 16 m / 1024-square setup, the
filter covers a wider world-space footprint, softening contact edges while
covering the extended coastal scenery. Terrain receives shadows but does not
cast them. Water retains its existing no-cast setting.

Quality application tracks beach/garden transitions as well as the selected
preset, restoring the garden's 1024-square map on return. Mobile opts out.
The beach uses ambient fill `{.20,.22,.24}` and sun intensity 2 so grounding
shadows remain visible against pale sand.

### Correction: missing shadows in the in-app browser

The earlier `beach-soft-shadows` validation established a WebGPU depth pass,
but did not establish visible shadows in the user's browser. That browser's
profile reported no WebGPU adapter and selected WebGL2, which had no shadow-map
implementation. The isolated engine now supplies one directional WebGL2 map
with nine-tap PCF, shared caster LOD/alpha handling and context restoration.
This is an engine change in addition to the Paws quality policy. It retains
[the water correction](beach-water-depth-fix.md); the clean pinned submodule
has not been advanced, and the engine branch has not been published. The local
engine commit is `077df1e` (on `codex/water-depth-transition`).

`node scripts/beach-shadows-smoke.cjs` passed on WebGL2 and WebGPU. On/off
comparisons darkened 21,369 and 12,842 pixels respectively in the central
receiver/caster region, with visible filtered transitions. Both passed 4x
MSAA, resize and beach-to-garden resolution restoration; WebGL2 also passed
context loss/restoration. Native `paws_release1_scene` passed in 27.41 s.
Evidence: `build/evidence/beach-shadows/webgl2/` and `webgpu-retry/`.
The final committed build also passed WebGL2 checks with no errors: 21,291
darker pixels, 21,476 intermediate pixels; see `final-webgl2/`. Its Wasm SHA256
is `70b4296b432e66563e8853d0ce7edb4ebec5717a6aa7a739aedef41298b8218b`.
The existing in-app tab was reloaded and its saved Pepper quest resumed;
visible soft shadows were inspected under Pepper, pots, rocks and the pier.
A model-load retry was needed during concurrent test loading; retry succeeded.

The initial concurrent WebGPU attempt hit `ERR_NO_BUFFER_SPACE` while loading;
the isolated retry passed without browser errors. These are desktop browser
checks, not physical-device or sustained-performance acceptance.

The beach adventure uses terrain and water from pinned WebEngine
`0426d9319de715584a2e34ff9862cd6ba873ecd0`, following
[the reference image](design/beach-adventure-reference-20261018.png).
Existing puzzle props and story landmarks remain placeholders where applicable.

## Ground and ownership

`src/beach_environment.cpp` assembles decorative terrain and water for all six
quests. Paws owns coastal heights, palettes, footprints and settings; WebEngine
owns meshes/LODs, layered materials, water shading and depth captures. No engine
changes or new imported 3D models are required. The scene owns the environment.

The heightfield covers 256 by 256 metres with 257 by 257 float samples, 64 chunks
and engine-generated LODs. Each quest's dimensions define a flat apron at Y=-0.02,
including a source-sample guard and extra room for requesting cats. Puzzle
transforms, targets and scripted contacts retain their existing elevations.
Dunes rise outside the protected rectangle; the seabed slopes down to about five
metres below it. Terrain creates neither physics nor selection components and
introduces no independent save state.

The later coastal kit clears a small level sand bed beneath the boardwalk, with
full source-cell padding and a smooth dune transition. Native sampling verifies
its rotated deck footprint in every layout, alongside the original flat-puzzle
checks. This perimeter contact adjustment adds no terrain physics owner.

Three layers blend warm dry sand, darker wet sand and sage dune cover. An original
deterministic seamless texture supplies restrained mottling; it is generated in
memory and shared, with no external texture/license dependency. This pass does
not add vegetation, rocks, boardwalks or replacement baskets.

## Water and framing

The subdivided local-XZ sea mesh sits at Y=-0.18 and uses `MaterialShader::Water`.
Low Gerstner waves, ripple normals, authored reflection colours, refraction,
absorption and depth foam show the submerged terrain transition. This is not a
breaking-wave or tidal simulation. The bridge runnel also uses engine water,
with minimal displacement and no screen-space effects. Planar reflections and
underwater captures are off. Water has no collision, buoyancy or puzzle authority.

The existing host supplies simulation seconds once per frame; pause/suspension
freeze water and reduced motion uses zero. No new clock/update owner is added.
Garden bowls and gutters retain their existing material settings.

General beach puzzles use the elevated 0.88-radian view, projected ground-depth
fit and the actual free HUD rectangle. Wide screens look slightly toward the
coast; portrait and short-wide screens keep the board-centred fit. Bridge and
shell-memory scenes retain their specialised framing, and the ending retains
its four-cat bounds checks. Beach far clipping is 80 metres; garden views keep
their previous 180-metre terrain range.

## Verification workflow

Build with `scripts/build.ps1 -Pinned -Configuration Release` and the installed
Emscripten SDK. Serve the exact `build/web-pinned-release/site` directory.
Localhost enables preview by default; `?devBeach=1` explicitly enables it and
`?devBeach=0` disables it. Enter through Adventures.

Native `paws_beach_environment` checks interpolated flatness for all board sizes,
dunes, seabed, shared LODs, full frustum coverage, water winding/subdivision and
material policy. `paws_starter_scene` and `paws_release1_scene` cover scene
lifecycle and existing content integration.

`scripts/beach-adventure-smoke.cjs` verifies actual renderer selection, terrain,
visible animated water pixels, water clock/pause, desktop/portrait/short-wide
views, cancellation, drag, Undo, reload, garden return, access restoration and
completion. `scripts/beach-story-smoke.cjs` checks all six requests and the saved
four-cat ending using real scene input. Set `QUEST_BACKEND=webgpu` or `webgl2`,
`PAWS_URL` and a separate `EVIDENCE_DIR` for each run.

## Acceptance on 18 October 2026

The pinned Release browser target built successfully. Native `paws_starter_scene`,
`paws_beach_environment` and `paws_release1_scene` passed. Flatness was additionally
checked against the exact six beach definitions, including the shell-chime board.
Native logs: `build/beach-native-tests.log` and
`build/beach-native-environment-final.log`.

First-quest acceptance passed on actual WebGPU and WebGL2 with no browser errors.
Both reported 64 terrain chunks, complete frustum coverage and a subdivided sea
with refraction and foam. The pixel check found 73,537 visible sea pixels and
6,108 animated pixels on WebGPU, and 53,703 / 501 respectively on WebGL2. These
are per-run visual checks, not performance comparisons. Pause froze water time.
The test also passed cancelled drag, Undo, exact saved-board reload, garden travel,
development-access restoration, portrait interaction and completed revisit.

Reports/screenshots: `build/evidence/beach-pass1/final-webgpu/` and
`build/evidence/beach-pass1/final-webgl2/`. Reviewed desktop, portrait and short
landscape captures. WebGL2 retains basic lighting and no shadow maps.

Both tests fetched the same Wasm bytes as the built file:
`fb220491f256fdb355b72ede7175c9b5455dda8fd52f8c1eab6b1f44162a5b8d`.

The complete six-quest story also passed on both renderers against that same
build, with zero browser errors. Each quest passed real scene gestures, Undo,
restart, saved-board reload and garden travel. The four-cat ending stayed inside
the portrait board bounds, persisted and replayed without granting base rewards.
Reports and per-quest desktop/portrait/short-landscape screenshots are in
`build/evidence/beach-pass1/story-webgpu/` and `story-webgl2/`.

Desktop Chromium acceptance does not establish physical mobile compatibility or
sustained performance. The reference's production props, vegetation, rocks and
boardwalk remain for a later asset pass.

The coastal material/model pass is tracked in
[beach-coastal-assets.md](beach-coastal-assets.md). The user supplied all four
manual Tripo exports on 2026-10-19; reviewed baskets, rocks, grass and boardwalk
now accompany the sand material. Editable packed sources and review evidence are
retained under `artifacts/model-jobs/beach-coast-20261018/review/`.

# Build and validation

Run from the selected repository root. Follow current README, `.emscripten-version`, CMake options and scripts if they differ from these landmarks.

## Windows browser build

```powershell
# Setup only when the local SDK is absent or mismatched.
./scripts/setup.ps1
./scripts/build.ps1 -Configuration Release
./scripts/serve.ps1 -Port 8080
```

Use `.tools/emsdk`; the inspected pin is Emscripten 6.0.11. Its wrappers are `upstream/emscripten/emcc.exe` and `emcmake.exe`. The browser target links `--use-port=emdawnwebgpu` and GLES3/WebGL 2 with `-sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2`. Verify the current pin instead of silently switching to a global SDK/version. Setup and first configuration may download dependencies. Linux/macOS setup and build commands remain in README.

Builds publish into `dist/`. Separate build directories can still publish the same `dist/`; coordinate shared-checkout browser builds. For Ninja `failed recompaction: Permission denied`, check lock ownership and prefer a fresh build tree without cleaning unrelated work:

```powershell
& ./.tools/emsdk/upstream/emscripten/emcmake.exe cmake -S . -B build/web-game-release -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build/web-game-release
```

Check listeners before serving; reuse the correct server or choose a free port. Keep servers managed and stop only task-owned processes. `serve.ps1` serves the repository `dist/`; for an isolated staged site, use `python scripts/serve.py --port 8081 --directory C:/absolute/path/to/site`. Verify that site contains the intended `index.html`, JS and Wasm, avoiding stale outputs/404s. Localhost works for development; deployment uses HTTPS. Publish the whole site with `application/wasm`, including worker wrapper/module/Wasm when used, and inspect hosting/CSP/CORS requirements. The current isolated-worker pipeline needs no cross-origin isolation.

Browser automatic mode supports WebGPU and WebGL 2. Use the playground's `?renderer=webgpu` and `?renderer=webgl2` to exercise each backend; these query options are host policy, while reusable initialization uses `RenderBackendPreference`. Test reduced fallback capabilities, unavailable-backend failure and context/device recovery when affected. Do not treat fallback as a guarantee of browser/device acceptance.

## Portable native checks

```powershell
cmake -S . -B build/native -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build/native
ctest --test-dir build/native --output-on-failure
```

Reuse a valid configured build; use a separate directory for generator/configuration conflicts. For narrow changes, select meaningful tests with `ctest -R` after checking registered names. Add regression coverage when it checks behaviour/invariants, not implementation wording. This native configuration tests portable core/GUI contracts, not browser rendering or a native game backend.

Inspect `ctest --test-dir build/native -N`/current CMake before selecting names. Relevant registered suites include `mobile_assets`, `render_quality`, `assets`, `application`, `selection`, `manipulation`, `physics_joints`, `movement`, `animation_control`, `root_motion`, `prefab`, `persistence` and `camera_controller`. For missing compiler runtime DLLs (for example exit `0xc0000139`), identify the configured compiler and put its matching runtime `bin` directory on PATH rather than mixing toolchains.

## Browser and gameplay evidence

Browser scripts use Node, Playwright/pngjs and a supported Chrome/Edge executable. Discover installed dependencies rather than assuming example paths exist. Follow each script's setup and test-fixture requirements:

```powershell
$env:NODE_PATH = 'path/to/test/node_modules'
$env:BROWSER_EXECUTABLE = 'C:/Program Files/Google/Chrome/Application/chrome.exe'
$env:DEMO_URL = 'http://localhost:8080'
node scripts/browser-smoke.cjs
```

Choose relevant scripts: `application-smoke.cjs`, `input-smoke.cjs`, `ui-input-smoke.cjs`, `gui-smoke.cjs`, `physics-smoke.cjs`, `spatial-query-smoke.cjs`, `physics-joints-smoke.cjs`, `skeletal-animation-smoke.cjs`, `assets-smoke.cjs`, `audio-system-smoke.cjs`, `persistence-smoke.cjs`, `render-quality-smoke.cjs`. Verify current files; some build explicit Wasm targets or host their own runners.

For the newer pipelines use `render-fallback-smoke.cjs` (backend acquisition/capabilities/recovery), `mobile-assets-render-smoke.cjs` (upload admission/progress on both backends), `texture-tool-smoke.cjs` (native mip/package tool) and worker coverage in `assets-smoke.cjs`. Set `ASSET_TEST_SITE` to the chosen generated `asset-tests` directory when that runner requires it. Inspect explicit CMake targets/options for portable Wasm checks, including `asset_browser_tests`, `spatial_query_tests`, `physics_joint_wasm_tests`, `animation_control_wasm_tests` and optional persistence/movement runners.

Sky changes use native `sky_tests`/CTest `sky`, `scene_archive` and `persistence`, plus `scripts/sky-smoke.cjs` and `scripts/sky-weather-smoke.cjs` against the selected browser build. The first covers original gradient/panorama controls; weather acceptance covers actual indexed sphere geometry, per-view camera centers, near/far and orthographic coverage, atmosphere noon/sunset/night, clouds/wind, scene lighting, pause and recovery on WebGPU and WebGL 2. Native coverage includes inward winding, stale/foreign sun rejection, clock wrapping, atomic state and format-6/legacy sky archives. Inspect current `docs/sky-validation.md`; older evidence is not a new run.

Water changes use native CTest `water`, `material`, `scene_archive`, `persistence`, `sky`, `render_frame` and `render_quality`, plus `scripts/water-smoke.cjs`, `scripts/water-depth-smoke.cjs`, `scripts/water-planar-smoke.cjs` `scripts/water-underwater-smoke.cjs` `scripts/water-caustics-smoke.cjs` and `scripts/water-interactions-smoke.cjs` on the pinned Release browser build. The script verifies both forced backends with actual pixels for independent mesh waves/ripple normals, reflection controls/IBL fallback, pause/resume, invalid clock/presets, alpha/mirrored state, MSAA, perspective/orthographic ordered views, resize, paused recovery, scene archive save/load/return and demo return/restart/orbit. Water-depth acceptance verifies actual refraction/distortion/absorption, isolated shoreline foam, strong-distortion foreground rejection, 1x/4x opaque color/depth capture, perspective/orthographic cameras, resize/recovery and optional target/pass costs. Planar acceptance checks actual nearby-object reflection pixels, isolated red-object reflection versus below-plane clipping, single/4x main MSAA, ordered perspective/oblique orthographic views, resize, paused recovery, scene archive restoration and optional pass/color/depth storage. Evidence goes to `build/evidence/water`, `build/evidence/water-depth` and `build/evidence/water-planar` by default. A direct top-down orthographic view hides object reflections behind their direct images; use the oblique inset for a meaningful pixel assertion. Native water coverage includes actual format-7 material compatibility with new effects disabled; current archives use format 16 (v9 planar, v10 terrain, v11 underwater, v12 material layers/detail, v13 caustics, v14 lazy terrain, v15 terrain patches, v16 scene fog). Actual v8 fixtures must retain refraction/foam while defaulting planar strength off. When constructing older fixtures from v10, remove its empty terrain-library list as well as newer feature fields before repairing header length/CRC. Underwater acceptance checks depth-varying fog, stronger red than blue absorption on near/far white markers, distance caps, surfacing/footprint exits, perspective/orthographic and ordered above/below views, untouched Overlay pixels, 1x/4x, resize, paused recovery, actual archive restoration, pure-underwater optional target/pass costs and zero unchanged WebGPU underwater view-uniform writes while paused. Evidence defaults to `build/evidence/water-underwater`. Use an isolated native build directory while concurrent work is changing shared configurations; inspect the existing CMake cache for the pinned local Jolt source path rather than assuming `_deps/jolt-src` contains it. The current water legacy fixtures remove later material-layer and empty terrain-list bytes before repairing older header length/CRC. Run affected material, compositing and fallback regressions as appropriate. Stage matching worker artifacts after archive format changes and record build/Wasm identity. Desktop Chrome does not establish physical mobile acceptance.

MSAA changes use native CTest `render_quality` and `scripts/msaa-smoke.cjs`: real 1x/4x diagonal coverage, setting rejection/toggling, profile reset, atomic framebuffer rejection, format fallback, ordered/empty views, GUI, skeletal/morph deformation, resize, memory and paused GPU/context recovery. Run existing `compositing-smoke.cjs`, `sky-smoke.cjs` and `shadow-smoke.cjs` with `MSAA_SAMPLES=4`. Build with the pinned local SDK and stage the generated site, web host, fixture models and Noto fonts together. Record the Wasm hash and desktop scope; attachment storage does not establish FPS or physical mobile acceptance.

Terrain changes use native CTest `terrain` plus affected material, mesh-detail, physics, scene-archive/persistence and renderer suites. Run `scripts/terrain-smoke.cjs`, `scripts/terrain-detail-smoke.cjs` and `scripts/terrain-edit-smoke.cjs` on the exact pinned Release build. Detail smoke forces both backends and verifies layer/weight pixels, camera/shadow submission independence, full/coarse/mixed coverage, source physics invariance, projection/resize, saves and paused recovery. Edit smoke adds real pointer/inspector brush input, release/cancel ownership, partial rebuild counts, paint geometry invariance, undo/redo pixels, edited LOD seams, one-at-a-time admission, bounded resident/CPU/GPU cache payloads, eviction, absent-chunk edits, resident/source query distinction, streamed archives, invalid preservation, ball contact after sculpt and recovery. Native coverage compares partial rebuilds with full regenerated normals/LODs, proves unchanged sharing/resource expiry/stale-handle rejection, validates source-only allocation budgets and retains the real v10 fixture. Current archives use format 16; check native/browser archive interchange. Record selected Wasm hash and evidence under `build/evidence/terrain-edit/` (plus base/detail regression directories). Keep browser builds isolated, e.g. `build/web-terrain-page-edit-release`; stage its host, worker, assets and fonts together. Canvas focus outlines can change screenshot edge pixels: blur before archive pixel comparisons rather than weakening terrain checks. GL shadows remain unavailable. Desktop acceptance/submission/cache results do not establish mobile acceptance, sustained performance or FPS gains.

Paging adds CTest `terrain_paging` and `scripts/terrain-paging-smoke.cjs` on both forced backends. Native tests compare paged base/LOD geometry with full-source chunks, validate codec/halo/transport, source-only and patch snapshots/collision, background completion, idle guards, explicit retries, stale revisions, eviction/resource expiry and late reports. Browser acceptance must use actual page HTTP and isolated worker generation: delayed requests leave main frames advancing; pending/source/resident/admission bounds hold; mixed LOD coverage and page weight pixels match; cancelled/reset jobs cannot repopulate scenes; HTTP/checksum failures stop until retry; resident snapshots load without the provider and recover after GPU/context loss. Preserve old terrain archive fixtures and check native/browser format-15 interchange. Stage terrain-worker.js and terrain-storage.js plus the matching asset_decode.js/.wasm, host, page files and fonts; `tools/terrain_demo_pages.py --output <directory>` authors the static fixtures and the demo build invokes it. Record both Wasm hashes and results in build/evidence/terrain-paging; see docs/terrain-paging-validation.md. Browser budgets exclude worker heaps, collider/driver overhead and external references; Jolt cooking and transport validation/copy remain owner work. No physical-mobile, hard frame-time or sustained-performance claim follows.

For CatGame terrain integration, resolve the selected engine first and verify the terrain, layered-material, LOD and editing/streaming APIs there. Run the engine terrain suites from that WebEngine checkout using its pinned SDK. Run Paws build/game acceptance from CatGame with its own scripts and generated site; verify the intended `paws.js` and `paws.wasm` are actually HTTP-served. Record engine pin/build identity and preserve the pin/local build separation. Engine playground evidence does not establish CatGame integration. This documentation-only refresh uses skill/content/link checks, not a new runtime acceptance run.

Paged editing/writeback adds CTest terrain_page_edit and scripts/terrain-page-edit-smoke.cjs to the prior four terrain browser suites, forcing both backends on one exact Release build (e.g. build/web-terrain-page-edit-release). Native tests cover cross-corner/halo atomicity, missing-source acquisition/cancellation, frozen smoothing, paint geometry/collider identity, unchanged resource sharing, handle preservation, invalid edit rollback, dirty pins and source bounds, old acknowledgements versus newer edits, write failure/retry, generation conflicts/atomic rollback and explicit discard. Browser checks use real IndexedDB and workers: delayed required pages cannot partially sculpt; four shared corner halos persist together; paint changes pixels without geometry rebuild; real clicks/drags/cancel own input; failed writes retain dirty ground and stop retry loops; generation compare-and-replace rejects a competing persisted snapshot; discard reloads it; saved overlays survive owner eviction and full page reload. Include sculpt/paint collision agreement, mixed LOD coverage, real save/download/open, malformed preservation and paused device/context recovery. Quota acceptance injects a QuotaExceededError at the adapter boundary, not actual disk exhaustion. Stage terrain-storage.js on host and worker along with matching worker/Wasm, host, pages and fonts. Record build/worker hashes and evidence under build/evidence/terrain-page-edit and see docs/terrain-page-edit-validation.md. Scene archives use format 16 and do not persist writeback controller state. Physical mobile acceptance and sustained performance remain separate.

Rendering-specific changes can use `compositing-smoke.cjs`, `shadow-smoke.cjs`, `material-smoke.cjs`, `material-graph-smoke.cjs`, `glb-import-smoke.cjs`, `morph-smoke.cjs`, `fx-smoke.cjs`, `instancing-smoke.cjs`, `frustum-culling-smoke.cjs`, `mesh-optimization-smoke.cjs`, `mesh-detail-smoke.cjs`, `mesh-detail-quality-smoke.cjs`, `cache-upload-smoke.cjs`, `frame-reuse-smoke.cjs` or `performance-smoke.cjs`. Check each suite's expected backend/fixture and env vars; WebGPU-specific counts and pixel expectations do not establish fallback parity.

Play the game itself: start/load, perform its representative mechanic, observe feedback/state changes, complete/fail and restart/transition. Exercise affected input ownership, resize and page suspension; verify save/reload and callback reconstruction when delivered. Playground smoke checks alone cannot prove a new game's behaviour. Capture console/GPU errors and useful canvas screenshots/results under `build/evidence/` or supported script destinations.

For performance claims compare matching Release builds, browser/GPU, viewport, scene/camera and quality. Distinguish cold loads, warm CPU work, GPU timing and frame intervals; note unavailable counters and scoped memory estimates. Desktop browser evidence/mobile emulation cannot establish physical Android/iOS controls, audible output, browser compatibility or sustained mobile performance. Report unavailable required environments as remaining limits.


For WebGPU material specialization, use `scripts/material-smoke.cjs`, applicable
terrain/layered-material tests, `scripts/water-smoke.cjs` or the depth/reflection
water suites, and shadow/recovery coverage. Inspect actual game pixels as well
as pipeline validation; fewer triangles alone do not prove an FPS gain. Ensure
other GPU-heavy applications are closed and record the loaded Wasm/build for
every counterbalanced performance run.


Third-increment editor changes also build/run `editor_structure_tests` / CTest
`editor_structure`. It covers component lifecycle, batch removal/reparent/duplicate
history, sibling order, cycle/shear/topology rejection, group transforms and scale,
conflicting external writers, independent physics/animation, suspension and exact
source bytes after Stop. `scripts/editor-structure-smoke.cjs` verifies real modifier/
range selection, hierarchy drag/drop and parent chooser, component add/remove,
axis/uniform scale gestures, grouped visual undo, physics rejection, isolated Play
physics, repeated Stop, copied-camera navigation, unchanged source bytes/pixels and
narrow layout on both backends. Set `DEVICE_SCALE_FACTOR=2` for high-DPI; default
evidence is `build/evidence/editor-structure/`. Hide the SVG overlay only in pixel
capture comparisons, so switching tool affordances cannot masquerade as a renderer
state difference. Run earlier editor/content/archive regressions against the same
HTTP-identified host/Wasm/worker. Record new evidence rather than citing older runs.

Caustics acceptance uses `scripts/water-caustics-smoke.cjs`, evidence default `build/evidence/water-caustics`: actual receiver pixels versus off, clock animation and zero speed, pause/resume, depth fade, dry/ceiling exclusion, perspective/orthographic and ordered views, 1x/4x, resize, paused recovery, caustics-only selection and archive loading. Enabled versus disabled caustics with fog retained must add no passes or textures; paused WebGPU must not rewrite unchanged uniforms. Native water coverage includes invalid-field rejection, shared/override round trips and actual v12 layouts loading with caustics off. Preserve v13 water fields when updating later archive formats; keep host and worker builds matched.

Interaction acceptance uses `scripts/water-interactions-smoke.cjs`, evidence `build/evidence/water-interactions`. Test isolated rings against an off control at the same clock, outward pixel-footprint growth, wake pixels behind the heading, future/expired/masked/other-elevation rejection, pause/resume, perspective/orthographic ordered views, 1x/4x, resize, identical paused recovery, scene-replacement clearing and actual moving-source demo controls. Native water tests cover defaults/limits, bounded count, complete invalid-snapshot rollback, age boundaries, mask staging and normalized wake headings. Optional interactions add no passes/textures; paused WebGPU staging must remain cached. Existing feature smoke scripts disable transient sources and repeat that state after demo restart before comparing old baselines. Check affected material/compositing/shadow/fallback paths after changing the private frame payload. No archive version bump is needed for these transient renderer inputs. Both installed and CatGame-local engine skills must be merged and verified, preserving the game's dependency/pin guidance; engine browser evidence alone does not establish CatGame integration.


Editor workspace: run `scripts/editor-layout-smoke.cjs` on both renderers at DPR1
and DPR2, followed by the three existing editor regression suites. See
[workspace guidance](editor-workspace.md) for staging, controls and limits.

## Height fog validation

Build/run native CTest `fog`, `sky`, `water`, `scene_archive`, `persistence`, `render_frame`, `render_view`, `fx`. `fog` covers analytic versus numerical rays, height/distance/start, extremes, rejection rollback, per-view payloads, archives and clone. Build with local pinned Emscripten 6.0.11, e.g. isolated `build/web-fog-release`; stage matching web host, main/worker Wasm, fixtures and fonts. Run `scripts/fog-smoke.cjs` with NODE_PATH, BROWSER_EXECUTABLE and DEMO_URL. It forces both backends and checks material/particle/alpha pixels, excluded layers, sky, camera opt-out, 1x/4x, perspective/orthographic, resize, paused recovery, archives, controls and water/underwater composition. Evidence: `build/evidence/fog`; consult `docs/fog-validation.md` for actually executed regressions. Update current archive expectations to v16 without relabeling historical fixtures. Desktop Chrome is not physical-device or sustained-performance evidence.

Editor camera changes: build native `input_tests`, `camera_tests` and
`camera_controller_tests`, run CTest `input|camera|camera_controller`, then
`scripts/editor-camera-smoke.cjs` on WebGPU/WebGL2 at DPR1 and DPR2. Keep the four
existing editor suites as regressions; use isolated contexts to preserve unsaved
work in the user's open editor. See [camera controls](editor-workspace.md).

Terrain editor: build/run CTest `editor_terrain|editor|editor_content|editor_structure|terrain`.
Run `scripts/editor-terrain-smoke.cjs` with DEMO_URL, BROWSER_EXECUTABLE,
EVIDENCE_DIR and optionally DEVICE_SCALE_FACTOR=2. Verify source/visual/collision,
one history command per gesture, cancellation, imported heightmaps, real scene
downloads/reopen, Play gating, floating Details and compact layout. Use actual
download paths for large scene files instead of numeric-array transport through
Playwright. Keep editor-camera/layout/editor/content/structure regressions and
terrain-edit-smoke. See [integration](editor-workspace.md).

Terrain layer textures: run native editor_terrain, terrain and existing editor
tests, then `scripts/editor-terrain-textures-smoke.cjs` on WebGPU/WebGL2 at DPR1
and DPR2. Include terrain/content/structure browser regressions. Verify actual
imported texture pixels, assignment/clear/no-op history, later painting, source
sharing isolation, weights/geometry/collision retention, Play and downloaded scene
roundtrip. Remove focus outlines only in screenshot comparison styles. Read
[layer texture guidance](editor-terrain-textures.md).


FX milestone validation: see [FX integration](fx.md). Build/run native `fx`,
`scene_archive`, `persistence`, `water`, `terrain`; run `scripts/fx-smoke.cjs`,
`scripts/fx-milestone-smoke.cjs` and `scripts/water-depth-smoke.cjs` on the exact
pinned-SDK Release site. The milestone suite covers both renderers, depth fades,
stretch, 1x/4x, ordered masks/views, resize/recovery, v17 save/load and fountain UI.


FX authoring: build/run native `editor_fx`, `editor`, `editor_content`,
`editor_structure`, `editor_terrain`, `fx`, `scene_archive`, `persistence`. Run
`scripts/editor-fx-smoke.cjs` on both backends at DPR1/DPR2, plus existing editor
structure and FX milestone regressions. See [FX authoring](fx-authoring.md).


## Game AI

Build `ai_tests` and run CTest `ai`, plus `application`, `movement` and `physics`
regressions. Run `scripts/ai-smoke.cjs` on the selected pinned-SDK browser site
with `DEMO_URL`, `NODE_PATH`, `BROWSER_EXECUTABLE`; it forces WebGPU and WebGL 2.
Verify live patrol pixels and actual controls; insert a blocker into a planned
route, move it onto the detour, and prove original-goal arrival. Assert per-step
query limits, bounded retry counts, unreachable-stop behavior, automatic recovery
when an obstruction clears, explicit retained-goal retry and cancellation during
Waiting. Also verify simulation pause, occluded memory, removed targets, restart,
resize, saves after return, re-entry, demo switching and reload. Use
`EVIDENCE_DIR=build/evidence/ai-recovery` to keep new evidence separate; see
`docs/ai-recovery-validation.md`. Do not claim walkability, crowds, global
scheduling or physical-mobile acceptance from these checks.
The `aiTest=1` fixture permits bounded fixed stepping only while paused. Active
custom demo actors explicitly reject saves; return destroys them and restores
ordinary saves. Do not claim built-in AI persistence or physical-mobile acceptance.


When staging a separate browser site, copy the selected generated site first,
then current `web/` host files last, matching the `web_assets` publication order.
Reused build directories can retain old HTML/JS adapters. Keep both main and
asset-worker JS/Wasm pairs matched, with fonts and fixture assets.

## Static navmesh validation

Build `navmesh_tests` and run CTest `navmesh`, `ai`, `application`, `physics`,
and `movement`. With repository-local Emscripten 6.0.11, build `web_assets` and
serve `dist`; run `scripts/navmesh-smoke.cjs` on actual WebGPU and WebGL2, then
`ai-smoke.cjs` for regression. Verify real arbitrary clicks, full route/walkable
overlay pixels, motor arrival around static collision, drag ownership, pause,
invalid/off-mesh goals, profile rejection, bake limits, resize and return/re-entry.
See selected engine `docs/navmesh-validation.md` for exact evidence. These checks
do not establish dynamic carving, general stair locomotion or physical-mobile
acceptance. Deploy matching main/worker JS/Wasm and RecastNavigation-LICENSE.txt.


### Terrain surface rendering

Build with repository-local Emscripten 6.0.11 and stage matching webengine and
asset_decode JS/Wasm, web shell and terrain-worker.js. Run CTest terrain,
terrain_paging, terrain_page_edit, material, scene_archive and editor_terrain.
Run scripts/terrain-surface-smoke.cjs on forced WebGPU/WebGL2 for separate normal,
height and projection pixel comparisons, LOD coverage, collision/query invariance,
archive v23 reload, malformed-file preservation, resize and recovery. Retain the
five previous terrain suites and material/water regressions as applicable. Read
selected engine docs/terrain-surface-validation.md for current evidence. Desktop
browser acceptance does not establish physical-mobile or sustained frame-time limits.


### Development server port selection (2026-10-18)

From the selected engine checkout, `scripts/serve.ps1 [-Port 8080]` tries up to
100 consecutive ports, capped at 65535. Busy or access-denied/reserved ports are
skipped by the Python server's actual bind, retaining the selected loopback socket;
Windows uses exclusive binding. Open the URL printed in the terminal rather than
assuming 8080. Existing servers are not stopped. `-PortAttempts 1` requires the exact
port and fails clearly if unavailable. Direct `python scripts/serve.py` defaults to
one attempt; opt in with `--port-attempts 100`. Invalid/missing build inputs and
unrelated bind errors still fail. Core/native or renderer rebuilds are unnecessary
for this launcher-only change. Verify by holding consecutive ports, starting the
PowerShell command, checking its printed fallback URL and making HTTP requests;
also check fixed-port failure, the 65535 boundary and unchanged Wasm MIME/no-store
headers. Game consumers keep their own server/build commands.


## Wheeled vehicle foundation

Build native vehicle_tests and run CTest vehicle plus affected physics/joints, editor,
scene_archive/persistence and prefab regressions. Build vehicle_wasm_tests with the
repository-local SDK and run its .js in Node. Build/publish web_assets, serve the
exact dist, set DEMO_URL/NODE_PATH/BROWSER_EXECUTABLE, and run
`node scripts/vehicle-smoke.cjs`; repeat with DPR=2 and separate EVIDENCE_DIR.
The suite forces WebGPU and WebGL2, checks actual backend, scripted driving, views/
masks/MSAA independence, history/Play/source preservation, file/IndexedDB restores,
blur/suspension and resize/recovery. Follow [vehicle integration](vehicles.md) for
contracts/defaults/limits. Physical device/controller/performance acceptance is
separate. Archive writer is v26; older effect/terrain field introductions retain
their historical version numbers.

## Navigation revision validation

Build navmesh_tests, ai_tests, application_tests, physics_tests and movement_tests;
run CTest `^(navmesh|ai|application|physics|movement)$`. Verify stale/duplicate and
failed publication, dirty query counts, removed support ahead of the agent,
original-goal recovery, retained total deadlines, finite replacement/search
limits, cancellation and explicit terminal retry. Build web_assets with the local
SDK pinned by .emscripten-version; serve matching host/main/worker artifacts.
Run scripts/navmesh-smoke.cjs on actual WebGPU/WebGL2, then ai-smoke.cjs for
regression. Evidence defaults to build/evidence/navmesh-updates; real controls
remove/restore the floor bridge and close/open a passage before explicit rebuilds.
See selected engine docs/navigation-updates-validation.md. This does not prove
automatic collision tracking, incremental carving or physical-mobile acceptance.

## State-machine and graphical AI editor acceptance

Build state_machine_tests, ai_tests, navmesh_tests, editor_tests,
editor_content_tests and editor_structure_tests. Run CTest
`^(state_machine|ai|navmesh|editor|editor_content|editor_structure)$`. Verify callback
order, timers, stop/restart, cycle/work bounds, exception cleanup/reentry rejection,
atomic invalid document preservation and exact .weai roundtrips. With pinned local
SDK build web_assets and run scripts/editor-ai-smoke.cjs on WebGPU/WebGL2 at DPR1
and DPR2. Verify real drag/Escape, edges/priority/delay/entry, isolated C++ preview,
source bytes/history, file import/export, malformed preservation and authored live
demo behavior. Retain ai-smoke.cjs, navmesh-smoke.cjs and editor-smoke.cjs regressions.
See selected engine docs/ai-state-machines-validation.md for exact evidence.
Desktop/DPR acceptance does not establish physical mobile or game-specific bindings.

## Driving Stage 2 validation

Build/run physical_surface_tests and physical_surface_wasm_tests (Node), then web_assets. Run scripts/physical-surface-smoke.cjs with actual DEMO_URL/NODE_PATH/BROWSER_EXECUTABLE, DPR1 and DPR2/separate EVIDENCE_DIR, forcing both WebGPU/WebGL2. Regress vehicle, physics, editor/archive and terrain paging/edit suites. See [contracts and validation guidance](physical-surfaces.md). Current writer is v26; v24 remains the vehicle-field introduction.

## Driving Stage 3 validation

Run CTest camera_follow and full native regressions after schema changes; run Node camera_follow_wasm_tests plus vehicle/physical_surface_wasm_tests and scripts/gamepad-adapter-tests.cjs. Publish web_assets and run scripts/driving-controls-smoke.cjs with real DEMO_URL/NODE_PATH/BROWSER_EXECUTABLE at DPR1/DPR2, both forced renderers. Regress vehicle-smoke, physical-surface-smoke and input-smoke (RENDERER=webgpu|webgl2). See [contract/validation guidance](driving-controls.md). Current archive writer is v26.

For playback rate/driving sound, run native `audio` and the full CTest suite, then
`scripts/driving-audio-smoke.cjs` on both renderers at DPR1 and `DPR=2`, plus
`scripts/audio-system-smoke.cjs`. Verify live samples, source identity, activation,
offset/completion/suspension, tuning file/local reload and editor Play/Stop source
hash; inspect screenshots. See [contracts and scope](driving-audio.md).

## Actor AI controller acceptance

Build ai_controller_tests plus state_machine, AI, editor/editor_structure,
application, scene_archive, physics and movement targets. Run CTest
`^(ai_controller|state_machine|ai|editor|editor_structure|application|movement|physics|scene_archive)$`.
Verify controller-before-free-motor ordering, lifecycle cancellation, independent
instances, parameter/schema validation, bounded logs, exact configuration codec
roundtrips, saved-target resolution, scene undo/redo and Play source preservation.
Build matching web_assets with the repository-local pinned SDK. Run
scripts/editor-ai-controller-smoke.cjs on WebGPU/WebGL2 at DPR1/DPR2 for real
assignment, missing binding/motor errors, all typed fields, .wescene download/reload,
Play motion, active-state/time/transition debugger and Stop preservation. Retain
editor-smoke.cjs, editor-ai-smoke.cjs and ai-smoke.cjs regressions. Read selected
engine docs/ai-controllers-validation.md for exact evidence and limitations.

Stage5: native `spline_route` and full CTest; build/run explicit
`spline_route_wasm_tests`, then `scripts/spline-route-smoke.cjs` on both forced
backends at DPR1 and `DPR=2`. Inspect solid/collision/closed geometry, source sharing,
source hash across Play, invalid/budget failures, file/IndexedDB reload, real vehicle
contact and recovery. Follow [route acceptance scope](splines-routes.md).

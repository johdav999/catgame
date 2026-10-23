# Planetary terrain stage 1

This is a development WebEngine feature. Verify `include/engine/planet.hpp` and
`cmake/planet.cmake` in the selected checkout; updating this skill does not advance
a Paws engine pin or add a planetary world to the game.

Read the selected engine's `docs/planet-terrain.md`, `docs/planet-validation.md`
and architecture section before integration. Link optional `WebEngine::Planet`
(`engine_planet`, enabled by `WEBENGINE_BUILD_PLANET=ON`, default ON). It depends
on engine_core; core has no dependency on it. `WEBENGINE_BUILD_PLANET_DEMO=ON`
defaults OFF and adds browser `planet_demo` / Windows `webengine_planet_windows`.

`PlanetPosition` holds double planet-centred coordinates. `relativeTo(origin)`
subtracts doubles before narrowing to float Vec3. `PlanetTile{face,level,x,y}`
addresses a cube-face quadtree; `PlanetTerrain::direction(tile,u,v)` uses [0,1]
coordinates. `height(direction)` / `surface(direction)` are deterministic global
source queries independent of LOD, origin or residency. `prepare(tile)` returns
an immutable PlanetPatch containing its double anchor, ordinary Mesh and bounds.

`PlanetTerrain::update(PlanetView,deltaSeconds)` returns PlanetDraw values with
immutable patch, parentWeight and conservative horizonVisible. The caller owns
the clock, scene and projection; place each actor at patch.anchor.relativeTo(eye),
keep the camera at zero, assign the ordinary mesh and set morph weight zero's
value to parentWeight. The example uses existing Application snapshots and a
bounded scene-owned actor/component pool. This is a render projection convention,
not a generic scene or physics rebasing API. Other objects need the same origin.
Neither the library nor its public headers own scenes, renderer/platform types,
callbacks, workers or physics. Release old draw lists/resources to bound external
memory. Mesh resources are immutable; morph weights belong to components.

Defaults: radius 6,371,000 m, height scale 12,000 m, 16 cells, max level 20,
240 leaf patches, target 3 pixels, 0.35 s refinement morph. Settings bounds are
radius 1,000..1e10 m, heightScale 0..5% radius, power-of-two cells 4..32,
maxLevel 0..24, leaves 6..1024, targetPixels .5..32, morphSeconds .05..5.
View FOV is .1..3 radians, viewport height 1..16384, update delta finite 0..1 s.
Invalid view/delta data rejects before modifying accepted state.

Six roots always cover the planet. One update prepares at most one four-child
split; all children prepare before replacement. Children morph from actual parent
triangles and finish before splitting again. Complete stable siblings merge with
hysteresis; a full budget reclaims less important groups to follow the observer.
Merges directly select the parent rather than reverse-morphing. Resident ancestors
add to leaf counts: a complete six-root tree has (4*leaves-6)/3 resident nodes.
Synchronous CPU count budgets are not frame-time or GPU-memory guarantees.

Matching direction samples and outward radial skirts cover visual LOD seams.
They do not create welded topology or planetary collision. Error is estimated
from source frequency/curvature; the hard budget can leave coarser detail or visible
material changes after rapid travel. Local float clipping adapts with altitude;
no logarithmic/reversed depth or multi-frustum planetary pipeline is implemented.

The shared flight example has WASD, radial Q/E, Shift x5 and right-drag look;
Escape/focus loss/cancellation release input. Guided descent and ascent each take
28 s. Orbit bookmark is an explicit reposition, not seamless-flight evidence.
The 3 m radial source clearance assist is demo policy, not landing physics.
Landing/collision, arbitrary-up walking, planet atmosphere/oceans, custom providers,
authored terrain, seeds, persistence and physical mobile acceptance remain absent.

Build with repository-local Emscripten 6.0.11:

```powershell
& ./.tools/emsdk/upstream/emscripten/emcmake.exe cmake -S . -B build/web-release -G Ninja -DCMAKE_BUILD_TYPE=Release -DWEBENGINE_BUILD_PLANET_DEMO=ON
cmake --build build/web-release --target planet_demo planet_tests --parallel 4
python scripts/serve.py --port 8168 --directory build/web-release/planet-site
```

Open ?backend=webgpu / ?backend=webgl2 on that site. It does not overwrite dist.
If sandbox temp permissions block Emscripten, set TEMP/TMP to a task-local folder.
Windows requires its normal installed SDKs and WEBENGINE_BUILD_WINDOWS=ON; build
webengine_planet_windows/planet_tests, launch --backend dawn or --backend d3d12.

Validate native CTest planet and Wasm `node build/web-release/planet_tests.js`.
Run `scripts/planet-smoke.cjs` with installed Playwright/pngjs through NODE_PATH.
It serves PLANET_SITE (default build/web-release/planet-site) on its own ephemeral
port; PLANET_URL optionally selects an existing server, PLANET_BACKEND limits
the run and BROWSER_EXECUTABLE chooses Chrome/Edge. It checks actual buttons,
continuous trajectories, visible pixels, fine refinement, controls/cancellation,
source clearance, resource bounds and return-to-space merges on both backends.
Windows --smoke <directory> captures PPMs; run tools/verify_planet.py for pixel
checks and PNG previews. Source/edge/morph/precision and residency checks pass
through the same CPU code on native/Wasm. Read the current validation record for
actual evidence; do not describe an earlier result as a new run.

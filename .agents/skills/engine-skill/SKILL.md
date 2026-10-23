---
name: engine-skill
description: Build and extend playable browser games using Johan's WebEngine C++20/WebAssembly engine. Use for game mechanics, levels, input, cameras, interactions, physics, assets, HUD, audio, saves and game integration in WebEngine, or engine extensions needed by those games. Applies to this repository's engine, not unrelated web game frameworks.
---

# Engine Skill

Turn game requests into playable WebEngine applications that compose existing systems. Keep game rules in application code; extract reusable mechanisms when a concrete mechanic needs them.

## Establish current contracts

For Paws, resolve WebEngine through the pinned `external/WebEngine` submodule by default, or an explicitly configured local checkout in ignored `developer-settings.json` / `CMakeUserPresets.json`. Follow the game repository's `AGENTS.md`, `README.md` and `docs/architecture.md` for dependency selection, consumer builds and runtime ownership. Engine contract paths below are relative to that selected engine checkout; game builds and acceptance run from Paws using its own scripts and generated site.

This repository-local copy was refreshed on 2026-10-15 from the installed `engine-skill` maintained by WebEngine at `C:/Users/Johan/.codex/skills/engine-skill`. On other machines, resolve that installed skill through the skill catalog. When refreshing, compare the complete skill folder, including supporting references, and retain this Paws dependency guidance. CatGame now pins published WebEngine `c819dbf07ee4f6a0810f8dbd017a441d856f326a` (2026-10-23), including portable modules/quests, consumer hosts and the M7 SDK handoff. Read docs/quest-module-m7.md for Paws release commands and scope. The source skill may describe later development features; verify availability in the selected checkout before using an API. Refreshing skill documentation does not advance the engine gitlink.

Read applicable `AGENTS.md`, then `architecture.md` before planning or changing engine APIs, platform/build boundaries or game/demo behaviour. Inspect `git status --short` and preserve unrelated work. Read relevant README sections and feature documents, then verify exact signatures and implementations in `include/engine/`, `src/` and `web/`. Select documents through [the contract map](references/contract-map.md).

Current source and contracts take precedence over this guide's API landmarks. Roadmaps/readiness reviews can lag implementation: verify claims before scheduling an existing system for reimplementation. Do not treat intended extensions as delivered features or expand a game request to implement unrelated services.

The supporting references were checked against the working checkout on 2026-10-11, including its uncommitted features. Recheck current contracts when using them. Read [feature integration details](references/feature-integration.md) when composing rendering backends, mobile loading, movement, precise interaction, animation or saved behaviour.

## Implement a coherent game

Infer mechanics, controls and target devices from the request. Resolve routine choices independently; ask only when missing information materially affects the result. For a new game, deliver a small playable loop with clear controls, feedback and appropriate completion/failure/restart behaviour. For an existing game, extend the requested behaviour within its established loop.

Keep levels, goals, scoring, progression, inventory, hints, move legality and undo policy in application/example code. Inspect `src/main.cpp` and CMake targets before choosing integration. Existing demo placement has documented exceptions; avoid copying those into new generic modules or reorganizing unrelated demos. Prefer a clear game-specific source boundary reusing the engine and browser host.

Compose only the systems the game needs:

- Own scenes through `Application`; assemble actors/components through `Scene::spawn<T>()`, `Actor::addComponent<T>()` and the owned hierarchy. Use `Prefab` for repeated configured hierarchies when useful.
- Map physical input to portable actions/contexts, then gameplay commands. Route GUI, camera and scene contacts explicitly so one gesture has one owner. Cancel capture/held commands on blur, context changes, transitions and target loss; keep DOM/key/touch details in adapters.
- Use character motors for command-driven movement, physics queries/sensors for collision/triggers, and selection/actions for interaction. Choose transform manipulation for eligible non-physics actors or joint/grab commands for physics actors. Avoid competing transform writers; collision layers are distinct from render masks.
- Optional `engine_ai` provides immutable `WaypointGraph` and static collision-baked `NavMesh` navigation through `NavigationSurface`, bounded A*, `PathFollower`, handle-safe sight and `DecisionClock`. `Navigator` (`WaypointNavigator` compatibility alias) owns a shared surface and motor handle, checks capsule clearance before movement, and supports bounded delayed recovery (default 64 queries/step, 256 expansions, four replans, 60 s total deadline). Call from owning actor fixed ticks before child motors and cancel on disable/detach, target loss or handoff. Include `engine/navmesh.hpp` and link `engine_ai`; private vendored Recast/Detour 1.6.0 types never cross portable headers. Scene bakes use synchronized registered static solid collision plus occupancy filtering; raw triangle bakes describe caller-defined surfaces/interiors. Defaults: .2/.1 m cells, .35 m radius, 1.9 m height, pi/4 slope, .3 m step, .5 m endpoint/vertical snap; 262144 cells, 2000000 raster/occupancy work, 4096 polygons. Bake synchronously on the idle scene owner and publish only a successful immutable replacement. Motor profile compatibility is checked. `NavigationSource` adds owner-thread revision invalidation before collision changes and one matching complete publication per dirty revision. Use `Navigator::navigateSource`; stale active routes enter WaitingForSurface before child motor ticks, spend no search queries while dirty, and replan retained goals after publication without resetting retry/deadline limits. Failed/stale bakes never reactivate old topology; terminal requests require explicit retry/new goals. No automatic collision subscriptions or worker-safe scene access is implied. See `docs/navigation-updates-validation.md`. Moving baked geometry requires explicit rebaking; runtime sweeps cannot prove support in stale topology. No dynamic carving, crowd avoidance, AI runtime checkpoint codec or general stair-walking acceptance is delivered. Games own goals, actions and work budgets across agents. See [game AI](references/game-ai.md), [navmesh integration](references/navmesh.md), `docs/navmesh-validation.md`, native `ai`/`navmesh` and both-renderer `scripts/ai-smoke.cjs` / `scripts/navmesh-smoke.cjs`. Development APIs may be absent from a game pin.
- Reusable flat `StateMachine` in optional `engine_ai` executes shared immutable `StateMachineDefinition` assets with named application behavior/condition bindings, enter/update/exit, per-agent simulation timers and ordered transitions. Defaults: one transition and 1024 conditions per step; limits 1..64 / 1..65536. Cycles defer at transition limits; condition exhaustion/callback exceptions clean up, mark Failed and rethrow. `StateMachineHost` is synchronous and non-retained; exit is noexcept, reentry is forbidden, and lifecycle/control handoff must stop the instance and cancel persistent navigation. Guard component lookups during deferred removal. `engine_editor` now publicly links `engine_ai` and exposes independent `AiGraphEditor` immutable document/history (default 128 edits). Details -> AI state machines opens graphical node/edge authoring, entry/priority/delay/binding edits, drag cancellation, independent undo/redo and isolated C++ preview with controllable conditions. `.weai` version 1 files retain definitions/layout, not callbacks or runtime state; load limit 512 KiB, 1..256 states, <=1024 transitions, finite positions within +/-10000, delays 0..86400, names/keys 1..256 bytes. Save graph files separately from scenes. Use in AI demo validates known application bindings before adoption; games supply their own hosts. No hierarchical/concurrent states, visual action scripting, graph autosave or automatic game callback discovery is delivered. See [state machines and graphical editor](references/state-machines.md), selected engine `docs/ai-state-machines-validation.md`, CTest `state_machine`/`ai`/`editor` and `scripts/editor-ai-smoke.cjs` on both backends including DPR2. A game pin may lack these development APIs.
- Optional `AiControllerComponent` attaches immutable graphs and typed `AiBindingRegistry` mappings to ordinary actors, with independent runtime/timers and a bounded 32-transition log. A controller must be a direct actor component; only one owns an actor. Generic `Component::fixedTickOrder()` defaults 0; stable ascending sibling fixed traversal runs AI at -100 before motors regardless of insertion order, preserving actor-first dispatch, hierarchy and render ticks. Registry keys bind C++ callbacks with Boolean/Number/Vector/saved-Actor parameters; built-ins are idle/move and always/flag/near. Complete configuration/schema/reference validation precedes replacement. Disable/detach/control handoff cancels commands; re-enable needs explicit restart. Supply optional `aiControllerCodecs(registry)` to SceneArchive save/load/clone and EditorPlaySession::start; payload v1 embeds definition/configuration, not callbacks, runtime timers or commands. Core stays independent of AI. Browser scene files/Play register the codec. Details -> Actor AI controller assigns the open .weai graph, reports missing bindings, supplies typed forms and scene undo/redo; Live debugger during Play shows selected runtime state/time/history while authoring stays read-only. Native custom registries must outlive their components/codecs. No automatic ground/navmesh, callback discovery, runtime checkpoints or custom-component subtree duplication is implied. See [actor AI controllers](references/ai-controllers.md), selected engine `docs/ai-controllers-validation.md`, CTest `ai_controller` and `scripts/editor-ai-controller-smoke.cjs` on WebGPU/WebGL2 including DPR2.
- Give each camera one writer; evaluate camera controllers after simulation and before rendering. Presentation poses, GUI focus and view selection must not determine authoritative gameplay state.
- Share immutable mesh/material/texture/skeleton/clip assets; keep transforms, overrides and playback state per instance. Gate transitions on asset preparation and retain the active scene on failure. Verify current decode/upload capabilities before assuming loading is stall-free or GPU-prewarmed.
- Optional terrain combines immutable float/image heightfields, layered materials, seamless camera/shadow LOD, full-source brushes and bounded streaming. `TerrainPagedStream` separately bounds source pages/jobs/admissions; browser workers read/generate chunks. Opt-in `enableEditing(writer)` queues atomic cross-page sculpt/paint, loads/pins every required halo and preserves handles; paint shares geometry/collision. Dirty pages stay pinned until generation-checked atomic writeback succeeds. Failures/conflicts retain edits and stop automatic retries; flush retries, clear rejects unsaved state, explicit discard reloads saved data. Native hosts supply IO/executors and atomic writers; browser loaders/writer share an IndexedDB namespace. Scene outlives the owner, and destruction discards unsaved state: flush/await clean before releasing it when edits must survive. Format 15 snapshots contain current source edits, excluding dirty flags/generations/jobs/provider/history. Read `docs/terrain.md` and [feature integration](references/feature-integration.md) for budgets, lifecycle, synchronous edit-rebuild costs and worker artifacts. Run CTest `terrain`, `terrain_paging`, `terrain_page_edit` and all six terrain browser suites on both backends. Demo `?scene=terrain` includes paged brushes, save/retry/discard and persistent reload; 16-stamp undo remains full-source only. GL supports terrain color/camera LOD with basic lighting/no shadows. Geomorphing, remote write adapters and automatic conflict merging remain extensions.
- Use optional `Scene::setSky(SkyBackground)` for gradient/panorama/atmosphere skies on both browser backends. `SkyGeometry::Sphere` draws a shared inward unlit mesh per camera; it never enters scene picking/shadows/physics. `SkyClouds` provides procedural coverage/density/wind phase. Opt into `SkyDayNight` and call `advanceSky(scene, chosenDelta, directionalLightId)` once after simulation to update sun/moon, ambient and existing IBL intensity. Choose clock and light ownership explicitly; resolve scene-scoped handles and restore alternative lighting policy deliberately. Set `RenderView::skyEnabled=false` for load-color views preserving earlier backgrounds. Format-6 archives retain sky/weather/cycle state, independently of GPU resources and caller handles. See `docs/sky.md`, native `sky_tests`, `scripts/sky-smoke.cjs` and `scripts/sky-weather-smoke.cjs`. The playground defaults to an atmosphere mesh sphere with clouds, 09:00 and a 180-second day; these are application defaults, while reusable scenes start with sky off. Clouds are 2D, and scattering is single scattering; no volumetric clouds, cloud shadows, multiple scattering or automatic IBL rebake is provided.
- Use the browser `Renderer` facade and reported `RenderCapabilities`. Automatic initialization prefers WebGPU and can fall back to WebGL 2 before canvas acquisition; the fallback has reduced lighting/features. Service recovery at host frame boundaries even while the renderer is not ready. Keep backend resource types private.
- Water uses `MaterialShader::Water`, `Material::water` and optional full `MaterialInstance::water` overrides on subdivided local-XZ meshes. Both backends support Gerstner waves, ripple normals, Fresnel/highlights, depth-aware refraction/foam, one-plane-per-view opaque planar reflections, and underwater fog/absorption with optional animated caustics. WebGPU can use IBL; WebGL 2 uses authored horizon/zenith colors. Sky is independent. Supply `Renderer::setWaterTime(seconds)` once per frame with explicit pause policy; views share it and the renderer never advances it. Bounds expand without changing CPU geometry, physics or picking. Refraction, foam, planar and underwater strengths default zero. Planar capture selects the first visible requesting plane, shares it with coplanar water, clips submerged objects and reverses winding; other planes retain fallback. `underwaterStrength` selects the first active masked World water containing the eye below its transformed local-Y=0 plane and within its local XZ bounds, even when the surface is offscreen. `underwaterFogDensity`, `underwaterColor`, `underwaterMaxDistance` and existing `absorption` control depth attenuation; use `doubleSided=true` to see the underside. Fog captures completed World color/depth after transparency, before Overlay/GUI, clips rays to the surface/sides, and retains per-view 1x/4x, resize/recovery and archive contracts. It has no bottom boundary or inferred arbitrary volume. Current v16 archives include v11 underwater settings and v13 caustics; older water defaults new effects off. Caustics strength defaults zero; scale/speed/depth fade control procedural receiver lighting inside submerged views, sharing the fog composite and caller clock. `Renderer::setWaterInteractions` copies up to 16 transient world-space ring/wake sources; applications own contact/motion emission. Surface normals/crests change without geometry, physics or added passes. Inputs share water time, filter by camera masks/age, survive GPU recovery and clear on scene replacement; they are not material/archive state. Extra reflection planes, transparent-depth fog and buoyancy remain extensions. Renderer time stays application state. Read `docs/water.md` and [feature integration](references/feature-integration.md); use CTest `water` and the six water smoke scripts in [build validation](references/build-validation.md). Demo: `?scene=water`, submerged/deep views, effect presets and colored distance markers.
- Optional MSAA uses `RenderQuality::msaaSamples=1|4`, `RenderCapabilities::msaa4x` and `Renderer::effectiveMsaaSamples()`. Profiles default to off; unsupported devices retain the requested policy and render at 1x. Scene layers/views resolve once before single-sample GUI; shadow filtering and canvas resolution remain separate. See `docs/rendering-quality.md#optional-msaa` and `scripts/msaa-smoke.cjs`.
- For larger content, use the optional isolated `browserDecodedAssetLoader`, per-frame `UploadBudget` and offline KTX2/fallback texture tooling where appropriate. These mechanisms are implemented; CPU asset readiness does not mean every draw is GPU-ready. Loading-screen and eviction policies remain application choices.
- Scene authoring uses optional `engine_editor`: `EditorSession` / `EditorPropertyRegistry` over an idle scene and stable-address asset library. **Scene editor** / `?editor=1` provides files, typed component/material forms, assets/placement/snapping, component add/remove, KeepWorld reparenting, ordered multi-selection, grouped move/rotate/scale/duplicate history and local autosave/recovery. `EditorPlaySession` owns a separate Application and built-in scene clone; Stop rebinds retained source views before releasing the preview, preserving authoring bytes, camera, selection and history. Immutable assets are shared; mutable simulation and clocks are isolated. Reconstruct game callbacks, input bindings, joints, GUI and audio explicitly when needed. Preserve physics topology, managed children and renderer-borrowed camera lifetimes. Gate authoring/files/autosave during Play. Read `docs/scene-editor.md` and [scene-authoring integration](references/feature-integration.md); run CTest `editor`, `editor_content`, `editor_structure` plus all three editor browser smoke scripts on both renderers, including DPR2. Advanced factories/inspectors, marquee selection, Play apply-back and prefab definition authoring remain extensions.
- Compose retained GUI, audio and persistence as the loop requires. Preserve browser audio activation/resume handling. Save game data with stable saved identities and custom codecs; reconstruct executable behaviour after loading.

WebGPU material pipelines in the development engine now specialize material
family, alpha mode, terrain layers, optional map presence and reflection clipping.
Applications continue to configure ordinary immutable `Material`/`MaterialInstance`
values; no game-side shader fork or new quality option is required. Verify this
implementation in the selected checkout: it is included in the current CatGame
engine pin. See `docs/rendering-quality.md#webgpu-material-pipeline-specialization`.

## Preserve runtime invariants

`Application::advance()` owns input snapshots, variable scene updates, fixed physics steps and presentation clocks. Do not run scene/animation updates again inside its callbacks. RAF timestamps enter as seconds. Use simulation time for gameplay, fixed ticks for fixed-step logic and an explicit input/presentation time policy for menus, cameras and audio. Pause and host suspension differ: preserve explicit pause through page visibility changes and avoid hidden-time catch-up. Standalone schedulers must deliberately follow the scene update contract.

Use `Scene::synchronize()` for lifecycle/registration reconciliation without ticks; `Scene::update(0)` still performs a variable tick. `fixedUpdate()` does not advance animation clocks. Opt game actor/component tick hooks in through their supported flags. Preserve bounded fixed-step catch-up and the application transition/preparation boundary before input snapshots.

The scene hierarchy owns nodes uniquely. Query pointers/references are borrowed. Delayed work should retain scene-scoped, generation-checked `ActorId`/`ComponentId` handles and resolve them before access. Destruction excludes targets immediately and defers removal until guarded traversal ends. Detach/reparent only at allowed idle boundaries; preserve cache invalidation and handle `KeepWorld` failure. Protected skeletal implementation children cannot be removed independently.

Declare lifetimes for renderer/camera/scene references, controllers, subscriptions and async interests. Release borrowed presentation references before scene destruction; dispose browser adapters before portable owners. Reject stale completions targeting replacement scenes. Validate candidates before publication so malformed data or failed construction preserves valid state.

Simulation positions/collision/game rules remain independent of render interpolation, culling and LOD. Consume animation markers/root motion explicitly; gameplay owns movement/collision policy. Physics saves are not exact solver rewind. Built-in animation controller definitions/state, markers and pending output are archived. Runtime handles, executable callbacks, input bindings, GUI trees, audio voices, camera behaviours and physics joints are not automatically restored: verify codecs and rebuild runtime behaviour intentionally. Reject unknown custom component types rather than silently degrading them.

Keep DOM, Emscripten, WebGPU and solver implementation types out of new portable interfaces. Update `architecture.md` in the same change if responsibilities, ownership, frame execution, dependencies, platform boundaries or extension status change. Keep affected feature contracts consistent with delivered code. Whenever new engine functionality is added, existing functionality is extended or architecture changes, update the installed source `engine-skill`, this repository-local copy and affected supporting references in the same task before reporting completion. Document delivered APIs, usage, defaults, limitations, architectural contracts, build commands and validation guidance.

Browser editor docking, floating panels, resizing, saved layouts and Quick Add are
documented in [editor workspace guidance](references/editor-workspace.md). Preserve
the existing canvas/control instances and keep layout storage outside scene history.

## Validate and hand off

Follow [build and validation guidance](references/build-validation.md). Use focused native checks for portable logic and actual browser play/smoke checks for controls, adapters and rendering. Exercise the requested loop and affected restart, transition and cancellation paths. Documentation-only changes need content/link checks rather than runtime builds.

Report delivered behaviour, launch instructions, checks actually run and material limits. For substantial phased work, leave useful evidence and a repository-local continuation note. Previous evidence is not a new run; core-only native builds are not desktop games; mobile emulation is not physical-device acceptance; fewer uploads/draws do not establish FPS improvement. Deployment and external actions stay within the user's authorized scope.

## Exponential height fog (2026-10-16)

Development WebEngine now has optional `Scene::setFog(FogSettings)` / `setFog(std::nullopt)` and `Scene::fog()`. `RenderView::fogEnabled` defaults true. Fog settings are color (linear display light), density, startDistance, baseHeight and heightFalloff; zero falloff is uniform exponential fog. New scenes default fog off. World materials, terrain, alpha surfaces and particles use their own positions; additive effects attenuate to zero. Sky integrates to the far plane; Background meshes, Overlay and GUI are excluded. Both backends share stable analytic math, with perspective/orthographic rays, MSAA and recovery. Water composites already fogged captures without repeat fog; underwater remains a separate approximate composite. Format 16 saves/clones scene fog and view opt-out; older archives load without fog. Read `docs/fog.md` and the fog entries in supporting references. This development feature is not evidence of availability in a game pin.

## Editor camera navigation (2026-10-16)

The WebEngine development editor supports right-mouse look/WASD flight, Q/E
vertical movement, Shift speed, middle-mouse pan and double-click/F outline
framing. Read [editor camera integration](references/editor-workspace.md).
Keep device bindings in the browser adapter and navigation in OrbitController;
camera gestures do not create object history or dirty revisions. Validate both
renderers and DPR2 with the camera smoke suite. This does not change game pins.

## Terrain authoring in the editor (2026-10-17)

The development browser editor now provides Quick Add Terrain and a Terrain
section in dockable Details: flat/imported red-channel heightmap creation,
Raise/Lower, Flatten, Smooth and four-layer Paint, with a projected brush ring.
EditorSession owns one bounded history command per complete stroke, with preview,
commit/cancel and existing save/autosave/Play integration. Stream/paged owners are
read-only in this editor. Read [terrain editor integration](references/editor-workspace.md).
Verify the consuming game's engine pin before assuming this authoring host exists.

## Terrain paint layer textures (2026-10-17)

Details → Terrain → Paint layer textures assigns or clears imported Assets images
for all four layers. Each assignment is undoable, updates the whole selected
terrain, and preserves weights, geometry, collision, tint and tiling. Painting,
scene saves/recovery and Play retain the bindings. Read
[layer texture integration](references/editor-terrain-textures.md) for the portable
APIs, ownership gates and focused validation. Both renderers are supported.


## Shaped and soft particle FX (2026-10-17)

Development WebEngine now supports shaped emission, drag/wind, seeded gusts, vector
motion curves, per-camera velocity-stretched billboards and optional opaque-depth
soft particles on both renderers. Defaults preserve legacy effects. Archive v17
retains definitions and live phases; read [FX integration](references/fx.md) for
APIs, ownership, limits, capture costs, build commands and validation. The playground
adds Fountain and mist. Verify the consuming engine pin before using these additions.


## FX authoring (2026-10-17)

Development WebEngine now provides a Particle FX inspector, curves/bursts and emitter
delay timeline, versioned .wefx assets and isolated previews with diagnostics.
Read [FX authoring](references/fx-authoring.md) for APIs, immutable history, renderer
rebind/cancellation, preview clocks, v18 persistence, builds and both-backend checks.
Verify the consuming engine pin; this documentation does not advance a game gitlink.

Ribbon trails: see [integration](references/trails.md) for bounded local/world paths,
per-camera rendering, editor preview and archive v19 contracts.

Plane particles: see [integration](references/plane-particles.md) for ground/supplied-normal
alignment, offset, local/world orientation, Ground ring authoring and archive v20.

Mesh particles: see [integration](references/mesh-particles.md) for shared static
geometry/materials, seeded XYZ rotation/spin, instancing, authoring and archive v21.


## Terrain surface relief (2026-10-17)

Development terrain materials now support optional per-layer normalHeightTexture
(RGB tangent normal / alpha height), heightBlend and world-space triplanar mapping.
Defaults preserve existing surfaces. Archive v23 persists these appearance fields;
use matching main/worker artifacts. Both renderers apply normals, while WebGL2
retains basic lighting and no shadows. See [surface integration](references/feature-integration.md#terrain-surface-relief-development-engine-2026-10-17)
and run scripts/terrain-surface-smoke.cjs on both backends. Verify the selected
engine checkout before consuming these development APIs in a pinned game.

## Particle contacts and projected decals (2026-10-17)

Development WebEngine adds opt-in swept-sphere particle collision with Bounce,
Slide, Kill and Settle, bounded query/impact queues, and CPU-clipped DecalComponent
projectors on solid opaque static mesh geometry. Applications consume queued
impacts and own mark creation/lifetime. Both renderers share the ordinary mesh
path. Read [contacts and decals](references/fx-contacts-decals.md) for APIs, defaults,
filters, budgets, ownership, editor integration, archive v22 fields (current writer
v24), build commands and validation guidance. Verify the selected engine pin.


Development server startup: `scripts/serve.ps1` now tries up to 100 consecutive
ports from `-Port` (default 8080), skipping busy/access-denied binds and printing the
actual URL. Use `-PortAttempts 1` for fixed-port automation. Direct serve.py stays
fixed-port unless `--port-attempts` is supplied. This is development tooling,
independent of renderer/scene state; see [build validation](references/build-validation.md).


Development WebEngine now provides optional VehicleComponent with shared immutable
four-wheel configuration, command-driven Jolt suspension/drivetrain/brakes, coherent
post-step telemetry, interpolated wheel visuals, COM/inertia and point-force APIs.
Editor properties/presets/history/isolated Play and v24 scene/application checkpoint
persistence are integrated. Use [vehicle integration](references/vehicles.md) for
defaults, topology/lifetime/input rules, partial-state limits and both-renderer
validation. Verify the selected game pin; driving rules remain application code.

- Driving Stage 2 adds immutable physical surfaces, bounded compound collision, source query/wheel/contact metadata and pre-solver closing velocity. Use [physical-surface integration](references/physical-surfaces.md) for friction policy, editor history, v25/WETP3 persistence, lifetime/budget contracts and both-renderer validation. Verify APIs in the selected engine checkout; consumer pins may lag.

- Driving Stage 3 supplies optional source-isolated gamepad input and explicit composed follow cameras with presentation sampling, bounded velocity look-ahead, typed authoring, isolated preview and v26 persistence. Read [driving input/camera integration](references/driving-controls.md) for defaults, ownership, cancellation, reset, limits and both-renderer validation; verify the selected consumer pin.

Camera ownership handoffs may use `OrbitController::adoptCamera(pivotDistance)` to resample the displayed local eye/yaw/pitch without moving it. The driving host adopts only when its composed rig last wrote that view, retaining ordinary archive orbit metadata. See [Stage 3](references/driving-controls.md).

Playback-rate voices and the optional driving audio example are covered in
[driving audio](references/driving-audio.md). Keep RPM/load/slip/surface sound policy
in applications; editor tuning has its own document/history, and audio runtime
state remains transient. Verify API availability against the selected engine pin.

Immutable splines and procedural route sweeps, protected static collision, grouped
editor authoring and v27 source-only persistence are documented in
[spline/route integration](references/splines-routes.md). Keep lane/junction policy
in games and choose explicit rebuild/query budgets. Verify availability at game pins.


## Module SDK release handoff (M7, 2026-10-23)

The published quest-module SDK at c819dbf07ee4f6a0810f8dbd017a441d856f326a adds
independent generic and custom quest-binding consumers, native/Node-Wasm CTest,
feature-omission coverage and docs/module-sdk.md. Runtime ownership and module
API 1.0.0 remain unchanged. Paws M7 pins that revision and supplies clean-source
web/Windows package validation and a full release matrix. Build consumers from
checked-in accepted assets; offline authoring must not run during consumer builds.
Read references/modules.md and references/consumer-hosts.md for the handoff.

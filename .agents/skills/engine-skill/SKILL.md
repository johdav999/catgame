---
name: engine-skill
description: Build and extend playable browser games and the optional Windows desktop host using Johan's shared WebEngine C++20 engine. Use for game mechanics, levels, input, cameras, interactions, physics, assets, HUD, audio, saves and game integration in WebEngine, or engine extensions needed by those games. Applies to this repository's engine, not unrelated web game frameworks.
---

# Engine Skill

Turn game requests into playable WebEngine applications that compose existing systems. Keep game rules in application code; extract reusable mechanisms when a concrete mechanic needs them.

## Establish current contracts

For Paws, resolve WebEngine through the pinned `external/WebEngine` submodule by default, or an explicitly configured local checkout in ignored `developer-settings.json` / `CMakeUserPresets.json`. Follow the game repository's `AGENTS.md`, `README.md` and `docs/architecture.md` for dependency selection, consumer builds and runtime ownership. Engine contract paths below are relative to that selected engine checkout; game builds and acceptance run from Paws using its own scripts and generated site.

This repository-local copy was refreshed on 2026-10-15 from the installed `engine-skill` maintained by WebEngine at `C:/Users/Johan/.codex/skills/engine-skill`. On other machines, resolve that installed skill through the skill catalog. When refreshing, compare the complete skill folder, including supporting references, and retain this Paws dependency guidance. CatGame now pins published WebEngine `461ab6dd1769171955254473c2b3623ad3dc615a` (2026-10-23), including portable modules/quests, consumer hosts and the M7 SDK handoff. Read docs/quest-module-m7.md for Paws release commands and scope. The source skill may describe later development features; verify availability in the selected checkout before using an API. Refreshing skill documentation does not advance the engine gitlink.

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
- Optional `engine_ai` provides immutable `WaypointGraph` and static collision-baked `NavMesh` navigation through `NavigationSurface`, bounded A*, `PathFollower`, handle-safe sight and `DecisionClock`. `Navigator` (`WaypointNavigator` compatibility alias) owns a shared surface and motor handle, checks capsule clearance before movement, and supports bounded delayed recovery (default 64 queries/step, 256 expansions, four replans, 60 s total deadline). Call from owning actor fixed ticks before child motors and cancel on disable/detach, target loss or handoff. Include `engine/navmesh.hpp` and link `engine_ai`; private vendored Recast/Detour 1.6.0 types never cross portable headers. Scene bakes use synchronized registered static solid collision plus occupancy filtering; raw triangle bakes describe caller-defined surfaces/interiors. Defaults: .2/.1 m cells, .35 m radius, 1.9 m height, pi/4 slope, .3 m step, .5 m endpoint/vertical snap; 262144 cells, 2000000 raster/occupancy work, 4096 polygons. Bake synchronously on the idle scene owner and publish only a successful immutable replacement. Motor profile compatibility is checked. `NavigationSource` adds owner-thread revision invalidation before collision changes and one matching complete publication per dirty revision. Use `Navigator::navigateSource`; stale active routes enter WaitingForSurface before child motor ticks, spend no search queries while dirty, and replan retained goals after publication without resetting retry/deadline limits. Failed/stale bakes never reactivate old topology; terminal requests require explicit retry/new goals. No automatic collision subscriptions or worker-safe scene access is implied. See `docs/navigation-updates-validation.md`. Moving baked geometry requires explicit rebaking; runtime sweeps cannot prove support in stale topology. Optional CrowdNavigation/DynamicNavigation now add coordinated passing and tracked-obstacle footprint rebakes; see references/crowd-navigation.md. General character solver contacts, runtime checkpoints and general stair-walking acceptance remain outside this contract. Games own goals, actions and work budgets across agents. See [game AI](references/game-ai.md), [navmesh integration](references/navmesh.md), `docs/navmesh-validation.md`, native `ai`/`navmesh` and both-renderer `scripts/ai-smoke.cjs` / `scripts/navmesh-smoke.cjs`. Development APIs may be absent from a game pin.
- Reusable flat or hierarchical `StateMachine` in optional `engine_ai` executes shared immutable `StateMachineDefinition` assets with named application behavior/condition bindings, enter/update/exit, per-agent simulation timers and ordered transitions. Defaults: one transition and 1024 conditions per step; limits 1..64 / 1..65536. Cycles defer at transition limits; condition exhaustion/callback exceptions clean up, mark Failed and rethrow. `StateMachineHost` is synchronous and non-retained; exit is noexcept, reentry is forbidden, and lifecycle/control handoff must stop the instance and cancel persistent navigation. Guard component lookups during deferred removal. `engine_editor` now publicly links `engine_ai` and exposes independent `AiGraphEditor` immutable document/history (default 128 edits). Details -> AI state machines opens graphical node/edge authoring, entry/priority/delay/binding edits, drag cancellation, independent undo/redo and isolated C++ preview with controllable conditions. `.weai` version 5 files (reading v1-v4) retain definitions/layout, not callbacks or runtime state; load limit 512 KiB, 1..256 states, <=1024 transitions, finite positions within +/-10000, delays 0..86400, names/keys 1..256 bytes. Save graph files separately from scenes. Use in AI demo validates known application bindings before adoption; games supply their own hosts. No concurrent states, visual action scripting, graph autosave or automatic game callback discovery is delivered. See [state machines and graphical editor](references/state-machines.md), selected engine `docs/ai-state-machines-validation.md`, CTest `state_machine`/`ai`/`editor` and `scripts/editor-ai-smoke.cjs` on both backends including DPR2. A game pin may lack these development APIs.
- Optional `AiControllerComponent` attaches immutable graphs and typed `AiBindingRegistry` mappings to ordinary actors, with independent runtime/timers and a bounded 32-transition log. A controller must be a direct actor component; only one owns an actor. Generic `Component::fixedTickOrder()` defaults 0; stable ascending sibling fixed traversal runs AI at -100 before motors regardless of insertion order, preserving actor-first dispatch, hierarchy and render ticks. Registry keys bind C++ callbacks with Boolean/Number/Vector/saved-Actor parameters; built-ins are idle/move and always/flag/not-flag/near. Complete configuration/schema/reference validation precedes replacement. Disable/detach/control handoff cancels commands; re-enable needs explicit restart. Supply optional `aiControllerCodecs(registry)` to SceneArchive save/load/clone and EditorPlaySession::start; payload v2 (reading v1) embeds definition/configuration, not callbacks, runtime timers or commands. Core stays independent of AI. Browser scene files/Play register the codec. Details -> Actor AI controller assigns the open .weai graph, reports missing bindings, supplies typed forms and scene undo/redo; Live debugger during Play shows selected runtime state/time/history while authoring stays read-only. Registry callback captures must remain valid for every live component/codec. No automatic ground/navmesh, callback discovery, runtime checkpoints or custom-component subtree duplication is implied. See [actor AI controllers](references/ai-controllers.md), selected engine `docs/ai-controllers-validation.md`, CTest `ai_controller` and `scripts/editor-ai-controller-smoke.cjs` on WebGPU/WebGL2 including DPR2.
- Typed per-agent AI blackboards extend StateMachineData with up to 64 AiParameter fields and independent controller live values. setBlackboard(patch) atomically validates types/ranges/actor IDs and movement constraints without restarting state/timers; restart retains inputs, configure resets defaults. AiBinding::sources maps parameters to compatible fields with contained numeric/vector bounds; callbacks receive resolved copies. Owner-thread actor fixed ticks feed perception before AI (-100) and motors. Graph editor field forms/history and Play-only live edits/transition guard labels preserve source defaults. WEAI5 reads v1-v4 and AIC2 reads v1; live inputs stay transient. See [AI blackboards](references/ai-blackboards.md), selected engine docs/ai-blackboards-validation.md, native ai_controller/state_machine/ai and scripts/editor-ai-blackboard-smoke.cjs on WebGPU/WebGL2 at DPR1/2. Typed queued occurrences are described in [AI events](references/ai-events.md).
- Hierarchical AI states use optional AiState parent/initialChild IDs, root graph entry and depth <=16. Each instance owns a single active path and ancestor timers; updates enter outer-to-inner, exits unwind inner-to-outer. Parent interrupts win before child edges; vector order resolves same-source priority. External transitions retain only common strict ancestors of both declared endpoints; group self/ancestor-target transitions reenter their subtree. Stop/failure cancels the complete path; budgets remain shared. activePath() and stateSeconds(activeId) expose borrowed/current diagnostics; transitioned(edge,fromLeaf,toLeaf) records one accepted edge after entry. Graph editor parent/default-child forms, grouped move/delete history, collapse and breadcrumbs compose existing ownership; collapse/focus are presentation only. Play highlights active ancestors/leaf and exact edges. WEAI5 embeds hierarchy/events; old v1-v3 files remain readable through AIC2/custom codec v1. See [hierarchical AI](references/ai-hierarchy.md), selected engine docs/ai-hierarchy-validation.md and scripts/editor-ai-hierarchy-smoke.cjs on both backends at DPR1/2. No parallel regions or history/internal transitions.
- Typed AI event queues extend immutable graphs with up to 32 AiEventDefinition schemas (8 parameters each) and AiTransition::event triggers; empty keys remain polled guards. Each noncopyable machine owns a bounded FIFO (default 32, max64), default8 examined events/step (1..64) and max32 event records. Post on the scene owner; events posted in StateMachine enter/update/guard callbacks wait until the next tick cutoff. Matching event and polled edges share parent-first/source-vector priority; consumed occurrences trigger once, unmatched/early/false-guard occurrences discard, budgets defer unexamined input, and overflow rejects newest. eventScope(activeId) captures instance/activation identity; exits cancel queued scoped inputs and late/cross-agent/reentered posts reject. Stop/restart/failure clears queues; configure resets runtime/history and rejects callback reentry. currentEvent is borrowed only during matched guard/transition callbacks; copy payload data into declared blackboard fields deliberately. Controller posts validate saved actor IDs; later resolution remains handle-safe. Event definitions/trigger forms and preview/Play injection expose typed payloads, scopes, outcome/edge/sequence traces and preserve source bytes. WEAI5 reads v1-v4; AIC2/custom codec1 retain definitions only, never queues/tokens/traces. See [typed AI events](references/ai-events.md), selected engine docs/ai-events-validation.md and scripts/editor-ai-events-smoke.cjs on WebGPU/WebGL2 at DPR1/2. Games still own animation/perception subscriptions; controller task execution is described below.
- State-owned AI task sequences provide AiTaskSpec typed constants/blackboard sources, per-task timeouts, ordered execution, Running/Succeeded/Failed and activation-scoped result events. Controllers own <=16 tasks/state and tick one per active state/step; cancel noexcept releases work on completion/exit/lifecycle. Built-ins wait, wait-event, move-to via explicit Navigator source and face-target; applications register animation/game tasks in the optional third AiBindingRegistry::create vector. setNavigationSource stops the controller; reattach providers after load/Play and restart explicitly. Graph forms and Actor Play diagnostics expose sequence order/progress/failures; raw graph preview does not execute tasks. WEAI5 reads1..5; runtime work stays transient. See [AI tasks](references/ai-tasks.md), selected engine docs/ai-tasks-validation.md, CTest ai_tasks and scripts/editor-ai-tasks-smoke.cjs on both backends/DPR1/2.
- Give each camera one writer; evaluate camera controllers after simulation and before rendering. Presentation poses, GUI focus and view selection must not determine authoritative gameplay state.
- Share immutable mesh/material/texture/skeleton/clip assets; keep transforms, overrides and playback state per instance. Gate transitions on asset preparation and retain the active scene on failure. Verify current decode/upload capabilities before assuming loading is stall-free or GPU-prewarmed.
- Optional terrain combines immutable float/image heightfields, layered materials, seamless camera/shadow LOD, full-source brushes and bounded streaming. `TerrainPagedStream` separately bounds source pages/jobs/admissions; browser workers read/generate chunks. Opt-in `enableEditing(writer)` queues atomic cross-page sculpt/paint, loads/pins every required halo and preserves handles; paint shares geometry/collision. Dirty pages stay pinned until generation-checked atomic writeback succeeds. Failures/conflicts retain edits and stop automatic retries; flush retries, clear rejects unsaved state, explicit discard reloads saved data. Native hosts supply IO/executors and atomic writers; browser loaders/writer share an IndexedDB namespace. Scene outlives the owner, and destruction discards unsaved state: flush/await clean before releasing it when edits must survive. Format 15 snapshots contain current source edits, excluding dirty flags/generations/jobs/provider/history. Read `docs/terrain.md` and [feature integration](references/feature-integration.md) for budgets, lifecycle, synchronous edit-rebuild costs and worker artifacts. Run CTest `terrain`, `terrain_paging`, `terrain_page_edit` and all six terrain browser suites on both backends. Demo `?scene=terrain` includes paged brushes, save/retry/discard and persistent reload; 16-stamp undo remains full-source only. GL supports terrain color/camera LOD with basic lighting (directional shadows require the development extension below). Geomorphing, remote write adapters and automatic conflict merging remain extensions.
- Use optional `Scene::setSky(SkyBackground)` for gradient/panorama/atmosphere skies on both browser backends. `SkyGeometry::Sphere` draws a shared inward unlit mesh per camera; it never enters scene picking/shadows/physics. `SkyClouds` provides procedural coverage/density/wind phase. Opt into `SkyDayNight` and call `advanceSky(scene, chosenDelta, directionalLightId)` once after simulation to update sun/moon, ambient and existing IBL intensity. Choose clock and light ownership explicitly; resolve scene-scoped handles and restore alternative lighting policy deliberately. Set `RenderView::skyEnabled=false` for load-color views preserving earlier backgrounds. Format-6 archives retain sky/weather/cycle state, independently of GPU resources and caller handles. See `docs/sky.md`, native `sky_tests`, `scripts/sky-smoke.cjs` and `scripts/sky-weather-smoke.cjs`. The playground defaults to an atmosphere mesh sphere with clouds, 09:00 and a 180-second day; these are application defaults, while reusable scenes start with sky off. Clouds are 2D, and scattering is single scattering; no volumetric clouds, cloud shadows, multiple scattering or automatic IBL rebake is provided.
- Use the browser `Renderer` facade and reported `RenderCapabilities`. Automatic initialization prefers WebGPU and can fall back to WebGL 2 before canvas acquisition; the fallback has reduced lighting/features. Service recovery at host frame boundaries even while the renderer is not ready. Keep backend resource types private.
- Water uses `MaterialShader::Water`, `Material::water` and optional full `MaterialInstance::water` overrides on subdivided local-XZ meshes. Both backends support Gerstner waves, ripple normals, Fresnel/highlights, depth-aware refraction/foam, one-plane-per-view opaque planar reflections, and underwater fog/absorption with optional animated caustics. WebGPU can use IBL; WebGL 2 uses authored horizon/zenith colors. Sky is independent. Supply `Renderer::setWaterTime(seconds)` once per frame with explicit pause policy; views share it and the renderer never advances it. Bounds expand without changing CPU geometry, physics or picking. Refraction, foam, planar and underwater strengths default zero. Planar capture selects the first visible requesting plane, shares it with coplanar water, clips submerged objects and reverses winding; other planes retain fallback. `underwaterStrength` selects the first active masked World water containing the eye below its transformed local-Y=0 plane and within its local XZ bounds, even when the surface is offscreen. `underwaterFogDensity`, `underwaterColor`, `underwaterMaxDistance` and existing `absorption` control depth attenuation; use `doubleSided=true` to see the underside. Fog captures completed World color/depth after transparency, before Overlay/GUI, clips rays to the surface/sides, and retains per-view 1x/4x, resize/recovery and archive contracts. It has no bottom boundary or inferred arbitrary volume. Current v16 archives include v11 underwater settings and v13 caustics; older water defaults new effects off. Caustics strength defaults zero; scale/speed/depth fade control procedural receiver lighting inside submerged views, sharing the fog composite and caller clock. `Renderer::setWaterInteractions` copies up to 16 transient world-space ring/wake sources; applications own contact/motion emission. Surface normals/crests change without geometry, physics or added passes. Inputs share water time, filter by camera masks/age, survive GPU recovery and clear on scene replacement; they are not material/archive state. Extra reflection planes, transparent-depth fog and buoyancy remain extensions. Renderer time stays application state. Read `docs/water.md` and [feature integration](references/feature-integration.md); use CTest `water` and the six water smoke scripts in [build validation](references/build-validation.md). Demo: `?scene=water`, submerged/deep views, effect presets and colored distance markers.
- Optional MSAA uses `RenderQuality::msaaSamples=1|4`, `RenderCapabilities::msaa4x` and `Renderer::effectiveMsaaSamples()`. Profiles default to off; unsupported devices retain the requested policy and render at 1x. Scene layers/views resolve once before single-sample GUI; shadow filtering and canvas resolution remain separate. See `docs/rendering-quality.md#optional-msaa` and `scripts/msaa-smoke.cjs`.
- For larger content, use the optional isolated `browserDecodedAssetLoader`, per-frame `UploadBudget` and offline KTX2/fallback texture tooling where appropriate. These mechanisms are implemented; CPU asset readiness does not mean every draw is GPU-ready. Loading-screen and eviction policies remain application choices.
- Scene authoring uses optional `engine_editor`: `EditorSession` / `EditorPropertyRegistry` over an idle scene and stable-address asset library. **Scene editor** / `?editor=1` provides files, typed component/material forms, assets/placement/snapping, component add/remove, KeepWorld reparenting, ordered multi-selection, grouped move/rotate/scale/duplicate history and local autosave/recovery. `EditorPlaySession` owns a separate Application and built-in scene clone; Stop rebinds retained source views before releasing the preview, preserving authoring bytes, camera, selection and history. Immutable assets are shared; mutable simulation and clocks are isolated. Reconstruct game callbacks, input bindings, joints, GUI and audio explicitly when needed. Preserve physics topology, managed children and renderer-borrowed camera lifetimes. Gate authoring/files/autosave during Play. Read `docs/scene-editor.md` and [scene-authoring integration](references/feature-integration.md); run CTest `editor`, `editor_content`, `editor_structure` plus all three editor browser smoke scripts on both renderers, including DPR2. Advanced factories/inspectors, marquee selection and Play apply-back remain extensions. Saved built-in prefab definition authoring is described in [prefab assets](references/prefab-assets.md).
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
retains basic lighting; directional shadows require the development extension below. See [surface integration](references/feature-integration.md#terrain-surface-relief-development-engine-2026-10-17)
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

Surface-following contact strips are available in the development engine through
explicit SurfaceTrail mode, supplied world positions/normals, bounded sampling and
existing FX authoring/preview. Read [surface trails](references/surface-trails.md)
for defaults, receiver policy, persistence, budgets and both-renderer validation.
Verify the consuming engine pin; games own tyre/contact emission policy.

## Custom GUI widgets and settings controls (2026-10-19)

Development WebEngine adds optional Widget::onPaint(GuiPainter&), onPointer,
onKey, onActivate, describeAccessibility and onAccessibleValue hooks. Local
design-unit painting/input inherits GuiSystem scaling/clipping, focus, capture
and semantic identity; no DOM/GPU types cross the portable interface. Handled
presses allow scroll takeover; Capture on Down reserves a drag. Deliver Cancel
on host ownership loss; hiding/disabling/clipping and Escape cancel captures.
Checkbox defaults unchecked; horizontal Slider defaults range 0..1, step .01,
value .5. Both notify only on changed effective values, including programmatic
setters. Slider ranges/values reject invalid input before replacement, clamp and
snap relative to minimum, and retain reachable maximum. Native checkbox/range
mirrors share C++ actions and values. Settings preview is ?gui=1&guiSettings=1;
it updates local presentation state, not an audio bus. Read
[custom widget contracts](references/gui-widgets.md), selected-engine
docs/gui-widgets.md and docs/gui-widgets-validation.md. Run native gui/input and
input_adapter, then scripts/gui-widgets-smoke.cjs on actual WebGPU/WebGL2 at DPR1
and DPR2, retaining gui-smoke/ui-input/input regressions. Explicit sizing and
unique ownership remain; no child removal or measurement hook is delivered.
Preserve callback lifetimes and avoid reentrant traversal/tree mutation inside
paint/semantic hooks. Verify the selected game engine pin before using these
development APIs; refreshing this guidance does not advance a consumer gitlink.

## Event-driven FX composition (2026-10-19)

EffectAsset now owns bounded acyclic Birth/Impact/Death rules referencing child
assets and/or finite projected decal presets. EffectComponent consumes optional
emitter events in Component::postTick after existing child variable ticks; new
children start next update. Root defaults: 64 child effects, 128 decals, 64 spawns
per update, depth 4 and 256 events per emitter. Overflow drops without backlog.
Children share root spawn/active/depth limits, preserve world event anchors and
inherit optional world velocity once at birth. Source lifetime owns descendants;
pause/stop/restart propagate, and completed one-shots retire. Game code selects
surface responses, damage and audio. Renderer/culling never owns simulation.

Decal lifetime/fade default zero (persistent). Trigger presets default to five
seconds/one-second fade; effective opacity changes without per-frame projection
rebuilds. Archive v29 stores graph dependencies and live child/anchor/decal-age
state; transient events and diagnostics do not replay. Editor Event composition
supports child presets/imported .wefx, decal settings, root limits, immutable undo
and isolated full-chain preview; Impact chain is the example preset. Read
[composition contracts](references/fx-composition.md) and
[validation](references/fx-composition-validation.md) for delivered APIs, ranges,
limitations and isolated local-SDK build commands. Verify the consuming game pin.


## GUI themes and state styles (2026-10-19)

Development WebEngine adds GUI-owned GuiTheme values (dark/default and light),
component defaults, partial local normal/state patches, and shared effective
GuiPainter::style()/GuiSystem::resolvedStyle() values. Theme replacement validates
all layers, copies the candidate, and preserves IDs, values, layout and focus.
Normal layers are defaults/common/component/local; active states apply checked,
hovered, focused, pressed, disabled, each common/component/local. Colors and opacity
are finite 0..1. Existing widget.style.color={r,g,b,a} assignment remains supported;
colors now use inheritable GuiColorOverride fields with reset()/hasValue(), rather
than writable channel members. Read effective colors from the resolved style and
let painter primitives apply opacity once. Parent-local styles do not inherit;
disabled state still follows active ancestors. See [GUI themes](references/gui-themes.md),
selected-engine docs/gui-themes.md and docs/gui-themes-validation.md. Demo:
?gui=1&guiTheme=1. Run native gui/input/input_adapter and scripts/gui-theme-smoke.cjs
on actual WebGPU/WebGL2 at DPR1/DPR2, plus widget/GUI/input regressions. No font/layout
metrics, transition animation or theme persistence are added. Verify API availability
at the consuming game's selected engine pin; skill updates do not change that pin.

## Water depth capture boundary (2026-10-19)

Water refraction and depth foam fade to surface shading between 75% and 98%
of the reconstructed near-to-far view-distance range on both backends. This
prevents a visible colour band when an opaque receiver leaves the depth
capture. Clear/background and foreground samples remain rejected; no material
or archive API changes. See `docs/water.md` and
`docs/water-depth-transition-validation.md` in the selected engine revision.
Verify the consumer pin includes the fix; no application shader fork is needed.


## Measured GUI layout (2026-10-19)

Development engine_gui adds Widget::setSizeLimits/setMinSize/setMaxSize,
setContentSize(width,height), setAlignment/clearAlignment/setContentAlignment,
and GridLayout{columns,columnGap,rowGap}. Explicit sizes/anchors stay default.
Natural or explicit size resolves before stretch, then finite nonnegative size
limits clamp it. Width resolves before wrapped text height. Containers measure
visible children/padding/gaps; dependent stretch is ignored during content
measurement to avoid cycles. Grid uses 1..64 equal columns, visible row-major
placement and maximum measured row heights. Alignment packs groups/cells without
flex weights. Layout stages measurements/bounds/lines/scroll extents and rejects
overflow or depth >128 / >65536 measurement requests before publishing. Custom
leaves use preferred sizes; no custom measure hook or child removal is added.
Read [GUI layout](references/gui-layout.md), selected-engine docs/gui-layout.md
and docs/gui-layout-validation.md. Demo: ?gui=1&guiLayout=1. Run native
gui/input/input_adapter, scripts/gui-layout-smoke.cjs on WebGPU/WebGL2 at DPR1/DPR2,
and affected theme/widget/GUI/UI-input regressions. Native runtime DLLs must match
the configured compiler. Keep adaptive column policy/content/game rules in apps;
verify APIs against the selected consumer engine pin before use.

## WebGL 2 directional shadows (2026-10-19)

The isolated development engine in CatGame `build/engine-water-depth-transition`
now supports one directional shadow map per view on WebGL 2. This is not yet in
the game's pinned dependency. Existing LightComponent shadow settings and
Renderer casting/receiving/culling/LOD controls apply; maxShadowMaps is capped
at one (zero disables), with device-bounded resolution and 1/9 PCF taps. The
DEPTH_COMPONENT24 target is consumed per view and recreated after context loss.
Opaque/masked World casters retain off-camera selection, CPU skin/morph poses,
mirrored winding and layered alpha cutouts. Ambient/emission remain unshadowed.
The existing sun-shadow demo controls exercise it; Paws beach acceptance uses
`scripts/beach-shadows-smoke.cjs` on both backends, including visible pixel
comparisons, MSAA, resize, scene return and WebGL context recovery. Basic lighting,
no PBR/IBL, individual draws and no GPU timing remain WebGL limits. See the
selected engine's `docs/rendering-fallback.md`. Verify checkout/pin availability.

Spot lights with optional shadows share eight masked light slots on both development
renderers, with typed editor cone/range/direction controls, selected outlines,
normal hierarchy mounting and v30 persistence. Read [spotlights](references/spotlights.md)
for defaults, bounds, ownership, capabilities and validation; verify the game pin.

## Formula circuit application example (2026-10-19)

The development WebEngine playground has ?scene=formula: a supplied-model driving
example composed from existing VehicleComponent, closed SplineAsset/RouteAsset,
TerrainAsset and CameraFollowRig. Read selected-engine docs/formula-demo.md,
src/formula_demo.hpp and src/formula_laps.hpp before adapting it. Race gates,
lap validity, timing, HUD and circuit placement are application policy, not core
APIs or archive state. WASD/arrows drive/steer, Space brakes, Shift handbrakes,
R resets; held touch controls use existing BrowserInput sources. The optimized
assets/formula1-red-driving.glb is derived offline with tools/prepare_formula_car.py
from the untouched formula1_red.glb (all 18,977 triangles retained; 2048 px textures). Its merged visual has baked wheels; physical
wheels still steer independently. Native formula_demo_tests and browser
scripts/formula-smoke.cjs verify scoped handling/race rules and both renderers.
No physical mobile, real-controller or real Formula 1 handling acceptance follows.
Verify these example files exist at the consumer pin; they do not update a game pin.

## General region residency

For static content beyond terrain pages, use optional RegionManifest/RegionStream
and existing AssetManager/Scene ownership. Separate visual and collision sources;
applications own predictive movement/teleport gates and explicit restore handoff.
Managed residents are transient and editor read-only; authored manifests/source
references and selected world state use v31 archives. Read [integration and validation](references/region-streaming.md)
for supported payloads, pinning, budgets, editor preview and lifetime limitations.
Verify availability in the selected engine checkout before using these APIs.

Formula circuit mouse steering is inverted and on by default in ?scene=formula. Click the
canvas, then move horizontally relative to its centre; a 4% dead zone straightens
and full steering is reached at 35% of canvas width. The HUD toggle disables it.
web/formula.js owns one allocated BrowserInput Move source; throttle/brake keep
independent sources. Held A/D/arrows override mouse steering until fresh movement.
Focus/context/lifecycle gates, pointer exit, reset/pause and resize neutralize it;
touch/pen events cannot write this source. No core API or native build changes
are required. Validate scripts/formula-mouse-smoke.cjs on both renderer backends.

## Animation library and visual state machine (2026-10-19)

The development engine now provides immutable animation libraries with stable clip
IDs, folder/tag/display metadata, validated same-rig clip imports and `.weanim`
persistence. Details -> Animation library & state machine authors typed gameplay
parameters, clip/blend states and conditional ordered transitions over the existing
player. Preview runs an isolated Play clone; Apply is one guarded undoable scene edit.
Use Boolean for persistent conditions and Trigger for one-shot requests. Save library
organization/layout separately from scene runtime state. Read
[animation authoring](references/animation-authoring.md) for API ownership, limits,
file semantics, lifetime guards and native/both-backend acceptance. Verify availability
in the selected game pin; skill updates do not change that dependency.


Formula easy handling is application-owned in src/formula_handling.hpp. The
FormulaDrivingAssist chassis component shapes desired commands on fixed ticks
before vehicle physics; no second solver, render clock or core API is added.
FormulaHandlingSettings defaults: 450 N m torque, 2.1 tyre friction, .30 rad front
steering, .12 s steering half-life, half steering at 18 m/s with .25 minimum,
33.3333 m/s forward and 8 m/s reverse throttle limits with 5 m/s taper, and .25
front torque share. Neutral/cancel, pause/reset and component disable/detach clear
filter/commands immediately; body/solver control stays with VehicleComponent.
Other vehicle defaults are unchanged. Limits fade engine input, not velocities
or coasting/impulses. Desired/filter state is transient with no Play/archive codec.
Browser formulaInputState reports requested input; vehicleState reports the
shaped command. Preserve the user-selected inverted mouse mapping. Verify
native formula_demo and formula_handling, and both Formula browser smoke scripts
on both renderers. Read selected-engine docs/formula-demo.md for the parameter
table and docs/formula-demo-validation.md for recorded evidence/limitations.

## Shadowed lighting and HDR post-processing (2026-10-19)

The development engine now supports directional/spot shadows on both browser
backends, optional2..4 camera-fitted sun cascades, and opt-in RGBA16F bloom with
exposure/saturation/contrast. Defaults keep post off and authored fixed sun volumes.
Read [lighting integration](references/lighting-post-processing.md) for budgets,
capability fallback, frame ordering, persistence limits and native/browser checks.
This supersedes older no-spot-shadow/no-WebGL-shadow notes for this checkout only.
Verify consumer pins; preserve Paws-specific dependency guidance.
The built-in ?scene=lighting showcase compares shadowed headlights, a torch and
cascaded sun with HDR comparison controls; see the lighting integration reference
for settings and scripts/lighting-showcase-smoke.cjs validation at DPR1/2.

## Animation composition (2026-10-20)

Use [animation layers and blend trees](references/animation-layers.md) for
speed/direction locomotion and masked carry/look/interaction poses. Flat 1D/2D
sample trees optionally synchronize phase; ordered override/additive layers have
independent clocks and numeric weights. The existing player owns all timing;
only base locomotion emits markers/root motion. Editor authoring, preview,
scene format33 and ANIM3 metadata retain composition and IK. Verify the selected engine
pin before calling these development APIs; retargeting remains separate.


## Inverse kinematics (2026-10-20)

Use [inverse kinematics](references/inverse-kinematics.md) for model-space hand/foot
placement and bounded head aim after animation layers. Controllers own ordered
immutable constraints; players own targets, buffers and diagnostics. Two-bone IK
uses a pole and optional tip orientation; look-at has a local forward axis and
swing limit. Explicit physics ground probes return values and never move actors.
Scene v33 retains constraints and live targets; ANIM3 libraries retain defaults.
Older formats load with IK off. Verify the selected engine pin first; this skill update does not advance
CatGame's dependency. Test native/Wasm plus the real demo/editor on both backends.

## Animation retargeting (2026-10-20)

Development WebEngine adds explicit clip baking between compatible characters
with different proportions/bind orientations. `animation_retarget.hpp` supplies
named one-to-one profiles, exact-name suggestions, median translation-scale
estimation and `retargetAnimations`; `AnimationLibrary::appendRetargeted` preserves
destination skin bindings and appends a complete immutable result. Strict
same-rig append/apply remains available. Editor **Retarget clips** accepts FBX/GLB/
`.weanim`, repairs mappings, saves `.weretarget` profiles and bakes one undoable
library edit before ordinary preview/apply. No runtime clock or archive version
changes. See [retarget integration](references/animation-retargeting.md) for
defaults, constraints, ownership and validation. Verify the selected game pin
before using these development APIs; documentation updates do not advance it.

## Prefab asset authoring (2026-10-20)

Development WebEngine adds saved PrefabAsset definitions, `.weprefab` files,
variants and explicit grouped instance propagation preserving supported overrides.
Scene v34 retains applied assets/bases and inert identity links. The independent
workbench reuses EditorSession history; strict built-in cloning, idle ownership,
conflict rejection and camera view lifetimes remain authoritative. Read
[prefab asset integration](references/prefab-assets.md) for APIs, defaults, limits,
persistence and both-renderer validation. Verify the consuming engine pin first.

## Detailed CPU profiling timeline (2026-10-20)

Optional FrameTimeline/ProfileSession/ProfileFrame/ProfileScope capture records
nested physics, animation, AI, loading and rendering work in bounded storage.
Capture is off by default; CPU wall time/self time, callback intervals, off-frame
tasks and GPU metrics have distinct boundaries. The browser provides selection,
filters/zoom, slow-record inspection and Chrome/Perfetto trace export. Read
[timeline integration](references/profiling-timeline.md) for lifetime, budgets,
instrumentation and validation. Documentation does not advance game pins.

## Adaptive graphics quality (2026-10-21)

Optional engine_core AdaptiveQuality gradually adjusts resolution, shadows and
available static LOD toward a chosen frame-time target. The selected settings
remain a ceiling; warmup, smoothing, sustained thresholds and slower recovery
bound changes. RenderQuality::detailScale composes existing camera/shadow LOD
without changing simulation. Browser controls opt in and expose diagnostics.
Read [adaptive integration](references/adaptive-quality.md) for APIs, defaults,
limits, owner-thread sampling, manual override, recovery and validation. Verify
the selected consumer pin; documentation does not advance it.

## Reusable Windows desktop host (2026-10-21)

Development WebEngine provides optional `engine_desktop` / `WebEngine::Desktop`
and public `engine/desktop.hpp`. DesktopHost owns Application/GUI/window/input/
rendering and keeps SDL/Dawn types private. The vehicle example uses this API
with an interactive modal pause/settings menu. Read [Windows build and scope](references/windows-native.md)
and [public API contracts](references/windows-desktop.md) for ownership, handles,
input cancellation, callback lifetime, commands and validation. Browser engine
systems/rendering remain shared. Verify the chosen engine checkout; this update
does not advance CatGame's pin or provide a Paws Windows host.

### Interactive Windows GUI (2026-10-21)

DesktopHost now connects the existing TextInput to native SDL UTF-8 text events,
selection/editing, Ctrl+A/C/X/V clipboard and IME preedit/commit. Text focus starts
and stops OS services automatically, cancels held sources and gates gameplay axes;
application shortcuts must honor DesktopEvent::guiConsumed. Composition tracks
widget identity/revision, restores committed state on cancellation and preserves
later application edits. Observe composing() before publishing committed values.
Limits: single-line/codepoint editing, ASCII-whitespace word navigation, 32 KiB
native edits/payloads; no undo, grapheme shaping/bidi or multiline editing.
Native Windows accessibility is now provided by the UI Automation bridge below. The vehicle example adds a persisted driver-name field; Enter
focuses Resume. D-pad/A/B/Start navigate the menu while driving waits for neutral.
Keep native SDL services private; browser text/DOM adapters remain independent.
Read [desktop contracts](references/windows-desktop.md) and
[GUI acceptance](references/windows-desktop-gui-validation.md). Run desktop_text,
desktop_api, application/input/gui, native vehicle/GLB smokes and both browser
GUI/vehicle backends as relevant. Installed IME UI and physical controllers need
separate acceptance. Verify consumer pins; this does not add a Paws Windows app.

### Native Windows platform services (2026-10-21)

Optional engine_platform_windows / WebEngine::Platform exposes
engine/desktop_services.hpp and engine/audio_desktop.hpp; WebEngine::Desktop
also supplies it. DesktopFiles owns configured asset/user paths, reads bounded
bytes and provides synchronous AssetManager loaders with copied configuration/
decoder lifetime. Defaults are executable/assets and SDL per-user organization/
application saves/settings. Keep path traversal/device-alias guards, byte limits,
same-directory temporary/flush/atomic replacement and old-save preservation.
Use ApplicationArchive/SceneArchive and validated staged publication; game save
contents, checkpoints, migrations and autosave policy remain application code.

Application-owned AudioSystem accepts DesktopAudioBackend with lazy default SDL
output, private shared PCM/device-clock voices, loops/rate/gain, stereo pan and
existing buses/fades/emitter/listener contracts. decodeDesktopWav supports bounded
PCM/float WAV; local encoded streams and native filter/room DSP are now supported.
HRTF and arbitrary compressed in-memory WAV decoding remain extensions. Device callbacks never
borrow scenes, widgets or user callbacks. Games own update/suspension policy and
restore audio assignments; no audio archive codec is implied.

DesktopHost::showFileDialog uses owner-thread synchronous Unicode Win32 Open/Save,
validates before input/clocks, cancels capture/held commands, suspends and resets
timing, preserves pause/suspension, and returns Selected/Cancelled/Failed. Call at
idle or GUI events, never inside Application frame callbacks. No asynchronous
dialog borrows escape. Vehicle menus load/play WAV, open models, save/restore
checkpoints and persist committed name/render/volume settings across launches.
IME preedit stays transient. The checkpoint restores pose at rest. --assets and
--user-dir select explicit example roots. Read [platform contracts](references/windows-platform-services.md)
and [acceptance](references/windows-platform-validation.md). Physical output,
controllers/IME and shipping remain separate acceptance. Native scene editing
now has a separate workbench; see the current Windows workbench reference. Verify
consumer pins; documentation does not add a Paws Windows app or advance its pin.


## Crowd avoidance and dynamic obstacles (2026-10-21)

Optional engine_ai CrowdNavigation coordinates existing navigators/motors from
one authoritative snapshot before all motor fixed ticks. DynamicNavigation tracks
solid body handles, carves conservative footprints and publishes bounded complete
navmesh rebakes through existing source revisions. Keep root-motor ownership,
explicit coordinator order, idle preparation, failure/dirty stops and finite
deadlines. No implicit scene phase or character solver contacts are added.
Read [crowd integration](references/crowd-navigation.md) for exact APIs, defaults,
work limits, cleanup, waypoint/support limits, transient state and both-renderer
validation. Verify the selected consumer pin before using development APIs.


## Environmental audio and streamed music (2026-10-21)

Development AudioSystem now supports bounded physics-ray wall muffling, explicit
shared room reverb and URI-only long music streams with common voice/bus controls.
Use [environmental audio integration](references/environmental-audio.md) for defaults,
optional backend methods, activation/suspension, media cancellation and blob lifetime,
frame ownership, persistence limits and native/Wasm/both-renderer checks. The
?scene=audio lab has wall/room comparisons and streamed local files. Consumer pins
must be verified; Windows audio now has a separate output/streaming/DSP adapter.
Automatic room volumes remain extensions.


## Cinematic sequence authoring (2026-10-21)

Use [cinematic integration](references/cinematic-sequencer.md) for camera shots,
object motion, animation, sound and FX on the scrubbable ?scene=cinematic timeline.
Keep immutable assets/explicit clocks in core, independent history and isolated
preview ownership in engine_editor, and gameplay bindings/handoff in applications.
Save .wecin references separately from source scenes; verify selected consumer pins.

### Broader Windows acceptance (2026-10-21)

Development DesktopHost now handles WM_POWERBROADCAST suspend/resume at idle
boundaries: cancel input, reset timing, retain explicit pause and focus/minimize
gates. Its private window hook is removed before window destruction. Windows
content scale converts logical GUI/pointer coordinates and native IME rectangles;
DesktopSize.pixelRatio includes display scale, while SDL window sizing uses window
coordinates. RenderQuality still bounds actual backing resolution.

Use [broader Windows acceptance](references/windows-acceptance.md) and the updated
[desktop contracts](references/windows-desktop.md). Build with
scripts/build-windows.ps1, then run scripts/windows-acceptance.ps1 -BuildDir
build/windows. The opt-in windows_acceptance target composes a portable gallery
from shared animation/terrain/water/particle/lighting APIs and captures real Dawn
D3D12 swapchains. Feature controls, bloom, underwater, shadows and MSAA must affect
pixels. Selected adapter, display/controller inventory, hashes, counters and
recovery comparisons appear in evidence. Use a new evidence directory per run.

DesktopSettings.allowGraphicsFaultInjection defaults false; opted-in diagnostic
tools may call simulateGraphicsLoss() at idle (disabled/reentrant throws;
not-ready returns false). This destroys a Dawn device, exercises its lost callback
and rebuilds resources; it does not reset a driver. Browser host behavior remains
independent. -ControllerSeconds 90 requests an operator physical-only check;
-Interactive leaves the animated gallery open after acceptance (Escape exits,
F11 toggles fullscreen). Fullscreen/resize/minimize and injected power-message
routing are distinct from physical sleep/wake, TDR and mixed-DPI acceptance.
Unavailable hardware is explicitly not-run; do not substitute virtual pads or
numeric DPI tests for physical evidence. Verify the selected consumer pin; these
development docs do not advance CatGame's pin or add a Paws Windows application.



## Recorded replay and ghosts (2026-10-21)

Use [replay integration](references/replay-recording.md) for bounded pose/event
recordings, scrubbable review and non-colliding ghosts on browser and native Windows.
Keep capture after authoritative simulation, playback on an explicit eligible clock,
and race/scoring/context compatibility in application code. Files are diagnostic
presentation data; recorded commands do not rerun the solver. Verify consumer pins.

### Windows distribution and diagnostics (2026-10-21)

Development WebEngine::WindowsRuntime / engine_windows_runtime supplies
engine/desktop_runtime.hpp without SDL/Windows/GPU headers or a core dependency.
DesktopSessionLog is optional, one process-owned session declared before host/
devices/workers and destroyed after them. It owns unhandled-exception/terminate
slots, optional stdout/stderr capture, UTC session logs and best-effort system
DbgHelp MiniDumpNormal files; no application callbacks run during crash handling.
Defaults: eight session logs, three dumps; explicit log writes cap messages at
16 KiB and stop above 4 MiB (redirected stdio is not hard-capped). Restore hooks/
streams at teardown; other libraries must not replace the owned process slots.
SEH/terminate dumps are tested; fail-fast, forced kills, stack exhaustion, disk
failure and driver hangs are not guaranteed. Nothing is uploaded automatically.

desktopExecutablePath() is CWD-independent. inspectDesktopPackage(directory)
returns version/verifiedFiles/errors/ready() from a bounded UTF-8 SHA-256 manifest;
limits are 1 MiB, 4096 members and 1 GiB/member. Paths/hashes/sizes validate before
launch; integrity is not publisher authentication. showDesktopError includes a
native support-log path. Keep this optional Windows concern out of portable core.

Read [distribution](references/windows-distribution.md) and
[validation](references/windows-distribution-validation.md). The /MT launcher
WebEngineVehicle.exe runs before SDL/VC loading, verifies the full package and
reports useful missing-DLL/version/architecture errors. The shared vehicle runtime
adds logs and a bounded --distribution-test report.json (requires isolated
--user-dir). Build scripts/build-windows.ps1, then scripts/package-windows.ps1.
Offline packaging audits x64 normal/delay PE imports, bundles required release
VC CRT from Visual Studio Redist, includes SDL/DXC/assets/notices and creates
deterministic player and separate PDB ZIPs. Fixed-input packaging is byte-repeatable,
not a reproducible compiler/toolchain claim. Artifacts publish only when complete;
use new output directories and preserve matching symbols/binary hashes.

Install.cmd uses Windows PowerShell 5.1/.NET for per-user staging, receipt ownership,
upgrade/rollback, HKCU Apps and Start menu integration. Unknown destination files
are protected; uninstallation retains user data/logs. Test with
scripts/test-windows-package.ps1 -Zip ... [-Integration] and fresh evidence;
Integration refuses existing registrations/shortcuts. The clean Windows Sandbox
fixture comes from scripts/windows-sandbox-package.ps1; networking/clipboard and
input device redirection are disabled, only payload/scripts and result folders
are mapped. Local system-only PATH/fresh-profile tests and generated .wsb files
do not prove a clean VM passed. Publisher signing/SmartScreen, enterprise policy,
clean-guest and hardware/driver coverage remain release gates. No MSI/MSIX,
auto-updater or online publisher is added. Verify consumer pins: documentation
does not advance CatGame's WebEngine pin or add a Paws Windows distribution.

### Windows background assets (2026-10-21)

Use [background assets](references/windows-background-assets.md) and its
[validation](references/windows-background-assets-validation.md) for optional
DesktopAssetExecutor (engine/asset_desktop.hpp, WebEngine::Platform). The owner
explicitly constructs the pool; no global pool or renderer/window is required.
Defaults: two workers (1..8), 64 pending jobs (1..256), 32 MiB source, 256 MiB
decoded transport/allocation, 30 s admission timeout. Windows byte caps are 512 MiB;
queue time is excluded. Overflow fails the request. Source progress can be 100%
while decoding. diagnostics() reports actual active/queued jobs.

decodedFileLoader/decodedAssetLoader reuse FBX/GLB/image/texture-bundle importers,
archive transport and hydrateDecodedAsset; native hydration also runs on a worker.
fileLoader<T>/assetLoader<T> accept thread-safe owning custom decoders, including
WAV. The older DesktopFiles::assetLoader stays synchronous. Shared AssetManager,
AssetRequest/AssetProgress, preparation, caching and consumer cancellation remain
authoritative. AssetSourceOptions and AssetDecodeOptions are portable policies;
browserDecodedAssetLoader(url,kind,policy,workerUrl) accepts the shared decode
policy while preserving the original BrowserDecodeOptions aggregate API.

Only manager.update/Application::advance publishes resources on the owner. Keep
scene/GUI/GPU changes and upload/reveal policy there. Workers copy configuration,
borrow source spans only during decode and never call owner-only APIs. Do not
capture/destroy the pool from decoders. A watchdog and final deadline gate reject
overdue results. Cancellation removes queued work or suppresses active publication;
native synchronous importers/blocked IO cannot be preempted, retain their slots
until return, and may delay joined teardown. Browser workers can be terminated.
Loaders hold weak executor references; expired services fail safely. Destruction
fails pending work and joins all threads; custom captures must outlive execution.

The vehicle demo imports previews asynchronously with progress/cancel and retains
the previous actor until successful owner-side adoption. Its explicit GLB decoder
retains confined companion-image loading on the worker. Bundled WAV loads in the
pool; Play sound stays disabled until ready. No native HTTP/terrain executor,
implicit companion discovery in generic decoded loaders, GPU prewarming, global
memory/LRU policy or hard importer execution deadline is delivered.

Build scripts/build-windows.ps1; focused CTest desktop_assets/assets/application/
mobile_assets and platform/demo persistence. Run scripts/windows-smoke.ps1 with
the skeletal GLB, scripts/assets-smoke.cjs against the matched asset-tests build,
and scripts/mobile-assets-render-smoke.cjs serially on WebGPU/WebGL2. Use the
repository-local SDK pinned by .emscripten-version and new evidence directories.
This documentation does not advance CatGame's pin or add a Paws Windows app.



## Windows audio and native scene workbench (2026-10-21)

Development DesktopAudioBackend now implements local encoded file streaming,
per-voice low-pass filtering and prepared room reverb through existing AudioSystem
contracts. Use desktopAudioStreamUri for local Unicode/space paths; Media Foundation
workers own bounded rings and generation-checked seeking. Browser HTTP/blob policy
stays independent. Read [Windows audio](references/windows-audio-environment.md).

Optional WebEngine::DesktopEditor / engine_editor_desktop exposes editor_desktop.hpp
and webengine_editor.exe: native Outline/Details, ordinary transforms/properties,
quick creation, GLB placement, history, native scene files and isolated Play. The
host's Application remains the sole scheduler. Workbench borrows must end before
scene shutdown; use editor.step/run and destroy it before its otherwise-unused host.
See [native workbench](references/windows-editor.md) for exact UI scope, ownership,
file preservation and limits; shared authoring forms are described below. Verify consumer pins;
these documentation changes do not advance CatGame or add a Paws Windows app.

## Selectable Windows graphics backends (2026-10-22)

DesktopSettings::backend defaults to Automatic (Dawn D3D12 → Vulkan → D3D11).
Explicit choices stay strict; backend() reports the current candidate. Milestones 1-8 deliver shared vehicle/native GUI,
resize/borderless fullscreen/captures, static conventional PBR/IBL, five material
maps with semantic mips/wrapping, first eight directional/point/spot lights, four
directional/cascade/spot shadow maps, cutouts, sorted transparency, winding and
checked 4x MSAA. Reuse Material/Texture/Environment, RenderFrame/FxFrame/planShadows.

Milestone 3 reuses Scene::setSky/setFog, SkyBackground/FogSettings, prepareSky/
prepareFog and skySphereMesh. Gradient/panorama/atmosphere/clouds/day-night payloads
render as fullscreen or inward sphere. Only applications call advanceSky; the
renderer never advances clocks. Fog affects static World meshes and sky, respects
per-view flags and additive extinction; Background/Overlay and GUI stay outside.
RenderQuality::post is off by default. Supported FP16 render/blend/sample/load
enables HDR scene rendering, optional quarter-size bloom, exposure/Reinhard/
saturation/contrast/sRGB composition, then GUI. Bloom may cross view boundaries.
Zero bloom skips extraction. This is SDR presentation, not HDR10 monitor output.
hdrPostProcessing reflects queried support; HDR 4x/resolve is checked separately.
Requested quality stays intact; diagnostics effectivePost/effectiveMsaaSamples
report the result. Attachment changes fence old resources and preserve captures.

Milestone 4 consumes existing SkeletalMeshComponent/AnimationComponent,
SkinPose/BoneUniform, SkinVertex and MorphPose without another scheduler or public
renderer API. Four-influence GPU skinning reads up to 1024 structured palette
entries (including identity slot zero); inverse-transpose normals and both tangent
axes retain handedness. Cached CPU morph vertices precede skinning in color and
shadow passes, including alpha modes, normal maps, mirrored placement, fog/HDR/MSAA.
Camera/shadow bounds match deformation; static LODs do not simplify these draws.
Independent players/weights remain per component; clips/layers/trees/IK/retargeting
continue to produce ordinary shared palettes. Applications still apply root motion.
Changed pose revisions copy once to reusable GPU buffers; unchanged poses upload
nothing. Pose caches weakly reference CPU sources and retire buffers behind fences.
Changing view lists clears collected references/LOD history; unbinding releases
collected CPU resources. Each pose cache caps at 4096; staging shares the 64 MiB
frame upload arena with object/view/GUI data. Invalid sizes/indices/nonfinite data
reject before recording, including when culling is disabled. No GPU clip sampling,
dual-quaternion skinning, cloth or new import format is added.

Milestone 5 uses existing MaterialLayers, materialTextures/TextureSemantics,
materialLayerParameters and TerrainComponent meshes without new portable APIs.
Four sRGB albedos, linear RGBA weights and four packed RGB-normal/alpha-height
maps support semantic mips, individual wrapping, global/per-layer UVs, weighted
color/alpha/roughness, active-layer height blending and signed world triplanar
projection. Missing/zero weights select layer zero; missing color is white and
missing relief is flat/zero. Conventional maps are inactive while layered;
scalar metallic/emissive, tint, alpha modes, PBR/IBL, shadows, fog/HDR/MSAA remain.
Static and skin/morph draws share matching layered cutout shadows. Nine-descriptor
tables include layered semantic roles in cache keys; object constants use 448
bytes in 512-byte aligned slots within the existing 64 MiB frame upload bound.
Shared camera/shadow LOD stays independent, including automatic projected-size
selection. Rendering never changes terrain ground/collision, paging or simulation.
Direct counters add layeredDrawCalls, colorTriangles, shadowTriangles and camera/
shadow LOD parts; these are collector/submission counters, not GPU timings.

Direct mesh instancing and semantic BC7 uploads are now supported. Direct device loss queues bounded reacquisition. See [Automatic Dawn API
fallback](references/windows-graphics-fallback.md). Full renderer parity and speed comparisons are not established.
Keep GPU types private and simulation independent. Verify consumer pins; this
does not advance CatGame's pin or create a Paws Windows host.

Build scripts/build-windows.ps1 -Backend Both|Dawn|D3D12 (Both default); direct-only
excludes Dawn lookup/linkage. Launch webengine_windows or webengine_lighting_windows
with --backend dawn|d3d12. The latter adds material/lighting/environment/post GUI
controls and --capture image.ppm (parent must exist); F11 fullscreen, Escape exits.
Run windows-animation-smoke.ps1, windows-environment-smoke.ps1, windows-lighting-smoke.ps1 and
windows-backend-smoke.ps1 plus native desktop_api/desktop_api_d3d12/render_quality/
adaptive_quality. WEBENGINE_D3D12_VALIDATION=1 requires Windows Graphics Tools.
Also launch webengine_animation_windows --backend dawn|d3d12 for playback/blend/
pose/morph controls and deterministic --capture image.ppm; F11 fullscreen, Esc exits.
Run browser morph-smoke with BACKEND=webgpu|webgl2, skeletal-animation-smoke
(WebGPU palette uploads) and animation-layers-smoke (both). WebGL2 CPU deformation
still uploads vertices per draw. Rerun sky-smoke/sky-weather-smoke/fog-smoke/lighting-post-smoke on forced
WebGPU/WebGL2 against the pinned local Emscripten build. WebGL2 retains basic
lighting, not PBR/IBL. Use fresh pixel/counter evidence and separate native/browser
results; neither establishes physical mobile or unsupported-adapter acceptance.

Read [backend contracts](references/windows-d3d12.md), [sky](references/sky.md),
[fog](references/fog.md) and [milestone 3 evidence](references/windows-d3d12-milestone3-validation.md).
Earlier milestone evidence remains historical.

Read [animated rendering](references/windows-animation-rendering.md) and
[milestone 4 evidence](references/windows-d3d12-milestone4-validation.md).


Launch webengine_terrain_windows --backend dawn|d3d12 for retained layer/relief/
projection and independent camera/shadow LOD controls. --capture image.ppm makes a
paused GUI capture; F11 toggles fullscreen, Esc exits. Run windows-terrain-smoke.ps1
for paired 46-case native pixels and counters, plus terrain/material/render_frame
and desktop_api tests. Run browser terrain-surface-smoke.cjs on fresh explicit
WebGPU/WebGL2 artifacts for layered surfaces, LOD, collision invariance, archive
round trips and recovery. WebGL2 supports shadows with basic lighting, no PBR/IBL.
Read [terrain](references/terrain.md) and [milestone 5 evidence](references/windows-d3d12-milestone5-validation.md).

Milestone 6 reuses MaterialShader::Water/WaterSettings, waterParameters,
waterReflection, underwaterView, WaterInteractionParameters and prepareUnderwater.
Direct water supports Gerstner displacement after morph/skin, moving normals,
Fresnel/IBL, depth refraction/foam, mirrored opaque/cutout World captures, reflected
height fog, underwater absorption/scattering/caustics and transient rings/wakes.
Mirrored double-sided PSOs preserve front-face normals independently of culling.
Viewport-clamped/scissored snapshots isolate cameras. At 4x, color averages and
depth takes the nearest sample; 1x loads directly. Underwater runs after completed
World and before Overlay/GUI. HDR inputs stay linear; non-HDR 1x accumulation copies
to the swapchain before GUI. Fence-safe targets shrink to 1x1 when effects are off.
Object constants are now 560 bytes in 768-byte slots aligned to 256 bytes; frame
payloads are 2352 bytes, sharing the existing bounded 64 MiB upload arena.

DesktopHost::setWaterInteractions(span) validates/copies up to 16 existing portable
WaterInteraction values after scene activation or from frame callbacks. Native
water uses Application simulation seconds, independent of GUI presentation; use
that same clock for startTime. Pause/suspension freeze waves, caustics and contact
ages. View edits/recovery retain snapshots; successful scene handoff clears them.
Invalid snapshots preserve previous values. Inputs borrow no nodes/callbacks and
are not archived. Applications own contact detection/emission and reconstitution.
Browser Renderer clocks remain caller-owned. Water surface defaults remain .12 m
amplitude, 3 m wavelength, speed 1; refraction/foam/planar/underwater/caustics are off
by default. Geometry must be sufficiently subdivided; height/displacement never
changes simulation or collision. Rectangular underwater and one reflection plane
per view remain approximations; transparent reflection sources are excluded.

Launch webengine_water_windows --backend dawn|d3d12 for retained water controls,
moving wake/contact emission, pause, HDR/MSAA and --capture image.ppm. F11 toggles
fullscreen; Esc exits; capture parent must exist. The example enables water scene
effects and HDR only as application policy. Run windows-water-smoke.ps1 for paired
63-case native pixels/counters, plus water/material/render_frame/desktop_api tests.
Run water-smoke, water-depth-smoke, water-planar-smoke, water-underwater-smoke,
water-caustics-smoke and water-interactions-smoke against fresh pinned local SDK
artifacts on explicit WebGPU/WebGL2. Report native/browser separately. No physical
mobile, other-GPU, sustained-performance or direct device-recovery claim follows.
Milestone 8 delivers mesh instancing and semantic BC7 uploads.
No fluid simulation, buoyancy, automatic emission or new archive version is added.

Read [water](references/water.md) and [milestone 6 evidence](references/windows-d3d12-milestone6-validation.md).

## Accessibility and native pointer contacts (2026-10-22)

Development DesktopHost automatically exposes retained GUI through Windows UI
Automation on Dawn and direct D3D12. Browser semantic DOM remains available on
WebGPU/WebGL2. Supply names/descriptions and live-region flags; decorative hidden
subtrees leave navigation. GuiAccessibleNode now copies clippedBounds and
keyboardFocusable in addition to full bounds. Native patterns are Invoke, Toggle,
single-line Value and RangeValue; native TextPattern and ScrollPattern are absent.
UIA queries retain snapshots, never Widget/Scene borrows. At most 128 queued
actions execute at the next owner-thread poll with current-ID/state revalidation;
lifecycle cancellation invalidates pending batches. Native actions are gated while
unfocused/minimized/suspended. Removed/retired providers explicitly disconnect.
Windows may return an empty default for a disconnected value getter; stale edits
must reject. UIAutomationCore/COM remain private to engine_desktop.

Native SDL touch/pen events produce InputPacket records with stable per-contact
IDs, logical/backing coordinates, pressure and PointerEvent-compatible buttons.
Mouse ID 1 is reserved; at most 32 non-mouse contacts are tracked. Tip/barrel1/
barrel2/eraser bits are 1/2/4/32. GUI has one capture owner shared across devices;
other scene contacts remain independent. Synthetic SDL mouse/touch duplicates are
filtered. Blur, pause, context/scene handoff and shutdown cancel contacts; proximity
loss cancels pen input. Reentrant GUI cancellation is generation-guarded. Games
choose native gestures/action bindings; no automatic native pinch/orbit/driving
mapping is implied. BrowserInput keeps existing gestures and cancels positionless
lost-capture events using retained finite coordinates/identity.

Read [contracts](references/accessibility-input.md) and
[evidence](references/accessibility-input-validation.md), then verify the selected
engine source. Build scripts/build-windows.ps1 and the pinned local Emscripten
scripts/build.ps1 -Configuration Release. Run desktop_accessibility,
desktop_pointer/desktop_pointer_d3d12, desktop_text, desktop_api variants, gui/input/
input_adapter; browser gui-widgets-smoke.cjs on both backends/DPR1/2 and
ui-input-smoke.cjs/input-smoke.cjs on both backends. For input-smoke use a clean
DEMO_URL plus RENDERER=webgpu|webgl2, not a query appended to DEMO_URL.
Native UIA client and injected SDL evidence do not establish physical touchscreen,
tablet, Narrator/NVDA speech, physical mobile or accessibility compliance acceptance.
This documentation does not advance CatGame's engine pin or create its native host.


## Shared authoring frontend and content iteration (2026-10-22)

Optional engine_editor_gui exposes EditorAuthoringPanel over borrowed idle
Scene/EditorSession/GuiSystem. DesktopEditor::authoring() embeds it; browser
?editor=1 offers Shared authoring tools using the identical retained frontend.
Six pages cover terrain stamps, clip/controller authoring, AI states/transitions,
nested prefab definitions/publication, FX emitter forms and cinematic tracks/keys.
Scene history owns assignments/placement; asset documents have separate history.
No automatic cinematic playback is added. Advanced browser graph/curve/timeline
controls remain separate. See [exact scope and API](references/editor-authoring.md).

Drain update(editable) at the idle owner before rendering. At most 32 queued GUI
commands; disable/drop pending work during Play, transform gestures and file
transitions. The scene/session/GUI outlive the panel; destroy it before releasing
those borrows. Hosts own IO: saveDocument/extension provide standard bytes/names;
markSaved only after successful persistence. openDocument validates before replacing
and requires explicit discard when tools are dirty. Native dialogs use bounded
reads/atomic writes. Browser async uploads check workspace generation/area/Play;
download requests retain dirty state. Keep pointer/key ownership with the shared
GUI while open; ordinary scene picking/camera gestures must yield.

AssetManager now supports explicit dependency invalidation and atomic staged
reload; AssetRequest revision/get change only after the complete batch succeeds.
Old shared pointers stay valid. AssetHotReload owns bounded source fingerprints,
not scenes or a scheduler; hosts supply fingerprint loaders and poll while paused.
Windows uses DesktopAssetExecutor, browser uses the isolated fingerprint worker
and fresh transport requests. Nested PrefabAsset links survive capture/archive;
refresh dependencies bottom-up and publish outer roots through existing history.
See [content iteration](references/content-iteration.md). Verify current consumer
pins before using any API; the development checkout does not prove a game pin has it.

Run native editor_authoring/desktop_editor/editor/editor_terrain/editor_fx/prefab_asset,
Node/Wasm editor_authoring_wasm_tests and editor-authoring-smoke.cjs on both forced
browser backends. The portable native test produces fixtures; the browser uploads
those and downloads five formats, then native tests reopen them. Build/validation
commands and measured boundaries: [evidence](references/editor-authoring-validation.md).


## Native graphics API fallback (2026-10-22)

DesktopSettings::backend now defaults to Automatic. Dawn-enabled Windows builds
try Dawn/D3D12, Dawn/Vulkan and Dawn/D3D11 once each, in that order. All use the
same WebGpuRenderer/WGSL, scenes, GUI and application as browser WebGPU. Explicit
Dawn, DawnVulkan, DawnD3D11 and Direct3D12 choices remain strict. A build without
Dawn has only direct D3D12 as its Automatic candidate. The vehicle defaults to
--backend auto; --backend dawn|vulkan|d3d11|d3d12 forces an API/implementation.
No mandatory Vulkan SDK, ANGLE or software renderer is added.

The private bounded owner replaces graphics at idle service after construction
or terminal failure; Dawn same-API recovery runs first. Retire devices/surfaces
and pending readbacks before replacement. Resolve retained camera handles again;
keep Application source/GUI/window ownership, quality/shadow/water inputs and
elapsed clocks. Clear water inputs on scene replacement. Cancel held input and
reset clock baselines/fixed remainder to avoid outage-time catch-up. Capabilities
can change, including timestamp queries: query again after a step, never retain
that borrowed reference across graphics replacement. Do not catch content/user
callback exceptions as fallback events.

backend() reports the current candidate; diagnostics.ready establishes usability.
Diagnostics add requestedBackend/backend/graphicsAttempts, and Automatic renderState
includes failed reasons plus actual adapter API. All-failed errors list attempts;
retryGraphics restarts a fresh bounded chain. Fault-injection tools may opt in and
call simulateGraphicsFailure at idle to queue terminal fallback. This is distinct
from simulateGraphicsLoss (real Dawn-device lost/recovery), and never resets the OS.
Vulkan/FXC runtime lookup uses a normalized absolute trusted System32 directory,
including its trailing separator, for the pinned Dawn restricted loader.

Read [native fallback API/ownership](references/windows-graphics-fallback.md) and
[validation/reproduction](references/windows-graphics-fallback-validation.md).
Build desktop_fallback_tests, desktop_api_tests, windows_fallback_smoke; run CTest
native policy plus strict/Automatic/Vulkan/D3D11 API suites, real three-API gallery
and verify_windows_fallback.py. Run windows-backend-smoke.ps1 -Backend Vulkan or
D3D11 for driving/native input/fullscreen. Also rebuild the pinned browser target
and run render-fallback-smoke.cjs for WebGPU preference, controlled acquisition
failures, WebGL2 fallback and restoration. Injected failures on one NVIDIA GPU do
not establish real driver-crash, missing-runtime, multivendor/older-GPU or physical
mobile acceptance. Verify consumer pins before adoption; no game pin is advanced.

## Direct D3D12 FX milestone 7 (2026-10-22)

Milestone 7 adds direct D3D12 billboard/velocity/plane FX, flipbooks,
ribbons/supplied-contact surface trails, mesh particles and soft opaque-depth fading.
Use existing ParticleEmitterComponent/EffectComponent definitions and FxFrame;
simulation, seeds, transforms, contact policy and archives remain portable. Alpha
meshes/quads/segments share center sorting before additive draws. FX depth-tests
without writes, uses premultiplied blending, alpha fog haze/additive extinction,
HDR/MSAA, masks and ordered views. Mesh particles follow ordinary materials/LOD
and individual direct mesh submissions; they do not cast shadows and require zero
softDistance. General mesh instancing now accompanies the dedicated particle path.

Soft FX shares one opaque World color/depth snapshot with water, taking minimum
4x depth; per-camera perspective/orthographic reconstruction and viewport clamps
isolate views. Underwater runs after World FX, before Overlay/GUI. Positive soft
distance defaults off (zero); active masked soft emitters conservatively allocate
scratch even when empty/paused. Disabled subtrees/absent masks do not. Transparent
surfaces never become soft-depth receivers. Approximate sorting remains.

Direct per-view streams retain numeric/GPU data without scene borrows. Unchanged
paused streams upload nothing; barriers and submission fences protect copies and
retired buffers. Identical host view snapshots retain streams/LOD history; changed
camera identities invalidate them. Preflight FX staging is at most 32 MiB within
the combined 64 MiB arena; meshes/FX share the 40960 draw and 65536 descriptor limits.
Counters expose particles, meshParticles, particleDrawCalls, trailDrawCalls,
trailSegments and changed particleUploadBytes/trailUploadBytes per view. Mesh instancing, BC7 uploads and bounded direct recovery are now delivered.

Launch webengine_fx_windows --backend dawn|d3d12 for deterministic paused presets,
restart, simulation playback, fog/HDR/MSAA; F11 fullscreen, Esc exits. --capture
image.ppm includes retained controls (create parent directory first). Build with
scripts/build-windows.ps1 -Backend Both|Dawn|D3D12; Both remains default. Run
windows-fx-smoke.ps1 for 102 paired captures, then fx/trail/plane_particle/
mesh_particle/material/water/render_frame and desktop_api/desktop_api_d3d12 CTest.
Validate a separate Dawn-free build. Browser suites fx-milestone-smoke,
trail-depth-smoke, plane-particle-smoke, mesh-particle-smoke and surface-trail-smoke
force WebGPU/WebGL2 against fresh pinned local Emscripten outputs. Record native
and browser separately; no other-GPU/mobile/throughput acceptance follows. No
consumer pin, Paws host, public renderer API or archive version changes here.

Read [native FX](references/windows-fx-rendering.md) and [milestone 7 evidence](references/windows-d3d12-milestone7-validation.md).

## Direct D3D12 recovery and performance milestone 8 (2026-10-22)

Direct milestone 8 adds bounded device recovery, general mesh instancing and
semantic BC7 uploads through existing portable resources. Typed removed/reset/hung/
driver errors queue owner-thread same-backend reacquisition; ordinary invalid data,
budget failures and user exceptions do not trigger it. Release old GPU/swapchain/
readback resources before replacement. Keep scene-scoped camera handles, GUI/font
state, quality/shadows, simulation clock and water snapshots. Resolve handles again;
skip removed cameras. Cancel pending direct captures with captureError. New devices
recheck capabilities and lazily upload all assets/poses/FX. Host unavailability
cancels held input, freezes advancement and preserves Running/Paused without catch-up.

There are three automatic attempts per unstable cycle, bounded backoff starting at
50 ms, and a budget reset after 120 successful presentations. Exhaustion reports an
error; retryGraphics starts a new cycle. Do not reacquire after unbinding/shutdown.
Diagnostics expose generation/deviceRecovery/recoveryAttempts/lastDeviceLoss and
monotonic presentation counts; ready gates use. Opted-in idle simulateGraphicsLoss
can call real ID3D12Device5::RemoveDevice when available. It is not an OS reset.
Dawn retains its existing recovery/capture behavior; strict direct never switches API.

Direct RenderFrame camera/shadow/reflection batches now instance compatible opaque/
masked parts, including material maps/layers/water and shared skin/morph poses.
Different poses/bindings/winding/ranges split batches; sorted transparent meshes
stay individual. allowInstanceReordering defaults false; opt in only when opaque
equal-depth precedence may change. Existing ObjectUniform fields use 256-byte slots
and dirty-range GPU copies, with zero paused object uploads. Per-view streams retain
no nodes and retire behind fences. Instance staging caps at 32 MiB within the
combined 64 MiB arena; draw/descriptor/resource limits still apply. Direct counters
submittedInstances/instancedDrawCalls/maxInstancesPerDraw count mesh submissions
across color/shadow/reflection; FX has separate counters. Do not infer speedup from
draw counts or assume cross-backend clear/pass counters have identical scopes.

RenderQuality.compressedTextures defaults true. Direct queries BC7 linear/sRGB
support and chooses the first supported Texture::compressed variant matching its
TextureSemantic. Validate all variants before publication: 4-aligned base <=4096,
complete halved logical mips through 1x1 and exact 16-byte 4x4 blocks. Device copy
footprints handle block rows/padding/tail mips. Color is BC7 sRGB; data/normal linear.
Keep RGBA pixels for disabled/unsupported compression, archives and other backends.
ETC2/ASTC use RGBA fallback on D3D12. GUI forces a separate RGBA cache identity.
Compression changes fence invalidated texture/material bindings. No runtime encoder,
transcoder, compressed archive payload or new public renderer/texture API is added.

Build scripts/build-windows.ps1 and a separate -Backend D3D12 tree. Run
windows-performance-smoke.ps1 for paired 22-case native pixels, real device removal,
257-to-3 direct color draws, BC7/RGBA selection and retained scenes/clocks/resources.
Run native desktop_recovery/desktop_fallback/desktop_api/desktop_api_d3d12/
instancing/render_quality/material/render_frame; repeat recovery/API in the Dawn-free
build. Regress FX/water/terrain/animation/lighting/vehicle. Browser Release uses
the pinned repository-local SDK; run render-quality-smoke, instancing-smoke and
render-fallback-smoke on fresh frozen outputs, with explicit WebGPU/WebGL2 coverage.
Record platforms separately. Other-GPU, physical mobile, sustained performance and
cross-device timing comparisons remain separate. Consumer pins/Paws hosts do
not change with this skill documentation.

Read [recovery/performance contracts](references/windows-d3d12-recovery-performance.md) and [milestone 8 evidence](references/windows-d3d12-milestone8-validation.md).

## Reusable compiled modules (2026-10-22)

Use optional `WebEngine::Modules` for explicit typed code-plugin composition:
versioned descriptors, bounded deterministic dependency validation, staged
installation and immutable snapshots retaining provider lifetimes. Application
sessions own runtime state and hold snapshots; there is no new clock, global
registry or dynamic loader. `examples/modules` is a non-quest plugin template with
native/Wasm external-consumer and disabled-module tests plus actual browser and
Windows demo evidence. See [compiled modules](references/modules.md) and selected
engine docs/modules.md / modules-validation.md. The M2 quest kernel now uses this
foundation; CatGame's pin is unchanged and does not expose these development APIs.

## Portable quest kernel M2 (2026-10-22)

Optional `WebEngine::Quests` now composes through `WebEngine::Modules` on native
Windows and browser Wasm. Use immutable validated catalogues retaining typed
handler/provider snapshots, independent owner-thread runtimes, scoped events,
bounded prepare/commit candidates and durable effect receipts. Keep puzzle rules,
economy publication, Application-derived clocks and persistence in the game/host.
Read [quest contracts and validation](references/quests.md). The 242-check native/
Wasm suite exchanges snapshots and intermediate traces; real WebGPU/WebGL2 and
Dawn/direct D3D12 samples verify module-composed completion, rejection and retry.
M3 now delivers lifecycle/persistent IO; Paws migration remains separate. Verify the consumer
pin: this development API is not in CatGame's currently pinned engine.


## Direct D3D12 GPU profiling

Use [GPU profiling](references/windows-gpu-profiling.md) for the current direct
timestamp/query contract and on-demand DesktopHost::gpuProfileTrace export.
diagnostics().performance includes shared gpu summaries plus gpuProfile nested
frame/view/pass events, inclusive/self times, sampled metadata and rolling 120-sample
statistics. Three fenced readback slots sample at 10 Hz without profiling waits;
scene/view/quality/attachment epochs and device recovery reject stale results.
WEBENGINE_GPU_TIMING=0 disables native queries/readbacks at device creation on
direct and Dawn. Browser WebGPU keeps ?gpuTiming=off; WebGL2 remains unavailable.
GPU clocks are not CPU-correlated. Keep GPU types private and simulation unchanged.
Run scripts/windows-gpu-profile-smoke.ps1 for both native backends plus a Dawn-free
build, and browser performance-smoke, performance-timeline-smoke and
render-fallback-smoke on fresh pinned output. Check actual timing hierarchy, trace
eviction, loss/recovery and exact profiling-on/off captures. No throughput or
other-GPU acceptance follows. Verify the consumer pin before using new APIs.


## Hybrid mesh terrain milestones 1–5 (2026-10-22)

Use TerrainData::holes and makeMeshTerrainActor for heightfields with static cave/
overhang sections. Full/chunk geometry, queries and collision omit masked cells;
cut chunks retain source hole boundaries at every LOD. withHoles(mask) and TerrainBrushMode::Hole/Fill create
immutable TerrainEdit revisions with holesChanged and sorted dirty chunks. Positive
strength applies binary cell-centre circles; zero is no-op. Filling restores LOD.
Eager/source-only edits and TerrainStream share unchanged meshes; source-only edits
stay lazy. TerrainPage wire/edit APIs do not support holes and reject Hole/Fill.
Collision-enabled empty terrain keeps disabled placeholder collider/body handles;
filling enables source collision. Terrain owns this enablement: disable its actor
to suspend collision independently. Never use render LOD for physics.

terrainRaycast(..., includeHoles=true) is authoring-only picking of uncut source;
default queries still omit holes and remain heightfield-only. Hybrid movement uses
PhysicsWorld and CharacterMotor. EditorSession groups stamps, cancellation and
undo/redo and retains exact collision snapshots. Browser Details has Cut hole and
Restore ground; native/shared authoring mode numbers 4 and 5 provide the same APIs.

The shared examples/hybrid_terrain demo owns cave dimensions, paired mask/mesh
publication, manual holes, protected entrance footprint and 32-command history.
Both hosts support pointer cut/fill, radius, width/height/depth, cancel and undo/redo.
Opening width/depth snap to grid; brush strokes affect surrounding ground. Edits
return the explorer outside. Scene checkpoints preserve application metadata for
continued editing; history resets after load (see milestone 5 below).
Optional engine_core buildTerrainSeam(fullSource, immutableMesh, settings) prepares
small-gap joins from hole/solid boundaries. terrainOrigin locates source sample
zero in mesh-local metres; axes and scale must align. Defaults: weldDistance .25 m
(less than half a source cell), blendWidth 1.25 m, maxEdgeLength .75 m; 2048 boundary
edges, 30000 output triangles, 8 million work units. Input <=4096 triangles with no
LOD; output can carry constrained derived LOD. Matching inserts source samples and welds nearby
vertices; opposing undersides and unmatched edges remain unjoined. It is not an
arbitrary Boolean operation. Invalid data/budgets reject without publication.
Materials use existing triplanar layers and mesh-UV distance weights, preserving
tiling and packed normal details. Inputs: opaque nonmetallic nonemissive PBR/unlit
solid colors or unweighted single triplanar layers. Shader families and triplanar
sharpness must match. Conventional UV maps, painted layers and height blending
reject. Rebuild after source/mesh/material edits, publish mesh and collider at an
idle boundary. Seam preparation adds no scene owner or renderer dependency.
The demo rebuilds seams with every hole/entrance edit and retains combined history.
Seam close-up and material-only comparison work in both hosts; disabling the blend
does not change geometry, collision or normal interpolation. Scene archives persist
prepared base results. No consumer pin is advanced.

Milestone 4 adds hole-aware terrain LOD: mixed tiles subdivide; only entirely
solid planar regions merge, with every hole/chunk/tile edge sample retained.
Curved cut regions retain source triangles (planarity tolerance .00001 m), avoiding
new shadow receiver displacement near openings. Dense cuts may admit no reduction.
Prepared cut-patch LODs are checked against canonical regenerated topology.
TerrainSeamSettings::lodLevels defaults 3, accepts 0..3; zero disables generation.
Cave LOD exactly welds attributes, locks all blend-band vertices and open/material
borders, then simplifies interiors. At most 64 parts with LOD enabled; each level
retains the 30000-triangle/90000-vertex cap. The three per-part simplifier passes
are bounded separately from the seam matching maxWork counter. Thresholds are
180/80/32 pixels, target ratios .5/.2/.06, relative errors .002/.008/.02; constraints
can prevent targets, and only strictly smaller candidates are kept. Collision
always uses the full base mesh; camera and shadow LOD are independently selected.
MeshOptimizer::generateLods(mesh, MeshLodConstraints{lockBorders, lockedVertices})
adds reusable optional constraints. The lock span is empty or one 0/1 byte per
source vertex, borrowed only during the synchronous call. Build the mask after
exact vertex optimization. Invalid masks preserve existing LOD. Default constraints
retain old behavior.

Milestone 5 adds Mesh::lodRecipe (optional MeshLodRecipe: version=1, levels=1..3,
lockBorders=true, owned empty or per-base-vertex 0/1 mask). Seam preparation attaches
explicit blend-band locks. MeshOptimizer::validateLodRecipe checks bounded static
input; regenerateLods stages/swaps only derived levels, never base/collision, and
is a no-op without a recipe. Caps: 90000 vertices, 30000 triangles, 64 parts.
Optimize before constructing a recipe; optimize rejects recipe-bearing meshes.
Explicit generateLods clears the old recipe; regenerateLods retains it.
Archive v36 stores recipes, reconstructs shared caches once before document
publication, and charges 512*vertices + 64*indices + 16*parts*vertices against the
aggregate maxAllocatedBytes budget in addition to decoded data. Decode caps locks
at 90000 bytes before allocation. Preparation is synchronous on the scene owner.
Versions 1–35/unmarked meshes load with no recipe or inferred cache; reprepare an
older seam once or attach explicit validated locks to a mutable base copy.
Heightfield LOD still rebuilds from saved source. Collision remains full detail.
Browser localStorage key webengine.hybrid.checkpoint.v5 (4 MiB decoded save cap)
and native atomic build/hybrid-terrain-checkpoint.wescene support actual live
scene replacement. Demo applicationData owns entrance dimensions, manual holes,
view/detail/blend/radius; saved IDs rebind host views before unloading old scenes.
History, gestures, tool mode and movement commands reset. Invalid loads retain the
live scene; subsequent editing and traversal work. No consumer pin changes.


Read [contracts and builds](references/hybrid-terrain.md) and
[milestone 5 validation](references/hybrid-terrain-save-load-validation.md). Build
browser host/worker together with repository Emscripten; standalone demo compile
and link must enable -fexceptions so rejected edits retain valid state. Run native
terrain_lod_archive, persistence, camera_follow, terrain_hole_lod, mesh_optimizer, mesh_detail, terrain_seam, hybrid_terrain, editor_terrain, editor_authoring, terrain, terrain_paging,
terrain_page_edit, scene_archive and render_frame; demo --smoke on Dawn and direct
D3D12; hybrid-terrain-smoke.cjs on WebGPU/WebGL2. Run editor-terrain-smoke.cjs
when changing main editor integration.
Physical mobile and large-world performance remain unvalidated.

## Quest host integration M3 (2026-10-22)

The shared quest sample now supports persistent saves and startup restoration,
Application-driven pause/suspension-safe timing, scene travel and native/browser
file exchange. Keep sample/game state outside disposable scenes, validate the full
economy/quest envelope before publication, and cancel delayed controls/imports
using the host epoch. Read [host integration](references/quests-host.md). Both
WebGPU/WebGL2 and Windows Dawn/direct D3D12 flows pass, including real file transfer
and process/page reload. Save is explicit, not autosave/cloud sync. M4 consumer
publication/host extraction and Paws quest/save migration remain separate work.

Shared browser/Windows game sessions and consumer packaging use [host integration](references/consumer-hosts.md). Verify the selected pin before consuming M4 APIs.


## Multiplayer foundation (M1)

Optional `WebEngine::Network` / `engine_network` supplies strict versioned binary
sessions, peer/entity identities, bounded commands and complete snapshots using
standard C++20. Browser Emscripten WebSockets and Windows WinHTTP adapters keep
platform APIs and owned IO workers out of portable headers. Applications map
network IDs to existing scene handles, validate payloads, own game rules and drain
messages on the scene owner. Dispose transport before resetting/replacing sessions.

The current M1 example supplies a C++ authoritative kinematic arena behind a
loopback Node/ws gateway, browser and Windows clients, and isolated test launchers
in both editors. Movement uses existing fixed ticks; server monotonic time owns
simulation. Full snapshots support late join/despawn; reconnect obtains a fresh
identity. Editor launchers retain source documents and do not replicate authored
scenes. No prediction, rollback, arbitrary component replication, matchmaking,
accounts, internet deployment or consumer pin change is implied.

Enable `WEBENGINE_BUILD_MULTIPLAYER_DEMO=ON` in both builds. Build native
`multiplayer_server`, `webengine_editor`, `network_tests`, `multiplayer_arena_tests`
and `network_transport_tests`; build browser `multiplayer_demo`, `web_assets` and
the two portable test targets with the pinned local Emscripten SDK. Install the
locked gateway package with `npm ci --prefix examples/multiplayer --ignore-scripts`.
Run protocol/arena checks on native and Wasm, then `scripts/multiplayer-smoke.cjs`
for paired WebGPU/Dawn and WebGL2/direct-D3D12 clients and actual editor launchers.
Keep local-loopback, desktop-browser, physical-device and production claims distinct.
Read [multiplayer contracts](references/multiplayer.md) and
[validation guidance and evidence](references/multiplayer-validation.md).
Verify these development APIs exist in the selected consumer pin before use.

## Authored multiplayer scenes (M2)

Optional `WebEngine::NetworkScene` / `engine_network_scene` layers on network core
and engine_core. `saveNetworkScene` / `loadNetworkScene` package existing archives
with a selected root mesh actor's saved ID, validating a static Node/Actor/Mesh/
Camera/Light profile before publication. One active camera view is required.
Unsupported components, custom behavior and ticking actors reject, including in
disabled subtrees. Limits: 4 MiB package, 32 MiB archive-accounted allocation,
2048 nodes, 256 resources, depth 32 and 262144 elements per collection.

`ContentReceiver` in network core stages ordered 16 KiB bootstrap chunks. Complete
bytes still require application/schema validation. `TransformReplicas` stages
complete validated local transforms and template instances before publishing
spawns/poses/despawns through ordinary scene-owned actors and ActorId handles.
The scene owns instances; use `apply({})` to remove them. Dispose managers before
their scenes. Finite/range and Node transform validation precede publication.
Server authority and movement remain example/application policy.

Both editors expose Host selected actor and Join authored scene; the M1 fixed
arena remains separate. Source documents/history/ordinary Play are retained.
Native `setMultiplayerSceneLauncher` synchronously borrows source/views/assets and
selected ID (empty means Join); the host owns serialization/process policy.
Browser uses the same C++ validator and an expiring single-use IndexedDB handoff.
The first valid package freezes content for the scene server process; later hosts
must match exactly, or restart the server for new content. Bootstrap precedes Hello.
The MP02 example replicates player-template instances with server-owned kinematic
movement, late join, reconnect and despawn. It does not replicate arbitrary
authored gameplay, physics/animation components, collaborative edits or prediction.

With `WEBENGINE_BUILD_MULTIPLAYER_DEMO=ON`, build native `multiplayer_scene_server`,
`webengine_editor` (including `multiplayer_scene_windows`) and `network_scene_tests`;
build browser `web_assets` (including `multiplayer_scene_demo`) and the test target.
Start `node examples/multiplayer/server.cjs build/windows/Release/multiplayer_scene_server.exe 8177`.
Run native and Wasm scene tests and `scripts/multiplayer-scene-smoke.cjs` against
fresh `dist`, covering WebGPU/Dawn and WebGL2/direct D3D12, both editor publishers,
authored visuals, source preservation and invalid/reconnect/disconnect paths.
Read [scene contracts](references/multiplayer-scenes.md) and
[validation](references/multiplayer-scenes-validation.md). This development feature
does not advance a consumer pin or establish physical-mobile/internet acceptance.

## Multiplayer prediction and reconciliation (M3)

M3 introduced MP03 with local player prediction and server
reconciliation; earlier M2 descriptions above refer to the historical MP02 scope.
The static scene profile and saved packages are unchanged. Editor Host/Join enables
prediction automatically on browser and Windows; M1 keeps its separate arena.

Header-only `engine::net::PredictionHistory` in engine/network_prediction.hpp belongs
to optional WebEngine::Network and has no core/scene/platform/clock dependency.
Initialize with reconcile(0,state,step), push consecutive sequenced input, and
reconcile applied acknowledgements by replaying remaining inputs. It stages all
work before publication. Future/regressing ack, invalid sequence/budget or replay
failure preserve valid history. State/input bounds are 256 bytes and the queue is
128 inputs. Call on one owner thread with a pure synchronous application-owned
step callback; reset on reconnect/replacement. Callback side effects cannot be undone.

The example shares deterministic kinematic moveStep policy, issues 60 Hz commands
with bounded client catch-up and consumes at most one command per server fixed
tick. Snapshots carry a uint32 applied-input acknowledgement after the transform.
Client presentation alone uses predicted poses; authority stays in session records.
History saturation stops new commands. General physics rollback, component
prediction, remote interpolation and correction smoothing remain future work.

Use the existing multiplayer scene build targets. Run native and Node/Wasm
network_scene_tests plus scripts/multiplayer-scene-smoke.cjs against fresh dist.
The smoke injects 120 ms per direction and checks prediction before authority,
convergence, both editor publishers, source preservation and lifecycle/rejection
paths on WebGPU/Dawn and WebGL2/direct D3D12. Gateway MULTIPLAYER_DELAY_MS defaults
zero and is a bounded local test facility, not deployment policy.
Read [M3 contracts](references/multiplayer-prediction.md) and
[validation](references/multiplayer-prediction-validation.md). Verify availability
in the selected engine pin; skill updates do not advance CatGame's dependency.

## Sculpted cave demo (2026-10-23)

The hybrid terrain example now uses a connected, sculpted limestone shell with
15,498 base triangles at its default entrance settings. Application-owned
`examples/hybrid_terrain/sculpt.hpp` shapes the walls and ceiling after seam
preparation, preserves the flat floor and hole rim, then regenerates constrained
LOD from the sculpted base. Collision always uses that full base mesh. The demo
requests adaptive `.4*sqrt(halfWidth*(depth+3)/39)` edge length and a 32-million
seam work budget; engine defaults and archive formats are unchanged. Periodic
256-square albedo and packed normal/height maps add fine stone relief. This is
demo content, not a general interactive mesh-sculpt tool. Older checkpoints keep
their saved geometry until an entrance/hole edit rebuilds the cave.

Keep shared face subdivisions aligned before deformation. Validate all eight
entrance size extrema with `hybrid_terrain`, plus `terrain_seam`,
`terrain_hole_lod` and `terrain_lod_archive`. The existing full browser smoke and
Windows Dawn/direct D3D12 smoke exercise editing, walking, LOD and save/load.
See [sculpted cave evidence](references/hybrid-terrain-sculpt-validation.md).
These development-demo changes do not advance the CatGame engine dependency.

## Multiplayer presentation smoothing (M4)

Authored-scene Host/Join clients now interpolate remote players and smooth local
positional reconciliation corrections automatically on browser and Windows.
Earlier M3 descriptions of these features as future work refer to its historical
scope. M4 retained MP03 wire data; saved scene packages and authoritative simulation stay
unchanged. M4 changes presentation only and does not add general physics rollback.

Include engine/network_presentation.hpp and link optional WebEngine::NetworkScene.
TransformTimeline(delay=.1,maxGap=.5,snapDistance=4) buffers at most 32 validated
receipt-time Transform samples. push stages validation, rejects regressing times
and coalesces equal times. sample(now) interpolates position/scale and shortest
Euler paths at now-delay; endpoints hold, without extrapolation. Teleports, long
gaps and scale-sign changes reset history. Caller owns monotonic seconds and
session/entity lifetime; no scene, renderer, platform or callbacks are owned.

CorrectionSmoother(halfLife=.08,snapDistance=4) rebases a compensating position offset
only for authoritative prediction corrections. present(prediction,deltaSeconds)
decays it exponentially, leaving prediction/session authority unchanged. Large
corrections/offsets or gaps above .5 seconds snap; invalid data retains valid state.
Rotation/scale remain predicted values. Reset on replacement/reconnect; apply output
to visual objects, never authoritative collision state. The client stages remote
buffers with complete snapshots, erases despawned histories and cancels all state
on Leave/error/reconnect. Editor source bytes/history stay isolated.

Build the existing scene targets and native/Wasm network_scene_tests. Run
scripts/multiplayer-scene-smoke.cjs on fresh dist for WebGPU/Dawn and WebGL2/direct
D3D12. It injects 120 ms per direction, 40 ms ordered jitter and one explicit
server test correction per player. Gateway delivery queues are FIFO, bounded to
64 messages/socket with at most two timers; close cancels them. All fault injection
defaults off in normal use. Verify remote interpolation, local visual decay and
convergence, prediction, rendering, source preservation and lifecycle paths.
Read [M4 contracts](references/multiplayer-presentation.md) and
[runtime evidence](references/multiplayer-presentation-validation.md). General
component/skeletal prediction, adaptive clock synchronization, extrapolation and
physical mobile/internet acceptance remain separate. Verify consumer pin support;
updating skills does not advance the CatGame engine dependency.


## Module SDK release handoff (M7, 2026-10-23)

The published quest-module SDK at 461ab6dd1769171955254473c2b3623ad3dc615a adds
independent generic and custom quest-binding consumers, native/Node-Wasm CTest,
feature-omission coverage and docs/module-sdk.md. Runtime ownership and module
API 1.0.0 remain unchanged. Paws M7 pins that revision and supplies clean-source
web/Windows package validation and a full release matrix. Build consumers from
checked-in accepted assets; offline authoring must not run during consumer builds.
Read references/modules.md and references/consumer-hosts.md for the handoff.

## Hybrid demo free camera (2026-10-23)

The cave demo supports right-drag mouse look and WASD flight, Q/E world vertical
movement, Shift boost and Escape cancellation. Browser users first focus the
canvas by clicking it; input fields retain typing ownership. Left-drag remains
hole painting. The application-owned `FlightCamera` in `flight.hpp` consumes
InputBindings/InputSystem actions on the Application input clock, with browser
focus/pointer capture and Windows DesktopHost bindings. It owns no scene pointer
or callback lifetime. Flight is an inspection camera without collision, independent
of the explorer. Camera transforms use the existing checkpoint archive; held
controls and gestures reset on load/cancellation. There is no new engine API,
pointer lock or separate scheduler. Speeds are 6/18 m/s, with clamped pitch and
bounded movement delta. GUI focus, blur, resize and Escape must release held keys.

Run `scripts/hybrid-flight-smoke.cjs` on WebGPU/WebGL2 and the native demo `--smoke`
on Dawn/direct D3D12; also run the existing hybrid terrain smoke to protect painting,
LOD and save/load. See [camera validation](references/hybrid-terrain-flight-validation.md).
Skill documentation does not advance CatGame's engine dependency.

## Multiplayer delta replication and interest (M5)

M5 introduced MP05 (0x4d503035) with per-peer
delta snapshots, periodic keyframes, baseline recovery and remote-player interest.
Earlier M3/M4 MP03 descriptions are historical. Rebuild scene server and clients
together; WES2 packages/archives and the separate M1 MP01 arena stay compatible.
Both editor source documents, history and ordinary Play remain isolated.

Include engine/network.hpp and link WebEngine::Network. On the server owner thread,
publish ServerSession::snapshot(allRecords) once per publication to maintain the
complete authoritative ownership registry. Then snapshotFor(peer, relevantIds,
forceFull=false) selects unique existing IDs and automatically includes everything
owned by that peer. Call at most once per ready peer per publication. Relevance
is application policy, independent of command authorization and presentation.
The engine owns no scene, position/spatial index, clock, IO or callback.

Each peer retains its own bounded reliable-stream baseline. Initial, forced,
resync and every 20 publications use full keyframes. Otherwise smaller deltas
carry complete upserts, removals and sparse changed-byte patches; unchanged records
have no entity payload. Metadata/size changes use upserts; encoding falls back to
full if a delta is not smaller. Send produced packets in order or close on failure:
baselines advance at generation, without client acknowledgements. This is a
reliable ordered WebSocket contract, not UDP/unreliable transport support.

ClientSession::receive stages complete reconstructed records before publication.
BaselineMismatch identifies a missing baseline; requestSnapshot emits a validated
Resync without consuming gameplay command sequence. The scene client coalesces
recovery to one request, retains valid state and skips deltas until a full packet.
Malformed payloads/operations/targets/offsets/capacity reject atomically. Bounds
remain 16 peers, 64 records, 256 bytes/payload, 32768 bytes/packet. Disconnect and
reconnect clear baselines. M3 replay and M4 timelines consume complete candidates;
interest removal discards replica/history, re-entry creates fresh presentation.

The example scene server owns local XZ distance policy: remote players enter
within 4 units, remain through 5, exit beyond 5; own players always remain visible.
These are example rules, not engine defaults or saved editor settings. Static
authored content still bootstraps in full. Filtering is not a secrecy boundary.
No scheduler, renderer, scene/archive or platform-adapter boundary changes.

Build native network_tests, network_scene_tests, multiplayer_scene_server,
multiplayer_scene_windows and webengine_editor; browser network_tests,
network_scene_tests and web_assets with the pinned repository SDK. Run native
network/network_scene/multiplayer_arena/desktop_editor CTests and Node/Wasm core
tests, then scripts/multiplayer-scene-smoke.cjs on fresh dist. It verifies
WebGPU/Dawn and WebGL2/direct D3D12 with both editor publishers, distance exit/
re-entry, smaller accepted snapshot traffic and one injected baseline recovery
per client. MULTIPLAYER_TEST_BASELINE=1 is gateway-only fault injection, default
off; normal delivery remains reliable/ordered. Native scene smoke lasts 20 seconds.
Read [M5 contracts](references/multiplayer-replication.md) and
[runtime evidence](references/multiplayer-replication-validation.md). Diagnostics
compare accepted snapshot bytes only, excluding bootstrap/commands/controls and
discarded recovery packets. General component replication, adaptive bandwidth
budgets, compression, unreliable transports, internet deployment, physics rollback
and physical-mobile/large-session acceptance remain separate. Verify the consumer
engine pin; updating this skill does not advance CatGame's dependency.

## Cave panel continuity repair (2026-10-23)

One Mesh resource does not establish smooth joins. The hybrid cave shares its
floor/wall limestone maps and tint, rounds interior cross-section corners up to
0.8 m while preserving the flat centre path and source rim, and accumulates
sculpt normals by coincident source position across attribute splits. Retain
source normals only in the actual terrain blend band; multiplying by displacement
fade reintroduced old panel creases. Full sculpted base collision and constrained
LOD regeneration remain unchanged. Older checkpoints keep their old geometry;
reapply entrance settings to rebuild them. Regression coverage checks shared
interior normals and rock textures, all entrance extrema, and browser/Windows
editing, LOD, collision traversal and checkpoint restoration. See
[continuity validation](references/hybrid-terrain-continuity-validation.md).


## WebGL2 material shader variants (2026-10-23)

For dense scenes with low WebGL2 FPS, see [material variant profiling and
integration](references/webgl-material-variants.md). Verify the consumer pin:
the specialized ordinary-material shader removes unused feature/discard paths
while preserving real models, lighting and resolution. Validate special materials,
context recovery, matched browser views/laps and native asset/placement contracts.

## Continuous cave loft (2026-10-23, single-cave stage)

This supersedes the earlier panel-based sculpt and panel continuity repair.
`examples/hybrid_terrain/cave_loft.inc` generates a single indexed source disk:
grid-sampled terrain rim, collar, arched mouth, varying shared tunnel rings and
closed rear dome. Floor, walls and ceiling share vertices. One limestone material
covers the cave; the separate overhang sample retains its existing geometry.
`sculpt.hpp` refines shared edges up to twice within the 30,000-triangle cap, adds
small continuous rock contours, reconstructs normals, welds matching attributes
and generates three constrained LOD levels. Default base count is 28,560 triangles.
The centre walking floor and terrain rim remain protected. Collision uses the
full base. Existing seam preparation uses maxEdgeLength=1.25, maxWork=32000000 and
lodLevels=0 before the demo's refinement; engine defaults remain unchanged.

Source topology and final indexed connectivity tests protect the continuous
surface. Test all entrance extrema, arched ceiling coverage before the rear taper,
and a separate closed-dome collision ray. Run the browser terrain smoke on
WebGPU/WebGL2 and native --smoke on Dawn/direct D3D12 for editing, LOD, traversal
and checkpoint restoration. Older checkpoints retain their saved geometry until
an entrance/hole edit rebuilds it. No new archive format or generic sculpt API.
See [current contracts](references/hybrid-terrain.md) and
[continuous cave evidence](references/hybrid-terrain-loft-validation.md).
Skill changes do not advance CatGame's engine dependency.

## Multiplayer editor authoring and diagnostics (M6)

Browser and native Windows editors now offer Add replication settings on enabled
root mesh actors. The inert settings component exposes Ownership (Player template
or Server actor), Relevance (Distance/Always visible/Owner only) and Enter/Exit
radii through the shared typed registry. Defaults: Player, Distance, 4/5 units.
Radii are finite, .1 <= enter <= exit <= 10000; server actors cannot be Owner only.
Structural/property undo/redo, dirty tracking, ordinary save/load/recovery and
duplication preserve settings. Runtime Host/Join never changes source/history.

Include engine/network_authoring.hpp. ReplicationSettings and its typed enums
are header-only annotations over existing Actor/Component types, with no protocol,
transport, callback or retained scene ownership. makeReplicationMetadata returns
a detached ordinary Component; adopt on the idle scene owner. Existing tags store
the reserved versioned WebEngine.Replication.v1 annotation, avoiding archive-format
changes. Typed getters validate; the Actor getter is optional and rejects multiple
settings. setReplicationSettings stages a complete ComponentSnapshot while
preserving unrelated tags/common state. isReplicationMetadata only recognizes the
marker; the typed getter validates it. Unknown/malformed annotations reject network
publication and remain inert/removable in generic scenes. Inspector typed fields
omit invalid annotations. EditorSession::addComponent("NetworkReplication") and
network.ownership/relevance/enter/exit properties use normal editor history.

The scene profile requires inert, enabled, childless, unticked metadata at identity
transform directly beneath an enabled root mesh actor. Replicated templates must
contain mesh visuals only. The selected template must be Player-owned (unannotated
players retain defaults); other annotated actors must be Server-owned. At most 48
server actors reserve room for 16 players within the 64-record cap. Static bootstrap
and its 4 MiB content/profile bounds remain. No arbitrary gameplay codecs/behavior
are admitted. Normal server actors receive no automatic motion; the explicit test
correction flag nudges them to verify authority.

NetworkScene::replicated returns a validated ReplicatedActor catalogue. Player kind
1 uses per-peer identities; server kinds start at 2, IDs at serverEntityBase=65536,
owner=0, sorted by saved ID. Scene servers cap lifetime peer admissions at 65535
to protect that reserved range; restart for new identities after exhaustion.
Client validation checks the catalogue before records, replay or timelines commit.
TransformReplicas(scene,map<kind,const Actor*>) stages detached prototypes, borrows
prototype pointers only during construction and keeps ordinary scene-scoped actor
handles. Scene owns replicas and must outlive apply/find. Existing single-template
construction remains supported. Kind changes for an existing entity reject before
publication. Removal/re-entry clears/recreates visual handles/history. Client source
templates are hidden after prototype preparation to avoid duplicate visuals.

relevantTo(settings,owned,wasVisible,distance) evaluates caller-owned finite local
XZ distance; own players always remain visible. Selection never grants command
authority, and owner 0 server records cannot be commanded by peers. M3 prediction,
M4 smoothing and M5 delta/recovery contracts still consume reconstructed complete
records. Scene schema is MP06 (0x4d503036); rebuild server/clients together. Legacy
unannotated WES2 packages keep defaults; archive/package envelopes and M1 MP01 do
not change. Earlier MP03/MP05 descriptions are historical. Core, standard-only
network, platform adapters, renderer and scheduler boundaries remain unchanged.

Browser Session diagnostics and Windows Diagnostics show authority/prediction X,
correction, pending inputs, deltas/full packets, recovery and accepted snapshot
bytes. Browser additionally lists displayed X, ack, equivalent full bytes and
entity kinds/owners. These read-only views inspect isolated runtime state; counts
refer to visible records. Leave/reconnect reset them.

Build native network_scene_tests, multiplayer_scene_server and webengine_editor;
browser network_scene_tests and web_assets using the pinned local SDK. The scene
test additionally links engine_editor for authoring acceptance; production module
dependencies do not change. Run native network_scene/network/multiplayer_arena/
desktop_editor/editor_content/editor_structure, Node/Wasm tests and
scripts/multiplayer-scene-smoke.cjs on fresh dist for WebGPU/Dawn and WebGL2/direct
D3D12. Verify authoring/save-load, both publishers, server-owned visuals and
unauthorized commands, diagnostics and the earlier recovery/lifecycle checks.
Native authoring acceptance uses programmatic EditorSession edits and file reopen;
browser tests use real fields. Distinguish those from native field pointer/keyboard
acceptance. Read [M6 contracts](references/multiplayer-authoring.md) and
[evidence](references/multiplayer-authoring-validation.md). Arbitrary replicated
component fields, ownership transfer, physics rollback, live editing, internet
hosting and physical mobile acceptance remain separate. Skill updates do not
advance CatGame's engine dependency; verify consumer pin support.

## Three-cave system (2026-10-23, current demo)

The continuous indexed loft now has three chambers and two descending passages:
Entrance Cave (nominal 6 m wide / 4.3 m high, floor 0 m), Grand Chamber (9/6 m,
floor -3 m), Lower Grotto (4.2/3.5 m, floor -8 m). Default total extent including
the approach is about 46 m, with 25,540 base triangles and three constrained LODs.
Entrance edits scale room widths and shift deeper rooms. All rooms/passages share
the source surface; full base geometry supplies collision. Static mesh ground
extends the plateau beyond the heightfield tile, outside brush/query ownership.
Application-owned point lights and chamber view/complete-walk controls serve both
hosts. Walk outside climbs back up. Flying controls remain unchanged.

HYBRID6 application metadata permits views 0..6 and validates deeper geometry;
HYBRID5 single-cave files remain readable using the same localStorage key and
engine archive v36. Optional decor is reconstructed on staged legacy loads;
legacy cave geometry remains until an entrance change rebuilds it. Moving and
gestures reset. Actual legacy save/load, upgrade and re-save were tested.

Run hybrid_terrain, terrain_seam, terrain_hole_lod and terrain_lod_archive. The
browser terrain smoke now also checks each chamber camera, complete motor descent
and ascent, plus a saved deep explorer/view. Native --smoke covers chamber views,
file restoration and the full route on both renderers. Preserve a running user
executable: an isolated OutDir with BuildProjectReferences=false can link the demo
against existing libraries; stage all existing runtime DLLs and assets/fonts beside it. Do not build
multiple native projects concurrently when they share intermediate directories.
See [contracts](references/hybrid-terrain.md) and
[three-cave evidence](references/hybrid-terrain-system-validation.md).
Skill changes do not advance CatGame's engine dependency.

## Terrain mesh formations (2026-10-23)

The new reusable `engine_core` surface operation supersedes the earlier loft
note's statement that no generic sculpt API exists, within this bounded scope.
Include `engine/terrain_mesh.hpp` and call `buildTerrainMeshFormations` with an
immutable ordinary Mesh and up to 32 TerrainMeshFormation placements. Anchor,
direction, length, baseRadius, tangential bend, irregularity and seed describe
joined tapered protrusions. Up forms floor stalagmites; down forms ceiling
stalactites; arbitrary oriented surfaces work. The source is unchanged on failure.
Local conforming refinement, stable edge/vertex anchors and shared normal
reconstruction retain continuous geometry across attribute splits. Nearby
disconnected sheets are excluded. Preserve open borders and inherited seam locks;
avoid same-facing footprint overlap. Defaults: edge radius/5, 30k triangles/90k
vertices/64 parts, 8M work steps (max 64M), 3 constrained LOD levels with tip locks.
LOD=0 disables derived geometry. Static meshes only; no Boolean union, scatter,
scene ownership, growth simulation or clearance guarantee. Conventional UVs may
stretch; triplanar limestone maintains texture scale.

The caller publishes the ordinary result mesh and full-detail collider together.
Existing v36 scene saves preserve baked geometry/tip locks and rebuild derived LOD;
editable procedural descriptors remain application metadata. Both shared cave demo
hosts call this operation through examples/hybrid_terrain/formations.hpp after
rock shaping, with two floor and two ceiling placements clear of the centre lane.
Entrance/hole edits regenerate them; undo/redo and checkpoints preserve geometry.
Older checkpoints need an entrance/hole edit to regenerate formations.

Build/run terrain_mesh_tests on Windows and Node/Wasm, plus hybrid_terrain,
terrain_lod_archive, terrain_seam and terrain_hole_lod native suites. Regress exact
edge/vertex and near-edge anchors, tilted directions, UV/material splits, smooth
normals, disconnected sheets, deterministic bend/seed, inherited locks and budgets.
Run the browser cave smoke on WebGPU/WebGL2 and native --smoke on Dawn/direct
D3D12 for rendering, controls, LOD, movement and checkpoint restoration. See
[formation contracts](references/terrain-mesh-formations.md) and
[validation](references/hybrid-terrain-formations-validation.md).
This skill update does not advance any consumer's engine dependency.


## Curved cave layout (2026-10-23, current demo)

The three-cave loft now offsets chamber centres to x=0/-6/+6 m, retaining floor
elevations 0/-3/-8 m. Smooth lateral transitions bend the passages left then right.
Default nominal cross-sections are 6x4.3 m, 10.8x6.5 m and 6.6x4.8 m; narrow
passages are 3x2.5 m, reduced to 2.2 m height at the minimum entrance ceiling.
Collision measurements verify each room is wider and taller than its passage.
The application-owned caveRouteX drives loft ring positions, camera/light offsets,
and tangent-based motor steering with bounded lateral correction. Geometry remains
one connected source surface, with the existing full base collision and LOD.

HYBRID7 identifies the curved layout; HYBRID5/6 remain readable. Base geometry
bounds distinguish curved and straight steering independently of render LOD, so
older saved straight systems retain their route. A real HYBRID6 save was walked
successfully. Source geometry is rebuilt only by authoring, not on checkpoint load.
Existing formation work is preserved. No new generic engine API or scene archive
version is introduced.

Run hybrid_cave_route plus hybrid_terrain, terrain_lod_archive, terrain_seam and
terrain_hole_lod. The route suite checks default and all eight entrance extrema,
actual room/passage clearances, offset floor/ceiling collision, curved descent and
restored ascent. Browser terrain smoke also checks camera x offsets and the deep
explorer checkpoint on WebGPU/WebGL2; Windows --smoke covers Dawn/direct D3D12.
All five focused suites and all four renderer flows passed. See
[curved layout evidence](references/hybrid-terrain-curves-validation.md).
These development-demo changes do not advance CatGame's engine dependency.

## Registered gameplay-component replication (M7)

The current optional NetworkScene profile supports application-registered custom
Component state on browser and native Windows, with shared editor creation,
properties, replication audience, undo/redo, duplication, save/load and isolated
Host/Join. Read [component contracts](references/multiplayer-components.md) and
[runtime evidence](references/multiplayer-components-validation.md). M1-M6 inventories
above describe their original stages; MP07 and the registered profile supersede
their static-only/current-schema claims. The example registers Vitals and Progress;
generic tests also register an independent inventory class. No game rules enter
engine_network/core. This does not advance CatGame's engine dependency.

Include engine/network_components.hpp and link WebEngine::NetworkScene. Add each
ReplicatedComponentType to an owning ComponentReplicationRegistry: existing unique
ComponentCodec (detached factory, no resolve), nonzero state version, declared
maxStateBytes, capture/validate, staged prepare and detached replica setup. Registry
copies own callbacks; captured dependencies must outlive every copy. Data is opaque
bounded bytes, never C++ object memory, runtime handles or executable code.
prepare cannot mutate live state. PreparedComponentUpdate owns all allocations;
commit() noexcept must not allocate, invoke callbacks or change topology. Validate
and prepare the complete snapshot before any pose/spawn/component publication.
Plans borrow live targets only for the immediate idle-owner transaction.

ComponentReplication Off/Everyone/OwnerOnly is persisted in ordinary reserved
Component tags through setComponentReplication; unknown/duplicate/malformed tags
reject network publication and remain removable in generic scenes. Extend the
editor property registry with componentReplicationProperties(registry,base).
Pass EditorComponentSupport{codecs,factories} to EditorSession; factories supply a
stable key/label and detached childless registered Component. DesktopEditor accepts
optional property registry/factories after codecs; browser app setup configures
equivalent support before binding. Preserve this support on reopen and Play.
Registered archive bytes participate in structural history fingerprints.
SceneArchive::duplicateSubtree/cloneSubtree accept optional custom codecs; used
resolve callbacks reject subtree copies, while full-scene load/clone keeps its
existing reference-resolution support.

NetworkScene save/load accept the same registry and retain its copy. WES3 checks an
exact canonical type/archive-version/state-version/max-size manifest before archive
factories. Ordinary scene archive versions do not change; old unregistered WES2
packages still load. Scene example schema is MP07 (0x4d503037): rebuild server and
clients together. WEN1 version 1 and M1 MP01 remain unchanged.

Only childless registered components directly beneath enabled replicated root mesh
actors are admitted. Topology remains fixed. Canonical slots/type keys/child indices
are content identity; bindComponentHandles adds generation-checked ComponentIds
per adopted instance, rejecting removed/replaced components. At most 32 types and
8 components/actor (including Off) are supported. Declared total state must fit
256 bytes: 36 pose +4 acknowledgement +4 count +16 per active component +state.
Manifest <=8192 bytes; existing package/peer/entity limits remain.

capture/encode/decode/validateComponentStates and prepareComponents handle bounded
fields and enabled flags. prepareComponentReplicas invokes host passive setup on
detached clones and disables variable/fixed ticks, including Off/hidden components.
Replica lifecycle callbacks must respect this setup. TransformReplicas takes codecs
and an optional synchronous Stage hook for prepared state after the first 36 pose
bytes; the default strict transform-only API is preserved. Scene owns live actors
and must outlive adapter calls. Clear instance bindings on despawn/re-entry/replace.

The authoritative host filters per-component audience before snapshotForRecords,
which checks projected IDs/owner/kind against the full authority registry and uses
existing delta/keyframe/resync. Missing owned records are included automatically;
explicitly project their payloads when appropriate. Command authority is independent
of projection/relevance. Client intents are validated by server game code; clients
cannot write authoritative fields. Authored bootstrap is public: OwnerOnly protects
runtime updates, not secrets in initial data. Server owner 0 has no private recipient.
Prediction/smoothing applies only to poses; no generic component prediction occurs.

Build native network_component_tests/network_scene_tests/multiplayer_scene_server/
webengine_editor and browser network_component_tests/network_scene_tests/network_tests/
web_assets with the pinned local Emscripten SDK. Run network_components, network_scene,
network, multiplayer_arena, desktop_editor, editor_content, editor_structure and
persistence CTests plus Node/Wasm component/scene suites. Run
scripts/multiplayer-scene-smoke.cjs against fresh dist for WebGPU/Dawn and
WebGL2/direct D3D12 with real applied fields, owner-only updates, authoring round
trips, both publishers and earlier prediction/recovery/lifecycle checks. Native
authoring acceptance is EditorSession API-driven plus saved-file reopen; browser
authoring uses actual controls. Do not claim native field pointer/keyboard or
physical mobile acceptance from these tests. Runtime component composition, generic
prefab spawning, RPC/events, physics rollback, ownership transfer and internet
session services remain separate work.

Archive validation/load performs ordinary lifecycle synchronization. Keep constructors and load-time callbacks safe before explicit host authority assignment; gate side effects/subscriptions on application roles. The detached replica hook does not configure earlier archive-validation documents.

## Terrain mesh lakes and finite water (2026-10-23)

Include engine/terrain_mesh.hpp and call buildTerrainMeshLake with a connected
upward-facing floor anchor, a 3..16 point strictly convex CCW XZ outline, local
water level, depth and bank width. Immutable output contains terrainMesh, waterMesh,
WaterRegion and waterLevel. Shared refinement preserves material/UV partitions
and protected source attributes, locks the shoreline/anchor for LOD, and clips
water to the actual submerged bed. Defaults: .12 m edges, .45 m maximum depth,
.3 m bank, 30k terrain triangles/90k vertices/64 parts, 8192 water triangles,
8M steps (max 64M), up to 3 reducing LOD levels. Concave outlines and protected
overlap reject; input remains unchanged. Publish terrain/collider/water together.

WaterRegion is a finite convex prism at surface-local Y=0, with negative bottom.
setWaterRegion in engine/water_region.hpp annotates the water MeshComponent using
bounded versioned ordinary tags; archives/clones/history retain it without a new
codec/format. Unknown/malformed reserved tags reject. Missing region preserves
legacy rectangular bottomless water. Underwater membership, CPU ray distance and
WGSL/GLSL/HLSL fog/caustic clipping share the polygon and finite bottom, including
transformed regions. The prism does not exactly model sloping bed volume; ground
collision supplies the bed. Existing first-match water and one reflection plane
per view remain. No swimming, buoyancy or fluid simulation is introduced.

The browser/Windows cave demo prepares a .45 m deep, irregular pond at -3.08 m in
the Grand Chamber through examples/hybrid_terrain/pond.hpp. Cave pond/Underwater
views inspect it. Entrance/hole edits stage both meshes and metadata; undo/redo and
checkpoint replacement retain them. Old checkpoints keep their geometry. Browser
water uses Application simulation seconds; native DesktopHost already does.
Run terrain_lake and terrain_lake_cave plus terrain_mesh/water/seam/LOD/archive
and route suites, Node/Wasm lake/formation tests, and full browser WebGPU/WebGL2
and native Dawn/direct D3D12 cave smokes with pond surface/submerged/below-region
captures. See [lake contracts](references/terrain-mesh-lakes.md) and
[validation](references/hybrid-terrain-lake-validation.md).
This update does not advance the CatGame engine pin.

## Mesh-shaped terrain cuts and enlarged grotto (2026-10-23)

Development engine_core exposes buildTerrainMeshHoleMask / cutTerrainHolesFromMesh
in engine/terrain_mesh.hpp. Pass an authored static entrance mesh with closed
terrain-contact boundary loops and an affine meshToTerrain transform. Preparation
uses uncut source heights and base geometry before seam/sculpt/LOD preparation;
it supports concavity, disjoint openings and nested islands through conservative
cell rasterization. Keep independent manual-hole ownership and publish the returned
immutable terrain edit through existing idle boundaries. Closed/buried meshes
make no cut; patches, partial rims, crossings, invalid data and exhausted budgets
reject. This is not arbitrary closed-volume Boolean excavation. Read
[mesh hole contracts](references/terrain-mesh-holes.md).

The shared cave demo now has a roughly tenfold-volume Lower Grotto, scaling width,
height and length by cbrt(10). A dry rock basin composes existing connected-floor
lake preparation, publishing only terrain geometry with full-base collision and
shoreline LOD locks. The dry walking centreline remains unchanged; the complete
route, lights and Lower grotto / Grotto basin views follow the larger room.
HYBRID10 saves the enlarged geometry and basin view while retaining older readers
and exact baked legacy geometry until an edit. This is application content, not
an engine placement rule. Read [hybrid terrain](references/hybrid-terrain.md).

Build browser hybrid_terrain_demo and matching webengine host/worker with the
repository Emscripten 6.0.11, plus native webengine_hybrid_terrain_windows. Run
terrain_mesh_holes_tests natively and under Node/Wasm, the nine focused terrain
CTests, hybrid-terrain-smoke.cjs on forced WebGPU/WebGL2, and native --smoke on
Dawn/direct D3D12 with verify_hybrid_terrain.py. Validate basin collision, dry route,
full descent/ascent, saved geometry/LOD, failed-load preservation and editing.
See [validation](references/hybrid-terrain-grotto-validation.md); distinguish actual
executed evidence from pending checks and do not infer physical-mobile acceptance.


## Open-path terrain cliffs (2026-10-23)

Include `engine/terrain_cliff.hpp`; `buildTerrainMeshCliff(*terrain, path, rock,
settings)` prepares a `TerrainCliffResult` containing a paired TerrainEdit, ordinary
static mesh and independent cut footprint. The left of the directed X/Z path is
high. Defaults: elevations 5/0 m, angle 85 degrees (60–89 supported), shoulders
1.25 m, end fade .75 m, blend width .75 m, interior edge .5 m, vertical rock relief
.08 m, seed 17, 3 constrained LOD levels. Open paths have 2–16 points, gentle turns
below 60 degrees and no crossings/close non-adjacent approaches. Full-source assets
only; the grid-aligned rectangular collar must fit within the terrain and contain
no existing holes or differing per-cell physical surfaces. Compatible opaque solid
or unweighted single-layer triplanar materials only. Defaults cap source cells at
262144, cut cells at 4096, output at 30k triangles/90k vertices, and preparation at
8M work steps (max64M). LOD=0 disables derived geometry.

Every original perimeter edge, height and normal remains matched to the source.
Shared-edge feature insertion/refinement has no T-junctions. Open ends/shoulders
fade into original terrain; continuous signed distance avoids discontinuities at
bends. Ground/rock blending uses existing triplanar material rendering and the
same private material-layer conversion as seam preparation. Lip/toe neighbourhoods
and boundary blend vertices are locked through LOD. Collision always uses the
base geometry; use physics queries for the replaced cells, not heightfield-only
terrainGround/terrainRaycast. Stage collision and publish the edit/mesh together
at an idle scene boundary. Preserve mesh-owned footprint independently when
recomposing brush/cave masks. Failure leaves all published inputs unchanged.

The builder owns no scene, subscriptions, storage or automatic rebuild policy.
No overhangs, closed mesas, arbitrary mesh Booleans or modifier stack are provided.
Ordinary scene archives retain baked geometry/materials/collision and LOD recipes;
save path/settings separately when nondestructive parameter editing is needed.
Both hybrid demo hosts expose **Cliff / terrain join**; the existing blend control
affects cave and cliff. Restore brushes skip cliff-owned cells. HYBRID9 adds cliff
binding and later combined demo envelopes retain it; earlier geometry stays valid.

Run terrain_cliff_tests on Windows and Node/Wasm, plus hybrid_terrain,
terrain_lod_archive, terrain_mesh, terrain_seam, terrain_hole_lod and
hybrid_cave_route. Run scripts/cliff-terrain-smoke.cjs on forced WebGPU/WebGL2,
the broader hybrid terrain smoke, and native --backend dawn|d3d12 --cliff-smoke
<output-directory>. tools/verify_cliff_pixels.py checks native detail, visible
blend changes and stable restored frames. See [contracts](references/terrain-cliffs.md)
and [evidence](references/terrain-cliffs-validation.md).
This skill update does not advance any consumer's engine dependency.

## Planetary terrain stage 1 (2026-10-23)

Optional `WebEngine::Planet` prepares a bounded procedural cube-sphere quadtree
with double planet-centred positions, camera-relative ordinary meshes, parent
morphs and visual seam skirts. The shared browser/Windows example supports
continuous guided and manual space-to-surface flight. Source-height clearance is
demo policy; collision/landing, planetary atmosphere/oceans and persistence remain
future work. See [planetary terrain](references/planet-terrain.md) for exact APIs,
defaults, ownership, builds, stage limits and per-target validation. Verify this
development API in the selected engine checkout before using it in a pinned game.

# Paws architecture

The phase-4 `paws_garden` library owns imported kit validation and explicit assembly
composition. `garden_preview` is an isolated Paws renderer acceptance executable;
its manifest-generated C++ data supplies asset IDs, transforms and anchors without
assuming GLB node hierarchy becomes actors. See [the kit contract](garden-kit.md).
It uses Application asset publication/clocks and checked scene handles; the host
owns HTTP/input/lifecycle events. Editable sources stay outside runtime staging.

Paws owns game-specific C++ code in `src/`, authored content in `assets/`, and its
browser shell in `web/`. WebEngine remains a separate repository/submodule.

`paws_rules` owns portable puzzle state, typed commands, revisions, bounded undo
and deterministic routes, with only standard C++ dependencies. `paws_content`
authors the missing-ball, tidy-toys and stuck-gate layouts and known solutions.
`PawsSession` owns one authoritative `GameState`; `Afternoon` owns three independent
attempts, ordered unlocks, the reward ledger and validated local snapshot codec.
No browser code determines move correctness.
See [the implemented content contract](phase1-content.md).

`paws_game` is the portable scene/presentation library, linked to `WebEngine::Core`
and the rules/content targets. `SceneBindings` map stable object IDs to checked
actor handles and project committed revisions; they never retain actor pointers.
`paws` is the Emscripten executable. It links `WebEngine::Browser`, owns the
browser frame loop, and exposes the small camera/lifecycle host interface.
The native configuration builds the portable library and its scene test; it does
not provide a native rendering backend or desktop game.

The runtime's `Application` owns the scene and drives variable/fixed updates.
The scene owns actors, mesh/light components and the camera. The orbit controller
and renderer borrow that scene's objects and are destroyed before `Application`.
Visibility/page lifecycle events suspend the application without advancing hidden
simulation time. Rendering recovery is serviced at frame boundaries.

The original starter/greybox scene remains an integration fixture and the initial
backdrop before production dependencies are ready. Play/Continue prepare the
appropriate production garden or puzzle scene. Help Pip prepares the selected
board. `garden_scene` composes a separate authored courtyard and three stable
furnishing spots using accepted immutable kit meshes. The three boards reuse routes, shaped toy sockets,
reachability and live switch/gate occupancy. Failed actions preserve committed
state. Commands carry attempt ID and expected revision; restart and cross-quest
activation invalidate older commands. Garden navigation suspends the attempt,
preserves its identity and publishes the courtyard after preparation. Phase 5
integrates the complete first-quest loop, canonical portrait, happy reaction and
explicit saved wallet/ledger/command identities over the phase-6 owner. Both use
`Afternoon` and WebEngine IndexedDB storage. Phase 7 adds the authoritative shop,
owned furnishings, placement transactions and once-only ending record to that
same owner. `furnishings.hpp` shares catalog prices, stable spots and footprints
between portable rules, GUI projection and courtyard assembly.

The version-5 `Afternoon` snapshot retains explicit portable attempt/revision
identities, wallet and reward records plus bounded command journals, owned items,
placements, ending status, snapshot/session revision, explicit user pause and
display-quality preference. Restore replays and checks the journals, purchases,
wallet, occupancy and ending eligibility; versions 1 through 4 remain readable.
IndexedDB acknowledgements publish Saved only for the latest
snapshot. The Paws browser coordinator coalesces moves, keeps five complete history
slots, holds a Web Lock for the normal save, isolates other sessions, and offers
validated file import/export with original-byte recovery. Hidden/page-away/frozen
reasons compose without clearing user pause. See [the durable save contract](phase8-saves.md).

`paws_ui` owns the game widget composition and copied `UiModel` projection. Typed
`UiIntent` commands retain attempt/revision checks through `PawsSession`. The
runtime detaches the renderer and semantic adapter before rebuilding the GUI at
frame boundaries. Modal sheets pause scene simulation and retain navigation/focus
return; domain navigation clears obsolete modal paths. A bounded completion
reaction permits Application's normal animation tick while the reward card is
actionable. Reduced motion samples a static pose. Each host pointer contact
retains one GUI/camera owner. Local font atlases,
original code-native skins/icons, portrait/landscape reflow and larger text share
engine logical coordinates. HTML is limited to startup/recovery. See the
[GUI integration contract](gui-integration.md) for later-phase bindings and gallery
isolation. The gallery is compile-gated OFF by default.

Runtime asset staging copies only `assets/runtime/` into site-relative `assets/`.
Editable masters and raw/rejected generations belong in `artifacts/`. The consumer
bridge and engine gitlink remain unchanged by phase 1. Later-phase code may be
present concurrently; its acceptance is tracked separately in implementation status.

## Phase 10 presentation and release boundary

Paws applies explicit resolution/shadow budgets through the pinned
`Renderer::setQuality`: Automatic chooses the conservative Mobile profile for
narrow, short or coarse-pointer displays; manual Mobile/Desktop override it.
Quality affects presentation only and persists in the same snapshot.
GPU recovery continues independently of scene advancement. Pre-render
application/GUI/audio wall time is passed into renderer measurements.

`cmake/BuildInfo.cmake` fingerprints the actual Wasm, runtime host/adapters and
runtime asset tree after staging. Uncommitted consumer work is identified by
payload rather than an unavailable game commit. The local release packager
validates those fingerprints and model catalogue hashes, copies the game
shell/runtime/assets/notices, and excludes developer preview pages and binaries.
Its manifest records remaining acceptance gates. See [phase 10](phase10-release.md).

## Engine integration

### Paws audio ownership (phase 9)

`paws_audio` composes the pinned `AudioSystem` with Paws' cue catalog, cooldowns,
variants, Music/Ambience/Effects buses, decode/voice budgets and bounded loop
policy. `BrowserGame` owns it and translates committed rules and presentation
markers into sound requests. It stops scene voices and invalidates pending audio
generations before restart, save import or scene replacement. Sound never grants
rewards or decides a puzzle result. Saved settings remain in `Afternoon`.

`web/paws-audio.js` is the host adapter for serial fetch, hash checks, engine
decoding, Wasm publication and gesture/lifecycle status. The device and playback
backend remain in `engine_audio_browser` and the staged `engine-audio.js`.
There is no engine fork or pin change. The manifest currently lists missing
production files; the integration cannot establish final-audio acceptance.
See [the phase-9 contract](phase9-audio.md) for budgets and production handoff.

The default `WEBENGINE_SOURCE_DIR` is `external/WebEngine`, pinned through a Git
gitlink. The local override is developer configuration only. Each mode has separate
build directories so dependency switching cannot accidentally validate the wrong
engine.

The published engine currently compiles the browser renderer into its playground,
and its demo/test copy commands assume a top-level engine project. Paws includes
the engine via `add_subdirectory(... EXCLUDE_FROM_ALL)` with engine tests disabled
in a function scope, then builds an independent browser renderer library from the
engine's implementation files and generated shader headers. No demo publication
target is built, so its top-level paths are never executed. All game site output
is contained in the Paws binary directory. Core library definitions and third-party
dependency pins remain owned by WebEngine.

The packaging bridge handles the current published renderer and the split backend
files in the adjacent development checkout. It uses an upstream
`WebEngine::Browser`/`engine_browser` target automatically once available. This
compatibility code should disappear after an upstream packaging pass provides:

1. A reusable browser-renderer target with transitive port/compiler/link flags.
2. Demo/test options defaulting to off when WebEngine is a subproject.
3. Engine-relative paths for all fixtures, shaders, assets and dependencies.
4. A consumer-safe site/license staging API and optional platform adapters.

WebEngine's Jolt dependency is fetched at its pinned version/hash during first
configuration. Browser site staging copies the linked third-party license notices.
The renderer currently requires `#viewport` and the engine status/statistics host
callbacks; `web/host.js` supplies that contract without copying the playground UI.

## Phase 3 character asset boundary

`paws_pip` is an optional portable Paws library for the canonical Pip asset,
validation, clip-name lookup and presentation selection. `pip_preview` is an
isolated browser executable linked to the pinned renderer and
`engine_assets_browser`. It publishes `/pip-preview.html` beside the game for
asset acceptance without replacing the production game/GUI flow. Its host owns
HTTP/status and browser events; `Application` owns publication, scenes, clocks
and animation ticks. Commands resolve a scene-scoped component handle. Borrowed
renderer/controller references are released before scene destruction.

The walk inspection lane also uses Application simulation time. `pipWalkStride`
and `pipWalkDuration` define the constant speed used by garden traversal; the
skeletal root remains stationary within the clip. The preview's scrub/view
controls are development tooling. See the [walk evidence](../artifacts/model-jobs/pip/2026-10-11-reference-walk/README.md).

Pip's GLB and same-model portraits live in `assets/runtime/models/pip/`; editable
masters, generation sources, scripts and correction records stay under
`artifacts/model-jobs/pip/2026-10-10-tripo-repair/`; the original canonical rig
authoring job remains under `2026-10-10/`. The manifest is authoring metadata, while
`pipActions` is the explicit runtime action contract. No animation event grants
progress or chooses a puzzle result. See [the asset contract](pip-asset.md).

## Phase 6 reusable puzzles and progress

See [the phase-6 contract](phase6-puzzles.md) for schema-2 roles, reachability,
shape matching, live switch/gate occupancy, command replay saves, generation
checks, production asset projection and the remaining phase-5/7/8 boundaries.
Browser scene preparation is cancellable and publishes at frame boundaries.
The game links the existing `paws_garden`/`paws_pip` libraries; all production
meshes remain immutable. No WebEngine source or dependency pin changed.

## Authored ball traversal

A successful Roll projects the validated route through straight sections and
sampled quarter-circle channels. `route_traversal.hpp` supplies a reusable,
distance-based sampler; puzzle presentation chooses speed (1.6 m/s), ball radius
and rolling rotation. It is shared by missing-ball and switch/gate boards.
The ball has one transform writer during traversal. Selection/hint revisions
preserve a running route; undo, restart, scene replacement and nonanimated restore
cancel it. Reduced motion immediately samples arrival. The first quest keeps its
board visible until arrival, then starts Pip's reaction and opens the reward card.
Logical success/rewards still commit and save atomically when Roll is accepted;
reload restores the destination without replaying a temporary animation.

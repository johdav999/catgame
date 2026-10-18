# Paws architecture

Puzzle courtyards use metre-scale bounds independently of their interaction
grid. Button's cushion basket retains 25 cm cells for fitting and dragging;
that local scale must not shrink the surrounding paving, enclosure or house
setback in `release1/integration/puzzle_scene.cpp`.

The optional [Loopia games platform](loopia-platform.md) owns cross-game accounts,
opaque versioned cloud snapshots and verified Stripe purchase records in a
separate PHP/MariaDB backend. Paws keeps gameplay/local saves and validates cloud
loads through Afternoon's existing staged import path. Browser host adapters own
network/UI policy; account entitlements are transient and never sourced from a
game save. The engine and dependency pin are unchanged.

The `beach_afternoon` development adventure contains six sequential quests and a
saved four-cat picnic ending through the existing Afternoon owner, portable
rules, retained GUI, scene adapter and browser save coordinator. Version-11 saves
retain six independent journals and the beach ending; versions 1–10, including
the first beach slice, migrate without resetting progress. `?devBeach=1` supplies
transient access; local saves never grant ownership. Cove landmarks project the
beach completion mask. Beach progress does not alter base coins, friendship,
chapters or garden picnic eligibility. See [the beach contract](beach-adventure.md).

The catalogue now includes two optional chapter-three families: six-stage scent
deduction and four-stage discrete torque balancing. Both use the existing
portable rule owner, retained GUI, scene adapter and journal/reward pipeline.
Version-seven saves append their attempts; version-six saves initialize them
without changing earlier attempts or picnic eligibility. See the
[scent and balance contract](trail-and-balance-quests.md).

Garden ground uses the selected WebEngine's immutable heightfield terrain and
chunked LODs when available. Paws owns hill heights, flat house/garden footprint,
ImageGen texture preparation and material weights in `garden_terrain.cpp`.
Enclosure ownership includes terrain across chapter transitions and quests;
terrain is decorative and introduces no gameplay, physics or save owner.
The existing pin retains lawn slabs until terrain support is deliberately pinned.
See [garden terrain](garden-terrain.md) for dimensions, API and verification.

Cat movement composes WebEngine's Jolt shape queries through the game-owned
`CatCollision` adapter. Invisible static solid proxies follow live prop geometry;
walking, detours and supported entry/leave paths are swept before position writes.
Physical arrival gates use animations. Puzzle correctness, scripted prop motion
and saved progression remain with the existing portable owners. See
[cat collision and furnishing travel](cat-collision.md) for geometry, lifecycle,
contact policy and scoped verification.

The Tidy toys basket opts into a conservative paw-clearance footprint for
approach planning. Its completion endpoint is derived from the visible rim,
with enough stand-off for the full animated lower-leg silhouette beyond the
root-centred body capsule. Restoration and reduced motion use the same contact.

Directional cat turning is game-owned in `pip_asset.cpp`. Each actor owns a
small `CatTurn` component; `CatCollision::follow` applies a shortest-arc heading
step capped at 3 radians/second while preserving collision-authoritative travel.
Garden presentation and sampled quest endings blend the existing walk with
`turn_left` / `turn_right` on the same 0.8-second gait clock. Arrival facing
finishes before furnishing or quest contact actions begin. Animation never
moves the root or commits progress. Reduced-motion projections and resets
place the final pose directly; pause does not advance heading or blend weight.
See [turning assets and verification](../artifacts/model-jobs/cat-turns/2026-10-14/README.md).

## Release-one production composition

Garden newcomers use `Presentation`'s scene-scoped arrival queue and swept cat
movement through the open enclosure gate. Chapter eligibility controls entry;
existing saved introduction flags distinguish arrivals from residents. The
browser commits the ordinary `Afternoon::introduce` intent after physical
arrival (or a request click during entry), then uses the normal save coordinator.
Arrival animation owns no rewards or progression. Simulation pause preserves
its route, reduced motion projects the final home pose, and scene replacement
discards transient travel. The garden HUD has no arrival dialogue or Meet button.

Quest completion keeps the world visible and presents game-owned 3D coin and
friendship-heart rewards above each requesting cat. `QuestRewards` in
`quest_rewards.cpp` holds scene-scoped actor handles and shared imported assets;
it owns only the 1.2-second coin / 2.2-second heart presentation and one-shot cue
edges. The browser starts it from the authoritative once-only reward receipt
after the existing cat reaction, supplies Application presentation delta, and
cancels it on undo, restart or navigation. Pause/suspension freeze it; reduced
motion shows still tokens. Completed-save loads and practice cannot replay a
reward grant. A compact receipt and Back to garden control replace the modal.
The original Afternoon wallet, friendship ledger and save codec remain the
owners. See [quest rewards and asset evidence](quest-rewards.md).

Quest water uses the selected WebEngine's `MaterialShader::Water` through
game-owned immutable material overrides in `water_surface.hpp`. Validated gutters
and the inlet retain their authored geometry with animated ripple normals; filled
drinking and picnic bowls use millimetre-scale waves at the kit's water anchor.
Purchased garden bowls share the same presentation. Route legality, ordered flow
reveal, bowl fill, undo/restart and rewards remain owned by the existing rules
and scene projection. Small surfaces leave refraction, foam and planar capture
disabled. The host supplies simulation seconds to `Renderer::setWaterTime` once
per frame; pause and suspension freeze it, and reduced motion selects zero.
`PAWS_WATER_SHADER` is enabled only when the selected engine provides `water.hpp`;
the current pinned engine keeps its imported material fallback. This integration
does not advance the engine gitlink. `paws_water_snapshot` exposes active shader
surfaces and the renderer clock for browser acceptance.
See [quest water presentation and verification](quest-water.md).

Release-one Prompt 1 freezes IDs, asset interfaces, portable intent/read-model
DTOs and disjoint module ownership in [release1/contracts.md](release1/contracts.md).
`src/release1/contracts.hpp` is value-only; it introduces no state owner.
The reusable `PictureSequence` family owns ordered response validation, replay
and undo through the existing attempt/journal. Its scene-local clue player uses
Application presentation time and checked actor handles. Reload replays the clue
while retaining the persisted answer prefix. See [picture-sequence quests](picture-sequence-quests.md).
Prompt 9 registers the delivered modules in the actual `paws` target. Production
now composes ten quests, four separately animated r24 cats, three chapters,
per-cat friendship, eight purchases/placements/uses and the independent picnic
ending. `PAWS_AFTERNOON 6` validates/migrates supported versions 1–5. Asset/audio
acceptance is tracked separately: missing final sound files do not become silent
placeholders or a production-ready claim. See [the integration handoff](release1/prompt-09-handoff.md).
`tools/release1-track` remains an isolated development harness; its copied owner
fixtures and preview entry points are excluded from production. The existing
Application, Afternoon and save coordinator retain their ownership.

The phase-4 `paws_garden` library owns imported kit validation and explicit assembly
composition. `garden_preview` is an isolated Paws renderer acceptance executable;
its manifest-generated C++ data supplies asset IDs, transforms and anchors without
assuming GLB node hierarchy becomes actors. See [the kit contract](garden-kit.md).
The reference-detail kit adds eleven Tripo-derived meshes. `addGardenEnclosure`
composes fourteen non-selectable perimeter instances under each chapter boundary;
the bench cushion is composed with the existing bench. Their asset preparation
uses the same garden catalogue and importer validation. A native vertex-clearance
check keeps decorations outside puzzle cells; they carry no puzzle/save state.
It uses Application asset publication/clocks and checked scene handles; the host
owns HTTP/input/lifecycle events. Editable sources stay outside runtime staging.

Exterior shrubs share an immutable three-level mesh chain assembled by
`addGardenEnclosure` from the accepted garden catalogue. Derived geometry reuses
the base material/texture bindings; screen-size selection and batching use the
existing WebEngine APIs. Dense planting covers the inner two metres; farther
cells are thinned and use broader clumps. Solid foliage culls back faces while
open leaf fragments remain two-sided. See [foliage rendering evidence](foliage-performance.md).

Paws owns game-specific C++ code in `src/`, authored content in `assets/`, and its
browser shell in `web/`. WebEngine remains a separate repository/submodule.

`paws_rules` owns portable puzzle state, typed commands, revisions, bounded undo
and deterministic routes, with only standard C++ dependencies. `paws_content`
authors the missing-ball, tidy-toys and stuck-gate layouts and known solutions.
`PawsSession` owns one authoritative legacy `GameState`; `Afternoon` owns its
three legacy sessions and seven new deterministic attempts, chapter unlocks,
the once-only reward ledger and validated local snapshot codec. It derives the
wallet and friendship; neither presentation nor animation can award progress.
No browser code determines move correctness.
See [the implemented content contract](phase1-content.md).

The first five quests also have version-2 layouts with shared parking, ordered
basket access, a second gate stage, three-crate ordering, and competing bridge
routes. Authored approach cells, gate-dependent parking, and packed-toy occupancy
are portable rule data. Save replay selects each recorded content version;
existing attempts retain their original layouts and history. Presentation uses
the same content identity for coordinates, framing, and interaction targets.
See [quest extensions and validation](quest-extensions.md).

The playhouse's version-2 cooperation sequence uses the same portable attempt:
continuous pressure switches, a crate helper, two gates and an undoable inside
latch. An explicit versioned Restart can upgrade its old attempt; the save
journal retains its initial content identity and replays across that recorded
boundary. Scene transforms never decide completion. See
[playhouse cooperation](playhouse-cooperation.md).

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

The original starter/greybox scene remains a portable integration fixture.
Production starts with an empty preparation view. Play/Continue prepare accepted
assets before candidate activation; selection prepares the selected board before
publishing its owner state. `release1/garden/presentation` composes one courtyard,
nine stable furnishing spots and independent cat playback. `release1/integration`
projects seven new boards using accepted props, including normalized rotated
cushion origins, checked route surfaces and simultaneous switches. The legacy
three boards reuse routes, shaped toy sockets,
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

The version-6 `Afternoon` snapshot retains explicit portable attempt/revision
identities, wallet and reward records plus bounded command journals, owned items,
placements, ending status, snapshot/session revision, explicit user pause and
display-quality preference. Restore replays and checks the journals, purchases,
wallet, occupancy and ending eligibility; versions 1 through 5 remain readable.
The historical MVP ending and new picnic record are independent. A detached
garden candidate validates four accepted models, contacts and all four seat
reservations before the host publishes the picnic record. Supplied celebration
props appear when picnic furnishings have not been bought. Replay awards nothing.
IndexedDB acknowledgements publish Saved only for the latest
snapshot. The Paws browser coordinator coalesces moves, keeps five complete history
slots, holds a Web Lock for the normal save, isolates other sessions, and offers
validated file import/export with original-byte recovery. Hidden/page-away/frozen
reasons compose without clearing user pause. See [the durable save contract](phase8-saves.md).

`paws_ui` owns the game widget composition and copied `UiModel` projection. Typed
`release1::Intent` commands carry generation/attempt/owner revision through
`Afternoon::dispatchIntent`; legacy physical controls map to the same path. The
browser adapter owns generation-checked scene gestures for all ten requests.
Mouse-up clicks dispatch rotation, retrieval, source release or bell taps;
dragging previews actor transforms without changing puzzle state. Drops preflight
the complete slide/pick-up/placement sequence against a copied owner before
dispatching fresh typed commands. A legal Connect action completes plank,
switch and picnic arrangements after placement. Rules remain authoritative.
The production GUI exposes instructions and feedback, with Undo/hints/restart
in Menu; it no longer presents numbered object or quest-action sheets.
Loose planks/cushions can rotate independently; turning a fitted piece removes
its socket occupancy until it is fitted again. Existing saved action journals
remain supported by the same version-6 owner. The runtime detaches the renderer
and semantic adapter before rebuilding the GUI at
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
`scripts/release1/integration/assets.py` merges accepted fragments, verifies exact
bytes/hashes/rights records and emits accepted registration/contact inputs.
Cat-specific surface measurements align rest/drink/scratch contacts; authored
perch/entry paths and reservations remain presentation. The production shell
is staged explicitly; galleries, track HTML and preview binaries are excluded.
The default browser build disables asset preview targets. Puzzle and garden cameras
fill the canvas beneath the retained GUI; the finale keeps its gathering visible beside
or above the closing card. Garden schedules run once in Application simulation.
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

The temporary consumer packaging bridge includes optional backend water
translation units when the selected engine checkout supplies them, alongside
the existing renderer facades. This supports the configured development engine
without changing the pinned revision or copying engine implementations.

### Daytime sky presentation

The browser coordinator applies the same daytime sky policy to the preparation
view and every candidate scene before publication, including garden, all ten
requests, restart, restored saves and picnic presentation. It uses WebEngine's
scene-owned atmosphere sky with `SkyGeometry::Sphere`, sparse static clouds and
a pale blue nadir for the elevated miniature-garden camera. The atmosphere sun
direction matches each scene's existing key light. Automatic day/night lighting
is disabled: ambient, key light, exposure and gameplay clocks retain their
existing owners. The engine draws its camera-relative sphere outside picking,
physics and shadow collection; no game actor, imported asset or save field is
added.

In the garden, continuing an upward background drag beyond the orbit horizon
tilts the camera in place, up to 66 degrees above the horizon, so WebEngine's
atmospheric sun disc can be viewed. The orbit position stays above the ground;
horizontal orbit and zoom retain the upward gaze. Dragging back down resumes
ordinary orbit. Reset view, scene transitions and viewport reframing clear this
transient game-owned look offset. Puzzle orbit limits remain unchanged.
Verify with `node scripts/garden-sun-smoke.cjs` against the local-engine site.

This requires the development engine's `engine/sky.hpp`. CMake reports whether
the selected checkout supplies the feature; the current pinned revision retains
its clear-color background. The verified sky build uses the explicitly selected
adjacent WebEngine checkout in `build/web-local-release`, separate from pinned
builds. Advancing the gitlink awaits an upstream commit of the sky API.

### Paws audio ownership (phase 9)

The application configures an ordered, repeating Golden Afternoon Garden playlist
from two user-supplied tracks. `GameAudio` preserves its current background voice
across scene changes, independently of scene-scoped effects, and loads one music
track at a time. Pending decodes still require the current generation. See the
[music delivery and memory limits](garden-music.md).

`paws_audio` composes the pinned `AudioSystem` with Paws' cue catalog, cooldowns,
variants, Music/Ambience/Effects buses, decode/voice budgets and bounded loop
policy. `BrowserGame` owns it and translates committed rules and presentation
markers into sound requests. It stops scene voices and invalidates pending audio
generations before restart, save import or scene replacement. Sound never grants
rewards or decides a puzzle result. Saved settings remain in `Afternoon`.

`web/paws-audio.js` is the host adapter for serial fetch, hash checks, engine
decoding, Wasm publication and gesture/lifecycle status. The device and playback
backend remain in `engine_audio_browser` and the staged `engine-audio.js`.
`BrowserGame` supplies a second engine backend for at most two UI feedback
voices. User pause suspends the world device while menu clicks/rustles remain
audible; host suspension stops both. `GameAudio` shares immutable decoded assets,
master mute and Effects settings and enforces the combined 14-voice limit. UI
voices contain no scene references and survive a world/pause generation change.
Small effects preload again after a cancelled scene decode. The portable
`release1::audio::effects` projection maps accepted before/after DTOs to cues,
including actual switch/gate press/release edges. It never commits rules.
There is no engine fork or pin change. The manifest currently lists missing
production files; the integration cannot establish final-audio acceptance.
See [the phase-9 contract](phase9-audio.md) for budgets and production handoff.

Garden ambience can be playable before final release acceptance. The host accepts
explicitly `playable` clips only with recorded technical validation, PCM bounds,
provenance and SHA-256 checks; `accepted` continues to mean final review and rights
are established. Packaging completeness still requires accepted audio. A portable
`GardenBirdSchedule` advances only during audible garden presentation, rotates the
three bird variants through GameAudio, resets on scene replacement and freezes on
pause, mute, zero ambience volume or host suspension without catch-up bursts.

Pip's ten generated effects use the same playable-versus-accepted delivery
policy. A portable `PipSounds` projection observes his already-simulated phase
and resolved actor position, emits stride-distance paw contacts and one-time
cushion/bowl cues, and requests a bounded rest purr. It owns no scene pointers or
game rules. Scene replacement resets this projection; zero simulation delta
preserves contacts across pause. Requests wait for gesture resume after loading
and cannot cross an audio generation. See [Pip sound delivery](pip-audio.md).
The remaining 49 delivered sound effects, distinct bell pitches and real-game
verification are recorded in [remaining sound effects](remaining-sound-effects.md).

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
For the little gate, the trigger plate and gate begin their motion only after
the ball reaches the route endpoint. Their opening sounds follow that arrival;
the browser rejects picking up the gated toy during traversal. Undo/restart
cancel pending arrival motion and sounds; reduced motion projects arrival directly.
Once the little gate finishes opening, the browser sends the existing mouse
retrieval command automatically, completing the quest through the normal saved
reward transaction. No additional player pickup is required.

### Paving presentation detail

After asset preparation and before scene publication, the game derives one
shared paving presentation mesh referencing the approved original near geometry
and two accepted camera-detail companions. Original texture resources are shared
across levels; gameplay/picking footprints use the original bounds. Closed reverse
faces are culled. Ground-level courtyard slabs receive shadows but do not render
buried geometry into shadow maps. Shrub positions, density, saved identities and
original detail thresholds remain unchanged; the default elevated view submits
nearer opaque shrubs first within compatible batches. No simulation state is
changed by detail selection. See [measured rendering work](foliage-performance.md).

The current local Release build also uses the configured adjacent WebEngine's
material-pipeline specialization. The reusable shader/cache implementation lives
in that engine checkout. The pinned submodule is unchanged; pinned builds retain
their existing shader implementation.

## Pip friendship story

The optional `pip_friend_ball` -> `pip_friend_view` -> `pip_friend_room` chain
unlocks after `pip_missing_ball`, independently of the chapter-two/main picnic
path. Each request pays its normal 20 coins and Pip friendship once, including
across Undo, practice, import and reload.

The ball request uses `Goal::Relay`: the first successful Roll records a catch-tray
checkpoint, reveals the second four channels, and does not complete the request.
Undo restores that checkpoint with the rest of the puzzle snapshot. The second
Roll completes the request. Channels beyond the checkpoint reject rotation until
it has been reached. Presentation animates the validated pre-command route and
blocks further ball edits during travel; Undo and Restart remain available.

The lookout route reuses the accepted plank vocabulary in a mirrored layout,
with a planter blocking the route and one long plank. Pip traverses the supported
route and the full lookout animation; five small butterfly effects reuse the
accepted leaf picture geometry. The shared-lookout puzzle first balances the empty
beam, then includes Pip's load and requires two distinct cushions while preserving
the middle approach. Numbered flowerpots supply the four movable weights.

Save envelope 8 contains fifteen ID-keyed quest records. Versions 6 and 7 retain
their ten/twelve records and initialize untouched optional story attempts. The
permanent lookout is derived from the `pip_friend_room` reward ledger rather than
an independently writable entitlement. It uses existing platform, cushion and ball
meshes; their existing provenance and licenses apply, and no new model files are
introduced. Pip's accepted cushion contact is translated onto the assembled deck,
with supported entry markers validated against actual collision geometry.

Clicking the empty cushion or choosing **Sit with Pip** reserves the lookout for
Pip, focuses the camera, plays the existing bounded purr cue and keeps him resting
until **Leave lookout**.
The visit is presentation state; the lookout entitlement survives save/reload.
Reduced motion uses the settled pose. Existing audio mute preferences apply.

Regression coverage: `paws_pip_friendship`, `paws_release1_rules`,
`paws_release1_progression`, `paws_release1_gui`, `paws_release1_scene`,
`paws_release1_final_garden`, and `scripts/pip-friendship-smoke.cjs`.
Browser fixtures are generated by `paws_pip_friendship_tests <directory>`; isolated
browser saves do not modify the user's normal profile. Evidence is written to
`build/evidence/pip-friendship`. Expected play duration still needs human playtesting.


## Mabel friendship story: A little patch of sunshine

The optional `mabel_friend_sun`, `mabel_friend_water`, and `mabel_friend_share`
chain follows Pip's shared lookout and the chapter-two gate milestone. The first
request clears two planters before building a supported ledge route. The second
reuses the relay checkpoint with water: filling the basin does not finish the
quest, the second route remains gated until then, and the branch into the dry
cushion is rejected. The third uses picnic placement rules with explicit sun,
preferred-seat, and clear-middle-path constraints.

Each chapter uses the owner command journal, undo/restart, once-only rewards,
free hints and normal dialogue flow. Save version 9 introduced these three records
(18 base quests); the subsequent pack envelope retains migration from versions
6–9 and initializes absent story attempts. No story visit state grants rewards.

The `mabel_friend_share` ledger unlocks a permanent two-cushion nook and water bowl.
These reuse accepted kit meshes and measured cat contacts; existing provenance
applies. The reward exposes the front garden paving without advancing chapter or
cat eligibility. **Sunny nook** or clicking its empty cushion reserves both cats,
focuses the camera, and leaves them resting until **Leave nook**. Idle pairs can
also visit briefly. Entry uses collision-checked paths and separate contacts;
reduced motion uses settled poses, and existing sound preferences apply.

Coverage: `paws_mabel_friendship` checks the basin branch, clear path, per-step
roundtrips, migration and reward idempotency. Production scene/garden tests check
endings and both cats' supported approach, untimed rest and cancellation.
`scripts/mabel-friendship-smoke.cjs` uses real mouse gestures, a basin reload,
permanent-nook reload and portrait layout. Evidence lives under
`build/evidence/mabel-friendship`; tests use isolated browser saves. Human pacing
and physical-mobile acceptance remain separate from these checks.

Quest marker taps open the selected cat conversation before the puzzle. The
conversation shows its title, portrait, request and explicit Help action; the
request catalogue stays in the garden menu. Resuming an in-progress puzzle still
returns directly to play. `scripts/story-dialogue-smoke.cjs` exercises the actual
world-cat click, visible dialogue and Help handoff.

## Browser graphics diagnostics (18 October 2026)

Automatic retains its original 1x resolution and 8,294,400 pixel ceiling on both
backends. The experimental 450,000-pixel WebGL2 cap was reverted because it
degraded scene and GUI sharpness. Mobile retains 921,600 pixels; High remains an
explicit 2x choice.

With `?profile=1`, host diagnostics retain the last 20 renderer status messages,
report secure-context/WebGPU exposure, probe default/low-power/high-performance
adapter availability, and report the existing WebGL context's vendor/renderer.
These diagnostics are local to the page; they do not enable WebGPU or override
browser policy. Adapter probes run only with profiling enabled.

`scripts/gpu-path-smoke.cjs` verifies full-resolution Automatic and quality
switching on both backends.

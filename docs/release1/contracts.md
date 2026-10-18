# Release-one contracts

Frozen 11 October 2026 by Prompt 1. [contract.json](contract.json) is the compact
machine-readable authority; [contracts.hpp](../../src/release1/contracts.hpp)
provides compilable portable DTOs. Prompt 9 now integrates these contracts in the
production ten-request, eight-item target. See [prompt-09-handoff.md](prompt-09-handoff.md)
for exact current payload/evidence and the remaining production-audio gate.
Changes to frozen interfaces require an explicit handoff patch and integration
review. No task may silently change another task's IDs, dimensions or action names.

## Audited baseline and ownership

Current source supersedes the initial greybox descriptions. `PawsSession` owns
one deterministic puzzle and its 128-step undo history. `Afternoon` owns three
legacy sessions and seven new attempts, accepted command journals, reward ledger,
derived wallet/per-cat friendship, settings, eight-item ownership/placement,
historical MVP ending and separate release-one picnic. `BrowserGame` composes
Application scenes, cancellable asset preparation, read models, GUI, audio and
the existing save adapter. `web/saves.js` remains the only persistence coordinator.
The browser host owns DOM, pointer/keyboard/touch, lifecycle, file and storage
callbacks. Rules never depend on transforms, physics, sound or animation.

Reusable engine APIs are available in the clean pin
`3fe7d1dc1dc610c703944803cbb717cd65024ec9`; local WebEngine has unrelated changes
and is not a release dependency. The pin is unchanged. Existing retained GUI,
typed commands, attempt/revision rejection, once-only rewards, shop transactions,
free furnishing moves, IndexedDB snapshots, five backups, import confirmation,
original-byte recovery, single-tab ownership, quality and pause policies remain.
Scene bindings use checked handles; controller/renderer borrows end before scene
destruction. No second session, wallet, reward service or save scheduler is added.

Prompt 1 restores explicit Move/Rotate controls in the production Objects panel:
they had been hidden although the documented keyboard journey required them.
Direct tap/drag and arrow/R shortcuts remain available. The phase-7 runner also
waits for the authored ball arrival before seeking the completion card.

Accepted Pip authority is the current `pip-fitted-blink-r21` GLB, its packed
eye-rim master and manifest. Preserve the reference-walk, connected-settle and
blink channels; do not overwrite with an older job export. Existing garden kit
revision 7 has 21 separately exported models and explicit assembly transforms.
Provenance/rights are the actual existing notices, not a newly inferred license.
Historical production records are inputs to the audit, not a fresh acceptance run.
The baseline 47-clip audio system is extended to 66 registered clips. Final
source/rights/listening acceptance remains open. The accepted r24 Pip/Button
variants supply compact sleeping curls without replacing the original authority.
Independent-player discovery/pacing and physical-device acceptance remain open.

## Stable identities, progression and economy

The exact ten quest IDs, content versions, requesters, prerequisites, role names,
four cats, eight item IDs/prices and milestone thresholds are in JSON. No ID
collisions exist with current `src/content.cpp`, `src/furnishings.hpp` or runtime
catalogues. Preserve `missing-ball-v1`, `tidy-toys-v1`, `stuck-gate-v1`,
`garden_cushion`, `drinking_bowl` and the three existing spot IDs verbatim.

Chapter 1 retains the existing ordered three requests. Completing it introduces
Mabel and Button and unlocks all four Chapter 2 requests. Those requests have no
ordering among themselves. Completing all four introduces Pepper and unlocks
bells and playhouse together. Completing both unlocks picnic. Introductions are
short once-only dialogue records, restored from saved progression; Pip begins
introduced. Purchases never gate quest access. All puzzle-local props are supplied.

Every quest commits 20 coins and one friendship point to each requester exactly
once, keyed by stable quest ID. Joint playhouse rewards Pip and Pepper; picnic
rewards all four. Maximum friendship is Pip 5, Mabel 3, Button 3, Pepper 3.
Pip milestones are 1/3/5; other cats use 1/2/3, with labels acquainted/friendly/
at home. Milestones present progress and never require equal quest counts.
Practice, undo, restart, hint, restore and replay grant no duplicate rewards.
Wallet is 20 times committed rewarded quests minus actual owned item prices;
ten quests yield 200, all eight items cost 190. Rewards, progress, purchases,
placements and ending records publish atomically in one owner revision.

Existing cushion/bowl unlock with the first Pip reward and retain price and
ownership. Scratching post unlocks after tidy toys; perch after sunny spot;
hammock after cushion bed; playhouse after the joint quest. Table and mat unlock
on Chapter 2 completion, before picnic. Availability distinguishes locked,
unowned/affordable, unavailable assets and owned. Unavailable art does not invent
a working production item. A purchased item moves without another charge.

## Logical placements and model anchors

One owned furnishing occupies one compatible authored spot. A spot accepts an
item only when its role and width/depth fit and no other item occupies it.
JSON freezes existing spot roles and six new spot IDs/capacities. Prompt 6 authors
their world positions, supported approach paths and reserved contact spaces in
its own module; Prompt 5 owns legality. The three legacy spots remain compatible
with both existing items, preserving all legal old placements. No unrestricted
construction. Puzzle arrangements are independent of garden purchases.

Coordinates are metres, Y up, +Z forward, +X right; logical route grid is 1 m.
Channels preserve 0.26 m trough width, floor y=0.06 and the existing endpoints.
Ball radius stays 0.105. Existing switch released/pressed plate y=0.09/0.055,
gate hinge (-0.54,0.08,0), +100 degree open angle and bowl water y=0.383 remain
as recorded in `docs/garden-kit.md`. Cushion root y=0.16 and bowl drink root
(0,0,-0.61327) are Pip contacts; new meshes require measured per-cat overrides.

New planks have 1/2 m lengths, 0.5 m width and matching y=0.16 support/port
planes. Route validity requires connected endpoints and supported traversal,
not rendered contact. Cushion fitting uses a 0.25 m subgrid: square 2x2, bar 3x1,
elbow three cells; JSON lists exact offsets. Rotate in quarter turns about the
footprint origin and normalize offsets before occupancy checks. Basket sockets
must admit those shapes without overlap; rules own the complete occupied cells.
Prompt 4 authors board extents, socket cells and known traces within these fixed
interfaces and reports them before Prompt 3 finalizes basket geometry. Prompt 3
can independently build the frozen shapes, ports and named role assets meanwhile.

Bench retrieval uses a logical reachable socket and restricted reach, not a cat
crawling under the bench. The clearance envelope is at least 0.6 m wide and
0.25 m high for the toy/retrieval view. Playhouse entrance is at least 0.6 m wide
and 1.1 m high; supported rest/perch surfaces must hold the final cat's measured
paw/body envelope. The item footprints in JSON are maximum placement footprints;
approach and cat-contact clearance is additional reserved presentation space.

Required anchors for each role are in JSON. Every anchor records a local
`position: [x,y,z]` and `rotationDegrees: [x,y,z]`, XYZ Euler degrees. Include
footprint, axis-aligned rest bounds and use-path/contact measurements in sidecars.
Anchors do not become rule state. Separately moving plates, gates, doors and
bells export as separate GLBs. Assemblies list their asset IDs and local position,
rotationDegrees and scale; the game explicitly creates their actors. No reliance
on imported empties/hierarchy becoming runtime actors.

Bells use leaf/star/paw symbols, visible clue order and bounded reset/retry.
Water is a deterministic port route with a supported surface/ribbon and final
bowl fill. Two-switch success evaluates simultaneous occupancy in one complete
committed state, with distinct Pip/Pepper assignments; it never latches an old
press. Picnic shade membership, orthogonal-cell adjacency and quiet-zone
membership are simultaneous logical predicates. Prompt 4 supplies authored
zones and satisfiable traces; visible symbols/text expose every clue when muted.

## Rig, actions and audio

`assets/runtime/models/pip/pip.glb` and its manifest freeze the 29 original bone
names, hierarchy, local rest/bind and inverse binds; two optional lower-blink
controls are supported. Preserve all original bind matrices numerically within
exporter float tolerance (1e-5), not merely matching bone count. Wrapper joints
are importer details, not saved identities. New cats use the same canonical rig,
maximum four normalized weights and no unweighted vertices. Root stays at bind
in every clip. World movement belongs to the application. Use zero-based 30 fps
samplers with endpoints, name-based clip lookup, opaque portable materials and
supported skeletal channels. No morph-dependent animation or fur shader.

[canonical-rig.json](canonical-rig.json) captures the actual current GLB's 31
skin joints, parents, local rest TRS and inverse-bind matrices with its source
hash. Use this frozen numerical snapshot when fitting new meshes; the original
asset authority remains unchanged. It excludes importer-only wrapper joints.

All four cats expose the nine current common actions in JSON. Existing action
names/semantics and 0.18 s blend survive; mesh-specific skin/contact fixes may
change poses. Extra named actions: scratch, perch_step, perch_settle,
playhouse_enter, playhouse_lookout, picnic_sit. They are required on the cats
that use those furnishings, optional on others. Prompt 2 supplies durations,
loop flags, static reduced-motion samples, seam checks and measured contact
anchors in its fragment. Natural sleeping curl is required for Pip and Button;
the current supported low rest does not close that gate. Pip accepted geometry
and blink/walk channels must survive a curl correction.

Preserve all existing 47 clip IDs and cue families. JSON freezes new families;
variant clip IDs use `<family>_01`, `_02`, etc. Bells map explicitly to symbols.
Reuse the existing music beds across chapters; final picnic music extends the
ending cue rather than requiring ten tracks. Preserve Suno music and ElevenLabs
effects requirements in `docs/phase9-audio-production.md` unless the user changes
provider. Prompt 8 supplies real files, rights, durations, loops, mix/headroom
and measured decode sizes; absence of inputs leaves production listening open.
Keep 14 total voices/12 effects, 48 MiB cached PCM, 24 MiB per clip and 400 ms
one-shot expiry. `CommittedEvent` requests effects after commit; generation,
attempt and revision reject stale requests. Audio never commits rewards.

## Portable command and read-model boundary

The header contains value-only `Intent`, `PuzzleView`, `ProgressView`,
`LegalAction`, `ObjectView`, `CommittedEvent` and cat/quest/item/spot views.
They contain stable IDs, logical cells, orientations and identity checks, no
DOM/GPU/engine handles. Frozen verb names live in `Verb`; they are not the
serialized numeric values of the old `Action` enum. Keep old Action ordinals
and journal encoding unchanged. Prompt 4 implements explicit mapping to legacy
dispatch or its new puzzle extension. Prompt 5 adds the integration patch for
`Afternoon` to own new attempts and atomically process those intents.

An intent's generation is the runtime session/restore generation; attempt and
revision come from the displayed authoritative view. Reject stale identity
before any mutation. Unknown quest/object/target, illegal orientation/footprint
or incompatible socket returns a readable reason and no partial commit. Views
are disposable projections; UI and scene never maintain a second legality
model. New puzzle sessions may own deterministic attempts but cannot own coins,
friendship or saves. Prompt 9 applies the Prompt 5 extension to the existing
`Afternoon`; copied track fixtures are never another production owner.

Read models expose progressive three-hint state, legal enabled actions/reasons,
route, symbols, current held switches, ownership, placements, chapter and each
cat's friendship. Prompt 7 renders these using retained engine widgets and
returns typed intents. Prompt 6 consumes the same immutable snapshots. Both
support static reduced motion; neither decides success from clip completion.
All physical inputs route to the same command processor. Existing save-status,
import/recovery, pause, sound and quality interfaces remain as documented in
`paws_ui.hpp`; new views compose them rather than replacing them.

## Disjoint task ownership and concrete harness

Every JSON track reserves `src/release1/<track>/`, `tests/release1/<track>/`,
`scripts/release1/<track>/`, `web/release1/<track>/`, its job/runtime namespace,
and `docs/release1/prompt-NN-handoff.md`. Exact module filenames are in JSON.
Additional files stay within the allocated directory. Cat/prop/audio fragments
have their own filenames. Asset tasks never overwrite shared production
catalogues or Pip/garden assets. Prompt 1 owns this document, JSON, DTO header,
validator and generic harness. Shared files listed in `integrationOwned` are
Prompt 9's responsibility after this baseline. Only Prompts 1/9/10 reconcile
`docs/implementation-status.md`. No concurrent edits to those files are needed.

| Task | Module dependencies | Entry point and test responsibility |
| --- | --- | --- |
| 2 cats | paws_pip, engine core/assets/browser | cat_assets.cpp; tests/release1/cats/tests.cpp; preview_main.cpp and task shell; canonical bind/actions and runtime contacts |
| 3 props | paws_garden, engine core/assets/browser | prop_assets.cpp; tests/release1/props/tests.cpp; preview_main.cpp and task shell; hashes/anchors/assemblies and runtime import |
| 4 rules | paws_rules/content, standard C++ | puzzles.cpp + content.cpp; tests/release1/rules/tests.cpp; debug_main.cpp and task shell; seven traces, predicates and stale commands |
| 5 progression | paws_content + task-4 module | policy.cpp + codec.cpp; tests/release1/progression/tests.cpp; versions 1–5 fixtures, candidate migrations and transactions |
| 6 garden | paws_game + tasks 2/3/5 modules | presentation.cpp; tests/release1/garden/tests.cpp; preview_main.cpp; handles/lifecycle/use cancellation and contacts |
| 7 GUI | paws_ui + tasks 4/5 modules | views.cpp; tests/release1/gui/tests.cpp; gallery_main.cpp; real intents, modal focus and responsive layouts |
| 8 audio | paws_audio + engine audio backend | cues.cpp; tests/release1/audio/tests.cpp; preview_main.cpp; catalogue/import budgets and actual playback when supplied |

The executable harness is `tools/release1-track/CMakeLists.txt`; it adds the
baseline with `EXCLUDE_FROM_ALL` and builds only an explicitly selected target.
No root-CMake registration is needed. It is not a new game owner. Empty sources
run the common MVP contract baseline, not acceptance of unfinished track work.
Each track replaces the test main and supplies its own source list. Dependent
tracks include producer module sources in that list after their handoffs, without
editing those sources. Core logic can also compile directly from standard C++.

```powershell
# Example: rules native. Absolute source/test paths are required for overrides.
cmake -S tools/release1-track -B build/native-release1-rules -G Ninja -DCMAKE_BUILD_TYPE=Debug -DPAWS_TRACK=rules
cmake --build build/native-release1-rules --target release1_track_tests
ctest --test-dir build/native-release1-rules -R '^release1_rules$' --output-on-failure
# After task 4 creates its modules, add on configure:
# -DPAWS_TRACK_SOURCES="<repo>/src/release1/rules/puzzles.cpp;<repo>/src/release1/rules/content.cpp"
# -DPAWS_TRACK_TEST=<repo>/tests/release1/rules/tests.cpp
```

Use the matching native compiler runtime on PATH. Change `rules` to any track
name in both build directory and PAWS_TRACK. Each browser track uses its own
SDK configure, shell and evidence directory:

```powershell
& <emsdk>/upstream/emscripten/emcmake.exe cmake -S tools/release1-track -B build/web-release1-cats -G Ninja -DCMAKE_BUILD_TYPE=Release -DPAWS_TRACK=cats -DPAWS_TRACK_SOURCES=<repo>/src/release1/cats/cat_assets.cpp -DPAWS_TRACK_BROWSER_MAIN=<repo>/src/release1/cats/preview_main.cpp -DPAWS_TRACK_WEB=<repo>/web/release1/cats
cmake --build build/web-release1-cats --target release1_track_site
./scripts/serve.ps1 -SiteDirectory build/web-release1-cats/site -Port 19612
```

Shell loads `release1_preview.js` and implements the pinned host callbacks;
adapt the existing Pip/garden preview hosts inside the task namespace, not in
shared web files. Harness stages runtime assets plus engine GUI/storage/audio
adapters, but never runs `paws_site` or overwrites another task's generated site.
Rules/progression need portable tests first; browser debug wiring belongs to
task 4 and later real composition to task 9. Native track 5 may validate pure
policy/candidate codec plus old Afternoon fixtures while its owner patch waits.
There is deliberately no claim that existing monolithic Afternoon or PawsUi can
be edited concurrently: integration changes are exact patches in handoffs,
prepared/tested against task-local copies where necessary. Copies are fixtures,
never shipped owners or a fork of the engine. New scene lifecycle changes must
run task-local handle/cancellation tests and the existing portable scene suites.

## Manifest fragment format

Task fragments use `paws-release1-fragment/1`, integer task number, namespace,
assets array, assemblies array and provenance references. An asset includes
stable versioned ID, relative site URL, actual SHA-256/byte length, rest bounds,
footprint, named local anchors and rightsRecord path. Cat assets additionally
include rig authority/action/contact measurements; audio includes cue family,
bus, duration, loop, PCM estimate and actual listening status. Existing reused
assets are references (`reuses` list) rather than renamed/re-exported copies.

```json
{
  "schema":"paws-release1-fragment/1", "task":3, "namespace":"release1/props",
  "assets":[], "assemblies":[], "provenance":[], "reuses":["garden/ball@1"]
}
```

Empty fragments represent preparation only. Do not write invented hashes,
licenses, final geometry or completed listening records. URLs remain beneath
`assets/models/release1/cats/`, `assets/models/release1/props/` or
`assets/audio/release1/`. IDs use `release1/<namespace>/<name>@1`. Runtime
directories contain accepted files only; raw, source and rejected work stays
under the assigned artifacts path. Prompt 9 merges fragments into production
registration and verifies every byte/notice, bounds, anchor and reference.

## Save migration and two ending identities

Prompt 1 does not change the current version-5 writer. Prompt 5 prepares a
version-6 extension to the same codec; Prompt 9 merges it with the existing
owner. Retain stable quest/content IDs and old action ordinals. Save new
quest/content versions and ledgers by stable ID rather than array position.

| MVP version | Existing migration input | Release-one policy |
| --- | --- | --- |
| 1 | settings + three content journals; replay derives rewards | preserve replayed attempts/rewards; only Pip receives old friendship |
| 2 | epochs, attempts/revisions, explicit wallet/ledger | validate against replay; preserve exact paid rewards and identities |
| 3 | two owned items, placements, MVP ending | preserve items, 30/25 prices and spots; keep old ending separately |
| 4 | save revision/writer identity/user pause | preserve recovery metadata and explicit pause |
| 5 | display quality | preserve quality; earlier versions default Automatic |
| 6 | ID-keyed ten attempts/rewards, four friendship/introductions, eight items, two endings | validate candidate fully before publication; unknown future/corrupt data rejects |

Old Pip friendship maps directly from the three committed Pip rewards. New
cats start at zero; new quests start unattempted/unrewarded. A completed MVP
unlocks Chapter 2 but grants no additional money/friendship. Missing introduction
records are derived from already unlocked chapters once, not a reward grant.
Original bytes survive failed imports or incompatible content. Keep 16 MiB
files, 100,000 commands per quest, bounded replay/128 undo and five recovery
snapshots; save receipts mark Saved only for the newest captured revision.
Restore rebases runtime generation/attempt identities and cancels scene/audio
work. Never serialize engine handles, callbacks, GUI, voices or clocks.

MVP ending is `mvp_pip_at_home`; new ending is `release1_garden_picnic`.
The old bool maps only to the former. The latter starts false for all MVP saves
and requires all ten completed reward records plus an owned placed furnishing.
Picnic completion grants its reward even if placement is still pending; placing
a furnishing later permits the ending. Supplied celebration props render it
without requiring purchased picnic table/mat. Record once, allow explicit replay,
continue shopping/free placement after ending. The old ending cannot suppress it.

## Verification and remaining acceptance

[prompt-01-handoff.md](prompt-01-handoff.md) records this run's commands, payload
identity, evidence, corrections and checks. Prompt 1 baseline evidence does not
prove new puzzles, missing cats/audio or release-one pacing. Desktop Chrome and
touch-oriented layouts remain scoped automated observations. Independent players,
natural curl, real audio listening and physical supported-device combinations
remain explicit gates for later owners. No commit, push or publication is included.

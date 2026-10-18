# Prompt 06 — living garden presentation

Implemented 11 October 2026. The independent runtime/harness work is delivered;
**four-cat and all-eight-use asset acceptance remains open on Prompt 2**. Its
accepted cat fragment, runtime exports and final contact handoff are absent.
Supplied Tripo archives and in-progress repair jobs exist and were preserved.
They were not promoted or substituted with repeated Pip meshes in the browser.

## Delivery and ownership

- `src/release1/garden/presentation.hpp`, `presentation.cpp`: extended courtyard,
  three visually unlocked areas, nine authored spots, four independent cat
  bindings, immutable asset sharing, request markers/entries, short attention
  approach/return, sparse reactions, reservation/path/use scheduler, placement
  previews, candidate construction, reduced motion and supplied picnic props.
- `src/release1/garden/contact_plans.hpp`, `contact_plans.cpp`: supplied prop
  anchors and **unverified** per-cat use guides for all eight furnishings.
  Complex use is enabled only by measured, verified contacts on the actual mesh.
  The two existing Pip contacts remain usable, with the low rest explicitly
  distinguished from natural sleeping curl.
- `src/release1/garden/preview_main.cpp`, `web/release1/garden/`: isolated pinned
  renderer harness, real owner commands, asset preparation/cancellation, camera
  views, actual typed purchases/placements and short introduction acknowledgments.
  HTML controls and the known-trace button are development tools, excluded from
  production composition. They are not the Prompt 7 retained GUI.
- `scripts/release1/garden/build.ps1`, `prepare.py`, `contacts.json`: reproducible
  native/browser builds, input hash checks and measured-contact intake.
  `inputs.inc` and `contacts.inc` are generated **harness** inputs in this task's
  source namespace. No unaccepted cat job export is staged by these scripts.
- `scripts/release1/garden/smoke.cjs`, `tests/release1/garden/tests.cpp`: actual-site
  inspection and portable lifecycle/reservation/owner-isolation regressions.

Only this task's directories and this handoff were written. Shared browser/GUI
composition, root build registration, production manifests, common architecture
and implementation-status edits remain with Prompt 9. No engine change, pin
update, agent, other-chat message, commit, push or publication. The pinned engine
is clean at `3fe7d1dc1dc610c703944803cbb717cd65024ec9`; unrelated adjacent-engine
and other-track work was inspected read-only and preserved.

## Runtime contract

`garden::Assets` contains shared immutable prop meshes and four `CatModel`s.
Missing cats stay absent. `build(assets, progressView, reducedMotion)` constructs
a complete candidate before replacing a valid outgoing scene. It validates cat
rig compatibility and every verified contact's actions, finite timing/root
samples and supported-path endpoint. Renderer/orbit borrows are exchanged through
Application's rollback-aware scene binding and released before scene destruction.
Asset requests retain a preparation generation; cancellation and later rebuilds
reject earlier interests. Failed candidates retain the valid outgoing renderer.

`Presentation::project` consumes a copied authoritative `ProgressView`. It does
not own purchases, occupancy, chapter legality, introductions, friendship or
rewards. Stale generation/revision projections are ignored. A generation change
cancels transient uses; a committed placement change cancels paths that could
cross the new obstacle. Preview/cancel does not change a committed use or owner.
Restore constructs idle cat bindings from the saved owner and does not acknowledge
introductions, replay rewards or automatically replay an ending.

`use` receives an exact `Stamp` and returns a scene/use-scoped `Ticket`. It checks
the displayed placement, identity, actual scene handles, final clip/contact
readiness and presentation reservation. Old tickets cannot cancel a replacement
use/scene. One bounded walking lane serializes approaches and departures; resting
cats keep their contact seats. Routes use a bounded authored quarter-metre grid,
avoid the bench, other furniture/door sweep and actual waiting/resting cat spaces,
and exclude the central puzzle apron. Routes are recomputed when a queued use
starts and when returning, since other cats may have returned home meanwhile.
Cancelling a resting cat during another approach also cancels the affected path.

The nine world spots preserve all three original IDs/positions. The six added
centres, in metres, are scratch `(-4,0,.4)`, perch `(-4,0,-2.1)`, hammock
`(4,0,-2.1)`, playhouse `(4,0,2)`, table `(-4,0,3.6)` and mat `(0,0,4)`.
Their role/footprint legality remains Prompt 5's policy. Cat home/request pads
lie along the back at z=-3.6 with 1.2 m lateral spacing. Puzzle scenes remain
separate; garden cats are never placed over a puzzle board by this module.

`gather` atomically reserves four separate table/mat seats and holds early
arrivals until companions arrive. Other furnishings reserve one cat at a time.
The small mat gets one centre seat plus grounded companions **outside** it;
four bodies are not claimed to fit on its 1.5 m footprint. Final body/contact
dimensions must replace the unverified guides. Supported elevation/entry paths
begin at their first measured root; cats do not first walk through a perch at
ground height. Actor movement remains application-owned; animation poses never
decide a puzzle result or commit an ending.

Call `update(scene, frame.simulation.delta)` once in Application's simulation
callback. It never ticks Scene or advances animation separately. Explicit pause
and host suspension freeze both travel and clip time. Reduced motion samples a
static pose at the measured root; it removes walking/elevation playback. Sparse
personality animation can be interrupted immediately by a request/use.

`celebrationProps(scene, stamp, true)` requires the owner's picnic-eligible/seen
read model. It shows supplied table/mat meshes when their owned equivalents are
unplaced, without buying them or modifying occupancy/rewards. It is explicit,
not triggered by a restored old ending flag. Prompt 9 owns the once-only
`RecordPicnic`/`ReplayPicnic` command and the closing GUI/audio flow.

## Prompt 2 dependency and measured-contact intake

As observed in this run, `docs/release1/prompt-02-handoff.md`,
`docs/release1/fragments/cats.json` and `assets/runtime/models/release1/cats/`
are absent. Mabel/Button/Pepper repair jobs are not accepted runtime delivery.
Accepted Pip remains the unchanged r21 GLB, SHA-256
`da50380685ce63cd7ea42d7ed3f49580ed607e4fc3a7ce9093f377533be241e2`.

The harness automatically reads a future cat fragment's GLB records using
`cat` (or `name`), `id`, `url`, `sha256` and `bytes`. It checks the actual runtime
files before compiling URLs. Populate task-owned `contacts.json` from the final
producer measurements, one record per intended cat/item:

```json
{
  "cat": "mabel",
  "item": "sunny_perch",
  "meshSha256": "<actual accepted mesh hash>",
  "verified": true,
  "evidence": "<existing repository-relative measured-contact evidence>",
  "naturalCurl": false,
  "contact": {
    "approach": [0, 0, 1.25],
    "root": [0, 0.46, 0],
    "yaw": 3.141592653589793,
    "enter": "perch_step",
    "loop": "perch_settle",
    "leave": "perch_step",
    "enterSeconds": 2.4,
    "leaveSeconds": 2.4,
    "still": 2.4,
    "supportedRoots": [[0, 0, 1], [0, 0.2, 0.575], [0, 0.46, 0]]
  }
}
```

This is a **format example**, not verified Mabel contact data. Root samples and
clip timings must be replaced with measured values. The script does not upgrade
a guide because an action name or filename exists. It requires the exact mesh
hash and existing evidence; the C++ candidate validates clips/path/timing. Inspect
actual paw/body contact, deformation, seams and reverse transitions before
setting `verified`. For hammock, fit the actual curled body against the supplied
curved cloth surface; a centre-height offset alone is insufficient.

Required remaining visual acceptance: four final identities, natural Pip/Button
curl; scratch/front-paw/post fit; Mabel supported perch ascent/descent; Button
hammock sag/contact; Pip/Pepper doorway clearance/enter/lookout/return; four-cat
table/mat body spacing; actual walk/blends and all reduced-motion samples on
those final exports in desktop and shadow-free views. The browser currently
renders all eight final **prop** furnishings and accepted Pip; missing cat uses
show explicit gates. No geometry or accepted Pip clips were overwritten.

## Verification and evidence

Pinned Emscripten 6.0.11 Release browser build and native Release task build pass.
The portable garden suite passes 73,901 checks, mostly per-frame cat clearance
probes, plus owner transactions, static contacts, reservation queues, stale
projection/tickets, cancellation during concurrent use, group seats, supplied
picnic props, pause/suspension, candidate rollback, scene replacement and reload.
Four synthetic actors/clip aliases are **test fixtures only**. These exercise
independent playback and all eight scheduler paths; they do not prove new cat
artwork or final animation/contact acceptance. Four unchanged baseline portable
scene/lifecycle suites pass.

The actual generated Chrome site checks ten real owner traces, 200/190 economy,
eight purchases/placements/prop views, unpurchased supplied picnic props, explicit
missing-contact group rejection, actual Pip walk/approach/drink/return, static
low-rest/drink, move/cancel during use, explicit pause and host suspension,
failed-candidate retention, rapid rebuild/cancel, one live scene, version-6
reload, portrait/short-landscape and keyboard inspection, HTTP failure/retry and
loaded GLB identities. Normal-page console/runtime errors are checked separately
from injected HTTP failure. Inspect the final report for the exact result.

Evidence: `build/evidence/release1/prompt-06/native-final.txt`,
`baseline-scenes.txt`, `inputs.json`, `browser-final/report.json` and screenshots.
The final browser report records its actual Wasm hash/browser version; the local
`scripts/release1/garden/record.py` writes a complete payload summary to the same
evidence directory. Earlier `browser/` and earlier native logs are historical.
Final observed payload: Chrome `154.0.8037.99`, 52 distinct loaded GLBs matching
staged bytes, eight browser check groups and zero normal-page console/runtime
errors. Wasm SHA-256:
`001b4b554936ee70e2627a2cd77fe6522559aa91d0ea1198a0c757ffb544e705`.
Early defects were corrected: non-existent Vec3 equality in a test, too-short
sequential-use observation, mat approach entering the reserved apron, whole-view
perspective clipping, queued-home clearance and return-route obstacle changes.
No failed record is treated as a pass.

Desktop automation and narrow layouts do not establish physical mobile, Safari,
independent-player pacing, final new-cat contact quality or sustained performance.
No final-audio/listening acceptance is claimed by this track.

## Reproduce

```powershell
# Run from the repository root. SDK/Jolt paths describe this observed machine;
# the script always uses the exact pinned engine and task-owned build directories.
./scripts/release1/garden/build.ps1 -Native -JoltSourceDir C:/Users/Johan/source/repos/WebEngine/build/dependencies/JoltPhysics-5.3.0
# Put the configured CMAKE_CXX_COMPILER directory on PATH for its runtime DLLs.
ctest --test-dir build/native-release1-p06 --output-on-failure
ctest --test-dir build/native -R 'paws_starter_scene|paws_phase[567]_scene' --output-on-failure
./scripts/release1/garden/build.ps1 -EmsdkPath C:/Users/Johan/source/repos/WebEngine/.tools/emsdk -JoltSourceDir C:/Users/Johan/source/repos/WebEngine/build/dependencies/JoltPhysics-5.3.0
$env:EVIDENCE_DIR='build/evidence/release1/prompt-06/browser-final'
node scripts/release1/garden/smoke.cjs
python scripts/release1/garden/record.py
./scripts/serve.ps1 -SiteDirectory build/web-release1-p06-pinned-release/site -Port 19616
# Open http://127.0.0.1:19616/
```

The smoke runner owns/closes its server and temporary browser context. The
existing `web/saves.js` coordinator is copied into this isolated staged shell
with `paws-afternoon` replaced by `paws-release1-garden-harness`; the production
source/normal save is unchanged. It retains Web Lock ownership, complete writes,
latest receipts and five recovery snapshots. No second storage subsystem or
partial wallet write was added. No local machine paths enter production source.

## Prompt 9 composition patch

After merging the Prompt 4/5 owner and producer modules, register
`presentation.cpp` and `contact_plans.cpp` in a game-owned library linked with
`paws_game`, the prop/cat modules and the patched progression owner. Keep
`preview_main.cpp`, generated harness inputs, HTML controls and test fixtures
out of production. Map accepted fragment records/measured contacts into
`garden::Assets` during the existing BrowserGame asset-preparation flow.

At the existing preparation boundary:

```cpp
auto candidate = garden::build(assets, afternoon.progressView(),
                               afternoon.settings.reducedMotion);
// Prepare and bind candidate.scene through the existing Application owner.
// Release old renderer/controller borrows before unloading the outgoing scene.
gardenPresentation = std::move(candidate.presentation);
```

After accepted owner commands, project its fresh `progressView`. Placement GUI
uses `afternoon.placementView(item)` and passes that already-computed legal flag
to `previewPlacement`; it must not copy placement rules. GUI use/request controls
capture `gardenPresentation.stamp()` and dispatch through the same owner/runtime
boundary for mouse, touch and keyboard. Map introductions to `introduce` only
after visible dialogue continuation. `requests(cat)` presents the owner's
unlocked request list; selecting still goes through `SelectQuest`.

In Application's normal simulation callback:

```cpp
gardenPresentation.update(scene, frame.simulation.delta);
```

Keep persistence in the existing BrowserGame/save coordinator, not this preview.
On import/restart/scene replacement, drop transient garden tickets and rebuild
from the validated owner; do not persist engine handles or reservation phases.
After the owner's accepted picnic record/replay, call `celebrationProps` and
`gather` with the fresh stamp. Finish final cat-contact/browser acceptance after
the actual Prompt 2 delivery, then exercise the production composed journey.

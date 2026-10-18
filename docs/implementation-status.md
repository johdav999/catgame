# Implementation status

## Complete beach greybox story, 18 October 2026

`beach_afternoon` now contains six sequential requests: Pip's beach ball,
Button's supported bridge, Mabel's shade panels, Pepper's four-stop basket trail,
a six-picture shell-chime tune, and four-cat picnic placement. Completion offers
Continue beach story; Adventures lists individual revisits and overall progress.
Named picnic mats carry distinct picture symbols. Saved cove landmarks project
the once-only beach completion ledger. The final Watch beach ending intent
records a seaside postcard independently of the garden ending and wallet.

Version-11 saves preserve versions 1–10, including the first beach quest's
original version, journal and completion. New journals initialize fresh.
Development access remains transient (`?devBeach=1`). Beach progression changes
no base coins, friendship, chapters or garden picnic eligibility.

See [the beach contract](beach-adventure.md), the complete story regression
`paws_beach_story`, retained GUI/scene tests and
`scripts/beach-story-smoke.cjs`. Final reports identify the exact tested build at
`build/evidence/beach-story/{webgpu,webgl2}/report.json`. Production beach art,
original audio, payment services, physical-device acceptance and first-time human
pacing remain outside this gameplay milestone.

## Beach adventure first milestone, 17 October 2026

`?devBeach=1` exposes Adventures from the garden. A Seaside Afternoon includes
Pip's runaway beach ball, a separate saved attempt, remembered garden return,
free hints, Undo, explicit restart and completion/practice without base-game
rewards. Version-10 saves retain beach progress and read versions 1–9.
Development access is transient and never restored from a local save.

The generated `build/web-beach-local-release/site` was exercised in desktop
Chrome on WebGPU and forced WebGL2 with real GUI activation, drag cancellation,
Undo, portrait dragging, route clicks, mid-beach reload, garden/reload/revisit,
access removal/restoration and completed revisits. Both reports passed with no
browser errors. Primitive closed blocks stand in for beach baskets; sand, sea
and dunes are greyboxed. This is one integration quest, with no payment backend,
production beach art or physical-device acceptance.
See [the milestone contract](beach-adventure.md) and
`build/evidence/beach-adventure/{webgpu,webgl2}/report.json`.

## Scent and balance requests, 17 October 2026

The catalogue includes Follow the scent (six evidence-and-choice stops) and
Pip's wobbly perch (four changing support/load setups). These optional later
quests target 6–10 minutes each; first-time human pacing remains unmeasured.
Both use normal unlocks, Undo, restart, hints, saved attempts and once-only
rewards. Version-seven saves preserve version-six progress and its original
picnic ending requirements. See [the contract](trail-and-balance-quests.md).

## All ten quests use scene mouse controls, 12 October 2026

All ten requests complete with clicks and drag-and-drop in the generated game.
Quest object/action panels and GUI route checks are removed. Clicks rotate loose
or fitted planks/cushions, channels and gutters, ring bells, release sources and
retrieve the bench mouse. Drags move props, fit cushions/planks and place kittens.
Valid plank, switch and picnic arrangements complete automatically. Rejected
drops preserve state and display feedback. Menu retains paused Undo/hints,
restart and service controls. Undo restores legacy actor poses immediately.

Pinned Release `paws-0.1.0-31f0ecfe283d39f9` passed all ten quests with mouse-only
puzzle actions at `http://127.0.0.1:8080/`, including the 200-coin ledger, blocked
retrieval, wrong switches/bells, invalid cushion/plank drops, cancellation and
Menu Undo/hints. No browser errors were recorded. Evidence:
`build/evidence/all-quest-mouse/report.json` and ten completion screenshots;
runner: `scripts/all-quest-mouse-smoke.cjs`. The local server was restarted on
this exact pinned site. Four affected portable suites passed: rules, progression
(including direct-rotation save replay), GUI and scene. Physical devices were
not tested. See [the complete controls table](quest-mouse-controls.md).

## Earlier Sunny spot control pass, 12 October 2026 (superseded)

Mouse release without dragging rotates a plank through the existing pick-up and
rotate commands. Dragging previews movement and fits the plank to the crossing
under the pointer on release. Unsupported, occupied, wrong-length or misaligned
drops preserve the attempt. Blur, resize and pointer cancellation restore the
committed presentation. The objective now describes click and drag controls;
This earlier pass retained Puzzle controls and a manual route check; the all-quest
pass above replaces those with direct gestures and automatic completion.

Pinned Release build `paws-0.1.0-9471c99dc65f64c5` passed the actual Chrome game
flow in `scripts/plank-controls-smoke.cjs`: first-click rotation, cancelled and
invalid drops, drag-away-and-return, three mouse placements and route completion.
Evidence: `build/evidence/plank-controls/report.json` and `route.png`. This is
desktop browser evidence; physical touch devices were not tested for this change.

## Release one Prompt 9 — integrated production review candidate, 11 October 2026

The actual production target now runs ten requests, four r24 cats, three chapters,
per-cat friendship, eight purchases/placements/uses and the separate once-only
four-cat picnic. The existing owner/save coordinator writes version 6 and migrates
supported MVP versions 1–5 without duplicate rewards or skipping the new finale.
Retained GUI, actual prop/cat/portrait assets, checked scene preparation, contact
paths, cancellation, reduced motion and cue composition are integrated. Development
track shells/fixtures/previews are excluded from production staging.

Exact clean engine pin: `3fe7d1dc1dc610c703944803cbb717cd65024ec9`.
Final Release build: `paws-0.1.0-074a5b58a29b2231`; Wasm SHA-256
`03963903380fb652765f624309489c2069cddbeb2eb4940da90f2147a1512ac9`.
Evidence: **21/21 portable suites**, all ten authored solutions and eight real
furnishing uses through the generated production site, alternate orders, accessible
layout/action checks, save/file/tab/lifecycle and actual WebGPU recovery. Final
mouse/touch/loading/restart/interruption checks and actual animated contact/gathering
reports are scoped in [the handoff](release1/prompt-09-handoff.md).

The site audits 56 GLBs and 94 hashed deliveries. Sixteen actual audio files are
technically playable locally; 50 clips remain undelivered and final rights/listening
remain pending for all 66 slots. The candidate is **not production-ready**.
Independent-player pace/discovery, final audio, sustained/device/Safari and final
package acceptance remain Prompt 10 gates. Earlier phase/Prompt 1 entries below
document historical baselines, not the current production scope. Unrelated
concurrent assets/audio work was preserved; no commit, push or publication.

## Release one Prompt 1 — contracts and verified MVP baseline, 11 October 2026

Implemented [release-one contracts](release1/contracts.md), the machine-readable
ID/progression/asset/ownership contract, numerical canonical-rig snapshot,
portable intent/read-model DTOs, isolated native/browser track harness and
contract validator. Seven later tasks have disjoint module/test/preview/asset
paths; shared owner changes and production registration remain Prompt 9 patches.
The production game still uses three quests, two items and the version-5 save
codec. The version-6 migration and separate picnic ending are frozen future work.

The fresh baseline found and corrected hidden Move/Rotate controls in the
production Objects panel. Explicit keyboard actions now coexist with direct
board gestures. The phase-7 runner now waits for authored ball arrival before
seeking its completion card. Portable UI regression coverage preserves identity
and disabled-action behavior. No engine source or pin changed.

Pinned Emscripten 6.0.11 Release, gallery OFF:
`paws-0.1.0-141c15263730f1df`; Wasm SHA-256
`98566692b5c8eae482e5b94b1b02b1a8eefdd8313d541751a3b980ec638ad250`.
Fresh acceptance: **13/13 portable suites**, isolated native baseline **1/1**,
contract validator/invariant checks, complete MVP earn/buy/place/reload/ending
journey, save/recovery/GPU cancellation checks, mouse/emulated-touch direct
completion, and actual accepted-Pip loading in the standalone browser harness.
Browser runs used Chrome 154.0.8037.99 with isolated contexts and zero errors.
Evidence: `build/evidence/release1/prompt-01/`; exact commands, failed-run
corrections, payload and ownership are in [the handoff](release1/prompt-01-handoff.md).

Final production audio/rights/listening, natural sleeping curl, missing new cat
meshes, independent-player discovery/pacing and physical supported-device gates
remain open. Historical phase evidence below remains historical. No commit,
push or publication was performed. Prompts 2/3/4/8 can proceed from this contract.

## Phase 10 — production review and local release candidate, 11 October 2026

Prompt 10's polish, packaging and reproducible acceptance tooling are implemented.
**The MVP is not production accepted:** all 47 final audio clips/rights/listening
reviews, the true cushion sleeping curl, independent first-player pacing and
physical-device acceptance remain open. No engine source/pin, user browser saves,
commit, push or publication was changed by this phase.

Delivered corrections:

- Real Automatic/Mobile/Desktop quality controls instead of the disabled future
  placeholder. Explicit pixel/DPR/shadow budgets follow the architecture's
  provisional baselines and preserve rule identities. Version-5 saves retain the
  choice; versions 1–4 migrate with Automatic and invalid quality rejects.
- Portrait HUD retains wallet/friendship/save status. Short landscape uses two
  compact cards after visual review caught an intermediate header overlapping
  upper puzzle pieces. Larger text, touch-sized controls and modal focus remain.
- Measured Paws application/GUI/audio wall time replaces the renderer's zero CPU
  update input. Final review records frame p50/p95/p99, renderer counters,
  scoped memory, startup transfer, GPU timing availability and absent-audio limits.
- Payload build/content IDs, validated model hashes and required notices,
  versioned ZIP/site package, nested-URL smoke, hosting instructions and explicit
  remaining gates. Developer preview binaries/pages are excluded from the package.
- Unsupported browsers keep the compatibility/Retry shell without entering Wasm
  initialization; supported startup waits for the deferred adapters.
- A fresh guided three-quest journey with blocked roll, three hints and undo;
  file/save/ownership/GPU recovery; responsive/grayscale/shadow-free board review;
  and a 15-minute automated desktop transition soak. This does not establish
  independent first-player discovery/pacing or physical mobile support.

Changed files: `src/afternoon.*`, `src/paws_ui.*`, `src/browser_main.cpp`,
`web/index.html`, `web/host.js`, `CMakeLists.txt`, `cmake/BuildInfo.cmake`,
`cmake/CopyLicenses.cmake`, `licenses/`, `scripts/package-release.py`,
`scripts/phase10-*.cjs`, `scripts/phase10-evidence.py`, phase-7/8 browser runners,
portable GUI/save tests, `tests/test_release_package.py`, package commands and
README/architecture/save/release documentation. See the flow profile, issue
ledger and reproducible commands in [phase10-release.md](phase10-release.md).
The concurrent GUI serving repair below was preserved; final acceptance uses the
frozen package, without relying on another chat's port-8080 server.

Pinned Emscripten **6.0.11 Release**, gallery **OFF**. Final build ID:
`paws-0.1.0-0819e5cf177e405e`; engine pin:
`3fe7d1dc1dc610c703944803cbb717cd65024ec9` (clean submodule).
Wasm SHA-256:
`f677b5106f336ffb3d5a6b45152f7f602d2cae5eac27c7639b3193efae46fa4d`.
Content tree SHA-256:
`1959fa2d10a07ef77fba0f6864c9fdf492b7c780fd385cde084568bb25bed7e3`.
Third-party notice tree SHA-256:
`24e12796be95110624b46f338bb1ebb7b71bfe1608848d67af160d729dcaae32`.

Final local package:
`build/releases/paws-0.1.0-0819e5cf177e405e.zip` and
`build/releases/paws-0.1.0-0819e5cf177e405e/site/`.
ZIP SHA-256:
`eabf818122d7b01b584267a5996f68b9c093c3fa519db50e9729e19f5764fcec`.
ZIP size: **11,889,035 bytes**. The staged notices include fonts, Pip/garden,
ufbx, cgltf, stb, meshoptimizer, Jolt, Emscripten and Dawn; notice tampering is
rejected by packaging and the notice tree contributes to the build ID.
Its `release-manifest.json` marks `review-candidate`, `productionReady: false` and
all 47 missing clips. Source masters remain in the repository's art jobs.

Verification: native **13/13**, package integrity/tamper **7/7**, audio importer
**6/6**. The corrected startup package passed fresh journey, recovery and
responsive/failure review in `build/evidence/phase10-startup-final/`. After adding
the toolchain notices and notice fingerprint, the sealed archive **also passes**
all three suites in `build/evidence/phase10-sealed/{journey,recovery,review}/`.
The combined command exited 0; each report records `passed: true`, no unexpected
errors and the final payload identity. Summary: `build/evidence/phase10-sealed/verification.json`;
current token/source-export audit: `build/evidence/phase10-sealed/visual-audit.json`.
Host: Windows NT **10.0.26200**, Chrome **154.0.8037.99**; available host adapters
Intel UHD Graphics 770 and NVIDIA GeForce RTX 5060 Ti. Browser-selected adapter is
reported as **intel / gen-12lp** (device/description withheld); the host inventory
alone does not establish a particular model or NVIDIA use. This records a tested
automated desktop combination, not an accepted physical-mobile support matrix.

Final-package cold startup on local gzip hosting: **11,638,320 compressed bytes
(11.10 MiB)**, **15,817,582 raw bytes** including the first garden; title ready in
**192 ms**, garden ready **610 ms** after Play, total **977 ms** including test
input settling. This is below the 15-MiB compressed allocation; localhost timing
does not establish Internet download latency. All five scoped allocation checks
in the final review pass; total process/driver memory and final audio remain open.

The original 900-second soak completed **42 garden/puzzle cycles**, recording
**48,408 RAF intervals / 955 renderer samples** under
`build/evidence/phase10-final/review/performance-samples.json`. Its terminal report
failed on BigInt serialization, so **that full run has no terminal pass claim**.
A reporting rerun exposed the unsupported-WebGPU startup initialization defect;
the serializer and startup gate were then fixed. The corrected short runs have
zero soak duration. All packages share the exact Wasm/content/engine hashes;
they do not constitute a 900-second rerun of the final shell. See
`scripts/phase10-evidence.py` and the scoped `verification.json`.

Retained desktop measurements (includes responsive review/capture stalls):

| Measurement | Observed | Provisional allocation / scope |
| --- | --- | --- |
| RAF interval p50 / p95 / p99 | 20.0 / 20.1 / 20.2 ms | Headless desktop scheduling; max capture/transition stall 440.1 ms |
| Wasm capacity peak | 64,880,640 bytes (61.875 MiB) | 192 MiB; capacity, not working set |
| Tracked GPU peak including GUI | 60,122,444 bytes (57.34 MiB) | 96 MiB; excludes driver/process allocations |
| CPU asset peak | 33,738,144 bytes (32.18 MiB) | Asset payload, not process memory |
| Color draws / triangles peak | 57 / 83,582 | 100 / 150,000 |
| Warm garden retained allocation | Same 56,825,620 GPU bytes, 34 mesh/68 texture cache entries before/after soak | Fixed Wasm/CPU asset capacity also; no process-level leak claim |
| Paws update mean-window p50 / p95 / p99 | 0.207 / 0.452 / 0.528 ms | Distribution of sampled rolling means, not per-frame percentiles |
| GPU timestamp mean-window p50 / p95 / p99 | 3.925 / 6.447 / 6.647 ms | Timestamp state enabled; sampled rolling means |
| Decoded audio peak | 0 bytes | Final clips absent; audio acceptance remains open |

Token contrast checks on actual screenshots: body/cream **10.89:1**, cream/teal
controls **6.31:1**, error/cream **5.61:1**. This verifies shipped flat tokens,
not every rendered label or independent accessibility acceptance. Current asset
hashes are validated; retained Pip r21 source/export and garden reimport renders
were compared with fresh runtime captures. The natural sleeping curl remains open.

Launch the package:
`./scripts/serve.ps1 -SiteDirectory build/releases/paws-0.1.0-0819e5cf177e405e/site -Port 8100`.
Development build: `build/web-phase10-pinned-release/site/`.
Run `npm run test:browser:phase10` with `SITE_DIR` set to the packaged site;
the complete default review includes a 900-second desktop soak.

Next: close the named audio and sleeping-curl art gates, then run an independent
15–20-minute first-player journey and at least 15 minutes on each supported
physical device/browser/OS combination. Import approved audio and rebuild/repackage
before final mix and performance acceptance. Desktop/touch emulation does not
close Safari, Android, iPhone, thermal performance or audible-output gates.

## GUI serving correction — 11 October 2026

The Objects/Actions and Hints sheets were already right-aligned in the current
source, but three Python servers sharing localhost port 8080 served phase-5,
phase-7 and phase-8 builds. Old cached JavaScript/Wasm persisted after refresh.
Those three specifically identified Paws servers were replaced with one current
pinned Release site at `build/web-phase9-pinned-release/site`.

`scripts/serve.ps1` now uses `scripts/serve-site.py`, which binds exclusively,
disables caching and gives relative scripts/Wasm/assets a build-specific base URL.
A duplicate port-8080 launch was verified to reject instead of sharing the port.
The current source's quality-report bridge also needed object-literal parentheses
inside `EM_ASM`; that compile fix preserves the quality work.

The rebuilt game restored the saved attempt and showed its selected Left pot
actions on the right in the user's actual in-app browser, **1724×837**, at
`http://127.0.0.1:8080/`. Screenshot:
`build/evidence/gui-right-panel/right-panel-1724x837.jpg`. Wasm SHA-256:
`73b428ed7f48573b98f862cec44a8d7377e71ab5757adb4e122649e95640c959`.
Earlier phase acceptance hashes below remain historical; the serving rebuild
includes concurrent current-checkout changes and is not a repeat of the complete
phase-9 audio acceptance journey.

## Phase 9 — audio integration delivered; production audio missing, 11 October 2026

Prompt 9's independent runtime/import work is implemented. **The phase is not
fully accepted: all 47 final provider clips, their rights records and final
listening acceptance remain missing.** The connected-plugin search returned no
ElevenLabs or Suno integration. No alternative music provider or silent placeholder
was used. Generated **0**, imported **0/47**, wired **32/32 families**, production
clips audibly verified **0/47**. The actual game stays playable quietly and explains
the missing files in Settings.

Delivered:

- Application-owned audio using the unchanged pinned engine backend, independent
  Music/Ambience/Effects settings and master mute, synchronous Play/Continue/Enable
  sound activation, readable retry, suspension and gesture resume.
- Committed action/reward/purchase cues, switch/gate changes, ball presentation,
  cat-use/contact cues, variants, cooldowns, sparse garden birds, bounded loops,
  scene-generation cancellation and expiring late effects. Music survives ordinary
  hints/moves; scene changes release voices. Ending is a one-shot per entry.
- A 12-effect / 14-total voice ceiling, 24 MiB decoded-clip and 48 MiB retained
  PCM limits, serial loading and non-overlapping music fade/release transitions.
  Backend and decoder copies are additional memory, not hidden by those counters.
- Exact [production checklist](phase9-audio-production.md), stable 47-entry
  manifest/C++ catalog, metadata template and reviewed-WAV importer with source
  hashes, notices, waveform envelopes, peak/DC/loop/budget checks. See
  [the complete audio contract](phase9-audio.md) and
  [source handoff](../artifacts/audio-production/README.md).

Changed audio-owned files: `src/game_audio.*`, `src/audio_catalog.hpp`,
`web/paws-audio.js`, `assets/runtime/audio/catalog.json`,
`scripts/prepare-audio-catalog.py`, `scripts/import-audio.py`,
`tests/audio_tests.cpp`, `tests/test_audio_import.py`,
`scripts/phase9-smoke.cjs`, `scripts/phase9-audio-harness.cjs` and production docs.
Integration edits are in CMake, browser runtime, retained UI, HTML, package scripts
and the optional phase-9 hook in the existing phase-7 journey. Concurrent phase-8
save/import/recovery work was preserved, including restoring audio preferences
and invalidating sounds when an imported afternoon replaces the scene.

Validation against engine pin `3fe7d1dc1dc610c703944803cbb717cd65024ec9`:

- Pinned Emscripten **6.0.11 Release**, gallery **OFF**, generated site
  `build/web-phase9-pinned-release/site`; Wasm SHA-256
  `a6588a3bc098286d6fffdfd335843328951eb01f53944caba6f8fecea4c15399`.
- Native audio, GUI and starter lifecycle tests passed **3/3**. Audio assertions
  cover activation, bus/mute controls, cooldowns, limits, loop expiry, scene
  invalidation, music replacement, ending replay and late/oversized decode rejection.
  Isolated Python import regressions passed **6/6**.
- Chrome **154.0.8037.99** completed all three quests, buy/place/move/cancel,
  cat-use, ending/replay/reload and 1440×900, 390×844, 844×390 layouts. Additional
  blocked-roll, hint, undo and return-ball actions observed **all 32 cue families**.
  No console/page errors. Mute, volume changes, nonzero mix, suspension and gesture
  resume passed using **test-only non-silent signals intercepted in memory**.
- Synthetic-run peaks: **7 voices**, **2,875,140 bytes** retained PCM,
  **176,396 bytes** for a single decoded clip, sampled mix peak **0.01578**.
  These tiny fixtures do not establish final mix/clipping, full-length decoder
  peaks, pleasant loops, musical continuity, audible output or physical-device support.
- A separate browser context used the actual production catalog: zero imported
  files, zero WAV requests, zero voices and a readable missing-audio message.
  Runtime screenshots were inspected. Evidence: `build/evidence/phase9/journey/`
  (`report.json`, `audio-settings.png`, `actual-missing-audio-390x844.png`, journey
  screenshots and save), plus `build/evidence/phase9/native-tests.log` and
  `build/evidence/phase9/verification.json`. The reused runner's report build label
  remains phase 7; its explicit site path and Wasm hash identify this phase-9 build.

Launch/build/test commands are in [phase9-audio.md](phase9-audio.md). Use
`./scripts/serve.ps1 -SiteDirectory build/web-phase9-pinned-release/site -Port 8099`.
Nothing was published and the engine working tree/pin was not changed.

Next: complete the 47-clip provider/source handoff, import approved files, listen
to five actual-backend repeats per loop and the complete journey, then measure
the final mix/decoder budget on supported devices. Phase 10 may continue other
polish, but cannot close production readiness while audio and prior art/device
acceptance gates remain open.

## Phase 8 — durable saves, recovery and lifecycle, 11 October 2026

Prompt 8 is implemented over the existing `Afternoon` owner and WebEngine byte
adapter. No engine source, dependency pin, commit, push or publication changed.
The shared checkout's concurrent art and phase-9 audio work was preserved; these
save checks do not establish final animation or production-audio acceptance.

Delivered:

- Version-4 portable snapshots add snapshot revision, writer session identity and
  explicit user pause. Versions 1–3 migrate. Content/attempt identities, current
  state, up to 128 undo steps, settings, wallet/reward ledger, purchases,
  placements and ending status restore through validated rule replay.
- Ordinary moves coalesce for 500 ms. Completion, purchase, placement, restart,
  ending and other consequential navigation/settings changes snapshot immediately.
  Five complete history slots precede current replacement; only the latest
  completed receipt can publish Saved. Failures remain visible with Retry.
- Title/pause Save and recovery has real file download and file-picker import.
  Import stays disabled until ownership and initial loading resolve.
  C++ validates a staged candidate, then the user chooses Keep current or Use
  imported afternoon. Failed/cancelled imports preserve the live owner; successful
  imports invalidate old commands and cancel pending scene preparation.
- Startup selects the newest valid complete snapshot. Damaged/future data stays
  byte-for-byte in its original database and is exportable when readable by the
  engine byte adapter. Recovered/fresh play uses an independent namespace.
- Web Locks give the normal save one writer. A second tab plays temporarily with
  export access. Missing locking uses a unique independent slot. Unavailable
  storage permits clearly identified temporary play; Retry cannot overwrite an
  unread original. Independent sessions explicitly require export before leaving.
- Hidden/page-away/frozen reasons compose, flush pending snapshots and clear held
  input. Explicit pause survives reload, suspension and GPU recovery; clocks do
  not catch up hidden time. Normal garden navigation resumes play.

Main files: `src/afternoon.*`, `src/browser_main.cpp`, `src/paws_ui.*`,
`web/saves.js`, `web/host.js`, `web/index.html`, `tests/phase8_tests.cpp`, the
version-3 migration fixture, `scripts/phase8-smoke.cjs`, CMake/package registration
and architecture/README updates. [The full contract](phase8-saves.md) records
format, ownership, undo policy, limits and validation commands.

Validation: pinned Emscripten **6.0.11 Release**, GUI gallery **OFF**; **9/9** focused
native rules/codec/GUI/scene/lifecycle/audio checks passed. Chrome
**154.0.8037.99** passed the generated-site save/recovery suite and the complete
fresh three-quest earn/buy/place/use regression, with **zero unexpected browser
errors**. The latter earns 60 coins/3 friendship, buys both items for 55 coins,
restores 5 coins and placements, and verifies once-only ending/replay. The save
suite covers real export/import, corrupt/future bytes, fallback history, write
failure/Retry, delayed receipts, two tabs, unavailable storage/locking, eight
rapid restarts, three scene replacements, held input cancellation, explicit pause
reload, actual GPU device destruction/recovery and cancelled asset loads.

Evidence: `build/evidence/phase8/report.json`, **10 screenshots**, downloaded save
files and `native-tests.txt`; full gameplay regression:
`build/evidence/phase8-gameplay/report.json`, **21 screenshots** and
`played-afternoon.save`. Desktop, 390 × 844 portrait and 844 × 390 short-landscape
recovery captures were visually inspected; the gameplay suite also exercised
larger text and Chromium touch. Lifecycle visibility/freeze/page-away events were
synthetically injected. Physical Android/iOS, Safari, OS process eviction and
crash durability remain unverified. Journals remain bounded, without compaction.

Final acceptance Wasm SHA-256:
`7e473f347b62c155479205bc8a067f4f9225b8066adbef29aa6df8670a6bc74b`.
Engine pin: `3fe7d1dc1dc610c703944803cbb717cd65024ec9`, clean submodule.
The exact tested concurrent Pip asset hash is recorded in both browser reports.
The frozen site's 65 files were verified against the generated output before
acceptance; later concurrent builds/art edits do not replace this evidence.

Launch the frozen acceptance copy:
`./scripts/serve.ps1 -SiteDirectory build/phase8-final-acceptance-20261011/site -Port 8098`.
Development output is `build/web-phase8-pinned-release/site`.

Next: prompt **9**, continuing the audio work already present in the shared
checkout. Keep this save owner/codec and restore boundary. Final provider assets,
rights records, audible loop/mix acceptance and unresolved art/device gates retain
their own phase evidence; this phase does not close them.

## Phase 7 — shop, placement and ending gameplay, 11 October 2026

The prompt-7 domain/runtime/production GUI are delivered. Its true sleeping-curl
art gate remains open. The separate “Fix cushion settling animation” task repaired
disconnected joints and published a supported low rest, preserving the concurrent
walk and eyelid channels. Rejected curl experiments were never promoted. Do not
read passing gameplay tests as production animation or physical-device acceptance.

Delivered:

- Quest one opens the garden shop. Exactly one cushion costs 30 coins and one
  drinking bowl costs 25. Three once-only 20-coin rewards buy both and leave 5.
  Invalid, insufficient, repeated and stale purchases reject atomically.
- Shop/details show final renders of the accepted meshes, names, prices, reasons,
  owned states and explicit Buy. Three authored spots show actual model/footprint
  previews, compatibility/occupancy reasons and explicit Place/Cancel. Moves are
  free; Cancel/Escape preserve ownership and the prior committed placement.
- Version-3 snapshots retain ownership, stable placements and ending status with
  the existing reward ledger/wallet/settings/attempt journals. Versions 1/2
  migrate with no invented furnishings. Restore rejects inconsistent funds,
  ownership, spots, occupancy or ending flags before scene publication.
- Checked scene handles and Application clocks drive bounded approach/settle/rest
  or approach/drink/return. The rear bowl's side approach stays inside the fence.
  Reduced motion uses static readable poses. Cat playback cannot grant progress.
- All three quests plus a placed furnishing record the closing moment once.
  The compact “A lovely first afternoon” card returns to garden; menu replay
  preserves progress and reload never automatically repeats a seen ending.
- Larger text, keyboard, mouse and Chromium touch share the retained engine GUI.
  Placement uses fixed confirmation controls and camera space beside/above sheets.

The portable catalog and rules are `src/furnishings.hpp`, `src/afternoon.*`;
presentation is `src/garden_scene.*`, `src/puzzle_scene.hpp`, `src/browser_main.cpp`
and `src/paws_ui.*`. Native tests are `tests/phase7_tests.cpp`,
`tests/phase7_scene_tests.cpp` and the extended GUI test. The browser runner is
`scripts/phase7-smoke.cjs` / `npm run test:browser:phase7`. Preview masters,
provenance and hashes are in `artifacts/shop-previews/2026-10-11/`.
See [the complete contract and phase-8 handoff](phase7-shop-garden.md).

Validation: pinned Emscripten **6.0.11 Release**, GUI gallery **OFF**; seven focused
native rules, GUI, starter lifecycle and phase-5/6/7 scene tests passed **7/7**.
Chrome **154.0.8037.99** exercised the generated production site through all three
quests and the earn/buy/place/use journey, cancellation, occupied placement, free
movement, coherent reload, ending replay, 390 × 844 and 844 × 390 larger-text touch
layouts. Evidence: `build/evidence/phase7/report.json`, 21 screenshots,
`played-afternoon.save`, and `native-tests.txt`. Report hashes identify the exact
build and Pip/prop/preview assets tested; rejected curl candidates are excluded.
The focused `scripts/phase7-use-smoke.cjs` also passed from the actually earned
snapshot: `build/evidence/phase7-use/report.json` and six additional captures cover
settle/rest/return, drinking, reduced presentation and replay with unchanged money.

Final prompt-7 gameplay Wasm SHA-256:
`66f4b224816f3ee103644d58466423f117e222b5a903f1cdfa35cf533bdf21ae`.
Engine pin remains `3fe7d1dc1dc610c703944803cbb717cd65024ec9` with a clean submodule.
Final merged Pip GLB SHA-256:
`83192482aae477fdaebf848436b63abd024ffca1110e4bfca02d3a6b99cbcdc2`.
The full Pip, walk-contact and phase-7 scene checks also passed against this merged
asset; their output is `build/evidence/phase7/merged-pip-native.txt`.
Pip rig/walk corrections remain separate asset work. No engine changes,
commit, push or publication were made by this phase.

Launch the frozen acceptance copy:
`./scripts/serve.ps1 -SiteDirectory build/phase7-final-acceptance-20261011/site -Port 8097`.
It is a byte-for-byte copy of the generated pinned site. An initial final run
reached all gameplay assertions but correctly failed its integrity guard when
concurrent staging replaced Pip during the run; acceptance was repeated on this
frozen copy. `build/web-phase7-pinned-release/site` remains the development output.
The full build/test commands are in the phase-7 contract. Phase 8 should extend the
same owner and IndexedDB adapter with coalescing/history, import/export, session
locking, temporary-session recovery and wider lifecycle checks. First close the
Pip curl art gate against actual furnishings; physical mobile and Safari
compatibility/performance remain unverified.

## Phase 5 — first production quest and garden loop, 10 October 2026

Prompt 5 is implemented over the accepted Pip and garden kit and the concurrently
delivered phase-6 `Afternoon` owner. The separate courtyard now replaces the old
puzzle backdrop during garden visits. Earlier phase entries below are historical
evidence; their statements that the authored garden was absent are superseded.

Delivered:

- Play/Continue prepares the courtyard or saved puzzle. The courtyard has Pip,
  sandstone paving/edges, a low boundary, bench, restrained planting, two pots
  and three stable furnishing spots with compatible 1.10 × 1.35 metre footprints.
  The first reward adds Pip's returned ball to the home scene.
- Meet Pip, short dialogue with his accepted portrait, prepared production board,
  selection/actions, legal rejection reasons, Undo, three progressive free hints,
  known solution, happy reaction, actual reward card and Back to garden.
- First completion atomically records 20 coins and one friendship in the existing
  owner. Journal allocation precedes gameplay mutation. Undo, restart and practice
  preserve earned rewards and cannot grant them again.
- Version-2 local codec explicitly stores wallet, friendship, reward records,
  stable attempt/revision identities, command epoch, active quest and settings.
  Bounded command replay validates those records and reconstructs current puzzle,
  selected object, hints, pause/garden state and undo history. Version 1 migrates
  on the next write; malformed/future saves preserve their original bytes.
- Back to garden preserves the interrupted attempt identity. Restart creates a
  fresh attempt. Scene publication invalidates old command revisions; checked
  actor/component handles and Application clocks own presentation.
- WebEngine IndexedDB transactions persist coherent snapshots. Saving/Saved/Failed
  reflect the latest receipt; failed writes have Retry and stale receipts cannot
  mark a newer snapshot durable.
- Complete scene construction precedes publication. Failed asset loads preserve
  the previous scene/model, with explicit Retry loading. Cancelled preparation
  cannot publish a late response. Modal history is cleared at domain navigation;
  completion Back/Escape returns through the garden owner.
- Pip's happy action advances through Application while the immediately actionable
  reward card is shown; reduced motion uses the static accepted pose. Production
  garden and puzzle framing were inspected at desktop, portrait and short landscape
  with larger text. Shared controls support mouse, keyboard and touch.

Primary phase-5 changes: `src/garden_scene.*`, `src/browser_main.cpp`,
`src/afternoon.cpp`, `src/puzzle.hpp`,
`src/puzzle_scene.hpp`, `src/paws_ui.*`, `CMakeLists.txt`,
`tests/phase5_scene_tests.cpp`, `tests/paws_ui_tests.cpp`,
`scripts/phase5-smoke.cjs`, updated `scripts/phase6-smoke.cjs`, package script,
README and current architecture/GUI/puzzle contracts.
[The detailed garden/save contract](phase5-garden-loop.md) records ownership,
spot IDs, migration and launch instructions. No model assets or engine source
were changed. Engine pin remains `3fe7d1dc1dc610c703944803cbb717cd65024ec9`,
with a clean submodule. Unrelated staged/untracked work was preserved; no commit
or push.

Validation: pinned Emscripten **6.0.11 Release** browser build passed with the GUI
gallery **OFF**. Focused native rules, GUI, starter lifecycle and phase-5/6 production
scene tests passed **5/5**. Chrome **154.0.8037.99** exercised the actual generated
site with **zero unexpected console/runtime errors**. The deliberately intercepted
HTTP 503 is recorded separately. The browser run checked interrupted garden and
puzzle reloads, unchanged attempt IDs, 20/1 restoration, no duplicate practice
rewards, actual happy animation, saved text/motion settings, all three larger-text
layouts, failed writes/Retry, delayed/stale receipts, GLB failure/Retry, cancellation
and Chromium touch selection/movement/undo with ≥44-pixel controls.

Evidence: `build/evidence/phase5/report.json`, **18 screenshots**, and
`native-tests.txt`. The garden, portrait dialogue, reward, portrait puzzle and
short-landscape captures were visually inspected; broad filled placement markers
were corrected to quiet outlines. Build label `first-afternoon-phase5-v1`;
quest content `missing-ball-v1`; garden kit revision 7. Final tested Wasm SHA-256:
`4885f3864e3fcfcf0d77b5a9cfea9a8be602b39ae748092a5017def91443261e`.

The updated existing three-quest browser runner also passed against this exact
build: `build/evidence/phase5-phase6-regression/report.json`. It completed all
three puzzles through keyboard and mouse, exercised portrait/short-landscape
touch, restored held toys and an occupied switch, verified 60 coins/3 friendship
and no duplicate practice rewards, and repeated asset/save recovery with zero
unexpected errors. This confirms the later-phase flow survives the new courtyard
and version-2 codec.

Launch:

```powershell
./scripts/build.ps1 -Pinned -Configuration Release -EmsdkPath <installed-emsdk> -BuildDirectory build/web-phase5-pinned-release
./scripts/serve.ps1 -SiteDirectory build/web-phase5-pinned-release/site -Port 19485
# http://127.0.0.1:19485/
npm run test:browser:phase5
```

The acceptance runner owns its port-19485 server; stop a manually started server
before using it. It uses isolated browser contexts and does not overwrite a user's
saved afternoon. Physical Android/iOS, Safari, real screen readers and sustained
mobile performance remain unverified. There is no blocker for this phase.

A task-owned preview server remains on `http://127.0.0.1:19486/`, serving
`build/web-phase5-pinned-release/site`; its Python process ID is `52964`.
Stop that process when the preview is no longer needed.

Next: prompt **6**, whose two puzzles are already delivered below, then prompt **7**.
Use `Afternoon` for purchase/spending/placement transactions and extend its versioned
codec; do not add a UI wallet or parallel save owner. The authored garden spots are
ready for placement. Phase 8 retains multi-tab ownership, bounded backups,
export/import and journal compaction; phase 9 owns sound.

## Phase 4 — production garden kit, 10 October 2026

Delivered prompt 4 as `garden/kit@1`, revision 7: all 21 GLBs covering paving/edge,
fence/planting/bench, two pots, straight/curved troughs, coral banded ball, mouse,
fish, three-recess basket, distinct blocking/shop cushions, separate switch and
gate parts, raised ceramic bowl and separate water. Shared metre interfaces and
manifest anchors are compiled into explicit Paws composition. No engine changes.

The [editable delivery](../artifacts/model-jobs/garden-kit/2026-10-10/README.md)
contains packed Blender masters, retained scripts, built-in ImageGen originals,
portable material maps, provenance, seven revision checkpoints, clean reimport,
measured budgets and inspected correction history. Accepted GLBs total 45,588
triangles, 49,216 exported vertices and 7,750,336 bytes. Whole-kit preview with Pip
and extra paving measures 86,002 visible triangles and 54 colour draws.

Runtime: reusable `paws_garden`, `garden_preview`, `/garden-preview.html`, generated
`garden_manifest.inc`, explicit transforms and generation-checked moving-part
handles. The [contract](garden-kit.md) describes loading, retry, pivots, ports,
socket clearances and Pip use anchors. Final fit and browser checks use the
concurrently delivered `pip-tripo-repaired-r7`; earlier original-Pip captures remain
iteration evidence. Other phase work was preserved.

Validation: pinned Release browser build passed at engine
`3fe7d1dc1dc610c703944803cbb717cd65024ec9`; native garden and starter-scene tests
passed 2/2. Clean Blender process verified all exports and packed images. Chrome
154.0.8037.99 exercised actual HTTP models, gate/switch poses, toy fit, Pip/bowl,
keyboard, rapid pose changes, load failure/retry, shadow-free rendering and
1440×900/390×844/844×390 layouts with zero normal-page console errors. Every served
GLB hash matches the delivered asset. Physical mobile was not tested.

Evidence: `build/evidence/garden/`, retained copies in the job's `browser-evidence/`,
and [evaluation/build hashes](../artifacts/model-jobs/garden-kit/2026-10-10/EVALUATION.md).
Changed phase-4 files: garden model job/runtime folder, `src/garden_kit.*`, generated
manifest, preview main/host, `cmake/GardenPreview.cmake`, root include,
`tests/garden_kit_tests.cpp`, generator/browser scripts and linked documentation.

Launch: build with `scripts/build.ps1 -Pinned -Configuration Release -EmsdkPath
<installed-emsdk> -BuildDirectory build/web-garden-pinned-release`; serve that site's
directory on port 19484 and open `/garden-preview.html`. Run
`node scripts/garden-smoke.cjs`; native checks and overrides are in the contract.

Next phase: prompt 5's production garden/first-quest loop using these assets;
prompt 7 owns purchase/placement and full cat approach/use transitions. No phase-4
blocker remains. Concurrent later-phase delivery below has its own acceptance scope.

## Phase 6 — reusable toy-basket and gate puzzles, 10 October 2026

Implemented `pip_tidy_toys` and `pip_stuck_gate` after the existing ball puzzle.
The checkout began with phases 1–3; phase-4 garden assets arrived during this
work and are integrated through their shared library and manifest. Phase 5's
authored garden and save owner were absent. This phase supplies the minimal
`Afternoon` progression/save owner needed by the requested three-quest acceptance;
it does not claim the separate phase-5 garden staging is complete.

Delivered:

- Version-2 definitions reuse sliding, occupancy, finite routes, shapes, sockets,
  reachability, linked switch/gate state and collection. Each quest has a replayed
  known solution and three useful progressive hints.
- Cushion access blocking, explicit one-toy pickup and matching Place actions;
  wrong shapes, occupied recesses and blocked targets reject atomically. The GUI
  previews each target's legal/blocked reason and reveals actions after selection.
- Ball-controlled pressure switch, hinged gate and reachable mouse. Returning
  the ball or changing the route releases the switch; undo restores ball, switch,
  gate and collectible coherently. Correctness is independent of animation.
- Three independent attempts, ordered unlocks, undo/restart and practice. Each
  first completion grants 20 coins and one friendship exactly once, for 60/3.
- Versioned bounded local snapshots replay accepted commands to reconstruct
  attempts, undo history, hints, reward records and settings. WebEngine's
  IndexedDB adapter persists coherent snapshots. Saved follows only the latest
  transaction completion; failed writes have Retry. Invalid saves stay untouched.
- Production Pip and garden-kit meshes remain immutable and shared. Explicit
  authored gate/switch/basket anchors, matching toy orientations and channel
  ports are verified in native production scene tests. Ball motion follows the
  channel path; discrete transforms use Application presentation time and reduced
  motion. Scene construction is staged, asset loading is cancellable, and failed
  loads retain the previous scene/attempt with Help Pip as Retry.
- Mouse, keyboard-equivalent and touch-oriented controls, portrait/landscape
  framing, shorter portrait titles and unclipped wide objectives. No dragging,
  hover, sound or timed input is required.

Primary files: `src/afternoon.*`, `src/puzzle.*`, `src/content.cpp`,
`src/puzzle_scene.*`, `src/browser_main.cpp`, `src/paws_ui.*`, `web/host.js`,
`web/index.html`, `CMakeLists.txt`, `tests/puzzle_tests.cpp`,
`tests/starter_scene_tests.cpp`, `tests/phase6_scene_tests.cpp`,
`scripts/phase6-smoke.cjs`, `assets/runtime/manifest.json`, `package.json`,
README and architecture/GUI contracts. [Phase-6 details](phase6-puzzles.md)
document schema, ownership, saves and the next-phase boundary.

Validation: focused rules, scene/lifecycle, GUI and production asset projection
tests passed **4/4** against the pin. The pinned Emscripten Release browser build
passed. Final browser acceptance passed in Windows Chrome **154.0.8037.99**
with **zero unexpected console/runtime errors**, on the actual generated game.
All three puzzles completed with keyboard and again with canvas mouse actions;
390x844 and 844x390 used Chromium touch taps with controls at least 44 pixels.
The run verified 60 coins/3 friendship, no duplicate practice rewards, held-toy
and occupied-switch reloads, Continue without implicit restart, asset-load
cancellation/failure/retry, and stale/failed save completion plus Retry. One
intentional HTTP 503 is recorded separately from unexpected errors.

Evidence: `build/evidence/phase6/report.json`, 29 distinct screenshots and
`native-tests.txt`. The script verifies the Wasm hash stays unchanged throughout
acceptance. Build ID `puzzles-phase6-v1`; content IDs `missing-ball-v1`,
`tidy-toys-v1`, `stuck-gate-v1`; garden kit revision **7**. Final Wasm SHA-256:
`01ac35829bd845c8111a0350fc6a1fc0c5f9f4c2617e9134f66148ad9af65352`.
The earlier focused failure preflight is retained separately under
`build/evidence/phase6-failure-preflight/`.

Launch and repeat:

```powershell
./scripts/build.ps1 -Pinned -Configuration Release -EmsdkPath <installed-emsdk>
./scripts/serve.ps1 -SiteDirectory build/web-pinned-release/site -Port 8096
node scripts/phase6-smoke.cjs
cmake --build build/native --target paws_rules_tests paws_scene_tests paws_ui_tests paws_phase6_scene_tests
ctest --test-dir build/native -R '^(paws_rules|paws_starter_scene|paws_ui|paws_phase6_scene)$' --output-on-failure
```

Native Windows GCC executables need their compiler runtime directory on PATH.
The browser runner defaults to installed Chrome; `PAWS_URL`,
`BROWSER_EXECUTABLE` and `EVIDENCE_DIR` override its environment. It uses isolated
browser contexts, so acceptance does not replace a user's existing afternoon.

Continue with **prompt 7** using `Afternoon` as the single transaction owner.
Extend its snapshot contract for spending, ownership and placements; do not
subtract from a separate UI wallet. Complete/reconcile the absent phase-5
authored garden first: Garden currently retains the latest puzzle scene behind
its HUD. Phase 8 still owns multi-tab coordination, backup/export/import and
broader recovery/compaction. Physical mobile, Safari, actual screen readers and
long-session/performance acceptance remain unverified. Phase-4 asset quality and
provenance remain that delivery's responsibility. No engine changes or gitlink
advance; pin `3fe7d1dc1dc610c703944803cbb717cd65024ec9`. Unrelated staged/untracked
work and concurrent garden-kit production were preserved; no commit or push.

## Phase 3 — Pip asset delivery, 10 October 2026

Delivered prompt 3 as `cats/pip@1`, revision 9: a continuous ginger-tabby mesh,
canonical metre-scale rig, generated coat-colour input baked into a unique UV
atlas, independent roughness/normal inputs, nine in-place actions and three
same-model transparent portraits. Final budget: 14,660 triangles, one material,
29 deform bones (31 imported engine joints), four weights maximum, 2,097,156-byte
GLB. More than three inspected correction cycles are retained, including fixes
for coat colour space, shoulder/tail weights, folded-leg contact and portraits.

The packed master, source scripts, textures, GLB, manifests, provenance and
evaluation are linked from [the asset delivery](../artifacts/model-jobs/pip/2026-10-10/README.md).
The [rig/runtime contract](pip-asset.md) defines stable bones, axes, clip timing,
static reduced-motion samples, root policy, anchors and phase-4/7 fit guidance.

Runtime integration is an isolated Paws preview, `/pip-preview.html`, plus the
reusable `paws_pip` library. It uses the pinned engine's HTTP asset manager,
SkeletalMeshComponent, animation crossfades, Application clocks and generation-
checked handles. Retry handles failed asset loads; the browser host owns input
and visibility events. Desktop and shadow-free presentations are available.
The concurrently authored phase-1/2 game and GUI were preserved; production
garden/dialogue/furnishing composition remains the later phases' responsibility.

Changed files owned by phase 3: `src/pip_asset.*`, `src/pip_preview_main.cpp`,
`cmake/PipPreview.cmake`, its root CMake include, `tests/pip_asset_tests.cpp`,
`scripts/pip-smoke.cjs`, `web/pip-preview.*`, `assets/runtime/models/pip/`, the
model job, and linked README/architecture/status documentation. No engine changes,
commits or pushes. Engine pin remains `3fe7d1dc1dc610c703944803cbb717cd65024ec9`.

Validation: pinned Release browser build passed; selected portable Pip and scene
tests passed 2/2. Clean-process master reopen and GLB reimport passed. Chrome
154.0.8037.99 loaded the actual generated site with zero console errors and passed
all nine clips, rendered animation, static poses, rapid changes, host suspension,
HTTP failure/retry and three viewport layouts. Evidence: `build/evidence/pip/`,
plus the model job's logs and [evaluation](../artifacts/model-jobs/pip/2026-10-10/EVALUATION.md).
Build/content hashes are recorded there. Physical mobile was not tested.

Launch: build with `scripts/build.ps1 -Pinned -Configuration Release` and an
installed `-EmsdkPath`; the verified separate output is
`build/web-pip-pinned-release/site`. Serve it with
`scripts/serve.ps1 -SiteDirectory build/web-pip-pinned-release/site -Port 8094`,
then open `http://127.0.0.1:8094/pip-preview.html`. Run
`node scripts/pip-smoke.cjs` with the documented environment overrides. Native
checks use `ctest --test-dir build/native-pip -R 'paws_pip_asset|paws_starter_scene'
--output-on-failure` with the compiler runtime on PATH.

Next: prompt 4 production garden/puzzle/furnishing models, using Pip's dimensions,
root/contact plane and muzzle anchor. Final furnishing fit is checked with those
assets and composed into gameplay in phases 5/7. Chirrup audio is phase 9. No
external tool blocker remains for this phase.

## Phase 2 — production GUI, 10 October 2026

The phase-2 GUI is implemented over retained engine widgets and wired to the
phase-1 puzzle that arrived concurrently. The engine pin remains
`3fe7d1dc1dc610c703944803cbb717cd65024ec9`; its working tree is unchanged. Existing
phase-1 and concurrent phase-3 files have been preserved. No commit or push.

### Delivered

- Cream panels, teal actions, rounded code-native skins and original 24-unit
  icons; selected, disabled, hover, pressed, focused and blocked states. Copy is
  editable C++, with semantic labels and descriptions. Opaque disabled text stays
  readable instead of inheriting the engine's default faded lettering.
- Locally bundled static Fraunces Semibold and Nunito Sans Regular/Semibold,
  licensed under OFL, with source commit URLs, axes, sizes and SHA-256 records.
  Å/Ä/Ö and lower-case accents are retained. No runtime font CDN dependency.
- Title/continue, garden HUD, Pip dialogue, puzzle HUD and semantic object/action
  sheet, three free hints, pause/settings, restart confirmation, completion,
  shop/details, placement, ending and save/recovery widgets. Existing puzzle
  actions dispatch through PawsSession with attempt/revision checks. Rewards,
  purchases and durable saves remain future domain services, never fabricated.
- Modal focus capture/return, non-destructive initial restart focus, 52-unit
  controls, wrapped long labels, 125% larger text, fixed Back footer, scrolling
  sheets, portrait/short-landscape reflow and safe-area canvas bounds. Camera
  framing follows viewport changes and leaves room for the landscape tray.
- One owner per GUI/camera gesture; mouse, semantic keyboard and touch input
  share engine control callbacks. Blur/resize/suspension cancel contacts. The
  Application owns clocks; modal sheets pause scene simulation. Motion settings
  initialize from the OS preference; current puzzle presentation is immediate.
- Truthful HTML startup/font/graphics recovery and Retry. Session-only settings
  expose future independent audio controls; sound and manual quality are clearly
  unavailable until their corresponding game services arrive.
- Development gallery, OFF by default via `PAWS_GUI_GALLERY`, available only
  when enabled and opened with `?gallery=1`. It explicitly labels fixtures and
  suppresses domain dispatch. It includes earned/practice, locked/poor/owned,
  valid/blocked placement and six saving/recovery states.

### Changed files and contracts

`src/paws_ui.hpp`, `src/paws_ui.cpp`, `src/browser_main.cpp`, `CMakeLists.txt`,
`web/index.html`, `web/host.js`, `web/style.css`, `tests/paws_ui_tests.cpp`,
`scripts/gui-smoke.cjs`, `scripts/prepare-ui-fonts.py`, `package.json`,
`assets/runtime/fonts/`, `artifacts/gui/fonts/`, `README.md`,
`docs/architecture.md`, and this handoff. See [gui-integration.md](gui-integration.md)
for ownership, producer/consumer data, font composition and later-phase APIs.
The old phase-1 browser scripts target retired HTML controls; the successor is
`npm run test:browser` / `scripts/gui-smoke.cjs`.

### Validation

Native UI, rules and scene/lifecycle checks passed **3/3**. The pinned Emscripten
6.0.11 Release build passed with the development gallery enabled. Final browser
acceptance passed in Windows Chrome **154.0.8037.99**, with **zero unexpected
console/runtime errors**. Report: `build/evidence/gui-phase2/report.json`;
**118 screenshots** are listed there, alongside the report.

- Solved the known puzzle with Tab/Enter through semantic object/action controls;
  checked blocked Roll, undo, all three hints, cancelled/confirmed restart,
  nested modal focus return, and larger text.
- Exercised **14 gallery surfaces × three sizes × two text scales**, plus actual
  game HUDs: 1440×900, 390×844 and 844×390; standard and 125% text. Enabled action
  targets met the 44-pixel minimum and fit their clips after keyboard reveal.
- Captured hover/pressed/focus/disabled/selected/invalid, earned/practice,
  affordable/poor/locked/owned, valid/blocked placement and six save states.
- Verified GUI drags do not orbit, background drags do not commit moves, and
  Chromium touch taps/scrolling at 2× DPI do not activate unintended commands.
- Verified OS reduced-motion initialization, injected safe-area insets with
  correct touch coordinates, and a real intercepted font-fetch failure. The
  latter showed HTML Retry and did not enter gameplay; its expected HTTP 503
  messages are separated from unexpected errors in the report.
- Inspected rendered title, puzzle, blocked state, portrait/larger text, short
  landscape and recovery screenshots. Corrected the focused scroll-panel fill,
  clipped gallery header, long button wrapping, fixed Back reachability, camera
  resize wiring and short-landscape framing during these visual passes.
- A final supplemental `accented-glyphs.png` capture confirms the actual engine
  atlas renders Å/Ä/Ö and å/ä/ö; it is additional to the report's 118 screenshots.
- Rendered palette pixels matched the authored tokens. Contrast: ink/cream
  **10.89:1**, cream/teal **6.31:1**, disabled ink/linen **8.56:1**, error/cream
  **5.61:1**. Functional borders use the contrasted teal or muted-ink tokens.

Build ID: `gui-phase2-v1`; content ID: `missing-ball-v1`.
Final tested Wasm SHA-256:
`66548d50b25c432e173af96ea8101f5bf6d2f067459223e33aaf5d2233bf5748`.
The acceptance script verified this hash stayed unchanged throughout the run.

### Launch and next phase

```powershell
# Use the configured pinned Emscripten build, or run scripts/build.ps1 -Pinned first.
cmake -S . -B build/web-pinned-release -DPAWS_GUI_GALLERY=ON
cmake --build build/web-pinned-release --target paws_site
./scripts/serve.ps1 -SiteDirectory build/web-pinned-release/site -Port 19474
# Open http://127.0.0.1:19474/ or /?gallery=1.
node scripts/gui-smoke.cjs
cmake --build build/native --target paws_ui_tests paws_rules_tests paws_scene_tests
ctest --test-dir build/native -R '^(paws_ui|paws_rules|paws_starter_scene)$' --output-on-failure
```

Continue with **prompt 3**, whose independent Pip work is already arriving in the
checkout. Supply accepted canonical Pip portrait textures through `UiModel`;
production 3D art is not a phase-2 substitute or acceptance claim. Phase 4 supplies
shop thumbnails; phase 7 supplies furnishing/footprint preview data. Phase 5 owns
reward transactions and first durable saves. Phase 8 completes recovery services.

The current world remains the greybox puzzle, including the temporary garden
view. No shop, save or reward fixture represents working progression. Physical
Android/iOS, Safari, real screen readers and real GPU-loss recovery remain outside
this desktop Chrome/emulated-touch evidence. No blocker remains in the GUI's
portable implementation; later art and services have explicit integration slots.

## Phase 1 — delivered, 10 October 2026

Implemented prompt 1, **Foundation and a playable puzzle prototype**. The engine
pin remains `3fe7d1dc1dc610c703944803cbb717cd65024ec9`, with no engine source changes.
The repository started with staged submodule setup and untracked foundation
files; those were preserved. No commit or push was made.

### Delivered behavior and boundaries

- `PawsSession` / `GameState`, stable quest/object/endpoint/attempt IDs, typed
  commands and expected revisions. Old commands reject without partial mutation.
- Two sliding pots with overlapping authored slot sets and occupancy checks;
  three straight and two curved channel segments with discrete rotation.
- A nontrivial 5x3 layout, three hints and an executable known solution.
- Explicit finite Roll evaluation. Blockages identify the cause and leave the ball
  at source. Success moves the presentation ball to the destination.
- Selection, legal-action descriptions, undo (128 snapshots), restart, completion,
  pause and an explicitly labeled temporary garden/continue screen. All changes
  are immediate, so no action depends on animation, sound or rapid reactions.
- Portable rules/content have no engine, renderer or browser dependency. Scene
  bindings resolve generation-checked handles and reject old projections.
- Runtime-only staging from `assets/runtime/`; source masters under `artifacts/`;
  relative site URLs. No imported phase-1 media or production models.
- Readable WebGPU/Wasm/startup failures and Retry. Application clocks and host
  suspension/recovery servicing remain authoritative.

Phase 1 intentionally has no reward economy, durable saves, production Pip,
production garden, production GUI or generated art. A page reload starts fresh.

### Primary files

`src/puzzle.hpp`, `src/puzzle.cpp`, `src/content.cpp`,
`src/puzzle_scene.hpp`, `src/puzzle_scene.cpp`, `src/browser_main.cpp`,
`tests/puzzle_tests.cpp`, `tests/starter_scene_tests.cpp`,
`CMakeLists.txt`, `cmake/StageAssets.cmake`, `assets/runtime/manifest.json`,
`web/index.html`, `web/host.js`, `web/style.css`,
`scripts/phase1-browser-smoke.cjs`, `docs/phase1-content.md`.

The typed version-1 schema, coordinate/port meanings, solution validation and
state contracts are documented in [phase1-content.md](phase1-content.md).

### Validation and evidence

- Pinned native build: rules and scene/lifecycle tests **2/2 passed**, including a
  final focused run after concurrent phase-3 test creation unblocked CMake.
  Log: `build/native/Testing/Temporary/LastTest.log`.
- Rules also compiled directly with GCC 16.1 using only C++ standard headers and
  the three Paws source/test files; passed. `build/paws_phase1_rules_tests.exe`.
- Pinned Emscripten 6.0.11 Release browser build passed. Consumer bridge retained.
- Actual generated WebGPU game solved using mouse and keyboard controls in
  Chrome 154.0.8037.99 on Windows. Blocked roll/moves, occupancy, undo, restart,
  pause, garden retention, hints, narrow layouts and emulated touch passed with
  **zero console/runtime errors**.
- Successful build ID (Wasm SHA-256):
  `40b78bc97e442e07bd11cc9ac547ab203ef83a7111fed1edf593134113b6c894`.
  Content ID: `missing-ball-v1`.
- Scoped success record: `build/evidence/phase1/accepted-phase1.json`.
  Inspected solved/blocked/portrait images are alongside it. Desktop viewport:
  1440x900; portrait: 390x844; short landscape: 844x390.
- Host pagehide/pageshow events were dispatched in automation. Physical mobile
  compatibility, real OS backgrounding, real GPU loss/recovery, screen-reader
  acceptance and sustained performance were not established.

### Concurrent phase work / next phase

Phase 2 GUI and phase 3 Pip work appeared in this same checkout during the final
phase-1 checks. Their changes were preserved. They now also touch CMake,
`src/browser_main.cpp` and `web/`. The temporary phase-1 controls were verified
before that replacement. A later run of the legacy browser script against an
intermediate phase-2 build timed out because its old Roll button was hidden;
`build/evidence/phase1/browser-smoke.json` records that separately. Do not treat
that mixed build as phase-2 acceptance or replace its new GUI with the old shell.

The board was visually small in phase-1 screenshots; responsive closer camera
framing was added to `BrowserGame::reset/resize`. That correction built, but its
final GUI/framing acceptance must accompany phase 2 because the concurrent GUI
replacement changed the host and viewport. Retained screenshots and the successful
build hash above identify exactly the phase-1 behavior accepted.

Continue with **prompt 2**, or finish the already-running phase-2 work. Use
`PawsSession::dispatch` as the single command path. Preserve attempt/revision
validation, legal-action reasons and binding projection. Replace temporary HTML
controls with engine GUI; update browser tests to exercise its actual semantics.
Do not fabricate saves, purchases or rewards in production flows. Phase 5 must
establish the atomic reward boundary before allowing undo across completion.

### Launch and checks

```powershell
./scripts/build.ps1 -Pinned -Configuration Release -EmsdkPath <installed-emsdk>
./scripts/serve.ps1 -SiteDirectory build/web-pinned-release/site -Port 8097
cmake --build build/native --target paws_rules_tests paws_scene_tests
ctest --test-dir build/native -R 'paws_(rules|starter_scene)$' --output-on-failure
```

Open `http://127.0.0.1:8097` and verify the title/site matches Paws. Port 8080 was
already serving an unrelated WebEngine playground during this task. The legacy
`scripts/phase1-browser-smoke.cjs` uses `PAWS_URL`, `BROWSER_EXECUTABLE` and
`EVIDENCE_DIR`; it targets the historical temporary controls, so phase 2 must
supply the successor acceptance path. Native Windows GCC tests require the
matching compiler runtime directory on PATH.

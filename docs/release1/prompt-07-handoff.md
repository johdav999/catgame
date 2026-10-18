# Prompt 07 — retained release-one GUI, dialogue and puzzle presentation

Implemented 11 October 2026 against the unchanged, clean engine pin
`3fe7d1dc1dc610c703944803cbb717cd65024ec9`. The isolated GUI harness plays all
ten requests through the real Prompt 5 Afternoon owner and Prompt 4 rules.
Production composition remains Prompt 9's allocated work. No other track,
shared production source, common implementation-status, asset, engine, gitlink,
user browser save, commit, push or publication was changed.

## Delivered paths

- `src/release1/gui/views.hpp`, `views.cpp`: copied typed puzzle/progress/service
  projections, retained HUD, request/friend list, object/action paths, progressive
  hints, clues, completion, eight-item shop/details, compatible placement and
  four-cat picnic closing/replay.
- `src/release1/gui/dialogue.cpp`: short introductions, request/completion copy
  for every quest, including joint participants and the picnic.
- `src/release1/gui/gallery_main.cpp`, `web/release1/gui/`: isolated pinned
  browser executable/host. Single contact owner, startup/Retry, lifecycle,
  actual v6 export/import confirmation and temporary-memory notice.
- `src/release1/gui/runtime/paws_ui.hpp`, `paws_ui.cpp`: **generated patch
  fixtures**, reusing the current PawsUi implementation plus the narrow seam
  described below. Never ship these alongside the real PawsUi.
- `tests/release1/gui/tests.cpp`: 1,994 passing assertions, including all ten
  traces through focused retained buttons/Enter, stale identity, hints, real
  purchase/place cancellation, ending/practice idempotence, save roundtrip,
  modal focus return and responsive target geometry.
- `scripts/release1/gui/prepare.py`, `integration.patch`, `build.ps1`,
  `smoke.cjs`, `failure-smoke.cjs`: reproducible fixture/registration preparation,
  independent builds and actual rendered browser acceptance.

The existing PawsUi supplies Fraunces/Nunito lettering, rounded cream/teal
skins, focus indicators, text wrapping, scroll reveal and modal return stack.
Settings, sound/quality controls, restart/import confirmation, recovery and
loading/failure surfaces retain their existing implementation. UI code owns
presentation selection and copied snapshots, with no second puzzle, wallet,
purchase or save owner. Every domain operation emits a typed `release1::Intent`.
Puzzle action buttons forward the exact authoritative `LegalAction.intent`,
enabled flag and reason. The GUI does not test move/fit/route legality itself.

## Presentation and interaction

The request list distinguishes locked, available and practice requests, shows
chapter progress and each cat's actual friendship/milestone. Introductions are
short immediate text and call the owner's once-only `introduce` method. Hints
remain three progressive free commands. Completion distinguishes newly earned
20 coins/one point per requesting cat from already-rewarded practice. The
picnic closing has four short cat lines, continued garden play and explicit
reward-free replay; the old MVP ending does not substitute for this record.

The isolated board is an explicitly labelled **numbered development projection**. It is gated by `Model.developmentReferences` (false by default); production draws the prepared world board in the reserved area. The development projection is
not final scene/model acceptance. Object numbers map to the named action list.
Loose multi-cell cushions include their complete silhouettes outside the basket
edge. The clue sheet shows their cells and clockwise quarter-turn orientations;
the action list exposes compatible destinations and authoritative blocked
reasons. Bells show the required/entered leaf/star/paw word-symbol sequence,
including the complete silent clue in the objective. Routes show a trace and
only label it connected after completion. Switch/cat/door states use held/open
or released/closed words. Picnic SHADE and QUIET cell sets, shared-edge
adjacency and the combined-check reason remain readable without colour, sound,
hover or dragging. Every action has the same object/action keyboard path.

HUDs reserve an unobstructed board area. Portrait uses a concise heading,
wallet/chapter/save strip and two-row tray; short landscape uses two compact
top cards and one two-control tray. Larger text reflows at 125%; sheets scroll
while Back and Place/Cancel stay outside the scroll region. Native/browser
checks reveal focused offscreen controls and enforce minimum 44-pixel targets.
Visual review corrected the initially clipped portrait status and partially
cropped loose cushions before final acceptance. Completion/closing navigation sits in the fixed footer; compact portrait rows keep opening focus from scrolling past the first cat lines. The host retains one contact
owner through release/cancel/blur and honours CSS safe-area padding.

Accepted Pip's actual portrait and all eight Prompt 3 thumbnail PNGs are loaded
in the harness. Thumbnail byte lengths and SHA-256 values are checked against
the producer's manifest before publication. No new art or rights record was
invented. Mabel/Button/Pepper portrait slots accept real textures; absent art
gets explicitly labelled development identity text only in this harness.

## Owner and production integration

Apply Prompt 5's owner/codec patch first. Its CMake portion supersedes Prompt
4's older proposed registration. Then apply this **unapplied** GUI patch:

```powershell
git apply --check scripts/release1/progression/integration.patch
git apply scripts/release1/progression/integration.patch
git apply --check scripts/release1/gui/integration.patch
git apply scripts/release1/gui/integration.patch
```

This task checked its patch against the current baseline without applying it.
It adds a forward declaration/friend and one optional composition callback to
PawsUi. At its existing GUI rebuild boundary, `Views` composes release-one
surfaces or declines so the original service/settings/confirmation builder runs.
With no attached Views, MVP composition remains unchanged. CMake registers the
two GUI modules and native test on `paws_ui`, links the existing `paws_content`
owner and defines `PAWS_RELEASE1_GUI_PRODUCTION=1`. Prompt 5's transitive
`PAWS_RELEASE1_PROGRESSION_PRODUCTION=1` chooses the actual patched Afternoon.
Do not register either generated fixture directory in production.

Prompt 9 attaches one `Views` next to the existing `PawsUi` in BrowserGame:

```cpp
views = std::make_unique<paws::release1::gui::Views>(*ui,
    [&](const paws::release1::Intent& intent) {
        // Existing guarded command boundary; then publish scenes/audio/save
        // only from the committed result, and reproject current snapshots.
        auto result = afternoon.dispatchIntent(intent);
        // SelectQuest -> prepared scene + Dialogue; completed -> Completion;
        // RecordPicnic/ReplayPicnic -> supplied celebration + Ending;
        // accepted PlaceFurnishing -> Garden. Retain failure reasons.
    },
    [&](std::string_view cat, const paws::release1::Intent& identity) {
        auto result = afternoon.introduce(cat, identity);
        // Save accepted introduction and reproject pending introductions.
    },
    [&](std::string_view item) {
        // Project afternoon.placementView(item); prepare the existing preview.
    });
```

Project `gui::Model` with `afternoon.puzzleView()`, `progressView()`,
`pendingIntroductions()`, `lastRewardGranted()` and selected-item
`placementView()`. Supply the existing truthful `UiModel services` from the
save/audio/preparation host, including **actual** `inGarden`, feedback and
receipt-derived save state. Pass accepted portrait/thumbnail textures by stable
ID. The callbacks must guard the normal runtime readiness/lifecycle boundary.
Preserve the existing legacy service callbacks for Settings, BackToGarden,
Restart, import/export, save retry and audio activation. Example complete wiring
is in the isolated `gallery_main.cpp`; its service host is intentionally limited
to this harness and must not replace production `web/saves.js`.

`Views` must outlive its attached widgets/callbacks. Before GUI rebuild or
destruction, release `renderer.setGui(nullptr)` and `attachBrowserGui(nullptr)`
as BrowserGame already does. Destroy Views before PawsUi, and release renderer/
controller scene borrows before Application. The harness follows this order.
The Application owns advance/pause/presentation clocks; the GUI adds no tick or
scene owner. Background return preserves an open Pause GUI.

The harness marks all furnishing references ready **only for isolated economy
verification with its labelled development board**. Production availability
must come from Prompt 6's actual prepared models/assemblies; do not copy that
readiness override. Production should use its scene boards with the same
object/actions and clue projections, and preserve its asset failure/recovery
and explicit-pause policy. World selection/drag is optional; equivalent GUI
buttons remain present. Views' board bounds provide a framing exclusion region.

## Current verification and payload

Pinned native Release GUI suite: **1/1 passed, 1,994 assertions**.
Existing UI, starter lifecycle, shop/save and garden-scene suites: **5/5 passed**.
Pinned Emscripten 6.0.11 Release browser build passed.

Final site: `build/web-release1-p07-pinned-release/site`.
Wasm SHA-256:
`449700068cc3ef3f9fa92e72dda465c3fcd36d0d9b4709a1df4727d5400b5dfe`.
Browser: desktop Chrome **154.0.8037.99**, pinned WebGPU renderer on Windows.
Content: `paws-release1/1`, all ten frozen content definitions.

Evidence under `build/evidence/release1/prompt-07/`:

- `native-final.txt`: portable GUI assertion log.
- `browser-final/report.json` and PNGs: all ten real keyboard completions,
  introductions/hints, 200 earned coins, actual 30-coin cushion purchase,
  cancelled and confirmed placement, four-cat ending/replay, practice without
  reward, v6 export/import cancellation/confirmation, nested modal focus,
  single gesture ownership, muted/reduced-motion controls and emulated touch.
- The final browser run has **zero console/runtime errors** and verifies the
  Wasm hash remains unchanged. Shape, simultaneous-switch and picnic surfaces
  were captured at **1440x900, 390x844 and 844x390**, each at standard/larger
  text: **18 layout cases**, board/actions/clues/settings captures. Enabled
  targets fit their clips after focus reveal. Final portrait board, landscape
  switch board, action sheets, picnic clues, shop and closing captures were
  visually inspected.
- Actual rendered cream/teal/ink token pixels were detected. Text contrast is
  **10.89:1 ink/cream** and **6.31:1 cream/teal**. State labels and numeric
  silhouettes carry information independently of palette differences.
- `services/report.json` and PNGs: all six explicitly simulated save-service
  states at the three sizes with larger text; real invalid-codec import leaves
  progress intact; Pause persists through synthetic host background/return;
  actual intercepted font HTTP 503 exposes readable HTML Retry. The expected
  fault is isolated from normal-run errors. Its payload hash matches the final
  journey. These UI status fixtures are not IndexedDB receipt acceptance.

The first browser-runner startup guard incorrectly referenced an undeclared
global; it was corrected to `window.pawsGuiReady` before the accepted runs.
Native test launch initially used the wrong MinGW runtime; the build script now
puts the configured compiler's matching `bin` first on PATH. These failed
attempts are not acceptance evidence. Historical `browser/` captures predate
the visual corrections; use `browser-final/` for the delivered payload.

## Reproduce and launch

```powershell
./scripts/release1/gui/build.ps1 -JoltSource C:/Users/Johan/source/repos/WebEngine/build/dependencies/JoltPhysics-5.3.0
./scripts/release1/gui/build.ps1 -Browser -EmsdkPath C:/Users/Johan/source/repos/WebEngine/.tools/emsdk -JoltSource C:/Users/Johan/source/repos/WebEngine/build/dependencies/JoltPhysics-5.3.0
$env:EVIDENCE_DIR='build/evidence/release1/prompt-07/browser-final'
node scripts/release1/gui/smoke.cjs
Remove-Item Env:EVIDENCE_DIR
node scripts/release1/gui/failure-smoke.cjs
git apply --check scripts/release1/gui/integration.patch
./scripts/serve.ps1 -SiteDirectory build/web-release1-p07-pinned-release/site -Port 19617
```

Open `http://127.0.0.1:19617/` and check the **Release-one GUI workshop** title
and visible development banner. The smoke runners each own/stop their server,
with ports 19617/19619; run them before starting the manual server. Override
`SITE_DIR`, `EVIDENCE_DIR`, `PAWS_TEST_PORT` or `BROWSER_EXECUTABLE` when needed.
Machine-specific SDK/Jolt paths above are example commands, not committed
dependency configuration. Build directories are isolated from other tracks.

## Remaining integration and external acceptance

Prompt 7's allocated GUI modules, dialogue, isolated real-intent acceptance and
registration patch are complete. Prompt 9 must attach them to production
BrowserGame, final Prompt 6 scene boards/cats/furnishings and the existing durable
save/audio coordinators. This harness proves GUI and owner paths, not the final
garden, model contact, animation, audio or production asset-preparation journey.
Missing cat portraits remain explicit slots; no development references should
ship. The final four-cat celebration scene requires Prompt 6/9's supplied props.
Physical mobile/desktop device matrix, Safari, real assistive technologies,
independent first-player discovery/pacing and full-release acceptance remain
later gates; Chromium viewport/touch emulation and scripted traces do not close
them. The common implementation-status document remains integration-owned.

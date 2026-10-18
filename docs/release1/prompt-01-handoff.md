# Prompt 01 handoff — contracts and baseline

Implemented 11 October 2026. The three-request MVP remains working. Release-one
features are allocated/frozen for later tasks; no new quest/progression owner,
engine changes, pin update, commit, push or publication was made.

## Deliverables and changed paths

- `docs/release1/contracts.md`, `contract.json`: ten quests, four cats, eight
  items/prices, unlock/reward/ending/migration policies, placement footprints,
  role/anchor/rig/action/audio interfaces, manifest format and seven disjoint tracks.
- `docs/release1/canonical-rig.json`: actual accepted Pip skin-joint parents,
  rest TRS and inverse binds, including source SHA-256 and numerical tolerance.
- `src/release1/contracts.hpp`: portable command/read-model/event DTOs only.
- `tools/release1-track/CMakeLists.txt`, `baseline.cpp`: independent native
  test and optional browser-preview build entry points without production registration.
- `scripts/release1/validate-contract.py`, `tests/release1/test_contract.py`:
  identity, DAG, reward distribution/economy, canonical authority and disjoint-path
  validation; optional fragment hash/namespace/rights-record checks.
- `scripts/release1/baseline-smoke.cjs`: sequential current-site journey,
  recovery and direct-controls entry point, owning isolated ports/browser contexts.
- `src/paws_ui.cpp`, `tests/paws_ui_tests.cpp`: restored explicit Move/Rotate
  controls and regression coverage for enabled/disabled commands and identity.
- `scripts/phase7-smoke.cjs`: wait for current ball completion presentation
  before seeking the reward card. This fixes the runner's timing assumption.
- `docs/architecture.md`, `docs/implementation-status.md`: current boundaries
  and scoped Prompt 1 evidence. The pre-existing untracked release brief is preserved.

Shared files touched here are now the baseline. Later tasks leave exact owner/
registration patches in their own handoffs for Prompt 9. No additional agents or
other chats were used. Local adjacent WebEngine has unrelated tracked/untracked
development work; it was inspected read-only and preserved. The pinned submodule
is clean at `3fe7d1dc1dc610c703944803cbb717cd65024ec9`.

## Payload and observed acceptance

Game base commit: `64a95f15ac530870e4ee689f801a121a0b5efe7a`, with this task's
uncommitted baseline correction. Pinned Emscripten 6.0.11 Release, GUI gallery OFF.

| Identity | Value |
| --- | --- |
| Site | `build/web-release1-p01-pinned-release/site` |
| Build ID | `paws-0.1.0-141c15263730f1df` |
| Wasm SHA-256 | `98566692b5c8eae482e5b94b1b02b1a8eefdd8313d541751a3b980ec638ad250` |
| Runtime content SHA-256 | `1959fa2d10a07ef77fba0f6864c9fdf492b7c780fd385cde084568bb25bed7e3` |
| Notice tree SHA-256 | `24e12796be95110624b46f338bb1ebb7b71bfe1608848d67af160d729dcaae32` |
| Accepted Pip GLB SHA-256 | `da50380685ce63cd7ea42d7ed3f49580ed607e4fc3a7ce9093f377533be241e2` |

Fresh scoped observations:

- All 13 portable suites passed, including rules, GUI, saves, scenes/lifecycle,
  garden kit and current Pip asset/walk/settle regressions.
- Isolated native track baseline passed (1/1): all three known solution traces,
  60 coins/three friendship, codec roundtrip and stale identity rejection.
  Only that test is registered by the final standalone harness.
- Contract validator and two invariant test groups passed; invalid identities,
  prerequisite cycles/references, reward totals and overlapping task paths reject.
- Chrome 154.0.8037.99 actual generated-site journey passed: all three quests
  through keyboard controls; blocked roll/hints/undo; buy/cancel/place/free moves;
  occupied spot rejection; real bounded cat uses; coherent reload and MVP ending
  replay; 390x844/844x390 larger-text/reduced-motion/touch-sized placement flows.
  Zero browser console/runtime errors.
- Recovery runner passed: mid-attempt reload/undo, coalescing, real file import/
  export and v3 migration, second-tab ownership, failed-write retry and latest
  receipts, five-history limit, synthetic lifecycle boundaries, actual test-device
  destruction/recovery, future/corrupt original-byte retention, temporary slots,
  repeated restart/scene transitions and cancelled preparation. Zero errors.
- Direct-controls runner passed all three quests with mouse and emulated touch,
  blocked/cancelled/legal drags, selected-piece rotation, keyboard arrow
  alternative and visible sampled ball travel before completion.
- Standalone browser harness compiled/staged and loaded the accepted Pip GLB
  through the pin; an actual browser selected a reduced-motion Garden walk action.
  This proves the harness, not new cat/model production.

Evidence is under `build/evidence/release1/prompt-01/`: `native-final.txt`,
`track-native-final.txt`, `contracts.json`, `journey-final/report.json`,
`recovery/report.json`, `direct/report.json`, `track-preview/report.json` and
their screenshots/save bytes. Production `site/build-info.json` identifies the
payload. [baseline.json](baseline.json) is the compact repository-local summary.

Early failed records are retained separately: `journey/` records the hidden
Move controls; `journey-corrected/` records the runner racing authored arrival.
Both are superseded by `journey-final/`. An early unfiltered CTest invocation in
the first harness build tried to run unbuilt inherited baseline tests; the final
harness disables baseline registration and passes in its fresh final directory.
The matching native compiler runtime was added to the process PATH; no machine
configuration was changed. Sandbox-blocked compiler execution was rerun with
approved access to installed tools.

## Reproduce

Run from the repository root. The SDK/Jolt paths below describe this observed
machine only; shared configuration continues to pin `external/WebEngine`.

```powershell
./scripts/build.ps1 -Pinned -Configuration Release -BuildDirectory build/web-release1-p01-pinned-release -EmsdkPath C:/Users/Johan/source/repos/WebEngine/.tools/emsdk -Jobs 4
cmake --build build/native --parallel 4
# Put the build's CMAKE_CXX_COMPILER directory on PATH for GCC runtime DLLs.
ctest --test-dir build/native --output-on-failure
python scripts/release1/validate-contract.py
python -m unittest discover -s tests/release1 -p 'test_*.py'

cmake -S tools/release1-track -B build/native-release1-p01-final -G Ninja -DCMAKE_BUILD_TYPE=Debug -DPAWS_TRACK=rules -DFETCHCONTENT_SOURCE_DIR_JOLT=C:/Users/Johan/source/repos/WebEngine/build/dependencies/JoltPhysics-5.3.0
cmake --build build/native-release1-p01-final --target release1_track_tests --parallel 4
ctest --test-dir build/native-release1-p01-final --output-on-failure

# Full rerun: scripts own ports 19601–19603 and close them on completion.
$env:SITE_DIR='build/web-release1-p01-pinned-release/site'
$env:EVIDENCE_DIR='build/evidence/release1/prompt-01-rerun'
node scripts/release1/baseline-smoke.cjs
# Interactive launch of this exact payload:
./scripts/serve.ps1 -SiteDirectory build/web-release1-p01-pinned-release/site -Port 19605
```

The recorded journey/recovery checks were invoked individually with
`SITE_DIR` as above, `PAWS_UAT=1`, ports 19601/19602 and their respective evidence
directories. Direct controls used `PAWS_DIRECT_ONLY=1` with the new combined
runner; its default direct server used 19603. Browser contexts were temporary,
not the user's browser/profile/database.

To reproduce the harness-preview proof, copy `web/pip-preview.html` into a
task-local preview shell as `index.html`, replacing `pip_preview.js` with
`release1_preview.js`; copy `pip-preview.js`, `pip-preview.css` and favicon.
Configure through emcmake with `-S tools/release1-track`, `PAWS_TRACK=cats`,
`PAWS_TRACK_BROWSER_MAIN=<repo>/src/pip_preview_main.cpp` and
`PAWS_TRACK_WEB=<shell>`, then build `release1_track_site`. This run used
`build/web-release1-p01-harness` and `build/release1-p01-preview-shell`.
New tasks must create their own shell/modules in the allocated namespace.

## Next owners and open dependencies

Prompts 2/3/4/8 can begin independently using the frozen paths and harness.
Prompt 3 can build fixed role interfaces while Prompt 4 finalizes authored
board/socket coordinates; read its handoff before final basket/assembly layout.
Prompt 5 depends on 4; 6 depends on 2/3/5; 7 on 4/5; 9 merges all owner patches.
Each task writes its own handoff and evidence/build suffix; no common manifest,
root build registration or monolithic owner edits during parallel work.

No migration is installed here: version 5 remains the production writer. Version
6 must preserve all supported v1–5 attempts/rewards/purchases/settings/recovery
metadata. Old Pip friendship maps only to Pip, new cats/rewards start at zero,
old MVP ending remains separate and cannot suppress release-one picnic. See the
full migration table and independent ending eligibility in the contracts.

Mabel/Button/Pepper user Tripo inputs, final audio files/rights/listening, natural
Pip/Button sleeping curl, independent discovery/pacing and supported physical
device acceptance remain open. Existing low-rest/bowl-use browser observations
are scoped presentation evidence, not natural-curl or final-art acceptance.
No physical mobile, Safari, sustained device-performance, accessibility
certification or 45–75-minute player acceptance is claimed.

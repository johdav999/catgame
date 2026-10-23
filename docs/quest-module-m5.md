# M5 — save-compatible quest pilots

Paws now uses the published `WebEngine::Quests` through the compiled module system
for two pilot quests: `pip_missing_ball` and `beach_fresh_water`. The engine pin
remains `76d17928a5e51bbaac53874867e08a510e5b9c8f`; M5 adds consumer integration.

## Ownership and transaction boundary

`Afternoon` owns an immutable `QuestAdapter` projection shared by speculative
copies. `paws.quest-pilots` registers its versioned fact contract and dependency
on `webengine.quests` through `ModuleComposition<quests::Bindings>`. The catalogue
reuses the built-in typed fact objectives and the existing prerequisite graph.
No browser, SDL, rendering, audio, scene callbacks or wall clock enter this layer.

Every accepted puzzle/common intent stages a full game candidate and a fresh
kernel transaction before publishing the candidate without allocation. Rejection
or comparison failure leaves the live journals, ledger and kernel unchanged.
Legacy definition allocations remain stable across publication for callers that
iterate a borrowed authored solution while dispatching moves.

The adapter compares prerequisite availability, current completion, durable earned
completion and stable receipt IDs across all 29 quests. The two pilots commit
their ledger entries through the kernel; their legacy live grant branches are
disabled. The remaining 27 quests retain legacy authority and run read-only
shadow comparisons. Shadow effects cannot mutate the wallet or UI and shadow
state is reconstructed rather than saved as independent truth. Pack access is
still transient Paws policy, checked in addition to kernel prerequisites.

Paws continues to own puzzle legality, history, undo, content upgrades, coins,
friendship, purchases, endings and presentation. The modern pilot observes the
existing water-basin `active` fact: stage `basin` advances to `puzzle` when filled.
Undo can regress that stage. Retry retains durable completion and the
`<quest-id>/first-completion` receipt, so practice cannot award a second reward.
The beach pilot never adds base coins or friendship.

## Version 13 and recovery

The writer now emits `PAWS_AFTERNOON 13`. The earlier sections retain their
existing journals, content IDs and economy validation. Before `END`, the additive
section contains exactly two ID-keyed pilot records:

```text
QUEST_KERNEL 1 2
"pip_missing_ball" "missing-ball-v2" <attempt> <earned> <completed> <checkpoint> "<stage>" "<receipt>"
"beach_fresh_water" "beach-fresh-water-v1" <attempt> <earned> <completed> <checkpoint> "<stage>" "<receipt>"
```

Versions 1–12 first pass the existing historical replay and ledger validators.
The kernel is reconstructed in replay mode, suppressing completion effects.
Version-13 records must then match that reconstruction, including actual content,
attempt identity, stage, completion and receipt. Missing, duplicate, unknown,
future-schema or inconsistent records reject the entire isolated save candidate.
Runtime activation tokens, queues, diagnostics and scene references are transient.
The save stores the two validated pilot projections rather than duplicating the
other 27 shadow states or embedding a second authoritative puzzle snapshot.

Before replacing an old browser save, the writer commits the exact original to
the non-rotating `pre-quest-m5` IndexedDB key. Failure to preserve it prevents the
new history/current write. Windows saves it as `saves/pre-quest-m5.save` before
allowing migration writes. Existing backups are never overwritten; secondary
writers cannot create backups or replace the current save. Original-save export
remains available after reload. Invalid startup bytes continue through the
existing separate recovery path. File import retains its source file and uses
the existing confirmation and scene-preparation boundary.

An M4 binary cannot read version 13. Deliberate rollback uses the preserved
version-12 original and an old build; it does not carry later version-13 progress.

## Verification

Generated evidence is under `build/evidence/m5/`:

- `native.trace` / `wasm.trace`: identical traces from 694 pilot, shadow,
  migration and rejection checks on each target; all 99 frozen save fixtures
  convert and round-trip. Forged attempts, earned flags, completion, checkpoint,
  content, stage, receipt, duplicates and missing/future sections reject.
- The historical replay suite exercises all 29 quests, 398 original solution
  commands, undo/restart/practice, supported old schemas and branch-order tests.
  All 101 M0 artifact hashes remain unchanged.
- The checkout already contained v2 windbreak and shell-encore layouts before
  M5. The replay tool selects their frozen v1 definitions for the old trace and
  allows their explicit restart upgrades to current content. Comparison excludes
  only those two journal sections and the resulting owner revision; other save
  fields and the original outcome trace still compare. Historical fixtures are
  never regenerated to hide a migration difference.
- Windows `native-legacy-seed` / `native-legacy-resume` and
  `native-water-seed` / `native-water-resume`: real SDL GUI input, shared typed
  puzzle commands, minimize/restore, process reload, basin undo, practice and
  garden travel with one scene and unchanged reward policy.
- `native-old-migration`: an existing profile migrates to 13 while preserving
  its exact old bytes. Browser pilot runs exercise the actual generated site on
  forced WebGPU and WebGL2 using mouse puzzle gestures and retained GUI controls.
- `interchange`: actual browser download/upload and Windows file import preserve
  progress, wallet and friendship on both browser renderers.
- `migration-storage`: controlled failure of the original-backup write leaves
  version-12 current bytes unchanged; retry completes migration and original
  export after reload returns the exact old file on both renderers.
- `webgpu-recovery` / `webgl2-recovery`: failed cached IndexedDB connections,
  quota failures, queued writes, retry, reload and second-writer protection pass.
- `identity.json` records the served Wasm, executable, pin and trace hashes;
  final interchange is recorded separately in `final-interchange`.

`scripts/quest-pilots-smoke.cjs` tests browser practice through the supported
recovery flow using validated post-completion undo saves from the portable test.
The existing completion screen offers Back to garden; it does not offer Menu or
a new undo control. Browser acceptance does exercise ordinary move/basin undo.
Completion undo is additionally covered by native/Wasm rules and the Windows host.

Builds use the pin. This run reuses the existing incremental directories
`build/web-pinned-m4-release` and `build/windows-pinned-m4`; their directory names
describe their origin, while M5 evidence records the current payload hashes.
Standard new builds use `scripts/build.ps1 -Pinned` and
`scripts/build-windows.ps1 -Pinned` as documented in M4. The portable migration
suite can be built independently:

```powershell
cmake -S tools/quest-baseline -B build/quest-pilots-m5 -A x64
cmake --build build/quest-pilots-m5 --config Release --parallel 4
ctest --test-dir build/quest-pilots-m5 -C Release --output-on-failure
python tools/quest-baseline/manifest.py --verify
./build/quest-pilots-m5/Release/paws_quest_pilots.exe build/evidence/m5/native.trace
# Build the same directory with emcmake in a separate Wasm build tree,
# then run paws_quest_pilots.js with Node and compare the trace hashes.
```

Native Windows and desktop Chrome evidence does not establish physical mobile or
other desktop OS acceptance. The Paws changes remain local alongside existing
work. M6 will migrate the remaining 27 quests and remove duplicate live authority.

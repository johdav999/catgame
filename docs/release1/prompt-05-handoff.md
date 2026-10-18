# Prompt 05 — chapters, economy and version-6 saves

Implemented 11 October 2026. The isolated owner extension passes **2,452
assertions**, the existing purchase/save suites against that extended owner,
all 13 unchanged portable baseline suites, and pinned WebAssembly compilation.
Production registration/composition remains Prompt 9's responsibility under the
release-one ownership contract. The shared game still writes version 5 until that
merge; this task does not claim a shipped ten-quest browser journey.

## Delivery and ownership

- `src/release1/progression/policy.hpp`, `policy.cpp`: frozen quest graph,
  four-cat friendship/milestones, eight-item economy, authored spot roles and
  footprints, owner commands, introductions and immutable presentation snapshots.
- `src/release1/progression/codec.hpp`, `codec.cpp`: version-6 writer/candidate
  reader, supported MVP migration, validated semantic replay and original-byte
  preservation through `inspectSave`.
- `src/release1/progression/owner.hpp`: selects the exact patched Afternoon
  fixture for isolated builds and the real Afternoon after production integration.
- `src/release1/progression/runtime/afternoon.hpp`, `afternoon.cpp`: generated
  **test fixtures for the shared-owner patch**, not an additional runtime owner.
- `scripts/release1/progression/prepare.py`, `integration.patch`, `build.ps1`:
  reproducible fixture/patch preparation, exact unapplied shared changes and
  native/Wasm entry points. `fixtures.py` records how historical save fixtures
  were derived from the existing checked-in MVP save.
- `tests/release1/progression/`: regression suite, versions 1–5 fixtures and
  generated adaptations of the existing phase-7/8 tests.

There is one Afternoon, one ten-entry reward ledger and one derived wallet.
Afternoon keeps its three original `PawsSession`s and journals and owns seven
Prompt 4 `Attempt`s with independent semantic journals. No second progression
service, save scheduler, engine fork or persisted engine/GUI/audio handle was
introduced. Shared source/build files and other tracks were not edited. Dirty
concurrent cat/prop/audio work and user-supplied mesh archives were preserved.
No agents, commit, push or publication.

## Implemented behavior

The prerequisite DAG preserves the first three ordered Pip quests, opens all four
Chapter 2 requests together, then bells/playhouse together, then picnic. Purchases
never gate a puzzle. All 24 Chapter 2 orders are tested, alternating both Chapter
3 orders. Ten once-only rewards yield 200 coins. Wallet is that reward count
times 20 minus owned-item prices; buying all eight costs 190 and leaves 10.
Undo, restart, practice and replay do not remove or duplicate paid rewards.

Friendship comes from the same ledger: Pip 5, Mabel 3, Button 3, Pepper 3. The
joint quest pays one point to Pip and Pepper; picnic pays one to every cat.
Milestones use Pip's 1/3/5 thresholds and 1/2/3 for the others. No redundant
mutable friendship or wallet can drift from the ledger.

Pip starts introduced. Chapter unlocks derive pending Mabel/Button/Pepper dialogue
records. `introduce(cat, expectedIntent)` acknowledges a shown dialogue once in
the same owner revision domain; cancellation must not call it. Migration creates
pending introductions for newly available cats rather than inventing already
shown dialogues. Acknowledged records survive save/reload. No introduction grants
coins or friendship, and unavailable chapters cannot acknowledge their cats.

Catalog IDs/prices/unlocks and all nine placement spots match the frozen contract.
Ownership, affordability, lock state and missing presentation assets remain
distinct. The two accepted MVP assets default available. The six new entries
default unavailable until presentation explicitly calls `setItemAvailable(id,true)`
after verifying its assets; merely owning a file or a catalogue entry does not
claim runtime acceptance. Playhouse references the actual prop handoff's
`assembly:playhouse_furnishing`, not an invented monolithic GLB ID.

Placement checks ownership, matching role, dimensions and occupancy. Moves are
free. Existing cushion/bowl still fit all three legacy spots, preserving old
placements. `placementView(item)` exposes enabled spots and the exact owner
blockage reasons; GUI need not duplicate placement rules. Unknown, locked,
unaffordable, unavailable, incompatible, occupied and repeated purchases reject
without changing the encoded owner snapshot.

The old MVP ending and new picnic ending are separate. Picnic eligibility requires
all ten paid quests and any owned placed furnishing; purchasing picnic props is
not required. Recording is once-only, explicit replay advances the presentation
generation without granting rewards, and shopping/moving remain available.

## Portable interface for Prompts 6 and 7

Use `owner.hpp` for task-local consumers. In production, its compile definition
selects the patched `src/afternoon.hpp`.

```cpp
auto progress = afternoon.progressView();
auto puzzle = afternoon.puzzleView();
// PuzzleView.actions already contain current IDs, legal flags and reasons.
auto result = afternoon.dispatchIntent(selectedAction.intent);

auto choose = afternoon.intent(release1::Verb::SelectQuest, {}, questId);
auto selected = afternoon.dispatchIntent(choose);
auto home = afternoon.returnToGarden(afternoon.intent(release1::Verb::Select));
auto buy = afternoon.dispatchIntent(afternoon.intent(release1::Verb::Buy, itemId));
auto spots = afternoon.placementView(itemId);
auto place = afternoon.dispatchIntent(
    afternoon.intent(release1::Verb::PlaceFurnishing, itemId, spotId));
```

Every release-one intent carries owner generation, active attempt, owner revision
and current quest ID. `SelectQuest.target` is the destination quest; the intent's
`quest` remains the currently observed quest. Other object/target fields retain
Prompt 4 meanings. Rotations are clockwise steps with `orientation=0`, not an
absolute pose. The owner remaps validated intent identities onto its private
Prompt 4 attempt and publishes the resulting owner revision once with rewards.

`ownerRevision()` is the atomic gameplay/progression revision. It is distinct
from `saveRevision`, which remains the existing snapshot/receipt counter managed
by BrowserGame. Runtime generation is not serialized. Selection, restoration,
pause and host suspension invalidate old typed intents. `setSuspended` is a
transient host reason; clearing it cannot clear `userPaused`. Use
`setUserPaused(value, expectedIntent)` for explicit pause/resume.

`active()` covers all ten quests. `quest(i)` and `session()` remain **legacy-only**
PawsSession APIs; `session()` checks its array bound instead of silently returning
the wrong old quest for a new active request. New scenes and GUI must use
`puzzleView()` and `progressView()`. For legacy scenes, `quest(0..2)` remains the
actual original session. Existing legacy command/garden APIs remain for MVP
compatibility; use the typed owner seam consistently in new production composition
rather than mixing two revision domains in a single GUI flow.

`ItemView.available` means presentation assets are ready, not affordable. Inspect
`unlocked`, `owned` and `reason` as well. Generic `ProgressView.spots` describes
occupancy; only `placementView(selectedItem)` answers item-specific legality.
`QuestView.completed` describes the current attempt; `rewarded` records an earlier
paid completion even after undo/restart. `lastRewardGranted()` is transient and
only meaningful after an accepted command; decoding never re-emits rewards.

## Save format and migration

Version 6 retains `PAWS_AFTERNOON` and wraps the existing validated version-5
section in `LEGACY`, using `std::quoted` escaping. That section preserves original
action ordinals, journals, attempt/revision IDs, settings, recovery metadata,
two old purchases/placements and the historical ending. Its currency projection
is only the legacy section's validation checksum, not another gameplay wallet.

The `R1`, `CATS`, `ITEMS` and `QUESTS` sections add selected stable quest ID, owner
revision, garden state, picnic record, derived-wallet checksum, ID-keyed cat and
item records, and all ten quest/content/reward/attempt identities. Only the seven
new quests add semantic journals here; legacy journals remain in their original
section. New verbs have explicit versioned textual names (`slide`, `fit`,
`tap_symbol`, etc.), never serialized enum ordinals. Slot ordering in the file
does not supply identity. Runtime asset readiness/suspension/generation are not
saved. The original bounded 128-step undo is reconstructed by rules replay.

| Input | Retained | Release-one initialization |
| --- | --- | --- |
| v1 | Settings and all three replayed command journals, undo and rewards | Pip receives the old paid quest count; no new rewards/items; automatic quality, legacy recovery defaults |
| v2 | Explicit old epoch, attempts/revisions, wallet and reward checksums | Same mapping; no new grants |
| v3 | Cushion/bowl ownership, prices, placements and historical ending | Old ending never marks picnic seen |
| v4 | Snapshot revision, writer identity and explicit user pause | Recovery semantics retained |
| v5 | Display quality | Same settings and metadata retained |
| v6 | All ten independent journals, ledger, four introductions, eight items, both endings | Full candidate validation before publication |

Old Pip friendship is exactly the number of paid original Pip quests. Mabel,
Button and Pepper begin at zero. A completed MVP unlocks Chapter 2, queues its
new introductions and leaves picnic unseen; it grants no extra money.

Reader checks the 16 MiB file cap, 100,000 commands per quest, section counts,
duplicate/unknown stable IDs, content versions, command ranges, attempt epochs,
prerequisites, replay legality, reward/friendship/wallet agreement, purchase
unlocks, placement ownership/roles/occupancy and finale eligibility. It validates
all seven journals with Prompt 4's actual command processor before publication.
Unknown future, corrupt, truncated and trailing content rejects.

`inspectSave(std::string)` returns an optional validated candidate plus the exact
original bytes and error. Before replacing a live owner, call
`candidate.invalidateImportedCommands(previous)`; prepare required scenes/assets
and publish with move assignment only after success. Deep owner copies replay the
codec and preserve transient generation/readiness; they are intended for existing
scene/import preparation boundaries, not a per-frame or per-command operation.

## Shared integration patch

`scripts/release1/progression/integration.patch` is **unapplied** and passes
`git apply --check`. It updates `src/afternoon.hpp/.cpp`, root CMake registration
and the old phase-7/8 save tests. Those tests now extract the legacy section for
their historical byte-level corruption fixtures instead of assuming the writer
still outputs a bare version-5 header. They are also run against the task fixture.

This patch's CMake portion **replaces Prompt 4's proposed split-library
registration**: add its rules/content and this progression/codec to the existing
`paws_content` owner target, avoiding a circular dependency between Afternoon and
the new catalogue. Both Prompt 4 and Prompt 5 test targets are registered. Do not
compile `runtime/afternoon.cpp` or a second copy of the owner in production. Merge
Prompt 8's independent audio registration alongside this, and rebuild all consumers
because Afternoon's layout changed.

Prompt 9 still owns BrowserGame/GUI/garden composition: replace its three-only
iteration, selection and scene paths with these views; use the owner identity for
import confirmations and new commands; restore new active quests through the
appropriate scene; acknowledge shown introductions; verify/publish furnishing
availability; route the independent picnic entry/replay. The old scene install
path cannot render a new request just because the codec can read it.

`web/saves.js` is unchanged. Keep its normal-tab Web Lock, independent temporary
namespaces, five history snapshots, complete-snapshot writes, latest-receipt
Saved status and export/import/original-byte handling. The extended owner still
exposes `saveRevision`, `saveSession`, `userPaused`, `encode`, `decode` and import
rebasing. Do not add another storage key, coordinator or partial wallet write.
End-to-end version-6 browser recovery belongs to Prompt 9's composed-site checks;
this portable delivery does not claim it was exercised here.

## Evidence and reproduction

Pinned engine `3fe7d1dc1dc610c703944803cbb717cd65024ec9` stayed clean. The adjacent
engine's unrelated changes were inspected read-only and preserved.

- Task native suite: 2,452 assertions plus the existing phase-7/8 suites against
  the extended owner. Covers five historical versions, interrupted independent
  attempts/hints/undo, rebasing, repeated/stale actions, all chapter orders,
  purchase failure/availability, free placement/occupancy, 200/190 economy,
  per-cat milestones, old/new finale independence, pause and corrupt originals.
- Existing portable baseline: 13/13 passed, including scene/lifecycle suites.
- Emscripten 6.0.11 Debug portable Wasm library compiled and linked as an archive.
  This is compilation evidence, not a generated browser game or device test.
- Exact patch applicability passes. Source fixtures and originals were preserved.

`build/evidence/release1/prompt-05/` records `native.txt`, `baseline.txt` and
`identity.json`. Native executable SHA-256:
`a7370c8825357dbb611ec6b3cf5317149985766d07b49f8331f6023d4d6860df`.
Wasm archive SHA-256:
`a462d85315040cf43105698bf4a1dafe3cfb54edd73ab50feb2c093c5714fc6f`.
An early compiler run caught a missing namespace brace. Combining historical
`main` functions into the harness required explicit return values; both were
corrected before the final passing run.

```powershell
python scripts/release1/progression/prepare.py
git apply --check scripts/release1/progression/integration.patch
# Put the configured native compiler's bin directory on PATH for its runtime DLLs.
./scripts/release1/progression/build.ps1 -JoltSource C:/Users/Johan/source/repos/WebEngine/build/dependencies/JoltPhysics-5.3.0
./scripts/release1/progression/build.ps1 -Wasm -EmsdkPath C:/Users/Johan/source/repos/WebEngine/.tools/emsdk -JoltSource C:/Users/Johan/source/repos/WebEngine/build/dependencies/JoltPhysics-5.3.0
ctest --test-dir build/native --output-on-failure
```

The SDK/Jolt paths describe the observed machine only; the harness enforces the
game's pinned engine. `fixtures.py` reproduces v1/2 historical-format projections
and v4/5 metadata additions from `tests/fixtures/phase7-completed.save`; v3 is an
unchanged copy of that actual existing fixture. No existing user save was written.

No asset, audio, physical-device, player-pacing or production-browser acceptance
is inferred. Those do not block the completed portable progression/codec work.

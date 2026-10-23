# Durable afternoon saves and browser lifecycle

M6 now writes `PAWS_AFTERNOON 14`, including full-catalogue quest-kernel records
validated against the existing journals and ledger. Versions 1–13 remain readable.
Browser `pre-quest-m6` and Windows `saves/pre-quest-m6.save` preserve the exact old
save outside rotating history before replacement; backup failures prevent a
migrated write. M5 backups remain intact. See [the M6 migration contract](quest-module-m6.md). The phase
descriptions below retain the historical format milestones.

Release-one Prompt 9 now writes `PAWS_AFTERNOON 6` through this same coordinator.
The ten attempts, per-cat friendship, eight purchases/placements and separate
picnic ending use the validated release-one codec. Supported versions 1–5 retain
their legacy journals/reward identities and historical MVP ending; migration
does not grant new rewards or mark the picnic seen. File confirmation constructs
a valid candidate before publication. Explicit Pause persists through import,
reload, composed background reasons and renderer recovery. The numbered phase
details below document the earlier MVP format; current evidence is in
[the integration handoff](release1/prompt-09-handoff.md).

Prompt 8 extends the existing `Afternoon` codec and WebEngine IndexedDB adapter.
It adds no second game-state owner and changes no engine sources or dependency pin.

## Format and publication

`PAWS_AFTERNOON 4` adds an explicit unsigned snapshot revision, writer session ID
and user-pause choice. Existing content IDs, active quest, attempt/revision IDs,
command epoch, settings, once-only reward ledger, wallet, friendship, ownership,
stable placements and ending state remain in the same coherent record. Versions
1–3 migrate through the existing validated replay path; version 4 reserializes
exactly. The deployed format remains the portable Paws text codec, rather than
introducing the proposed generic ApplicationArchive envelope during this phase.

Phase 10 advances the writer to `PAWS_AFTERNOON 5`: one additional line after
the snapshot/session/pause metadata stores quality (`0` Automatic, `1` Mobile,
`2` Desktop). Versions 1–4 migrate to Automatic. Unknown quality values reject
before replacing live state. All existing record and reward validation remains.

Undo history, selection, hints, held toys and switch state are reconstructed by
replaying accepted typed commands through the actual rules. The existing limit of 128
undo steps remains in force. Journals retain at most 100,000 accepted commands
per quest; files are limited to 16 MiB. Journal compaction is not implemented.
Unsupported versions/content, impossible actions, inconsistent ledgers/placements,
invalid metadata, oversized or trailing data fail before publication.

`browser_main.cpp` constructs the candidate scene and validates renderer rebinding
before replacing the owner. Import also rebases attempt identities above both
sessions, cancels pending asset preparation and rebuilds GUI/settings/presentation.
Runtime handles, callbacks, clocks, voices and renderer objects are never loaded
from file. Audio restoration uses the concurrent phase-9 integration's settings
and scene reset boundary; final production sound acceptance belongs to phase 9.

## Persistence and recovery

`web/saves.js` owns browser policy, while `EngineStorage.IndexedDBStorage` owns
transactions. The normal database namespace is `paws-afternoon`. Ordinary moves
debounce for 500 ms. Completion, purchase, restart, placement, ending, navigation,
settings and explicit pause changes snapshot immediately. Visibility/pagehide/
freeze flush an already captured snapshot; shutdown completion is never assumed.

Writes run in order. Each writes a complete `history-0` through `history-4` slot
before replacing `current`. Older history is replaced cyclically, keeping at most
five backups plus current. A crash between the two transactions still leaves a
complete recoverable history record. Startup validates all six bounded candidates
and selects the highest compatible snapshot revision. There are no independent
wallet or reward writes.

Saving becomes Saved only after the latest request's receipt. Failed transactions
show Failed/Retry; receipts for older requests cannot change the newest status.
Manual Retry captures the latest model. No unbounded background retry runs.

Retry first drains queued writes, disposes the cached byte adapter and opens a
fresh connection in the same namespace. This recovers a browser-closed IndexedDB
connection without deleting records, releasing writer ownership or reloading the
game. Queued operations acquire their adapter when they execute, so writes made
during recovery cannot use the disposed connection. Only the latest save receipt
can set or clear the displayed error. The recovery panel explains quota and
storage-permission failures; `pawsSaveDiagnostics().lastWriteError` retains the
bounded browser error details until a successful latest write.

`scripts/save-recovery-smoke.cjs` exercises the generated site in isolated browser
contexts: closed connections, persistent quota errors, debounce/retry overlap,
reload and second-tab ownership. `SAVE_FIXTURE` optionally supplies a local export
for validating preservation of actual quest progress, without using the player's
browser profile. `QUEST_BACKEND=webgl2` selects fallback-renderer acceptance.

2026-10-22 recovery acceptance: build `paws-0.1.0-7c98762033396405`
passed this suite on WebGPU and WebGL2. Reports are under
`build/evidence/save-recovery/{webgpu,webgl2}/report.json`. The actual port-8080
tab was exported before reload, its validated export restored through Import,
and saved progress verified after another reload: 240 coins, 13 total friendship,
9/11 beach requests including the completed shell workshop. Screenshots are
`build/evidence/save-recovery/live-saved.png` and `live-beach-progress.png`.
The old live tab still failed Retry with disk space available; its original
exception was discarded by the previous catch handler. A previously full disk
is the suspected trigger, not a proven browser exception. The reproduced defect
is deterministic: a closed cached database raises `InvalidStateError`, and the
old Retry reused it indefinitely. The new retry reopens storage without clearing
records. This is browser save-adapter acceptance; native persistence is unchanged.

If any stored candidate is malformed or from a newer format, its original data
stays in its original database. A valid backup can be recovered, or a fresh game
started if none is usable. Further play writes an independent random namespace.
The recovery surface explains this and offers an exact original-byte download
when the engine adapter could read the byte record. A structurally corrupt
IndexedDB wrapper also remains untouched, but cannot be exported through the
adapter's byte interface.

An independent namespace is unique to the current page lifetime. It deliberately
does not reuse a sessionStorage key that another duplicated tab could inherit.
The player is told to export it before leaving; opening the usual URL continues
to inspect the usual slot. A slot-selection library is outside this phase.

## Tab ownership and file UI

A feature-detected Web Lock, `paws-afternoon-writer`, stays held until the browser
terminates the page, including suspension/BFCache. A second tab may read the same
coherent snapshot and play in memory, clearly marked Temporary/another tab. It
cannot write to the shared slot. Without Web Locks, a random independent database
is used. IndexedDB failure permits identified temporary play with file export;
Retry uses an independent namespace so an unread original cannot be overwritten.

Save and recovery is available on both title and pause screens. Export downloads
the current coherent model as `paws-afternoon.save`. Import uses the browser file
picker after session ownership and initial loading resolve, enforces the byte
limit, validates in C++, then presents an explicit
Keep current / Export / Use imported afternoon choice in retained engine GUI.
Cancellation or failure leaves the current game unchanged. A stale file-read
completion cannot replace a newer attempt/revision. File metadata is not rendered
as markup or used as executable data.

## Lifecycle and acceptance

`web/host.js` composes hidden, page-away and frozen reasons, cancels held GUI/camera
input, and sends suspension to Application. Clearing one reason cannot resume
another. Application rebases clocks without catch-up. GPU recovery continues at
frame boundaries. Pause remains explicit across suspension and reload, while
normal navigation back to the garden resumes play. Renderer/controller references
are rebound before the previous scene is destroyed.

The generated-site suite `scripts/phase8-smoke.cjs` uses isolated browser contexts
and an owned loopback server. It covers real download/file selection, migration,
invalid imports, bounded history, reordered receipts, write failure/Retry,
two-tab isolation, unavailable storage, absent locking, backup/original recovery,
pause reload, actual GPU device destruction, lifecycle boundaries, cancellation
and rapid restart. Hidden/freeze/page-away events are synthetic browser lifecycle
injections; they do not establish OS process eviction or physical-mobile behavior.
`tests/fixtures/phase7-completed.save` is the v3 migration fixture copied from the
recorded phase-7 played afternoon (5 coins, both furnishings, ending seen).

```powershell
./scripts/build.ps1 -Pinned -Configuration Release -EmsdkPath <installed-emsdk> -BuildDirectory build/web-phase8-pinned-release
npm run test:browser:phase8
$env:SITE_DIR='build/web-phase8-pinned-release/site'
$env:EVIDENCE_DIR='build/evidence/phase8-gameplay'
$env:PAWS_TEST_PORT='19488'
npm run test:browser:phase7
./scripts/serve.ps1 -SiteDirectory build/web-phase8-pinned-release/site -Port 8098
```

The phase-7 runner rechecks the full fresh earn/buy/place/use/reload journey on the
new save implementation. Reports record exact Wasm/asset hashes, Chrome version,
screenshots and errors. See implementation status for the final results.
Physical Android/iOS, Safari, OS tab eviction and crash durability remain untested.
# Restart adventure

The game menu offers **New Game** directly below **Restart puzzle**,
separate from restarting one puzzle or replaying the picnic ending.
Its confirmation defaults to **Keep playing** and offers **Export save** before
resetting. Confirmation creates a fresh Chapter 1 garden, clearing all ten quest
attempts, rewards, friendships, introductions, purchases, placements and finales.
Sound, display and accessibility preferences are retained. Command identities and
save revisions advance past the previous adventure, preventing stale commands or
older autosave history from restoring its progress. The normal save coordinator
persists the new campaign and reports failures through Save and recovery.

The separately labeled **Replay picnic celebration** replays the recorded ending,
not the adventure. After the celebration has been seen it is available from a
puzzle menu as well as the garden: the host prepares a garden candidate and the
existing finale before publishing either. Coins, rewards and puzzle attempts are
retained. Before the ending is seen the menu explains the unlock requirement and
points to **New Game** for a full reset.

`scripts/restart-adventure-smoke.cjs` checks cancellation, reset from a completed
save, persistence after reload, retained preferences and narrow-screen access in
an isolated browser profile. Native progression tests also verify cleared
furnishings, relocked quests and earning the first reward again.


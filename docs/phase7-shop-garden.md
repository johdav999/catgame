# Phase 7 shop, furnishings and closing moment

The owner/runtime/UI implement prompt 7 over the pinned WebEngine. Animation art
acceptance is tracked separately below; gameplay checks cannot establish that
Pip's curl is production ready.

## Authoritative transactions

`Afternoon` owns the fixed catalog: one garden cushion for 30 coins and one
drinking bowl for 25. Quest one opens the shop; its 20 coins are insufficient for
either item. Three once-only rewards earn 60 coins; both purchases leave 5.
Buy and Place carry the active attempt and expected revision. Invalid IDs, stale
commands, insufficient funds, repeated purchases and non-garden commands leave
the portable snapshot unchanged. A committed action increments the same session
revision used by GUI commands. String allocation precedes placement mutation.
Coins derive from reward and purchase ledgers, rather than a second mutable wallet.

`furnishings.hpp` defines three 1.10 × 1.35 metre spots: `bench_corner`,
`sunny_corner` and `quiet_corner`. Catalog footprints enforce compatibility; one
item occupies a spot at a time. Moving an owned item is free. Preview selection
is separate from committed placements, with independent checked preview actors.
Place commits; Cancel/Escape discards only the preview, including when an item was
already placed. Conflicts preview the actual model and footprint with a reason.

## Presentation and lifecycle

Shop/details use inspected transparent 512px renders of the unchanged accepted
garden meshes. Placement shows the actual model and footprint; camera framing
leaves space beside or above the sheet. Larger text and shared input use retained
C++ GUI. Place/Cancel stay in a fixed footer.

Pip walks an authored clear lane to the selected anchor, settles/rests or lowers/
drinks for five seconds, then gets up and returns along the lane to idle. The rear
bowl uses a side approach inside the fence. Cushion get-up samples settle in
reverse. Reduced motion uses a static rest/drink pose and returns without travel.
Menu pause and hidden-page suspension use Application clocks; scene replacement
clears checked handles. Playback never changes progress or hides entry controls.

All three completed quests plus any placed owned furnishing make the ending
eligible. Its flag records before showing the compact card. Continue returns to
the garden; menu replay becomes available. Restore/replay never grant another
reward or automatically repeat an ending already seen. The closing presentation
uses the same bounded furnishing sequence and accepted Pip identity.

**Art gate:** the original `curled_rest` read as a compact crouch. The independent
“Fix cushion settling animation” work has repaired its disconnected joints and
published a supported low rest. A true sleeping curl remains unaccepted. Candidates in
`artifacts/model-jobs/pip/2026-10-11-cushion-use/` were rejected and never promoted.
Final pose/deformation and full revised use acceptance remain required. These
gameplay checks do not establish a production-ready MVP.

The repaired low rest itself passed the full phase-7 journey and focused use
captures on the frozen generated site; see `build/evidence/phase7-use/report.json`.
`scripts/phase7-use-smoke.cjs` reloads the actually earned phase-7 snapshot and
rechecks real cushion/bowl playback, reduced motion, replay and unchanged economy
when an accepted animation asset is updated. It is not a shortcut to pose acceptance.

## Saves and phase 8

`PAWS_AFTERNOON 3` adds two ownership flags, quoted stable placement IDs and the
ending flag before the three existing journals. Versions 1/2 migrate with no
invented purchases, placements or ending. Restore validates unlock/funds,
ownership, known compatible spots, occupancy, ending eligibility, explicit wallet
and replayed rewards before publication. UI, actors, previews and cat animation
are reconstructed. Undo keeps the existing replay behavior. Invalid/future bytes
remain preserved by the host.

Purchase/placement/ending immediately snapshot through the existing WebEngine
IndexedDB adapter. Saved follows the newest receipt; failed writes offer Retry.
Phase 8 must harden coalescing/history, import/export, second-tab ownership,
storage-unavailable recovery and wider lifecycle acceptance without adding another
wallet or persistence owner. This phase does not establish browser locking or
export/import acceptance.

## Verification and launch

`tests/phase7_tests.cpp` covers atomic/stale/repeated rejection, economy,
compatibility/occupancy, free moves, once-only ending, v3 roundtrip, corrupt fields
and v2 migration. `tests/phase7_scene_tests.cpp` loads actual models and covers
preview cancellation, bounded use, stride-speed travel, static poses, pause,
suspension and stale handles. GUI checks cover stable spot commands, captured
confirmation, narrow footer bounds and Escape cancellation.

`scripts/phase7-smoke.cjs` drives the actual generated game with keyboard, mouse
and Chromium touch. It completes all puzzles, buys both items, cancels/places/
moves, rejects occupied spots, captures use and verifies reload, 5 coins,
ownership, placements, no repeated automatic ending and replay. Its isolated
origin does not touch the user's existing save. Evidence includes hashes, report,
screenshots and `played-afternoon.save`. See [status](implementation-status.md)
for the exact build. Physical mobile browsers and Safari remain untested.

```powershell
./scripts/build.ps1 -Pinned -Configuration Release -BuildDirectory build/web-phase7-pinned-release -EmsdkPath C:/Users/Johan/source/repos/WebEngine/.tools/emsdk
./scripts/serve.ps1 -SiteDirectory build/web-phase7-pinned-release/site -Port 8097
npm run test:browser:phase7
```

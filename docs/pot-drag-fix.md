# Pot drag position correction

## Shared-track recovery, 16 October 2026

The reported layout had Left pot in `(1,1)` and Middle pot in `(1,0)`.
Middle pot cannot move west from there: `(0,0)` belongs only to Left pot's
track. Its downward route is occupied by Left pot. The reversible recovery is
Left south to `(1,2)`, then Middle south/east to `(2,1)`. This restores the
starting arrangement without Restart or a save reset. The former generic
blocked-slot message and faint, overlapping track inlays concealed this move.

`slidingRoutes` now provides a single portable complete-path preflight for
browser drops and contextual cream/teal parking outlines. Selecting a sliding
obstacle shows only reachable destinations; selecting a channel hides them.
`slidingAdvice` names the blocking obstacle when no move is free and tells the
player how to free the track. Rejected drops retain the entire attempt and name
an occupied destination where applicable. The earlier grab-offset correction,
authored tracks, puzzle solution and save codec remain unchanged.

Native `paws_rules`, `paws_phase6_scene` and `paws_starter_scene` passed 3/3,
including the exact reported arrangement, backward recovery, a legal corner
drag, current occupancy in the outlines, selection changes and Restart.
Browser acceptance used the actual in-app browser with real pointer drags:
the final pinned build restored the exact layout after reload at 2187x964;
the configured local build at port 8080 loaded the same stacked layout and
recovered both pots at 1280x720 without Restart, rewards or replacing the
normal save. Both showed the named blocker and contextual cream outlines.
Evidence: `build/evidence/pot-recovery/browser.json`, screenshots and HTTP
JavaScript/Wasm checksum records in `site-identity.json`. No browser errors.

The configured adjacent WebEngine checkout initially failed its Wasm build in
`src/terrain_paging.cpp`: one `auto` declaration tried to deduce both a 64-bit
page key and a 32-bit `size_t` index. Splitting it into two declarations repaired
that build without changing behavior or the game's engine pin.

15 October 2026. The first three quests' sliding objects now commit the same
actor position that the player sees during dragging.

## Root cause and reproduction

The legacy browser adapter retained the pointer's original grab offset when
moving its preview. On release, it discarded that offset and projected the
pointer directly onto the paving to choose a logical cell. A pointer on a tall
pot's body or rim projects onto the ground behind the visible pot. The shallow
puzzle camera made that difference large enough to select an extra row or an
unauthored cell, producing a jump or the misleading blocked-slot message.

`scripts/pot-drag-smoke.cjs` reproduced this on the actual served game at
2187 × 964. A real pointer drag intended to move Left pot from `(1,2)` to
`(1,1)` instead committed `(1,0)`, moving two rows. Its preview was at the
intended intermediate square. The failing report, original build identity and
screenshot are retained in `build/evidence/pot-drag/before/`.

## Correction

`BrowserGame::objectPointer` converts `finished.preview` to the grid when
committing sliding pieces. This is the actor origin used by the visible preview,
with the original grab offset already applied. Grid spacing still follows the
production/fixture scene convention. The existing complete-path preflight
checks all intermediate cells before committing any move.

This applies to the pots, blocking cushion and sliding crate using that adapter.
Puzzle layouts, legal slots, gate rules, save format, camera, models and authored
solutions remain compatible. Cancellation and paused Undo continue to restore
committed actor poses immediately.

The older browser runners dragged from the prop centre directly to a ground
cell. That gesture exercised the faulty pointer-to-ground mapping and missed the
preview mismatch. Maintained runners now retain the grabbed offset for numeric
sliding destinations. The focused regression also grabs the body, upper rim and
side, and asserts the actual logical cell after every drop.

## Checks

- `paws_rules` and `paws_phase6_scene`: passed; output in
  `build/evidence/pot-drag/native-results.txt`.
- Served-game focused pointer regression: passed all 13 drags, including
  one-cell moves from three grab positions, an occupied shared slot, blur
  cancellation, paused Undo, a multi-cell drag, save/reload and the completed
  channel route with 20 coins. Report and screenshots:
  `build/evidence/pot-drag/after/`.
- The same 13-drag regression passed against the pinned-engine Release build:
  `build/evidence/pot-drag/pinned/report.json`.
- The first three quests completed through real mouse gestures, including both
  pot puzzles, the cushion, the gate's crate, paused Undo, blocked moves and a
  final reload retaining the 60-coin reward ledger:
  `build/evidence/pot-drag/first-three-quests/report.json`.
- HTTP payload hashes match the generated build records in
  `build/evidence/pot-drag/site-identity.json`. The served configured-local-engine
  build is `paws-0.1.0-4880ff4b78af085d`; the pinned build is
  `paws-0.1.0-2370fb782a5a6d0a`. Both are Release builds with galleries disabled.

Browser checks use isolated saves and real pointer events. They do not alter the
user's save. Desktop Chrome evidence does not establish physical mobile-device
acceptance.

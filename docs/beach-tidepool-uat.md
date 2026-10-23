# Tidepool quest feedback — 22 October 2026

Local Paws browser acceptance against the pinned Release site served on port
8080. Build `paws-0.1.0-8ebef25a599096bc`; Wasm SHA-256
`2780ff96d64bbd6fe161533258126e0556bd0ee731c684896d53f0a28bf16b07`.
The served metadata and downloaded Wasm match the generated site.

| ID | Severity | Flow / observed defect | Fix and acceptance | Evidence |
| --- | --- | --- | --- | --- |
| TIDE-01 | P1 | Dismiss a freshly opened quest introduction: the puzzle remains active but the HUD says Our garden, hiding its goal and errors. | Empty navigation history returns to the authoritative garden/puzzle context. Back and Escape retain the quest HUD; opening/closing its menu does not resurrect the introduction. | Native GUI regression; browser Escape regression and screenshots. |
| TIDE-02 | P2 | Planks cannot fit beside occupied basket landings, without visible explanation under the incorrect HUD. | Objective names Button's cushion goal and plank gestures. Rejection names the blocking basket; rejected drops preserve pieces. Move baskets, build the six-span route, and complete the quest with real gestures. | Isolated browser playthrough, including Undo, Restart, reload and garden revisit. |
| TIDE-03 | P2 | Basket move highlights disappear on release; hints use unexplained Parking/Landing names. | Tapping selects a gold-outlined basket and keeps only its reachable white destination outlines visible. Blocked selections name their obstruction. Selection survives pause and moves, clears on a new scene/attempt, and never enters saves. Names are Outer, Middle and Pool-side basket; hints identify them by cushion/pool landmarks. | Native scene assertions; browser mouse/touch selection, blocked basket, menu/resume and complete solution. |
| TIDE-04 | P2 | Plank fit guidance disappears after release and does not distinguish orientation from blocked supports. | First tap selects without changing the board; another tap rotates. White outlines mean legal now; broken amber outlines mean legal after one quarter-turn. Both sets come from authoritative movement probes, excluding occupied, blocked and unsupported gaps. Text explains rotation even without colour cues. | Native two-gap rotation probe; browser selection without mutation, menu/resume, touch rotation, blocked-drop preservation and full 23-step solution. |

The existing layout and content ID are unchanged; saved journals remain valid.
HUD height also reserves the renderer's trailing-space wrap so the last
instruction line stays visible at the user's 1705×964 viewport.
The direct three-wide-gap route intentionally needs more long planks than are
available. Move the outer basket (nearest the cushion) two spaces toward the
cushion, the middle basket one space the same way, then the pool-side basket
into the middle basket's former space. Use the route around the pool.

Native rules, beach-story and production GUI tests pass. Browser evidence is in
`build/evidence/plank-guidance/{webgpu,webgl2}/report.json`; viewport captures cover
1600×1000, 390×844 and 844×390. The focused mode is
`TIDEPOOL_ONLY=1 node scripts/beach-outings-smoke.cjs`, with `PAWS_URL` and
`QUEST_BACKEND` selecting the site and renderer. Test profiles are isolated
from the user's saved game. Physical mobile hardware is outside this check.
Basket selection additionally passes the portable scene suite and emulated
touch taps after finger release. This does not establish physical-device testing.

The first plank acceptance run exhausted the C: drive while saving screenshots.
Only duplicate verification downloads and regenerable build/download caches were
removed. Browser evidence was rerun sequentially with a 1 MiB disk-cache limit.

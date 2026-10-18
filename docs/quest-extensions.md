# First five quest extensions

New adventures use version 2 of the first five layouts. Quest IDs, unlocks,
rewards, and the input vocabulary remain the same. Rules stay in Paws; the
WebEngine pin and runtime asset files are unchanged by this extension.

| Quest | Added decision | Authored mouse solution |
| --- | --- | --- |
| Pip's missing ball | Two pots share parking spaces; the vacated space is also part of the route. The source bend can instead lead to a visible catch tray. | 5 slides, 9 quarter turns, 1 release |
| Tidy the toy basket | The cushion's two parking places expose different pickup positions. All three recesses have access from the visible side aisle, so packing order is free. | 3 slides, 3 toy drags |
| The stuck little gate | Gate opening unlocks the pot's side parking space. The crate cannot clear Pip's approach until the pot vacates its final parking space. | 4 slides, 5 quarter turns, 1 release; automatic retrieval |
| Mouse under the bench | Three crates share spaces; the upper crate moves away and returns. Both lane and reach position must be clear. | 5 slides, 1 retrieval click |
| The sunny spot | The direct route needs two long planks, but only one is supplied. The side route uses one long and four short planks. A planter must clear two landings. | 2 slides, 5 quarter turns, 5 plank drags; automatic route check |

Counts describe the supplied solution, excluding selection, menus, hints, and
the automatic commands within a drag. They are not minimum-move claims.

## Controls and reversibility

Drag pots, cushions, crates, and the planter along their visible tracks. Click
channels or planks to turn clockwise. Drag toys into their shaped recesses and
planks into supported crossings. Click the ball to release it; click Button's
mouse after clearing the bench path. The correct final bridge arrangement
completes automatically.

Any basket placement can be reversed by dragging that toy back to its
original home outline. This uses the existing pick-up/place transaction, with
a `home` destination. No new input gesture or action panel is required. A
rejected drop preserves the committed placement and holding state; selecting
the object may independently advance the command revision.

### Tidy toys side access repair, 15 October 2026

The version-2 walkable graph omitted `{3,1}` and `{3,2}`, beside the mouse and
fish recesses. That forced every basket route through the ball recess `{4,0}`.
Once the ball was packed, a correctly exposed fish or mouse was rejected with
"The basket passage is blocked", despite the production scene's open side.
The graph now includes both missing aisle cells. Subtle guides approach each
recess from that side, and the objective/hints no longer impose packing order.
The cushion still controls pickup access; wrong shapes and cancelled drops
preserve the committed arrangement. Packed toys can still be returned home.

This is an additive connectivity correction to `tidy-toys-v2`: original solutions
and saved commands remain legal, and version 1 is unchanged. No save format,
reward policy or WebEngine code changes are needed.

The native `paws_quest_extensions` suite covers all six packing orders and the
original known solution, undo/restart and journal restoration. For isolated
browser fixtures, run `paws_quest_extension_tests.exe` with
`build/evidence/tidy-side-access/fixtures` as its argument. Then run
`node scripts/tidy-side-access-smoke.cjs` against the generated site (`PAWS_URL`,
default port 8156). `TIDY_BACKEND=webgl2` selects the fallback backend.
The runner restores a real saved attempt with the ball already packed, performs
the reported fish/mouse drags in both orders, checks wrong-recess/cancelled drops,
reloads midway and verifies the once-only reward after completion/reload.
Reports and screenshots are under `build/evidence/tidy-side-access/`.

Verified on 15 October 2026: the new regression failed with the reported basket
passage error before the repair, then all 530 quest-extension checks passed.
The four focused suites (`paws_rules`, `paws_quest_extensions`,
`paws_release1_progression`, `paws_phase6_scene`) passed. Local and pinned Release
browser builds succeeded. WebGPU and WebGL 2 each passed both ball-first orders
with no console/page errors on build `paws-0.1.0-b88acdb55f37def2`, Wasm SHA-256
`c752b2786091b512ad6aeb0935a6412dde485b80c719b5195ee4bd495ee01c9d`.
The user's existing browser save was resumed without moving its toys or changing
its 20 coins. The engine gitlink is unchanged. This is desktop browser evidence.

The gate's ball stays on its switch during obstacle moves. Turning a channel
still recalls the ball and closes the gate. During the ball's visible journey,
the browser rejects gate-board movement and retrieval. Once it arrives, the
gate opens, but completion waits for a clear approach. The browser preflights
retrieval after each obstacle move and commits the usual once-only reward when
the route is clear. Undo and restart cancel pending retrieval and presentation.

## Saved content versions

The version-6 save envelope stores each attempt's actual content ID. Restore
selects the matching version before replaying the journal and validates the
legacy and outer content identities against each other. Versions 1 through 5
of the save envelope and version-1 puzzle content remain supported.

Existing adventures retain their original layouts, moves, Undo history, and
reward records. Starting from the beginning uses the new layouts. Unplayed
later quests in an older save retain the content version recorded in that save.
No existing save is reset or silently reinterpreted against changed rules.

The later [playhouse cooperation extension](playhouse-cooperation.md) also keeps
v1 journals readable. Its explicit Restart/practice flow upgrades that quest to
v2, retaining earlier earned rewards and replaying both content versions across
the recorded restart boundary.

## Validation

`paws_quest_extension_tests` checks all five old and new solutions, shared
parking, the catch branch, basket ordering and recovery, the two-stage gate,
three-crate ordering, limited plank lengths, blocked landings, Undo, restart,
save replay after every new solution command, and once-only rewards.

`scripts/quest-extensions-mouse-smoke.cjs` plays the actual generated game with
mouse events, including menu navigation. Its default run covers the first five quests,
the new blocked/recovery cases, cancellation, paused Undo, restart, and a final
reload. It reads diagnostic coordinates and state without dispatching gameplay
commands through JavaScript. Set `PAWS_URL` to the explicit generated site;
`EVIDENCE_DIR`, `QUEST_COUNT`, `VIEWPORT_WIDTH`, and `VIEWPORT_HEIGHT` are optional.
Set `QUEST_COUNT=10` to include the remaining five quests. The existing
`scripts/all-quest-mouse-smoke.cjs` also exercises all ten with mouse puzzle
gestures and keyboard menu navigation.
Screenshots and the build identity are recorded with the report.

### Recorded acceptance, 14 October 2026

- Final first-five mouse run: **5/5 completed**, including mouse menu scrolling,
  blocked choices, recovery, paused Undo, restart, and reload with 100 coins.
  Report: `build/evidence/quest-extensions-final/report.json`.
- Final payload: `paws-0.1.0-47ff0bbe86041902`, pinned engine
  `3fe7d1dc1dc610c703944803cbb717cd65024ec9`, Release configuration. Wasm SHA-256:
  `8796cbbe8e94704c7517533b634cc0eee7531d89157fc01599f65cbde73ab75d`.
- Existing ten-quest runner: **10/10 completed**, with 200 coins and no browser
  errors, on `paws-0.1.0-cdb6c55e3fb4ad82` before the final blank-content-ID
  rejection was added. Report:
  `build/evidence/quest-extensions-existing-runner/report.json`.
- Final focused native suites: **7/7 passed** (`paws_rules`,
  `paws_release1_progression`, `paws_release1_rules`, `paws_quest_extensions`,
  `paws_release1_gui`, `paws_phase6_scene`, `paws_release1_scene`). Includes
  restoring and continuing a version-1 bench journal alongside version-2 quests.
  Output: `build/quest-extensions/native-results.txt`.
- The broader native suite also reports existing failures in `paws_ui`
  (expects the old "Restart adventure" button label) and `paws_release1_props`
  (texture-size assertion). Those checks are outside the quest extension scope.

Browser evidence is desktop Chrome at 1600 × 1000. It does not establish
physical mobile acceptance.

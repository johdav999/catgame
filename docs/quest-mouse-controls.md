# Quest mouse controls

Implemented 12 October 2026. Quest mechanics use the 3D scene; object/action
menus and separate GUI route checks are removed. Navigation, settings, save
recovery, Undo, hints and restart remain in Menu.

| Quest | Click on mouse release | Drag and drop | Completion |
| --- | --- | --- | --- |
| Pip's missing ball | Rotate channels; release the ball | Pots along their slots | Ball reaches Pip |
| Tidy the toy basket | — | Blocking cushion; toys into matching recesses | All three toys fitted |
| The stuck little gate | Rotate channels; release the ball; retrieve the mouse if needed | Pot along its lane | Arrival opens gate and retrieves mouse |
| Mouse under the bench | Retrieve the mouse after clearing the lane | Crates along their slots | Valid near-reach retrieval |
| The sunny spot | Rotate planks | Planks between matching supported endpoints | Supported route connects automatically |
| A bed for Button | Rotate cushions | Cushions into basket cells | Every basket cell covered without overlap |
| A drink after playing | Rotate gutters; release water from barrel (bowl also checks flow) | Gutters remain fixed | Water reaches the bowl |
| The garden bells | Ring the pictures in clue order | Dragging a bell does not ring it | Correct five-picture sequence |
| Two kittens, one doorway | — | Cats to marked destinations; crate along its track onto the paw switch | Pip latches the doorway; both kittens inside (v2) |
| The garden picnic | — | Bed and bowl onto marked spots | Shade, quiet and adjacency all valid |

Clicks rotate on release, with an 8-pixel drag threshold. Crossing that threshold
and returning to the start still counts as a drag. Transient previews are cancelled
on blur, resize, pointer cancellation, stale revision, scene replacement or missing
targets. Illegal drops do not leave partially picked-up pieces. A fitted plank or
cushion can be turned again; its old socket is released until the next valid drop.

Sliding pieces preserve the original grab offset through both preview and drop.
The visible actor origin chooses the destination square, so grabbing a pot's
rim or side does not select a different paving row. See the
[pot drag correction and regression](pot-drag-fix.md).

Every drag interaction now shows cream-white destination rectangles with teal
edges. Crates (including all three in Button's mouse quest and the playhouse
crate), toys, planks, basket cushions, kittens and picnic props share this
guidance. Holding a piece isolates its destinations; releasing or cancelling
restores the available spaces. Sliding destinations include complete reachable
tracks, never spaces behind another piece. Pickup, footprint, orientation,
occupancy and gate rules are preflighted through the same commands used by a
drop, on copies of the attempt. The hints do not change saves or progress.
Rotating a plank or cushion refreshes its available placements. Fixed channels
and bells retain their tap instructions. Menu, completion and scene transitions
hide the transient movement outlines. Button's HUD explains all five crate
slides before asking the player to collect the mouse.

`node scripts/movement-guidance-smoke.cjs` exercises all ten quests with real
mouse gestures and checks visible outline geometry during legal drags; set
`PAWS_URL` to the exact rebuilt site. The portable integration scene suite also
checks that every movement in the ten quest solutions has a destination hint
without changing the owner's journal or revision.

Run `node scripts/all-quest-mouse-smoke.cjs` against the generated pinned site;
set `PAWS_URL` for the active server. It uses mouse events for every puzzle action,
real UI navigation between requests, and an isolated browser save. It checks all
ten completions and the 200-coin once-only reward ledger, removed action-menu
navigation, rejection/cancellation cases, and paused Menu Undo/hints.
Results and screenshots are under `build/evidence/all-quest-mouse/`.
`scripts/plank-controls-smoke.cjs` retains the focused plank regression.

Portable validation covers rules, progression, GUI and scene contracts. Browser
evidence is desktop Chrome; it does not establish physical touch-device acceptance.

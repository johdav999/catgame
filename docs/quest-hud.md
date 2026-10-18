# Compact quest header and Menu

The garden and puzzle views now expose one top-right **Menu** button. The bottom
tray and separate wide Saved card have been removed. The top-left card grows to
740 logical pixels on wide screens and includes its title, chapter/coins/save
line and the current puzzle objective. Garden copy invites the player to select
a cat. Narrow screens use the whole card width below a reserved heading/Menu row;
the card height accounts for wrapping and larger text.

Puzzle controls remain available under Menu → Puzzle controls, and by selecting
a scene object. Requests & friends is under Menu in the garden. Entering puzzle
controls or hints clears the owner's explicit pause flag, while the modal still
pauses scene simulation through the existing UI lifecycle. This permits Undo and
other commands after opening controls through Menu.

## Scoped UAT

Product: Paws C++/Wasm game, pinned WebEngine build, fresh local Chrome saves.
Build: `scripts/build.ps1 -Pinned -Configuration Release` (explicit SDK/build
directory used in this run). Evidence: `build/evidence/quest-hud/`.
Flows: garden navigation; puzzle header; Menu/settings/focus; projected pot
drag and Undo; gate quest header; normal and larger text at desktop, portrait,
landscape and 2048x900 wide desktop.

| ID | Finding | Acceptance | Verification |
| --- | --- | --- | --- |
| HUD-01 | Bottom controls and duplicate wide Saved card obscure the scene | One top-right Menu and one objective card | HUD control/position assertions and responsive screenshots |
| HUD-02 | Opening controls through Pause left the owner paused and rejected Undo | Controls work through Menu; modal simulation pause remains | Real projected drag and Undo at three viewports |
| HUD-03 | Portrait Menu could overlap the status text | Reserve a heading row and use full width below it | Larger-text portrait capture inspected |

Native release-one GUI tests pass, including all ten requests and responsive
focus/target checks. Browser acceptance is `scripts/quest-hud-smoke.cjs`; its
report records the build and scoped results. Desktop emulation does not establish
physical-device acceptance.

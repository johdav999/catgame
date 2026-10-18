# Paws GUI integration

## Reference styling (17 October 2026)

The production retained GUI follows
[the professional puzzle reference](design/paws-professional-puzzle-reference-20261017.png):
opaque cream cards with quiet linen edges and a small ink shadow, teal primary
actions, rounded line icons, centered control labels, Fraunces 30px headings and
Nunito Sans 20px body/control text. Controls retain a 52px minimum height and
the existing larger-text setting. Keyboard focus keeps a distinct teal/cream ring.
The shared skin applies to menus, settings, hints, shop and recovery as well as
the garden/puzzle HUD; no screenshot is used as a control texture.

Wide objective cards are capped at 480 logical pixels. Pip's ball uses the short
reference objective; its full interaction instructions are available in Hint.
The normal puzzle HUD provides a centered Undo/Hint tray, dispatching the existing
stamped intents. Scent and balance keep their specialized choice trays and retain
Undo/Hint in Menu. The camera's available board rectangle excludes the bottom
controls, including short landscape layouts. Wallet/save information and the live
FPS readout remain available in Menu; garden status and save failures remain visible
in the objective card. Generation, puzzle rules and persistence ownership are unchanged.

Validation and screenshots are recorded in
`build/evidence/gui-reference-style/`; see `layout-report.json` and the renderer
playthrough reports there. These are desktop Chromium checks with portrait/touch
emulation, not physical-device acceptance.

Restart puzzle is available from both puzzle and garden menus. Garden confirmation
prepares the current request's puzzle scene and restarts even an unfinished attempt
before publishing it. Cancel retains the attempt; earned rewards and furnishings
remain unchanged. `scripts/restart-puzzle-smoke.cjs` covers both menu paths and reload.

Release-one Prompt 9 composes `release1::gui::Views` into the same retained tree.
Ten quests use scene clicks and drag-and-drop through the owner's legal action
projection and generation/attempt/revision intents. Quest object/action sheets
are removed from production navigation. Menu retains Undo, hints, restart and
services. Actual four-cat portraits and eight prop
thumbnails come from accepted fragments. The production board uses the GUI's
unobstructed rectangle. The ending card leaves the actual four-cat gathering
visible on wide, narrow and short layouts. Track/gallery shells remain isolated.
See [the current integration evidence](release1/prompt-09-handoff.md).

Placement lists only spots whose role and footprint suit the selected furnishing.
Suitable occupied spots retain their blockage reason. Opening the sheet previews
the current spot, or the first clear suitable spot, with a visible selection and
enabled Place action. Only Place commits the move; Cancel preserves the saved
position and coins. The scratching post therefore opens on its scratch corner
without unrelated blocked destinations obscuring it. Short layouts use compact
instructions above the fixed Place/Cancel footer.

Run `node scripts/scratch-placement-smoke.cjs` after building the production game;
set `SITE_DIR` to its absolute generated `site` directory. The default is
`build/web-scratch-placement-pinned-release/site`. This regression uses isolated
saves and checks mouse selection, emulated touch Place/Cancel, standard/larger
text, portrait/short landscape, and IndexedDB reload. Evidence is written to
`build/evidence/scratch-placement/report.json`; physical devices remain untested.

## Direct puzzle controls (12 October 2026)

All ten quests complete using their 3D objects. Click to rotate channels, gutters,
planks and cushions, ring picture bells, retrieve Button's mouse or release the
ball/water source. Drag movable props and kittens onto their authored destinations.
Plank routes, simultaneous switches and picnic clues complete automatically when
valid. Invalid drops and cancelled gestures preserve committed state; rejection
reasons appear in the quest header. Undo and hints work while Menu remains paused.
See [the per-quest controls and verification](quest-mouse-controls.md).

The following 11 October evidence describes the earlier direct-control baseline:

Click/tap a channel once to rotate it; other objects were selected on click/tap.
The same one-click rotation applies to Mabel's water gutters, without opening
the object sheet. Rotation commits on release, respects cancelled/dragged contacts
and stale intent checks, and remains undoable. Browser acceptance is in
`scripts/single-click-rotate-smoke.cjs`, with evidence under
`build/evidence/single-click-rotate/`: first mouse click and touch tap on visible
straight/curved channels at three viewports, rotation Undo, pot drag/Undo and the
later water quest's first click. These are desktop/emulated-touch checks.
Drag pots/cushions along their authored slots and release to commit a validated
path. Blocked or off-path drops restore the original pose. Drag toys into matching
basket recesses; the gate mouse can be dragged out after its route is reachable.
Click/tap the source ball to Roll. At that point Move/Rotate buttons were hidden
in the game action sheet while Roll, Undo, Hint and selection remained there.
The 12 October change removes that quest action sheet entirely.
With the canvas focused, arrows move a selected sliding object, R turns a selected
channel, and Space rolls the ball.

The host gives GUI first refusal, then captures each contact for object control
or camera orbit. Mouse and touch use the same C++ controller and camera rays.
Dragging only previews transforms; rules, saves and rewards change on valid release.
Attempt/revision checks reject stale gestures. Escape, blur, resize, suspension,
pointer cancellation and scene changes discard previews. An object drag never
orbits the camera. Selection uses engine interaction components and checked handles.

The historical `npm run test:browser:direct` suite wrote
`build/evidence/direct-controls` and covered the earlier desktop/emulated-touch
interface. Use `scripts/all-quest-mouse-smoke.cjs` for the current quest interface.

Phase 2 uses the pinned `engine_gui` and `engine_gui_browser` libraries. Visible
controls are C++ retained widgets rendered into the WebGPU canvas. HTML owns only
startup, compatibility, retry and renderer recovery. The engine's invisible native
semantic tree supplies keyboard and assistive-technology access.

## Ownership and command boundary

`PawsUi` owns its `GuiSystem`, fonts, skin images, navigation and session settings.
`BrowserGame` owns it alongside `Afternoon` and its `PawsSession` attempts. The renderer and browser GUI adapter
borrow its tree; both detach before a layout/state rebuild or destruction. UI
callbacks queue changes; the tree is rebuilt at the next application frame, never
inside an engine event callback. Old semantic widget IDs cannot activate a new
tree. Modal navigation retains the initiating accessible name and restores focus
to that action when returning. No raw scene pointers are captured by controls.

`UiModel` is a copied read model. `UiIntent` carries a typed action, stable target,
attempt and revision. `BrowserGame::intent` translates the current puzzle actions
through `Afternoon::dispatch` and its active `PawsSession`, then projects the committed model and scene. The GUI
does not decide occupancy, route correctness, rewards, purchases or durability.
Move targets currently encode `<object-id>/<direction-index>` in this adapter;
that is not player-facing copy or a persistence format.

`Play`, Help Pip, object selection, legal movement/rotation/Roll, undo, restart,
back to garden, hints and camera controls are wired to the delivered phase-1
model. The authored courtyard suspends the current attempt across visits/reloads. Modal
sheets pause Application simulation; their explicit action buttons can still
dispatch a discrete command. Host suspension preserves that pause policy.
Restart gets a confirmation and resets hint disclosure for the new attempt.

## Later-phase contracts

| Surface | Required producer data and consumer command |
| --- | --- |
| Continue | Set `canContinue` only after a valid save candidate exists; handle `Continue` through the restore service before presenting it as restored. |
| Dialogue / ending | Supply `pipPortrait` from the accepted canonical Pip render. UI does not generate or substitute a cat identity. |
| Reward card | Set `rewardsReady`, `rewardGranted`, `rewardCoins`, `rewardFriendship` from the committed reward transaction. A practice completion displays no new grant. Phase 1 reports completion without inventing coins. |
| Shop | Set `shopOpen`, wallet, ownership and accepted preview textures. Handle `Buy` atomically in the economy model, including repeated/stale commands. The UI disables locked, unaffordable and owned purchases. |
| Placement | Supply the scene's furnishing/footprint preview, selected stable spot and legality reason. Handle SelectObject, Place and CancelPlacement. Replace the three human-readable spot targets with the authored stable IDs when phase 7 binds the garden. |
| Save/recovery | Publish Temporary, Saving, Saved, Failed, SecondTab or Loading from the storage coordinator. Saved must follow successful transaction completion. Set `storageAvailable` for import capability and `canExport` when there is exportable state, independently of `canContinue`. Bind RetrySave, ExportSave and ImportSave to the phase-8 host service. |
| Audio / motion | Read `UiSettings` on the Settings intent, apply independent music/ambience/effects volumes and mute to the future audio backend, and use reduced-motion poses for future animations. Current visuals change immediately and do not animate. |
| Quality | Automatic renderer policy is current. Manual quality control is explicitly unavailable until supported game settings are integrated. |

Gallery fixtures never grant rewards, change ownership or write a durable save.
Normal play projects real progression and persistence. Future controls are enabled
only by producer capability/state.
Font and icon assets already ship. Accepted Pip portraits now ship through
the model's `pipPortrait` field. Item previews and scene footprints remain
integration slots for phase 7.

## Layout, typography and input

Cream, linen, ink, teal, muted ink and error ink use `design.md` values in
`paws_ui.cpp`; browser recovery uses the corresponding CSS tokens. Standard
buttons are 52 logical pixels tall (never below 44), separated by 12 pixels.
Panels use 16-unit corners, buttons 12-unit corners and 2-unit edges. Focus uses
the teal/cream double outline; disabled labels keep full ink contrast on linen
while the actual engine button is disabled. A sibling lettering image avoids
the pinned engine's automatic low-contrast disabled-text dimming.

The engine exposes one font per GUI. Body labels use Nunito Sans Regular through
the normal atlas. Short headings and control lettering are composed from the
same trusted engine glyph/atlas API into immutable Image widgets, using Fraunces
600 and Nunito Sans 600 respectively. Copy remains editable UTF-8 C++ strings,
with matching accessible names. No screenshot supplies text. Long labels wrap
without reducing font size. Fonts retain Å/Ä/Ö and lower-case accents.

The original code-native 24-unit icon family is retained in `PawsUi::tile`.
It uses 2-unit rounded line silhouettes for home, menu, hint, undo/rotation,
coin, paw and check. Labels accompany functional icons. Rounded backgrounds and
icons are application styling, composed with the existing engine Image API;
no engine sources or core renderer were forked.

Larger text scales geometry, glyphs, gaps and hit-testing to 125%. Modal content
scrolls while Back stays outside the scroll region. Portrait keeps two action
rows; short landscape reflows to one row. Safe-area insets bound the canvas,
so rendering, pointer coordinates and logical layout share the same inset area.
The viewport meta tag does not disable user zoom. Board framing follows canvas
aspect changes. The three puzzle boards now compose the production kit; the separate authored
garden is composed by `garden_scene` with three stable furnishing spots.

Each pointer contact retains one GUI/camera owner through release or cancel.
GUI hit testing runs first, and modal sheets block scene gestures. Scroll-drag
suppresses click activation. Blur, resize and suspension cancel captures. The
engine adapter owns Tab/Enter/Space and semantic activation; the host owns Escape
navigation before the adapter's default focus-clearing handler.

## Development gallery and repeatable checks

The gallery is gated by the CMake option `PAWS_GUI_GALLERY`, OFF by default, and
requires `?gallery=1`. When enabled, fixture data goes only to an isolated UI
projection. `PawsUi::emit` suppresses domain dispatch, including Buy and saves.
The gallery contains all 14 surfaces and explicit reward/practice, shop,
placement and six save-state examples. Its banners disclose that no progress is
changed. Do not enable the option for a production package.

```powershell
cmake -S . -B build/web-pinned-release -DPAWS_GUI_GALLERY=ON
cmake --build build/web-pinned-release --target paws_site
./scripts/serve.ps1 -SiteDirectory build/web-pinned-release/site -Port 19474
# In another shell:
node scripts/gui-smoke.cjs
cmake --build build/native --target paws_ui_tests paws_rules_tests paws_scene_tests
ctest --test-dir build/native -R '^(paws_ui|paws_rules|paws_starter_scene)$' --output-on-failure
```

Use an already configured pinned browser directory, or first run the documented
`scripts/build.ps1 -Pinned` with Emscripten 6.0.11. The browser script accepts
`PAWS_URL`, `BROWSER_EXECUTABLE` and `EVIDENCE_DIR`. It uses the actual named
keyboard path rather than calling puzzle commands to solve the puzzle. The old
phase-1 HTML smoke scripts target retired controls and are historical evidence.

Font sources, exact upstream commit URLs, axes, hashes, transformations and OFL
records are in `assets/runtime/fonts/manifest.json`. Regeneration uses
`scripts/prepare-ui-fonts.py` and fontTools 4.60.1; raw variable fonts stay under
`artifacts/gui/fonts/`. Normal builds use only the checked-in static font files
and need no network font access or font-production tools.

## Phase 6 integration

`BrowserGame` now routes through `Afternoon`, which owns three `PawsSession`
attempts. Quest dialogue, objective, title, unlocks, reward cards, legal pickup/
placement, switch status and Return ball come from committed rules. Selecting an
object returns focus to its actions. Hints and settings persist. Scene preparation
shows cancellable Loading and keeps the prior scene on failure. `canContinue`
follows validated restore; Saved follows the latest IndexedDB transaction.
File export/import are still unavailable domain services. The gallery
continues to suppress progress writes. See [phase 6](phase6-puzzles.md).

## Phase 7 integration

The menu opens the shop after quest one from the garden. Two real model renders,
prices, purchase reasons, owned labels and named placements project authoritative
state. Details dispatch Buy and BeginPlacement; placement buttons use stable spot
IDs and confirmation retains the displayed item/spot plus attempt/revision.
Place and Cancel remain in a fixed footer at larger text sizes. Actual model and
footprint previews appear in the courtyard beside the sheet, with green/coral
validity and readable occupancy explanations. Escape dispatches CancelPlacement.
The compact ending card has Continue in garden; the menu offers replay only after
the ending flag is recorded. Gallery commands remain isolated. See
[phase 7](phase7-shop-garden.md).

## Phase 10 presentation

Settings cycles Automatic → Mobile → Desktop quality and emits a typed Settings
intent with `quality` target; the owner applies pinned renderer budgets and saves
the choice with the afternoon. Keyboard focus stays on the renamed control.
Portrait HUD includes wallet, friendship and save status below the heading.
Short landscape uses a compact title card and a separate compact status card,
preserving the visible upper puzzle route. Failure status retains error text and
Menu → Save and recovery. See [final review](phase10-release.md).

## Chapter introductions

The garden HUD projects pending introductions as a visible `Meet <cat>` action
with welcome copy. Chapter-two and chapter-three request cats remain gated by
their saved introductions, but the player can now open that dialogue directly
from the garden, including after restoring a save. Once introductions are complete,
the HUD returns to the request-marker instruction. GUI regression tests exercise
both chapter transitions, save restoration and return from the welcome dialogue.

## Button's crate tracks

The bench quest shows the two authored crate tracks with direction chevrons and
parking outlines. A dashed route connects Button to his mouse and changes from
warm ink to green once the blocking crate leaves the lane. Paw-reach socket outlines
are omitted from this scene because they were mistaken for crate destinations.
The HUD follows the current arrangement: move the rear crate sideways, move the
front crate toward the house, then retrieve the mouse. Drops match the dragged
crate's preview position, retaining its grab offset, rather than matching the
pointer's ground ray. The focused browser check covers off-centre grabs, invalid
drops, blocked retrieval and the completed ending.

## Opt-in WebEngine profiling

Open the game with `?profile=1` to show the existing WebEngine CPU/frame/GPU
metrics in a small diagnostic overlay. The host publishes the latest rolling
snapshot, build identity, viewport and quality budgets to the hidden
`#paws-profile-data` element for read-only inspection/export. It does not create
another render loop or alter quality. Frame windows contain up to 120 samples;
GPU timestamps exclude GUI, queue waiting and presentation. Normal game URLs
retain the existing snapshot callback without the diagnostic DOM work.

## Reversible idle head correction

Idle playback selects `relaxed_idle_forward` when it exists. The original
`relaxed_idle` remains available; `?idlePose=original` disables the correction for
the entire browser session without changing saved game state. Interaction clips
retain their original poses and enter/exit through the existing crossfade. The
preservation records, editable masters and original exports are retained in
`artifacts/model-jobs/idle-forward/2026-10-14/`.

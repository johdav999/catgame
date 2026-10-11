# Paws GUI integration

## Direct puzzle controls (11 October 2026)

Click/tap an object to select it; click/tap the selected channel again to rotate.
Drag pots/cushions along their authored slots and release to commit a validated
path. Blocked or off-path drops restore the original pose. Drag toys into matching
basket recesses; the gate mouse can be dragged out after its route is reachable.
Click/tap the source ball to Roll. Move/Rotate buttons are temporarily hidden in
the game action sheet; Roll, Undo, Hint and selection remain available there.
With the canvas focused, arrows move a selected sliding object, R turns a selected
channel, and Space rolls the ball.

The host gives GUI first refusal, then captures each contact for object control
or camera orbit. Mouse and touch use the same C++ controller and camera rays.
Dragging only previews transforms; rules, saves and rewards change on valid release.
Attempt/revision checks reject stale gestures. Escape, blur, resize, suspension,
pointer cancellation and scene changes discard previews. An object drag never
orbits the camera. Selection uses engine interaction components and checked handles.

Run `npm run test:browser:direct` against the actual built site (`PAWS_URL`, default
localhost:8080). Evidence goes to `build/evidence/direct-controls`. This suite
replaces button-based movement checks for the current controls and covers desktop
mouse and phone-sized emulated touch; it does not establish physical-device acceptance.

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

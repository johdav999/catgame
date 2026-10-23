# Five more seaside quests

The beach pack now contains eleven quests. After Everyone has a place, choose
Continue beach story to begin the five new outings. Existing players use
Adventures -> Start / Continue beach. Local preview access remains unchanged;
this work does not publish the site or enable purchases.

| New quest | What the player does | Authored operations |
| --- | --- | --- |
| Button's tidepool expedition | Clear three interdependent baskets; rotate and fit six spans around the pool, avoiding a three-wide-gap shortcut and loose supports. | 23 |
| Fresh water around the dunes | Park three baskets, turn four gutters, fill a catch basin, then redirect the second four-gutter route and release the basin without wetting the mat. | 15 |
| A sheltered shell workshop | Cover an irregular sixteen-cell workshop with eight panels, choosing among fourteen crossing support pairs. Combine four crossbars and four uprights; locally valid choices can strand another cell. | 21 |
| Pepper's answering shell song | Watch and repeat two four-picture phrases; the answer trades leaf and paw symbols while preserving the star. | 8 |
| Pip's lantern landing | Place four numbered lantern weights in each of four configurations, accounting for changing supports and Pip's load; test each equilibrium. | 20 |

Operations include pickups and checks; mouse dragging combines pickup with fit,
and valid routes/coverage auto-check. These counts describe authored solutions,
not measured first-player playtime or minimum move counts. Hints remain free,
with Undo, Restart and interrupted quest saves.

## Shell workshop revision (2026-10-22)

`beach-windbreak-v2` replaces six independent, name-matched placements with one
connected coverage puzzle. Every offered support pair covers real marked cells;
six plausible alternatives overlap cells needed elsewhere. Eight panels have
exactly enough coverage, so there is no spare panel to hide a mistake. An
independent exhaustive search of all eight-of-fourteen choices proves exactly
one complete support arrangement; panel identities are interchangeable. Hints
progress from forced outer cells to the complete arrangement. Wrong placements
can be moved without restarting; time pressure and penalties are unchanged.

Shade panels now share plank selection: first tap selects, second tap rotates,
white outlines show current fits, and broken amber outlines show quarter-turn
fits. Shared support posts are permitted by Shade definition validation, as
they already were for Planks. Existing imported panel/post meshes are reused.

The v1 definition remains immutable for journal replay. Existing completed and
unfinished saves retain it until Restart. The completion card offers **Try
revised workshop** for v1 and **Replay workshop** for v2, using the existing
restart confirmation. Completion credit, other quests and rewards are preserved.
No save-format or engine revision change is needed.

Verification uses `scripts/workshop-smoke.cjs`, the native beach story suite
(including exact-cover search, repair of a wrong crossing and old journal
upgrade), rules, GUI and scene suites. The browser fixture is produced by
`paws_beach_story_tests build/evidence/beach-outings-start.save` as the sibling
`.save.workshop` file. Browser reports/screenshots are under
`build/evidence/workshop/{webgpu,webgl2}`. Touch checks are emulation, not physical
mobile-device acceptance.

Final acceptance: build `paws-0.1.0-3a6b19ba1cdda808`, Wasm SHA-256
`ffd60f2ed3380ca1bb701f3d9e9aa6cbfa4f84655280f61f4b4a6c2b4a297c8a`.
Both WebGPU and WebGL2 workshop reports pass the full real-input solution,
portrait selection/rotation, Undo, Restart, mid-attempt/completion reload and
completion-card replay. Native `paws_beach_story`, `paws_release1_rules`,
`paws_release1_gui` and `paws_release1_scene` pass (4/4).

### Workshop camera repair (2026-10-22)

The board-size estimate clipped the loose front-row panels: it omitted their
mesh overhang and perspective enlargement. Workshop framing now includes the
full panel meshes, their original tray positions in all four orientations,
and the support area. It fits those bounds into the free HUD rectangle using
perspective depth, keeping the tray reserved as panels are placed.

Build `paws-0.1.0-9c6fff8edac48abb`, Wasm SHA-256
`7df03fcb1fa84a32a9c375c0936c49a88fadd8226efb85d4bcda553b1418d97a`
passes both WebGPU and WebGL2 workshop play-throughs. The browser regression
checks all 64 panel bounding-box corners at 1705x964, 2071x964, 390x844 and
844x390, after portrait rotation, mid-puzzle reload and completion-card replay.
Screenshots and reports are in the workshop evidence folders above. This repair
changes browser camera policy and its diagnostics only; portable game rules,
save data and the engine pin are unchanged. The earlier native suite results
above apply to the quest revision, not a new native run for this camera fix.

### Simplified workshop guides (2026-10-22)

The workshop now uses selection-only guidance, replacing its earlier white/amber
rotation legend and permanent blue endpoint outlines. With nothing selected,
only soft sand fills mark uncovered cells. Selecting a shade shows its current
legal fits in white. The explicit **Rotate** button turns that selection. A double-click (or quick
double-tap) on the same shade also turns it once; separated clicks only select. During a drag, only the nearest legal fit within the
snap radius is highlighted, with a translucent shade preview. Release commits
that same target through the normal PickUp/Fit commands. Invalid drops and
pointer cancellation leave the attempt unchanged. Successful drops clear the
selection. Other quests retain their existing guidance.

The header counts distinct covered target cells, so overlaps never inflate
progress. Covered fills fade away over 0.2 seconds and return on Undo; reduced
motion applies the visibility change immediately. Selection and previews are
presentation state and do not change saved puzzle rules or rewards.

Build `paws-0.1.0-e069a9acd01c1a1a` (Wasm SHA-256
`8b84b2fcf78d56a240a0420d157112c03d7eddfa005b570d8502d7ba54656403`)
passes the updated workshop browser script on WebGPU and WebGL2, including
portrait touch selection, explicit rotation, one-target previews, invalid drag
cancellation, coverage/Undo, all four viewport sizes, the full solution, reload
and replay. Evidence remains in `build/evidence/workshop/{webgpu,webgl2}`.
Native rules, GUI and scene suites pass; the scene suite is rerun after the final
marker tint/fade adjustment. No new models or engine changes are involved.

## Integration and compatibility

The new quests use existing portable Planks, Water, Shade, PictureSequence and
Balance rule families. Picture playback/definition bounds increase from eight to
twelve entries, with tests for the full sequence and rejection beyond the limit.
Game rules, dialogue, unlock policies, scene projection and save ownership remain
in Paws; WebEngine's pin is unchanged.

IDs append at indices 24–28; all original indices and content revisions remain
stable. Version 12 reads versions 1–11. A completed v11 beach picnic keeps its
postcard and journals; the tidepool expedition unlocks and later outings remain
locked. The original picnic ending is still available at the picnic, independently
of the five new completion records. Finishing the last outing returns to that
picnic. Adventure completion now requires eleven quests. Base coins, friendship,
chapters and garden picnic eligibility are unaffected.

Each outing has progressive hints, request/completion dialogue and an earned
cove landmark. The lantern balance has four shared-mesh draggable lanterns and
numbered tags. Other new quests reuse the reviewed beach kit and supported
water/route props. Panel staging uses two rows to keep loose pieces separated.

## Tripo lantern

The built-in ImageGen reference was submitted once through Tripo; the provider
charged 60 credits. A UUID-format response required a task-local resume helper,
not a new job. Runtime export: 7,998 triangles, one material, derived 1024px maps,
approximately 0.48 x 0.65 x 0.42 metres. The packed Blender master, raw 4096px-map
source, reference/prompt, hashes, three-stage renders and provenance are retained.

- [Runtime GLB](../assets/runtime/models/beach/lantern-v1.glb)
- [Editable master](../artifacts/model-jobs/beach-lantern-20261021-network/lantern-master.blend)
- [Reference](../artifacts/model-input-images/beach-lantern/reference-v1.png)
- [Asset review and provenance](../artifacts/model-jobs/beach-lantern-20261021-network/README.md)
- [Rights record](../assets/runtime/models/beach/lantern-NOTICE.md)

## Validation

Native tests cover every authored solution, invalid actions, replay/Undo/restart,
old-save migration, persistent ending, unlock order, once-only progress, scene
construction and retained GUI. Browser acceptance uses
scripts/beach-outings-smoke.cjs against the generated pinned Release game.
It begins with a fixture produced by the real progression owner after completing
the original six quests; all five new quests use actual scene gestures and GUI.

All seven focused native suites pass: progression, rules, beach access/story,
GUI, beach environment and scene integration (57.77 seconds total). The story
suite includes 1,602 assertions. Log: `build/beach-outings-native.log`.

The WebGPU run completed all five outings and picnic replay, with no browser
errors: `build/evidence/beach-outings/webgpu/report.json`. It verifies the
served coastal model hashes and four imported interactive lantern instances.
Screenshots cover 1600x1000, 390x844 and 844x390 viewports. A focused follow-up
at 390x844 verifies the account launcher now clears the quest controls and
the rendered Hint accepts pointer input:
`build/evidence/beach-outings/webgpu-layout/report.json`.

The separate fresh-browser WebGL2 run with reduced motion also passes all five
quests, reload/travel, Undo/Restart and saved picnic replay, with no browser
errors: `build/evidence/beach-outings/webgl2/report.json`. The final packaged
build is `paws-0.1.0-516e6f4386a3f80e`, using unchanged engine pin
`0426d9319de715584a2e34ff9862cd6ba873ecd0`. The initial full WebGPU run precedes
the CSS-only account-button adjustment; the focused WebGPU layout check and
full WebGL2 run use the final package. The twelve-picture clue wait is 60 seconds
in automation because application-time playback advances more slowly on the
fallback test renderer; no wall-clock performance claim is made.

The native fixture can be regenerated with `paws_beach_story_tests` followed by
the output path `build/evidence/beach-outings-start.save`. Run the browser
script with `PAWS_URL` pointing at the generated site; `QUEST_BACKEND=webgl2`
forces fallback in a fresh browser. `BEACH_LAYOUT_ONLY=1` checks the narrow
screen control fix. Use the configured compiler's runtime DLL directory first
on PATH when executing native fixture generation on Windows.

Physical mobile devices and first-player pacing are outside this automated check.

Workshop double-click follow-up: browser build `paws-0.1.0-3e55e89cdd66f133`
uses the application input clock with a 400 ms interval and 12 px tolerance.
Drag/cancel, changed quest/attempt/revision, and pause invalidate the click pair.
The real-input workshop check now covers one rotation per double-click,
separated single clicks, Undo, and the existing full placement/reload flow.

Eight-picture shell-song follow-up: `beach-shell-encore-v2` uses two four-picture
phrases, with matching objectives and hints. Existing v1 journals retain their
twelve-picture definition; Restart upgrades only that attempt to v2. Native
rules and beach-story suites pass, including 1,891 story checks and legacy replay.
`scripts/encore-eight-smoke.cjs` passes on WebGPU and forced WebGL2 against build
`paws-0.1.0-16ac1d78a03218e8`: legacy Restart upgrade, eight real shell inputs,
four-answer save/reload, completion, and preserved milestones/rewards. Reports
and screenshots are in `build/evidence/encore-eight/`. Concurrent initial runs
hit startup asset-loading failures; the serial reruns passed without page errors.
The live preview was restarted into the eight-picture attempt, retaining 240
coins and 13 hearts, and left at `0 / 8 correct` for the player.

Paw-bell audio repair (2026-10-22): cue dispatch and the file load were correct,
but the former source was resampled from 400.74 Hz to 1046.502 Hz, shortening
its recording to 0.52 s with most energy in the first 50 ms. Peak normalization
hid the weak sustained ring (overall -27.21 dBFS RMS versus leaf -19.67).
`scripts/repair-paw-bell.py` derives the high note from the existing leaf recording,
retaining provider provenance, source hashes and rights records. The replacement
is 0.88 s, -19.75 dBFS RMS, with a sustained 150-450 ms window at -19.37 dBFS.
The script guards duration and sustained energy; no audio generation or gain
increase is involved. `scripts/encore-eight-smoke.cjs` checks an actual paw-shell
tap starts this recording and measures its output after Effects gain, alongside
the saved eight-picture completion flow. Evidence remains in
`build/evidence/encore-eight/`; repair metrics are in
`artifacts/audio-production/reviews/paw-bell-repair.json`.

Balance tilt repair (2026-10-22): positive rule torque means right-heavy, but the
scene previously applied positive Z rotation and raised that end. The presentation
now lowers the heavier end. Native scene regression uses legal placements and
transformed mesh endpoints for all three balance quests, checking both signs,
equilibrium, and reduced motion; it failed before the sign correction and passes
after it. Story and GUI suites also pass. `scripts/lantern-tilt-smoke.cjs` uses
real drags, verifies endpoint heights against the HUD, reloads a left-heavy
arrangement, and tests an authored equilibrium at 844x964. Evidence is under
`build/evidence/lantern-tilt/`. No rule, solution, or saved journal changed.

The pinned browser build also required an API-availability fallback for an
in-progress shared-GUI refactor: newer engines retain in-place clear; the pin
retains its existing owned-GUI reset. An obsolete ignored generated site at
`build/web-release1-p09-pinned-release/site` was removed after disk exhaustion;
its sources/build configuration and the active preview were preserved.
Both WebGPU and WebGL2 pass on `paws-0.1.0-85fe1bb65da2abe6`; measured
right-minus-left heights are -0.24495 m for right-heavy, +0.24495 m for
left-heavy, and exactly zero at equilibrium. The player's saved setup 2 was
restored without restarting the quest.

Balance setup success (2026-10-22): each accepted intermediate equilibrium now
opens a persistent success card naming the completed setup and the count balanced.
It compares the support position and Pip load before/after, explains that the
pieces returned to storage, and provides an explicit `Start setup N` button.
The card pauses/blocks the puzzle until acknowledged; status/save projections
retain it. Acknowledgement is presentation only. Final equilibrium retains the
existing quest completion screen; loading a save restores its active setup
without replaying an acknowledged card. Balance indices are derived view data,
with no save-format or journal changes. All three balance quests share this UI.

Native GUI tests exercise every transition, failed tests, final completion and
four viewport sizes. The story suite passes unchanged. Browser acceptance in
`scripts/lantern-tilt-smoke.cjs` completes all four setups through real drags,
checks the comparison text, tests reload and captures portrait/landscape cards.
Evidence: `build/evidence/lantern-tilt/`. Final browser package:
`paws-0.1.0-4fd37f9007a2ac29`, using the repository's independently advanced
engine pin `76d17928a5e51bbaac53874867e08a510e5b9c8f`. Windows Release host
build also passes at `build/windows-pinned-m4/package/Release/paws_windows.exe`.
Both WebGPU and forced WebGL2 pass all four setups with no page errors. The
player's current setup 2 was retained when refreshing the preview; its success
card will appear after the next accepted equilibrium.

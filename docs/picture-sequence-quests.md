# Picture-sequence quests

Pepper's garden bells is now a memory quest. A standing garden blackboard shows
leaf, star, leaf, paw and star, one at a time for one second each. It then goes
blank and accepts bell taps. Taps during playback leave the attempt unchanged.
Wrong responses clear the entered answer without a penalty. Click the blackboard
or use **Replay pictures** for another showing; replay clears the answer first.
Restart replays from the beginning. Menu, hidden-page and host suspension freeze
the clue clock. Reduced motion retains the timed pictures because they are the
clue itself, with no camera or symbol animation needed.

The reusable portable family is `rules::Family::PictureSequence`; `Bells` remains
an enum alias for existing consumers. `rules::pictureSequence` constructs a
definition from quest/content IDs, title, response objects, a picture order and
seconds per picture. Response object IDs and picture IDs may differ. It generates
the known solution using each response's `symbol`, rather than assuming the
symbol names are object IDs. Definitions accept 2–8 unique response symbols,
1–8 sequence entries, repeats, and durations from 0.25 to 10 seconds.

```cpp
auto d = rules::pictureSequence("other_memory", "other-memory-v1", "Garden pictures",
    {{"flower_button", "Flower", "flower_prop", rules::Piece::Bell, {0,0}, 0,1,{}, {},"flower"},
     {"moon_button", "Moon", "moon_prop", rules::Piece::Bell, {1,0}, 0,1,{}, {},"moon"}},
    {"moon", "flower", "moon"}, .75);
```

Author and register the corresponding response meshes and
`sequence_picture_<symbol>` GLBs in the game's accepted asset catalogue, and add
the definition to the normal quest/progression catalogue when making another
playable request. The example above demonstrates the type; it adds no extra quest
to the ten-request progression. Future response props can use their own mesh
roles; clicking a response in a picture-sequence quest routes to `TapSymbol`.

`PictureSequencePlayback` is a portable presentation value in the scene bindings.
The host advances it using `ApplicationFrame::presentation.delta`, with zero delta
while paused. The scene resolves generation-checked picture actor IDs and enables
only the current one. The rules own entered symbols, undo, completion and reward
eligibility; presentation cannot grant progress. The host gates all `TapSymbol`
intents during playback, including the direct scene path and GUI callbacks.

The new `ReplaySequence` verb is appended to the existing verb enum and stored as
`replay_sequence` in version-6 semantic journals. Existing enum values, quest IDs,
content IDs, required answer order and prior journal entries are retained.
Undo can restore a pre-replay prefix. Save/reload retains a partial answer and
replays the whole clue before accepting the remaining response taps. Playback
time is deliberately transient; no new save coordinator or reward owner exists.

The HUD shows the current picture number or entered-answer count, never the full
answer strip. The camera fits the board, response props and cat into free HUD
space; short landscape uses a side card and portrait reserves space for Menu and
Replay. Existing keyboard-accessible controls remain available.

Validation: seven portable suites (rules, progression/save replay, scene,
starter lifecycle, kit, enclosure and GUI) plus the actual generated browser
game. Browser checks cover one-second timing, early tap rejection, pause,
wrong answer, board click replay, reload after a prefix, once-only reward,
portrait/landscape framing and HTTP/SHA256 identity of the four new GLBs.
Evidence: `build/evidence/picture-sequence/browser/report.json`.
Physical-device performance and compatibility remain unmeasured.

Bell-click regression (14 October): error feedback changed the HUD's height,
which caused the host to reset the fitted camera. In an orbited, zoomed retry,
the first correct leaf tap moved the star target by 122.7 CSS pixels. A click at
the star's prior position then picked the leaf and cleared the answer to 0/5.
The memory HUD now reserves a stable height for playback, progress and feedback;
correct/error receipts keep the user's camera and response positions unchanged.
`scripts/bell-click-smoke.cjs` reproduces that retry with real pointer gestures,
checks zero target movement, and completes using the visible symbol faces.
Portable GUI coverage checks the same frame invariant at desktop, portrait and
short-landscape sizes. Evidence: `build/evidence/bell-click/browser/report.json`.

For click diagnosis, `window.pawsSequenceDebug` retains the last 200 sequence
events locally: picked response, pointer coordinates, tap/cancel, clue gate,
expected symbol and entered count before dispatch, then the result/count after
dispatch with attempt/revision. Set `window.pawsDebugSequence=true` or use
`?debugSequence=1` to also print `[Paws sequence]` console messages. These logs
are diagnostic only and do not change the answer, clue timing or save.

Generate the legal Chapter 3 fixture with the native `paws_picture_sequence_fixture`
target, passing `build/evidence/picture-sequence/chapter3.save` as its output path.
Serve the generated game, set `PAWS_URL`, then run
`node scripts/picture-sequence-smoke.cjs`. Fixture generation uses ordinary
authoritative commands in an isolated test owner; production exposes no unlock
or reward bypass.

Asset sources: [blackboard model job](../artifacts/model-jobs/picture-sequence/2026-10-14/README.md).

The current blackboard and bell assembly use two Tripo image-to-model generations,
fitted and reviewed through PawModels. One imported frame replaces the three
repeated supports; three separate bells retain picking and Pepper's animation.
The authored cream pictograms remain exact on both the board and the bells.
[Current models, sources and provenance](../artifacts/model-jobs/sequence-tripo/2026-10-14/README.md).

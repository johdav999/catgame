# Prompt 04 — seven puzzles and portable rules

Implemented 11 October 2026 against engine pin
`3fe7d1dc1dc610c703944803cbb717cd65024ec9`. All ten definitions validate;
the seven new requests complete, undo and restart through `release1::Intent`.
The production MVP and its three existing puzzle definitions/command processor
remain unchanged. Production registration belongs to Prompt 9.

## Delivered paths and ownership

- `src/release1/rules/puzzles.hpp`, `puzzles.cpp`: typed immutable definition,
  independent attempt, deterministic predicates, bounded history, validation,
  command preflight and frozen `PuzzleView` projection.
- `src/release1/rules/content.hpp`, `content.cpp`: ten-entry catalogue, three
  unchanged legacy definitions and seven version-1 release-one definitions.
- `src/release1/rules/debug_main.cpp`, `web/release1/rules/`: isolated Wasm
  harness with explicitly temporary engine cubes and HTML inspection controls.
- `tests/release1/rules/tests.cpp`: 628 native assertions.
- `scripts/release1/rules/build.ps1`, `smoke.cjs`, `integration.patch`:
  task-local builds, generated-site acceptance and deferred build registration.

No shared build, manifest, browser composition, Afternoon, save codec, GUI,
accepted cat/prop asset, common implementation-status or engine file was edited.
Concurrent cat/prop job/runtime files and existing dirty Prompt 1 work are
preserved. No agents, other-chat messages, commit, push or publication.

## Rule/owner interface

`paws::release1::rules::Attempt` is a puzzle attempt extension intended to be
owned by the existing Afternoon, never a second wallet or progression service.
Its constructor copies a `Definition`; `definition()` only exposes const data.
`logicalState()` is const and covers new typed families. Legacy/water attempts
delegate to the existing `PawsSession`; their portable state is projected through
`view()`. The three legacy wrappers exist for all-ten harness coverage; Prompt 5
must preserve the actual existing three sessions and their migrated journals.

`dispatch(Intent)` checks generation, quest, attempt and expected revision before
any mutation. Rejections preserve state/revision/history. Accepted puzzle changes
publish one revision and a maximum 128 prior states. Hints are free, monotonic
and bounded at three; undo restores logical pieces/sequence/completion while
retaining revealed hints. Restart clears history/hints and advances attempt.
Completed attempts allow selection, hint, undo and restart, then reject further
puzzle operations. No rule grants a reward or owns a saved friendship record.

`command(Step)` attaches current identities, useful for authored trace execution
and journal replay. Replay saved semantic steps with freshly attached identities;
never submit persisted runtime generation/revision as a new live command.
`activate(generation)` invalidates commands without erasing progress/history.
`setPaused(bool)` rejects commands while paused and invalidates pending input.
The host composes explicit pause and suspension; ending suspension does not clear
explicit pause. Production generation issuance remains application-owned.

Use `view().actions`: every candidate includes a complete typed intent, useful
label, enabled state and blocked reason calculated by the **same** command
processor, without publishing a change. No GUI legality copy is needed. Rotate
is one clockwise quarter turn, not absolute orientation assignment. Fit targets
are stable socket IDs; cushion Fit also uses `destination` (column,row). Planks
and cushions are picked up before rotating/fitting, including repositioning a
fitted piece. Picnic supplied props move directly to named authored spots.

The frozen DTO is unchanged. Shape footprints use its `ObjectView::footprint`;
the basket exposes all ten cells. Picnic SHADE/QUIET clue objects expose zone
cells through that same field; adjacency has a separate active clue. Switch/cat
and playhouse-door active states are derived from current assignments. Route
views expose the validated path; planks expose their full supported span.
Bells expose required and entered symbols separately. Definitions supply board
dimensions, three hints and portrait/landscape safe margins of at least 24 px.

There is no save codec here. Prompt 5 owns bounded semantic journals, validating
replay before publication, old save migration and the atomic once-only reward
ledger. It must preserve hints, attempt identities and history according to the
save contract, rather than serializing handles or exposing mutable State.

## Layouts and known solution traces

Coordinates are zero-based column,row with North decreasing row. World
conversion and cat movement remain presentation-owned. Shape subcells are
0.25 m; route/plank logical cells are 1 m. Every layout has three progressively
specific hints in `content.cpp`. The third states a complete fresh-attempt trace.

| Request | Layout and trace |
| --- | --- |
| Existing Pip requests | Unchanged `missingBall()`, `tidyToys()`, `stuckGate()`; original IDs, actions, hints and solution traces |
| `button_mouse_bench` | 5x3. Lane crate starts (1,1), upper crate (1,0). Walk lane (0,1)→(1,1)→near socket (2,1), mouse (3,1). Upper crate east, lane crate north, Retrieve mouse from `near_reach`. Far socket (4,2) cannot reach. No crawling under bench. |
| `mabel_sunny_spot` | 5x4. Start (0,2), supported 2 m edge to (2,2), 1 m edge to (2,1), 1 m edge to perch (3,1). Pick up/rotate once/fit Long→`long_socket`, Step→`step_socket`, Finish→`finish_socket`; Connect. Length/alignment/support/occupancy all validate. Unsupported decoy gap (3,3)→(4,3) rejects. |
| `button_cushion_bed` | 5x5, stock tray row 4. Basket cells: (0,0),(1,0),(0,1),(1,1),(2,0),(2,1),(2,2),(3,1),(4,1),(4,2). Pick square, fit `basket` at (0,0); pick bar, rotate once, fit at (2,0); pick elbow, rotate once, fit at (3,1). All ten occupied once completes. |
| `mabel_drinking_route` | 5x3, barrel (0,2)→bowl (4,0), west bowl entry. Fixed gutters at (1,2),(2,2),(2,1),(2,0),(3,0). Rotate Source once, Lower three times, Middle once, Upper once, Finish once; Roll/check flow (Connect alias also supported). Uses existing bounded port tracer. Water/filled bowl present validated success. |
| `pepper_garden_bells` | 3x2, Leaf/Star/Paw bell row. Tap leaf, star, leaf, paw, star. Wrong symbol commits an empty entered row with visible reset feedback, no penalty; undo recovers the previous prefix, restart clears it. Required sequence bounded to eight pictures. |
| `pip_pepper_playhouse` | 5x3. Pip→`pip_switch` (1,1), Pepper→`pepper_switch` (3,1), Connect/open doorway. `waiting` (2,2) releases the moving kitten's switch immediately. Distinct assignments on the same current state are required; historical presses never latch. |
| `garden_picnic` | 5x4 with eight authored destinations. SHADE: (1,0),(2,0),(1,1),(2,1),(3,1). QUIET: (1,1),(1,2). Supplied bed→`spot_1_1`; bowl→`spot_2_1`; Connect/check clues. Alternate bowl (1,0) also works. Bed needs both shade and quiet, bowl needs shade and orthogonal adjacency, and occupancy cannot overlap. |

### Basket/plank handoff to Prompt 3

Frozen cushion offsets remain square 2x2, bar 3x1 and elbow three cells. Clockwise
rotation maps (x,y) to (-y,x), then normalizes the footprint minimum to (0,0).
Accepted origins in the trace are respectively (0,0), (2,0), (3,1). Author the
ten-cell basket mask above and `socket_square`, `socket_bar`, `socket_elbow`
anchors using these normalized fit origins. Basket spans 1.25 x 0.75 m; tray
positions are debug inventory anchors, not additional occupied basket cells.
Runtime rules address stable role assets, independent of provider geometry.

Plank support/endpoint pairs are listed above. Endpoints have the frozen matching
port width and y=0.16 support plane. The 2 m plank spans two grid edges; the
1 m planks span one. Supported traversal is a visited-node graph search and
cannot loop forever. Asset meshes/animation never decide connectivity.

### Framing and accessible presentation handoff

Both aspect ratios must fit the entire playable board, required strip/zones,
endpoint/support labels and destinations outside UI exclusion areas; no target
may require rotating the phone. Keep a minimum 24 logical px framing margin,
and reflow for larger text. Stock trays can be separate object controls. Every
operation has a named object/action path and discrete destination; no precise
dragging, sound, colour discrimination or hover is required. Picnic zone names
and bell picture names remain visible. The harness is a scrolling development
inspector, not the final retained GUI or production camera/pacing acceptance.

## Verification and evidence

- Native task suite: **628 assertions passed**, including every solution,
  read-model action availability, invalid layouts/references, rotations/occupied
  cells, restricted reach, unsupported planks, cyclic/disconnected routes,
  simultaneous switches, bell error/reset/undo, all picnic predicates,
  independent attempts, 128-step history, hints, pause and stale identities.
- Existing portable baseline: **13/13 suites passed**, including scene/lifecycle,
  legacy rules, UI, saves, Pip asset/walk/settle and the garden kit.
- Pinned Emscripten **6.0.11 Release** harness compiled and staged. Chrome
  **154.0.8037.99** generated-site tests cover all ten layouts with keyboard at
  1440x900 and mouse at 844x390; all seven new layouts with emulated touch at
  390x844. Each gets three hints, completion, undo, final-step replay, restart
  and complete replay. Larger text and reduced motion are enabled.
- Browser checks cover visible wrong-bell reset/undo, stale revision rejection,
  explicit pause across suspension, no horizontal clipping and 40 repeated
  quest changes per layout with bounded temporary scene mesh counts. Final
  report has zero console/runtime errors. Full-page captures inspect named
  cells and the actual pinned-renderer board.

Evidence: `build/evidence/release1/prompt-04/native-final.txt`,
`baseline-final.txt`, `browser-final/report.json` and its 30 screenshots. The
browser report records exact Wasm/JS/shell/CSS SHA-256 and byte lengths. Build:
`build/web-release1-p04-pinned/site`. Earlier `browser/` run retains a missing
favicon 404; fixed with an inline favicon and superseded by `browser-final/`.
Installed toolchain execution required sandbox escalation; a stalled unapproved
direct test process was stopped and the final approved CTest run passed.

No physical mobile/Safari, player discovery/pacing, final asset/audio or final
retained-GUI acceptance is inferred from these development checks.

## Reproduce

```powershell
# From the repo root; compiler's matching runtime bin must be on PATH.
./scripts/release1/rules/build.ps1 -JoltSource C:/Users/Johan/source/repos/WebEngine/build/dependencies/JoltPhysics-5.3.0
./scripts/release1/rules/build.ps1 -Browser -EmsdkPath C:/Users/Johan/source/repos/WebEngine/.tools/emsdk -JoltSource C:/Users/Johan/source/repos/WebEngine/build/dependencies/JoltPhysics-5.3.0
$env:EVIDENCE_DIR='build/evidence/release1/prompt-04/browser-rerun'
node scripts/release1/rules/smoke.cjs
# Interactive task-local launch (serve.ps1 expects the production paws binary).
python scripts/serve-site.py --directory build/web-release1-p04-pinned/site --port 19614
```

Open `http://127.0.0.1:19614/`. The acceptance script owns/closes its own server
and browser contexts; it never opens or changes a user save slot. SDK/Jolt paths
above describe this machine only. Script defaults keep build directories scoped;
the harness enforces `external/WebEngine` with no local engine substitution.

## Deferred integration patch and next owners

`scripts/release1/rules/integration.patch` is the exact **unapplied** CMake
registration patch for Prompt 9. It separates pure rules (paws_rules dependency)
from typed content (existing catalogue dependency), registers meaningful native
tests, and links both into the final browser target. It does not pretend build
registration supplies progression or scenes. `git apply --check` verifies the
patch without editing shared files.

Prompt 5 owns seven extensions alongside Afternoon's three existing sessions,
unlocks/rewards/migration and journal codec. Prompt 7 consumes copied PuzzleView
and command intents. Prompt 6/9 project role assets and route/held/zone state,
using checked handles and Application clocks. Browser callbacks remain host-owned.
Prompt 9 must reconcile those owner interfaces, add production puzzle scene
composition, and exclude `web/release1/` inspection shells and preview binaries
from production staging/package (root staging currently recursively copies web).
Update the architecture at integration with: “Afternoon owns three legacy
PawsSessions and seven release-one puzzle extensions; new typed definitions and
predicates remain portable in release1/rules, with rewards and persistence in
Afternoon and copied PuzzleView/intents at presentation boundaries.”

Shared production content remains the MVP until that merge. No asset-production
dependency blocks these rule checks; missing final cats/props/audio block their
later presentation acceptance, rather than replacing rules with stand-ins.

# Pepper's picnic basket: world interactions

Pepper's beach quest uses three clickable pieces of evidence and three destination
assemblies. The HUD offers a single Clues & paths notebook button. Other
scent quests retain their existing presentation.

Paw prints lie on the sand. Blue cloth is snagged on the rope, grass, driftwood,
shells, shelter support or basket described by the current stop. A fish scrap on
a picnic cloth gives Pepper something to sniff. Destinations show the actual
rocks, rope post, dune paths, striped/plain shelters and shaded/sunny baskets.
The two additional v2 stops show forked driftwood and boulder/shell alternatives.
The historical four-stop v1 journal remains intact; current six-stop v2 journals
use the same presentation path.

Tap/press a prop to identify it; release to investigate or choose it. Evidence
captions, an examined count and a three-item checklist remain in the quest card.
Clue signs 1 (prints), 2 (threads) and 3 (scraps) say Examine, then gain
a check and Examined state. Ground boundaries distinguish all six interactive
assemblies from scenery. Path signs A, B and C say Read clues until all
three clues are examined, then all three say Follow. Every path receives
the same treatment; no marker consults the authored answer. Caption space is reserved so feedback does not
move the next pointer target. Generous actor-local selection proxies cover each
assembly, including its label. Clues & paths is directly available in the HUD
and also in Menu. The notebook shows each clue's examined state, retains only
discovered evidence, and provides equivalent keyboard actions. Every destination remains available there
once the three clues are examined; disabled choices never disclose the answer.
Step 1 directs the player to the numbered clues; Step 2 asks them to compare
all three clues and choose a lettered path. Wrong choices retain discoveries.
All three optional hints now offer deduction guidance without listing solutions.
Short landscape headers scroll within the left column; the notebook has a fixed
Back button and scrolls its evidence and route controls. Portrait places the
notebook button above Account & cloud so their touch targets never overlap.

The interaction labels are original code-generated UI art, using the existing
OFL Nunito Sans font and Paws cream/teal palette. Regenerate the embedded PNGs
with `python scripts/generate-pepper-markers.py`. They add no model dependency,
solid collision, light requirement or game state to the saved attempt.

Pepper follows collision-checked routes and starts her sniff/reaction only on
physical arrival. New inspections can replace an unfinished inspection. A wrong
destination prompts her personality reaction and return, preserving discoveries.
A correct choice shows her checking that destination before revealing the next
stop. The basket discovery plays a sniff and a settled sit before the existing
reward presentation. Menus freeze travel; Undo/Restart cancel it. Reduced motion
projects the settled state, including when enabled during travel. Completed
saves restore the basket scene without replaying the temporary celebration.

`Afternoon` and the existing Inspect/FollowTrail commands remain the sole state,
journal and reward owners. `SceneBindings::PepperTrail` contains disposable
presentation data and generation-checked component/actor handles. Scene clocks
cannot advance a stop. The model pass does not change the engine, content
identity, save codec or trail rules.

## Asset reuse

The assemblies reuse the accepted Paws garden/release-one kit and coastal Tripo
exports; their existing provenance and license records apply. The blue thread
uses narrow instances of the accepted picnic cloth. Small shell keepsakes reuse
the shell-chime leaf mesh laid on the sand. The plain shelter retains the plank
assembly. Striped shelters and the forked branch now use the reviewed Tripo
[trail models](beach-trail-models.md), including a smaller branch clue. Blue cloth
remains separate and is placed at the appropriate fork or front/rear support.
Both landmark meshes participate in the existing triangle collision queries.

## Verification

Verified on the pinned Release build. The [accepted evidence summary](../build/evidence/pepper-world/summary.json)
records the build hashes, renderer/motion scope and per-version results.

- Build: `build/web-pepper-world-pinned-release/site`, exact pinned WebEngine
  `0426d9319de715584a2e34ff9862cd6ba873ecd0`.
- Native: `paws_starter_scene`, `paws_release1_scene` and `paws_release1_gui` in
  `build/native-pepper-world-pinned`; both historical and current trail versions,
  physical arrival, reaction completion, wrong paths, pause, restart, restored
  completion, stable framing and keyboard alternatives.
- Browser: `scripts/pepper-world-smoke.cjs`, using isolated fixture saves from
  `paws_release1_scene_tests build/evidence/pepper-world/fixtures`.
  Evidence and per-backend reports live in `build/evidence/pepper-world/`.
- Browser moves use real pointer gestures and retained keyboard controls.
  Fixture insertion only establishes preconditions. Viewports: 1600x1000,
  390x844 and 844x390; the WebGL 2 desktop pass uses 1280x800.
  WebGPU exercises full motion. WebGL 2 exercises larger text and reduced motion,
  including pausing an active investigation before enabling those settings.
  This does not establish physical-mobile acceptance.

## UAT ledger

| ID | Problem / acceptance | Repair | Evidence |
| --- | --- | --- | --- |
| PEP-01 | Quest requires a bottom button tray | Direct clue/destination picking and optional keyboard menu | Browser start captures; GUI regression |
| PEP-02 | Generic markers/pots do not describe destinations | Per-stop kit assemblies and physical evidence | Browser stop captures; scene regression |
| PEP-03 | Cat reactions must accompany discoveries without changing rules | Arrival-driven sniff, puzzled return, transition and basket sit | Native arrival assertions; browser inspection/wrong-trail captures |
| PEP-04 | Feedback or resize must not hide/move the next target | Reserved caption space and quest-specific camera fit | Three viewport captures; pointer-coordinate stability assertion |

All four ledger items are verified within the native/desktop-browser scope
above. Both content versions complete, preserve discoveries on wrong paths,
support keyboard inspection and Undo, resume mid-trail, and revisit the completed
save without another reward or a replayed discovery animation.

The combined WebGPU test completed v1, then Chrome reported
`ERR_NO_BUFFER_SPACE` while loading v2. The v2 full-motion run passed in a fresh
browser process; the reusable test now launches a fresh process per version.
The earlier short full-motion WebGL wait expired while the cat was still moving;
the accepted fallback scope is explicitly reduced motion and larger text.

Initial review found edge-on paw prints, a clipped foreground cat, and a fixed
travel timeout that could react before arrival. The retained corrections orient
and enlarge the prints, frame the full apron, and require actual route arrival.

## Quest clarity acceptance, 21 October 2026

[Evidence summary](../build/evidence/pepper-clarity/summary.json) records the
final build and scope. This pass changes interaction presentation and hint copy;
the trail answers and saved content identities remain compatible.

- Native pinned-engine suites passed: `paws_starter_scene`, `paws_release1_scene`
  and `paws_release1_gui` (2,805 GUI assertions).
- Final browser build: `paws-0.1.0-f39eb0cdd11745d2`, Wasm SHA-256
  `19ddf7bb2d0cac2a62affe64164ae8ffef1d6936613d1580374756d9d0bb2189`.
  It uses the existing local water/shadow engine `077df1e72364229295dc746768b806531ab37eb4`;
  this pass does not advance the repository's engine pin.
- [WebGPU](../build/evidence/pepper-clarity/webgpu-accepted/report.json): full-motion
  four-stop quest, real pointer and keyboard input, wrong choices, Undo, mid-trail
  reload and completed-save reopening. Zero JavaScript/console errors.
- [WebGL2](../build/evidence/pepper-clarity/webgl2/report.json): forced fallback,
  six-stop quest, larger text and reduced motion, including the same interactions
  and save checks. Zero JavaScript/console errors.
- Portrait 390x844, short landscape 844x390, and desktop 1600x1000 / 1280x800
  retain all six targets. Notebook targets are at least 44 px and do not overlap
  Account & cloud. Clue feedback leaves target coordinates unchanged.

Repeated full asset reloads in one headless Chrome process encountered
`ERR_NO_BUFFER_SPACE` and an asset network failure while the renderer remained
ready. Failed attempts are retained in the evidence directory. The accepted
WebGPU run reopens the completed save in a fresh Chrome process with its original
IndexedDB state; the mid-trail reload remains in the same process. WebGL2 also
passed the completed-save check with a same-process reload. This desktop-browser
evidence does not establish physical-mobile acceptance.

## Physical path features, 21 October 2026

The original assemblies exposed interaction labels but left several route
attributes implicit. All alternatives now have visible tracks, with the same
visual emphasis regardless of the answer. They reuse the reviewed coastal kit;
no new GLBs, provider generations, content versions or save changes are involved.

| Stop in v2 | Features visible in the three alternatives |
| --- | --- |
| 1 | Exposed shoreline rocks; a rope-wrapped wooden post behind a windbreak; a smooth stone behind the same windbreak. |
| 2 | A trail around the left or right of two hummocks, low grass and blue thread on dry sand, or a rippled wet runnel with wet stones. |
| 3 | Footprints pass the left or right of forked driftwood; blue cloth catches on the low or high arm. Every option has sheltered dry ground. |
| 4 | A broken, projecting rock face versus a broad smooth boulder; two enlarged shells and blue thread, or a wet runnel. |
| 5 | A striped or plain fabric roof; tracks and blue cloth end at the front support or continue around the side to the rear support. |
| 6 | A woven basket with blue cloth across its lid; a shell or grass at its side; an overhead canvas cover or an open sunny position. |

The historical four-stop version uses the same assemblies for its four rounds.
The route surfaces are small shared masked PBR sand/water textures on flat
presentation planes. Water has a contrasting surface, ripples and shoreline
stones; a canopy communicates shelter even when shadows are unavailable. Blue
cloth uses an explicit matte-blue instance material so the source picnic mat's
green pattern cannot obscure it. All terrain and gameplay contact planes remain
flat. Labels sit ahead of the tracks, and proxies cover the larger arrangements.
The camera fits the six targets more closely without moving them when clues are
examined. Wrong choices do not remove inspected evidence or mark a correct path.

Validation is recorded in `build/evidence/pepper-paths/`. The dedicated
`scripts/pepper-path-visuals.cjs` opens no notebook: it examines clues and chooses
paths with real scene clicks, tries a wrong option at every stop, and captures
all six stops at 1600x1000, 390x844 and 844x390. The native scene regression also
checks physical left/right tracks, wet/dry surfaces, proxy coverage, both saved
content versions, stage visibility, full-motion arrivals and restart.

The preview is built in `build/web-plants-release/site`, retaining the current
plant-grounding work. Its compiled game is byte-identical to the renderer-tested
`paws-0.1.0-4785691e45ce1adb` build (Wasm SHA-256
`1d5caea58e7cdeaf6dc62885673448e0c6ce8f57d6263cb4f2be5e34f5ddb5f6`).
The existing local water/shadow engine remains
`077df1e72364229295dc746768b806531ab37eb4`; the game engine gitlink is unchanged.

Native `paws_starter_scene` and `paws_release1_scene` pass (296.80 seconds
combined), including both four-stop and six-stop content and physical arrival.
[WebGL2](../build/evidence/pepper-paths/webgl2-final/report.json) passes all six
stops with reduced motion, no notebook use and no JavaScript/HTTP errors.
Screenshots include [left/right dry/wet trails](../build/evidence/pepper-paths/webgl2-final/stop-2-1600.png),
[high/low branches](../build/evidence/pepper-paths/webgl2-final/stop-3-1600.png),
[shelter fronts/rears](../build/evidence/pepper-paths/webgl2-final/stop-5-1600.png)
and [basket shade/shell comparisons](../build/evidence/pepper-paths/webgl2-final/stop-6-1600.png).

An initial full-motion WebGPU run reached the final stop, but disk exhaustion
prevented saving its last screenshot. That run is retained as failed evidence
under `webgpu-final/`; it is not counted as completed acceptance. Removing only
the redundant generated assets from the stopped temporary test site recovered
space. The repository's original assets and active preview were preserved.
Desktop viewport emulation does not establish physical mobile performance.

[Final WebGPU acceptance](../build/evidence/pepper-paths/webgpu-accepted/report.json)
completed all six stops with full motion on the user's `8080` preview, using only
world clues and paths. Wrong-choice recovery passed at every stop; all three
viewport captures passed; no JavaScript or HTTP errors were recorded. The
reduced-motion WebGL2 run above uses the identical Wasm/content build.

## Persistent clue evidence, 21 October 2026

The HUD now keeps the full text of each examined clue in a numbered list.
Unexamined entries prompt the player to inspect them; wrong-choice feedback is
shown separately below the evidence. Moving to the next stop clears the old
evidence. Row heights are reserved for the authored clue text so inspecting a
clue does not move the scene targets. The notebook remains optional.

Native `paws_release1_gui` passes, including evidence visibility, next-stop reset
and stable board bounds. The browser test examines all clues, tries a wrong path
and asserts that all three clue texts remain visible at each of the six stops.
[WebGPU](../build/evidence/pepper-clue-hud/webgpu/report.json) and
[WebGL2](../build/evidence/pepper-clue-hud/webgl2/report.json) both pass on the
actual `8080` preview with reduced motion, at 1600x1000, 390x844 and 844x390.
Neither run reported JavaScript or HTTP errors. This is desktop browser viewport
emulation, not physical mobile acceptance.

This HUD build supersedes the earlier preview above:
`paws-0.1.0-4bbe5dd8b3420d8d`, Wasm SHA-256
`d0ae56daead0ebc94c5995324509cdcb6aa6a62a779e657bd7b6b1451da433bb`.
The engine revision and quest content are unchanged.

Clue evidence is now revealed on Pepper's physical arrival at the selected clue,
rather than at command acceptance. The scene adapter holds a transient mask of
inspections awaiting arrival; both the HUD and notebook use that presentation
mask. The examined ground marker changes at the same arrival event. Previously
revealed evidence remains visible during later travel. Reduced motion reveals
the settled inspection immediately, and restored saves show their recorded
evidence. The mask is scene-owned and does not change the saved rule journal.

The arrival regression checks every simulation step before and after arrival in
both saved quest versions; native `paws_release1_scene` passes (262.98 seconds).
The browser regression checks hidden evidence during travel and visible evidence
at arrival for each clue, then exercises wrong choices and the existing three
viewport layouts. Arrival evidence is under `build/evidence/pepper-clue-arrival/`.
The generated preview build is `paws-0.1.0-d0f944671d8de648` (Wasm SHA-256
`2cd11c16f3238cf63e2918518422382391147c3ee3399c1e811f4c959caa80b3`).
[WebGPU](../build/evidence/pepper-clue-arrival/webgpu/report.json) and
[WebGL2](../build/evidence/pepper-clue-arrival/webgl2/report.json) both pass all six
stops with full motion on `8080`, with no JavaScript or HTTP errors. Captures show
the [unrevealed clue during travel](../build/evidence/pepper-clue-arrival/webgpu/clue-before-arrival.png)
and [revealed clue on arrival](../build/evidence/pepper-clue-arrival/webgpu/clue-after-arrival.png).

## Vertical evidence trail, 21 October 2026

The evidence now forms a column up the beach: prints (1) in the foreground,
threads (2) midway, and scraps (3) nearest the three path alternatives. Small
neutral paw prints connect the clues and face towards the fork. They stop before
the choices and never identify an answer. The numbered signs sit beside their
clues with matching selection proxies, leaving the footprint trail exposed.

The camera frames the longer column and the full path row together. Pepper's
waiting position is beside the first clue. The landward scenery band is moved
back for this quest, while the existing pier stays at its shoreline position.
The extended trail remains on the existing flat sand. Clue arrival timing and
saved puzzle cells retain their existing owners.

Native `paws_release1_scene` passes (262.78 seconds), including the ordered
column, flat sand, both saved quest versions and physical clue arrivals.
[WebGPU](../build/evidence/pepper-vertical/webgpu/report.json) completes all six
stops with full motion, including evidence hidden until arrival;
[WebGL2](../build/evidence/pepper-vertical/webgl2/report.json) passes all six
layouts with reduced motion. Both check foreground-to-background clue order,
real pointer input, wrong-choice recovery and all six targets at 1600x1000,
390x844 and 844x390, with no JavaScript or HTTP errors. See the
[desktop layout](../build/evidence/pepper-vertical/webgpu/stop-4-1600.png) and
[portrait layout](../build/evidence/pepper-vertical/webgl2/stop-4-390.png).
This is desktop viewport emulation, not physical mobile acceptance.

The active preview is `paws-0.1.0-759c278350bd6a59`, Wasm SHA-256
`cbde9bd134ea2980b9f6a64ebb7bf9d69df200a2ffd10c9a3b20c4d345ab266d`.

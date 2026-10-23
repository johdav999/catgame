# A Seaside Afternoon: playable beach adventure

Five additional outings now follow the original picnic, bringing the pack to
eleven quests. See [new quests, save compatibility and validation](beach-outings.md).
The original six-quest milestone and its preserved postcard are described below.

Pepper's basket quest now uses clickable world clues, destination props and cat
reactions. Its bottom choice tray is removed. See [world trail interactions and
verification](pepper-world-trail.md) for the four-/six-stop compatibility path.

The first [environment pass](beach-environment.md) adds flat-apron terrain,
side dunes, animated shallow water and camera framing. Puzzle props and cove
story landmarks still include greybox assets.

The development pack `beach_afternoon` contains six sequential quests and a
separately recorded picnic ending. It uses the existing Paws rules, scene
preparation, retained GUI and IndexedDB coordinator. It is a gameplay milestone;
production beach art, original beach audio, keepsakes and payment services remain
outside this delivery. First-time human pacing has not been measured.

## Play

```powershell
./scripts/build.ps1 -Configuration Release -BuildDirectory build/web-beach-local-release
./scripts/serve.ps1 -SiteDirectory build/web-beach-local-release/site -Port 8194
```

Open `http://localhost:8194/?devBeach=1`. Choose Play/Continue, then Adventures
and Start / Continue beach. The primary action resumes the first unfinished
request. Each completion offers Continue beach story. Adventures also lists
individual unlocked requests; Revisit preserves their boards. Use Menu → Restart
puzzle to explicitly replay a completed request.

| Quest | Rule family | Story change |
| --- | --- | --- |
| Pip's runaway beach ball | Sliding and rolling | Park four baskets sharing dune spaces and connect five channels. |
| A little bridge for Button | Supported planks | Clear two landing baskets and connect five winding spans; avoid loose supports and a firm dead end. |
| Mabel's shady corner | Supported shade panels | Cover eight mat cells with four panels; avoid firm posts outside the mat. |
| Pepper's missing picnic basket | Six evidence-and-choice stops | Compare three clues at each stop, including side, height and shelter. |
| The shell-chime melody | Eight-picture memory sequence | Remember two phrases with a repeated leaf; free visual replay remains available. |
| Everyone has a place | Four-cat picnic placement | Park three baskets off the approach, then match the four mats to the cats' clues. |

Hints are free, wrong moves preserve committed state, and Undo and Restart work
through the authoritative owner. New attempts use the six `beach-*-v2` layouts.
Their authored solution lengths are 14, 18, 13, 24, 8 and 13 semantic operations
respectively (previously 10, 10, 6, 16, 6 and 5). These include pickup and final
checks; browser dragging combines some operations. Difficulty comes from shared
parking, route planning, shade coverage, evidence comparison and memory. There
are no timers or hint penalties. First-time human pacing still needs playtesting.

The final completion offers Watch beach ending. This records the seaside
postcard and shows all four cats at their assigned mats with closing dialogue.
Replay beach ending is available after recording. Ending replay and all quest
revisits leave base coins, friendship, chapters and the garden ending unchanged.

## Access and saves

Localhost launches enable the development preview by default, so Adventures
remains available without a query string in both garden and puzzle pause menus.
`?devBeach=0` disables it for access-boundary checks; `?devBeach=1` explicitly
enables it on other hosts. This transient development access is absent from saves
and is not an entitlement, receipt or security mechanism. Other normal launches
hide the entry. Opening an active beach save without access returns to its remembered
base request while retaining the beach journal. Accounts and real-money purchase
verification remain future work.

`Afternoon` remains the sole progression/save owner. `QuestPolicy::pack` separates
adventure progress from base rewards. Six once-only completion records determine
beach unlocks and visible cove milestones. Base quest IDs, indices, completion
requirements and reward calculations remain stable.

Version 11 adds five quest journals and `BEACH_END`, validated against completion
of all six beach quests. Versions 1–10 remain readable; in particular a first-
milestone version-10 beach save retains its original layout, journal, completion
and remembered garden request. New quests initialize fresh. Imported bytes can
validate content history without granting current access. The `HOME` row remains
an ID-keyed reference to the base request selected before travel.

The original six v1 definitions remain available for exact save replay. Loading
an old attempt preserves its pieces, journal and completion. Menu → Restart
puzzle explicitly starts the longer v2 layout and records the upgrade in the
same journal; subsequent reloads validate both versions in order. Completed
milestones and the ending remain earned, without additional base rewards.

The beach ending is recorded by an explicit generation/attempt/revision-checked
intent. It does not write the garden picnic flag or a wallet. Replaying a save
checks each semantic command, prerequisites, content version and completion
ledger. Saved ending flags without completed beach history are rejected.

## Runtime and scene ownership

The browser adapter owns URL access, HTTP preparation, lifecycle and the existing
save coordinator. Navigation prepares a detached candidate before publishing it;
failed loading retains the active owner and scene. Leaving preserves the current
attempt and invalidates delayed scene commands. Only one loaded scene is retained.
WebEngine is unchanged; the shared pinned dependency is not advanced.

Beach views reuse the compact cove composition. Recovered ball, bridge, canopy,
basket, chime shelter and picnic blanket landmarks appear from the saved beach
completion mask, including after scene rebuild. These are presentation projections,
not additional progression owners. Scene-scoped handles update visibility, and
four named, symbol-marked mats determine the final cats' resting positions.
The closing card samples the final picnic-sit pose before simulation pauses,
and all beach scenes use the current free GUI rectangle for camera framing.

Shade panels use supported, oriented two-cell sockets. Every panel must be
fitted on distinct firm supports and cover all eight target cells before Connect
can complete a v2 attempt. The picnic places four named mats, rejects occupied
spots in both basket/mat directions and blocked approaches, then checks the
cats' preferences together.
Existing plank, scent and picture-sequence processors supply the other quests.
Pictures communicate the full chime sequence without requiring sound.

## Coastal presentation and provenance

The scene uses heightfield terrain, layered sand/dune materials and the engine
water shader. Water has no swimming, tide or progression role. Reviewed manual
Tripo exports supply closed picnic baskets, rocks, dune grass and a rope-sided
boardwalk; scenery stays outside the playable apron and has no interaction or
collision providers. The lower baskets hide currently covered legacy channels
until moved, preserving reliable centre-grabs. Existing cats and kit meshes retain
their provenance/licenses. Shade panels and some progress landmarks remain simple
geometry; marked shell chimes use the current picture/bell kit.

Coastal model sources, packed Blender review, immutable runtime hashes and
both-renderer evidence are documented in [the coastal asset pass](beach-coastal-assets.md).
Further asset production follows the required Tripo reference-first and PawModels
review workflow. This pass adds no new sound files.

## Verification

- `paws_beach_adventure`: first-slice access, save replay, travel, undo,
  independent completion, malformed content and old base-save migration.
- `paws_beach_story`: every move of all six authored solutions roundtrips;
  every quest leaves/resumes, undoes and restarts; version-10 migration,
  prerequisite gating, shade coverage, both basket/mat collision directions,
  picnic constraints and ending validation. All original v1 solutions still
  replay, and explicit upgrades to each v2 solution persist through reloads.
- `paws_release1_gui`: actual retained adventure selection, six story steps,
  per-quest continuation, explicit ending and saved postcard.
- `paws_release1_scene`: actual scene transitions/projection, saved cove
  landmarks, center-grab safety and four cats resting at their assigned mats.

```powershell
node scripts/beach-story-smoke.cjs
$env:QUEST_BACKEND='webgl2'
node scripts/beach-story-smoke.cjs
```

The smoke starts fresh and uses keyboard activation plus real scene clicks and
drags. Every quest tests Undo, hints, explicit Restart, mid-quest reload and
return/reload/revisit. Portrait screenshots and portrait gesture checks accompany
the full story. The ending is saved, reloaded and replayed without base rewards.
Set `BEACH_LEGACY_UPGRADE=1` to load the six original saved layouts, explicitly
restart each into v2, and repeat the full real-input journey. Current difficulty
evidence is under `build/evidence/beach-difficulty/`, with fresh and upgrade runs
for both renderers and separate native logs.
Reports and screenshots live in `build/evidence/beach-story/{webgpu,webgl2}/`;
the native acceptance log is `build/evidence/beach-story/native-tests.txt`.
Reports identify the tested renderer, build and Wasm digest. Native tests and
resized desktop browser views do not establish physical mobile acceptance or
human pacing/art approval.

Difficulty expansion verification, 19 October 2026: seven affected native suites
passed, including validation of the actual clue interaction bounds. The final
WebGL 2 run completed all six v1-to-v2 upgrades, real-input solutions, Undo,
Restart, reload, garden travel and ending replay with zero browser errors, using
reduced motion. See `build/evidence/beach-difficulty/final-webgl2/report.json`.
An ordinary-motion WebGPU run completed all six expanded quests and the ending
checks, then failed its clean-console assertion on `ERR_NO_BUFFER_SPACE`. A later
repeat encountered a rendering failure during travel. These local repeated-run
limits remain recorded alongside the earlier clean fresh-game WebGPU/WebGL 2
reports; they are not claimed as clean final WebGPU acceptance.

Final acceptance on 18 October 2026: all 11 affected native suites passed,
followed by a final scene regression after symbol normalization. Both browser
backends completed the full fresh-game journey on the same generated local-engine
Release build with zero browser errors. The smoke asserts all eight bounds corners
of each of the four cats remain within the portrait ending's play area.
`build/evidence/beach-story/acceptance-summary.json` links the build, Wasm digest
and reports; the final scene log is `native-scene-final.txt` alongside the native
suite log. This evidence does not establish physical mobile or human creative
acceptance.


## Complete coastal prop kit (2026-10-20)

The remaining nine requested prop groups now use 19 reviewed runtime GLBs from
15 Tripo ImageGen-reference-first sources. These replace the beach bridge
planks and timber landings, shade-panel greyboxes and posts, Pepper's rope-post
and loose-shell substitutes, shell chimes and frame, and four picnic cushions.
Two rock forms and three grass variants diversify the existing edge planting;
shell keepsakes are scattered outside the clear playable apron. Saved cove
milestones use the same reviewed kit. The earlier basket, boardwalk, channels,
striped shelter and branch remain in place.

Puzzle roles, arrangements, progression, input and saves are unchanged. The two
bridge lengths preserve the 0.16 m walking surface, with measured level centres.
Shared canopy supports are instantiated once. Chimes keep independent actors,
large interaction bounds, picture symbols and existing sequence behaviour.
Picnic mats have four material colours plus the existing cat identity symbols;
the settled cats use each mesh's actual top instead of the old cushion offset.

Runtime files and immutable hashes are listed in
`assets/runtime/models/beach/manifest.json`. Original provider output, packed
editable Blender source/master files, all three review stages, provenance and
contact measurements live in `artifacts/model-jobs/beach-complete-20261020/`.
The 19 new files total 43,549,120 bytes and 40,399 triangles across unique assets.
They use 512/1024-pixel maps; instantiated scene counts vary by quest.

Validation details: `artifacts/model-jobs/beach-complete-20261020/EVALUATION.md`.

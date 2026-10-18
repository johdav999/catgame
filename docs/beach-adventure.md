# A Seaside Afternoon: playable greybox adventure

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
| Pip's runaway beach ball | Sliding and rolling | Recover Pip's ball for the picnic. |
| A little bridge for Button | Supported planks | Reach a dry, quiet nook across the runnel. |
| Mabel's shady corner | Supported shade panels | Cover all four marked mat cells. |
| Pepper's missing picnic basket | Four evidence-and-choice stops | Recover the blanket and basket through the dunes. |
| The shell-chime melody | Six-picture memory sequence | Gather the friends with a visual and musical tune. |
| Everyone has a place | Four-cat picnic placement | Match named symbol mats to shade, quiet, adjacency, sunshine and approach-clearance clues. |

Hints are free, wrong moves preserve committed state, and Undo and Restart work
through the authoritative owner. The first quest retains `beach-pip-ball-v1`
unchanged. All five additions have their own stable quest and content IDs.

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

Shade panels use supported, oriented two-cell sockets. Both panels must be
fitted on distinct firm supports and cover all four target cells before Connect
can complete the attempt. The picnic places four named mats, rejects occupied
spots and blocked approaches, then checks the cats' preferences together.
Existing plank, scent and picture-sequence processors supply the other quests.
Pictures communicate the full chime sequence without requiring sound.

## Greybox and provenance

Flat sand, a decorative sea strip, low dunes, a shallow runnel and shade panels
are code-composed primitives. Water has no swimming, tide or progression role.
Closed primitive blocks stand in for beach baskets: the open toy-basket and
crate assets expose the channel beneath their center grab points. Existing Pip,
Mabel, Button, Pepper and kit meshes retain their existing provenance/licenses.
The marked shell chimes use the current picture/bell kit as placeholders.

No new generated production models, textures or sound files are introduced.
Future beach asset production must follow the required Tripo reference-first
workflow and PawModels normalization/review before replacing placeholders.

## Verification

- `paws_beach_adventure`: first-slice access, save replay, travel, undo,
  independent completion, malformed content and old base-save migration.
- `paws_beach_story`: every move of all six authored solutions roundtrips;
  every quest leaves/resumes, undoes and restarts; version-10 migration,
  prerequisite gating, shade support, picnic constraints and ending validation.
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
Reports and screenshots live in `build/evidence/beach-story/{webgpu,webgl2}/`;
the native acceptance log is `build/evidence/beach-story/native-tests.txt`.
Reports identify the tested renderer, build and Wasm digest. Native tests and
resized desktop browser views do not establish physical mobile acceptance or
human pacing/art approval.

Final acceptance on 18 October 2026: all 11 affected native suites passed,
followed by a final scene regression after symbol normalization. Both browser
backends completed the full fresh-game journey on the same generated local-engine
Release build with zero browser errors. The smoke asserts all eight bounds corners
of each of the four cats remain within the portrait ending's play area.
`build/evidence/beach-story/acceptance-summary.json` links the build, Wasm digest
and reports; the final scene log is `native-scene-final.txt` alongside the native
suite log. This evidence does not establish physical mobile or human creative
acceptance.

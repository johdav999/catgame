# Missing-ball content schema, version 1

Phase 1 authors immutable typed C++ data in `src/content.cpp`, validated by
`validateContent`. There is no external parser or JSON quest loader yet. The same
definition and rules run in native tests and browser Wasm. This deliberately
implements only the current route objective; the broader root architecture is a
proposal for later phases.

| Field | Contract |
| --- | --- |
| `schemaVersion` | Exactly 1; unsupported versions are rejected. |
| `contentId`, `questId` | `missing-ball-v1`, `pip_missing_ball`; nonempty stable identities. |
| `width`, `height` | Board bounds, at most 16 x 16; current board 5 x 3. |
| `sourceId`, `destinationId` | Stable `source_ball`, `pip_destination`; distinct from each other and all object IDs. |
| `source`, `sourceExit` | Ball cell and first outgoing direction. |
| `destination`, `destinationEntry` | Terminal cell and required incoming side. |
| `objects` | At most 64 uniquely named objects; pot, straight or curve only. |
| object `initial`, `orientation` | In-bounds cell and clockwise quarter-turn, 0–3. |
| pot `slots` | Unique authored in-bounds cells, including the initial cell. |
| `solution` | Typed Move/Rotate/Roll trace; validation replays it and requires completion. |
| `hints` | Three free progressive hints for the authored puzzle. |

Coordinates start at the top left, x east and y south. Directions are North=0,
East=1, South=2, West=3. At orientation 0, straight ports are N/S and curve ports
N/E; clockwise rotation adds one to both ports modulo four. Channels are fixed.
Pots move one adjacent cell per command, only to their own slots. Pot occupancy
and channel occupancy are separate layers: a pot may cover a channel, blocking
the ball until moved away. Two pots cannot share a slot; two channels cannot share
a cell. No object or sliding slot may occupy an endpoint.

`evaluateRoute` traverses the exact committed board from the source. It tracks
visited (cell, incoming port) pairs; loops, off-board exits, empty cells, pots,
wrong channel openings and wrong destination entry return a finite blocked
result. A failed Roll changes neither puzzle state nor revision. Only a
successful explicit Roll commits completion. Physics is never initialized for
this puzzle.

`PawsSession` owns `GameState`, content and up to 128 undo snapshots. Typed
commands contain attempt ID and expected revision. Every accepted command
advances the revision; Restart also advances the attempt and clears history.
Invalid/stale commands leave the state untouched. Undo can reverse prototype
completion because phase 1 grants no rewards; phase 5 must establish the reward
transaction boundary before introducing progression.

Scene bindings map the stable object identities to generation-checked `ActorId`
handles. Projection validates all bindings before writing, rejects old
attempt/revision projections and resolves actors afresh. Restart reprojects the
same scene. Browser callbacks remain in the host; renderer/controller borrows
are released before `Application` destroys its scenes. No async assets or save
callbacks exist in this slice.

Only `assets/runtime/` is staged at relative `assets/` URLs. Its manifest has
schemaVersion 1, contentId and an initially empty imported asset list. Future
entries must record stable ID, relative URL, SHA-256, byte length, source/provider,
transformations and provenance/license notice. Source masters and rejected media
belong in `artifacts/`, outside staging. Procedural greybox geometry is not a
production model delivery.

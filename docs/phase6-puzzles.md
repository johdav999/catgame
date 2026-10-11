# Three reusable requests

The authored order is `pip_missing_ball`, `pip_tidy_toys`, `pip_stuck_gate`.
`src/content.cpp` contains each immutable definition, three progressive hints and
an executable known solution. `validateContent` replays every solution with the
same command processor used by the game. New definitions use schema 2; the
existing schema-1 missing-ball definition remains supported.

## Rules and identities

`PawsSession` owns one attempt. `Afternoon` owns three separate sessions and a
once-only completion/reward ledger. Completing a request grants 20 coins and one
friendship in the same command transaction. Undo, restart and practice never
remove an earned reward or grant it again. A request unlocks only after the
previous request has earned its reward. Switching requests preserves each attempt
and its undo stack, selection and hint stage. Cross-quest activation allocates an attempt
epoch greater than every existing attempt, rejecting old cross-quest commands.

The reusable vocabulary is sliding obstacles, straight/curved channels, shaped
toys, shaped recesses, a ball-occupied switch, a linked gate and a collectible.
Stable roles bind the requested item, switch and gate. Reachability traverses
authored accessible cells from a fixed access point, with obstacles and closed
gates excluding cells. Visual meshes do not determine legality.

Tidy the basket starts with a cushion blocking access. Slide it north or south,
pick up one toy, then choose an explicit matching recess. Wrong shapes, occupied
recesses, blocked access and picking up a second toy reject without mutation.
Undo restores held state, recess occupancy and cushion position together.

The gate request requires moving a pot and turning three route segments. Roll
puts the source ball on the pressure switch. Gate state is derived from that
occupancy, never latched independently. Return ball or edit the route to close
the gate; undo restores the preceding occupancy. Retrieve the mouse only while
the linked gate is open and its cell is reachable. Undo retrieval restores the
mouse, and undo Roll also releases the switch and closes the gate.

## Presentation and loading

The browser composes phase-3 Pip and the phase-4 garden kit through shared
immutable engine mesh assets. Independently moving switch plate and gate leaf
are explicit actors positioned from the kit's authored anchors. Basket anchors
and toy orientations share the basket's rotation. Channel mesh ports map to the
same discrete orientations as route rules. Production scene tests verify these
transforms and sharing against actual GLBs.

Scene bindings hold generation-checked handles. Sliding, pickup/place and gate
motion interpolate committed results; rolling samples the channel route. Rapid
commands replace presentation targets without delaying logical outcomes. Reduced
motion applies final transforms and static Pip poses. `Application` supplies the
presentation clock and advances skeletal animation once.

Asset preparation uses the engine asset manager and a cancellable
`AssetPreparation`. The outgoing scene remains active while dependencies load.
All CPU scene construction completes before the renderer is rebound and the
application changes active scene; old controller/renderer references are released
before unloading. A failure exposes Retry loading and Back while retaining the
previous scene. Escape
or Back cancels the preparation interest; a late asset completion has no path to
publish that abandoned scene. This is not a claim of asynchronous GPU prewarming.

Mouse, touch and keyboard equivalents share retained engine-GUI actions. Selecting
an object reveals its actions. Legal targets have a reason and explicit Place
button; no precise drag or hover is required. Invalid targets remain readable.
The HUD uses shorter portrait titles and expands wide objectives to avoid clipping.

## Local snapshot contract

Phase 5's save/economy implementation was absent at the start of phase 6. The
minimal delivered owner is therefore `Afternoon`, not a second copy of an existing
phase-5 service. Phase 7 should extend it for spending, ownership and placements.

The original version-1 `PAWS_AFTERNOON` payload stores content IDs, active quest, settings
and accepted command records per quest. Phase 5 now writes version 2 with explicit
wallet/reward records and attempt/revision identities, validating those fields
against replay and retaining version-1 reads. See [the current codec contract](phase5-garden-loop.md).
Decode builds a separate candidate and
replays bounded records through current rules. This reconstructs attempts,
selection, hint disclosure, undo history and the reward ledger without trusting
saved balances or meshes. Unknown schemas/content, illegal records, truncation
and trailing data reject before publication. Limits are 16 MiB and 100,000
records per request. Unknown formats reject; version-1 journals upgrade to version 2.

The host uses WebEngine's `IndexedDBStorage`, namespace `paws-afternoon`, slot
`current`. Each write captures a coherent snapshot. Only the latest write's
transaction completion can show Saved; failure stays visible with Retry. Loads
are generation-checked, and a late load cannot replace a played afternoon.
Settings persist with the snapshot. A damaged save remains untouched.

This establishes local reloads, not phase-8 recovery: multiple-tab ownership,
backup/export/import, quota recovery and long-session journal compaction remain
later work. Do not use the earned-coins calculation as a spending ledger in phase
7; add explicit validated purchase transactions to the same owner.

## Scope and continuation

The two requested puzzles and their production-kit composition are delivered.
Phase 5 now publishes the authored courtyard and integrates canonical Pip
portraits, reaction playback and validated saves into the first-quest loop.
Shop purchases, furnishing placement and the ending
remain prompt 7. Asset provenance, source masters and final asset-quality
acceptance belong to the phase-3/4 deliveries; phase 6 does not alter their meshes.

Run `scripts/phase6-smoke.cjs` against the built game to exercise the three quests,
keyboard/mouse replay, portrait/landscape touch controls, reloads, loading
cancellation and storage completion/failure paths. See implementation status for
the latest actual run and evidence. Physical mobile compatibility and performance
require physical-device evidence.

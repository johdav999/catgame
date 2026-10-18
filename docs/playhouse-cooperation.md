# Help both kittens get inside

Version 2 of `pip_pepper_playhouse` is a cooperation puzzle. Pip's paw switch
holds the storage gate open. Pepper can enter the alcove and help slide its
crate outside. The crate follows a visible track to the paw switch, replacing
Pip. Pepper's star switch and the weighted paw switch open the main doorway.
Pip must enter before reaching the inside latch. The latch holds that doorway
open after Pepper leaves the star switch. Completion requires both cats inside.

Crate feedback checks the current sliding rules before drop proximity. Tapping
or dragging the locked crate, including beyond the track, explains the missing
gate or helper instead of suggesting an available move. Once Pepper reaches
storage, holding the crate isolates white rectangles for all reachable track
spaces. Its paw-switch rectangle sits above the raised plate so it stays visible.

The authored mouse solution has nine drags:

1. Pip to paw switch.
2. Pepper to storage mat.
3. Crate out of storage onto the first track square.
4. Pip to his waiting mat.
5. Crate along the track to the paw switch (one drag across several slots).
6. Pepper to star switch.
7. Pip to the inside entry mat.
8. Pip to inside latch tile.
9. Pepper to her inside mat.

There is no timer or score penalty. Instructions follow the committed state.
Blocked moves explain the missing gate, helper, or free space. A rejected drag
is preflighted on a copied owner and commits no partial sliding moves. Undo
reverses the existing command history, including the latch; Restart clears the
crate, cats, switches and latch. Moving the crate across several slots creates
one undo entry per slot, consistent with other crate tracks.

## Rules, scene and assets

The existing portable `Attempt` owns occupancy, gate conditions, the latched
state and completion. Its read model exposes both gates and the latch.
Presentation consumes those values, and scene time cannot commit a puzzle move.
Both gates change their silhouette on opening; the switches depress under
weight. The crate fits the paw plate's footprint. Matching paw/star pictures,
paving inlays and the crate's bordered track connect the visible mechanisms.

The playhouse is a cutaway assembled from the already accepted platform, fence,
gate, plank and symbol meshes. Low walls and a rear roof canopy keep the inside
mats and latch visible. Storage uses the same fenced kit and a separate moving
gate. These are new scene instances of existing assets; no runtime mesh,
texture, source master or asset license is changed. See the original
[prop provenance](../artifacts/model-jobs/release1-props/2026-10-11/PROVENANCE.md)
and [garden kit](garden-kit.md).

## Existing saves

The version-6 envelope remains unchanged. Existing v1 attempts load with their
original rules, state, moves and Undo history. A deliberate Restart or the
existing practice flow opts an old playhouse into v2. That restart records
`pip-pepper-playhouse-v2` as its target. The journal keeps its starting content
identity, so restore replays old moves against v1, then crosses the explicit
restart boundary into v2. Rewards earned on either layout remain once-only.
Other quests, furnishings, friendships and purchases are retained.

## Verification

18 October 2026 crate-guidance regression: configured-local Release build,
`paws_playhouse` and `paws_release1_scene` pass. The actual playhouse drag suite
passes on WebGPU and forced WebGL2, including blocked/off-track feedback,
isolated legal rectangles, cancellation, whole-track movement, Undo, reload,
completion and reward-free practice. Evidence: `build/evidence/crate-guidance/`.
The broader native GUI suite fails separately on the `pepper_garden_bells`
development numbered board with `Invalid GUI geometry`; this is outside the
playhouse's production scene acceptance.

`paws_playhouse` covers blocked shortcuts, helper access, simultaneous occupancy,
continuous gate release, crate substitution, the persistent latch, Undo,
Restart, replay after every move, and old/new save journals with once-only
rewards. `paws_release1_scene` checks imported meshes, gate projection, cat
clearance, paused Undo and reset poses as part of all ten scene traces.

`scripts/playhouse-smoke.cjs` loads an isolated chapter-three test save into the
actual generated game. Every puzzle operation is a pointer drag. It checks
blocked moves, cancellation, multi-slot dragging, paused Undo, mid-quest reload,
completion, practice, the old-save upgrade, and portrait/landscape destinations.
Run `paws_playhouse_tests --fixtures build/evidence/playhouse` to produce the
test saves, then set `PAWS_URL` to the intended generated site and run the script.
Reports, build identity and screenshots are kept under `build/evidence/playhouse`.
Desktop browser and viewport emulation evidence do not establish physical
mobile-device acceptance.

### Recorded acceptance, 15 October 2026

- Six focused native suites passed: `paws_playhouse`, `paws_release1_rules`,
  `paws_release1_progression`, `paws_quest_extensions`, `paws_release1_gui` and
  `paws_release1_scene`. The scene test includes all ten solution traces and
  vertex checks that the open door clears the crate. Results:
  `build/evidence/playhouse/native-results.txt`.
- Final pinned-engine Release Chrome run passed without page/console errors:
  `build/evidence/playhouse/browser-pinned-final/report.json`.
  Build `paws-0.1.0-909b24dd265bfd3a`; Wasm SHA-256
  `a362f109c6c9754b296e6d4b47e5d3d7fb56d7d64b6cd9aaef01a9f02bc7bac8`.
- Final configured-local-engine Release run on `http://localhost:8080/` passed
  the same journey without page/console errors:
  `build/evidence/playhouse/browser-local-verified/report.json`.
  Build `paws-0.1.0-9a44b96e2452f084`; Wasm SHA-256
  `c6d000f81a251f5afb9014d06e5ebd033919c0596499ead69c80c63f3f04a418`.
- Both browser runs include completion, mid-quest reload, old-save practice,
  once-only coins, paused Undo, cancelled/blocked drops, a nine-drag solution,
  and destination framing at 1600 × 1000, 390 × 844 and 844 × 390. Narrow
  viewports were layout checks; the full drag sequence ran at desktop size.

Visual review corrected the main door to a right hinge, preventing its open
leaf from overlapping the weighted crate. The rear canopy has visible supports.
The final pinned and served-site screenshots show the corrected assembly.

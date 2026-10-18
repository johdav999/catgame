# Sunny spot presentation

12 October 2026. Scoped polish of the existing plank quest, preserving its rules,
saved identities, undo and rewards.

The composition places Mabel on a paw mat beside the start. Shared intermediate
landings are instantiated once and narrowed to expose the crossings. The finish
uses the existing sunny perch with its steps facing the approach and a cushion
on the deck. Existing accepted meshes and materials are reused without asset
replacement or new texture sources.

Paired endpoint guides and two rows of small inlays indicate supported gaps.
Dragging a plank widens and warms matching-length guides; cancellation restores
their quiet state. Occupied crossings hide their inlays. Unsupported gaps do not
display the supported-crossing affordance. These are presentation components,
not saved game state or alternative move validation.

The HUD shows fitted bridges out of three and a short objective. Landscape phones
use a side card. Camera fitting uses the unobstructed HUD rectangle while keeping
the garden full screen. Mabel's completed route includes the approach and the
perch step; undo returns her to the start. Reduced motion places her at the finish.

## Verification

- Pinned Release browser build: `build/web-garden-details-pinned-release/site`.
- `paws_release1_scene`: all ten quest traces and scene transitions, plus guidance
  cancellation/state isolation and hiding completed crossing guides.
- `scripts/sunny-spot-presentation-smoke.cjs`: desktop mouse completion, rejected
  length/alignment drops, blur cancellation, drag-versus-click ownership, and
  unobstructed plank targets at 390 × 844 and 844 × 390 with larger text from the
  imported fixture.
- Captured and inspected desktop, portrait, landscape and drag states under
  `build/evidence/sunny-spot-presentation/`; exact build hashes in `report.json`.

Browser viewport checks are desktop emulation, not physical-device acceptance.
This is scoped quest-presentation evidence, not certification of the whole game's
release readiness.

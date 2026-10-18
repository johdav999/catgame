# New-cat welcome

When a newcomer enters, the nearest preceding resident walks to a meeting point
inside the gate. The newcomer crosses the gate and waits for its host. They face
one another and exchange the existing attentive-request animation and meow,
one after the other. The resident then leads along the shared collision-checked
route, about 1.2 metres ahead. Each finishes at its own original starting spot.

Arrival reserves the walking lane and clears casual wandering. Other newcomers
wait their turn. The first cat, or a scene without a suitable resident, retains
the existing solo arrival. Presentation does not introduce a cat or award
anything itself: `arrivalComplete` is published only when both cats are home,
and the browser owner performs the normal introduction/save operation.

Pause preserves the pair and greeting clock. Reduced motion places both at home
without playing the sequence. Replacing the scene releases the sequence; saved
introductions do not replay on reload. Existing direct quest interaction remains
available during arrival.

Each member of the pair independently completes its travel and facing step.
After reaching its endpoint it enters idle once and keeps that animation running
while its partner catches up. The same rule applies to the shared trip home.
Previously, the waiting cat was resampled as walking and cross-faded to idle on
every update, repeatedly resetting its animation clock and producing a stuck,
flickering pose. The welcome coordinator now retains per-cat completion flags
and resets them only at a stage boundary or the start of another welcome.

Verification:

- `paws_release1_garden`: consecutive arrivals, pause projection, gate crossing,
  reduced motion and presentation/progression separation.
- `paws_release1_final_garden`: production-model outward walk, full greeting,
  shared return without pair overlap, and both home endpoints; existing ambient
  and furnishing checks remain enabled. Real animation-player updates also check
  that early arrivals' idle clocks advance while waiting, for each newcomer.
- `scripts/cat-welcome-smoke.cjs`: actual WebGPU/WebGL2 welcome captures, greeting
  pause, paired return, saved introduction and reload. Evidence is under
  `build/evidence/cat-welcome/`.

The 2026-10-18 repair was verified with both native garden suites and the generated
local Release game. Updated browser evidence is under
`build/evidence/cat-welcome-fix/`. An isolated test binary that restores the old
per-frame walk/idle behavior fails the waiting-animation regression; the fixed
implementation passes it.

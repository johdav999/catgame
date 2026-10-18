# Between-quest garden activity

Introduced cats occasionally take short walks through the unlocked garden, pause
to look around, lie down for 20–45 seconds, wake up and eventually walk home.
Each cat has its own scene-local random schedule. Initial delays are staggered;
home pauses last 12–30 seconds. After at most three walks, the next walking
destination is home; earlier home returns can also be selected.

The existing collision-aware route planner starts ambient paths at the cat's
current position. Candidate destinations remain in unlocked garden bounds,
avoid furnishings and other cats, and reserve room for the resting silhouette.
Only one cat travels at a time, but other cats may pause or rest independently.
Walking retains the verified 0.45 m/s speed and distance-appropriate footsteps.
Settling, resting and waking reuse the imported animation clips.

This is garden presentation only. It does not change saves, quests, rewards or
friendship. Scene replacement discards schedules. Pause freezes the simulation;
reduced motion and the picnic finale disable wandering. Explicit attention,
furnishing use and group gathering clear ambient activity using the existing
home-reset policy. New ambient trips wait until furnishing users return home.

Verification: `paws_release1_final_garden` covers walks, prolonged rests, actual
home returns, collision clearance, save invariance, pause, reduced motion,
attention and all furnishing flows. `paws_release1_garden` retains its synthetic
contract checks. `scripts/garden-wandering-smoke.cjs` observes the built game on
WebGPU and WebGL2, captures the walk/rest/return phases, checks pause and verifies
that a furnishing interaction takes priority. Evidence is written under
`build/evidence/garden-wandering/`.

Residents also occasionally visit one another, exchange the existing greeting
poses and request meows, then follow a shared garden loop and return to their
own home spots. The first attempt is after 30 seconds; completed social strolls
have a random 45–85 second cooldown. Busy cats and furnishing reservations delay
these attempts. Routes avoid other cats and maintain a larger lead gap around
corners. The pair occupies the walking lane together while other cats wait.
New arrivals and player interactions take priority over social activity.

Resident greetings reuse the newcomer presentation controller without marking
an arrival complete or changing progression. Native garden tests cover the full
loop, spacing, repeat visits and cancellation. `scripts/cat-social-smoke.cjs`
checks the built game on both renderers, including pause, home return and reload;
its reports and screenshots are under `build/evidence/cat-social/`.

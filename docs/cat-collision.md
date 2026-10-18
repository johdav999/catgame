# Cat collision and furnishing travel

Cat travel uses WebEngine's Jolt `PhysicsWorld::sweep` and `overlap` queries
through the game-owned `CatCollision` adapter. A clear route and a clear endpoint
are required; a large frame step consumes each route segment rather than taking
a shortcut across its corners. Blocking contacts stop travel and cannot advance
a cat to its use or completion pose.

Solid props have invisible, scene-root static body proxies on collision layer 14.
Their generation-checked source component handles follow live positions,
rotations, scales and hierarchy visibility. Most shapes use the imported mesh
triangles so bench gaps, playhouse doorways and curved hammock cloth remain open.
Balls use a sphere centred on the visual asset's centre, including its scale.
Collapsed import triangles are removed before collision cooking. Placement
previews, water, clue marks, paving and foliage do not create solids. Proxies
have no visual mesh or interaction component and cannot intercept scene picking.

The Tidy toys basket additionally reserves paw clearance. Its selected solid
uses a convex box around the transformed basket footprint, padded by 0.5 m in
the horizontal plane. Together with the 0.22 m body capsule, this keeps the
animated lower legs outside the low rim while approaching and turning. This
opt-in `CatCollision::build` target applies only to that quest's basket; ordinary
props and supported furnishing entries keep their existing geometry policy.

The completion endpoint is measured from the live basket's outer side, with a
0.75 m root stand-off. Previously the fixed endpoint cleared the body capsule
but allowed the forward paws to enter the basket. Both content versions now
keep the low skinned silhouette at least 7.5 cm outside the rim throughout
`quest_basket_finish`. The same endpoint is used for reduced motion and completed
save restoration. The basket actor is held by a scene-scoped handle; reward
coins/hearts follow the corrected cat position without changing progression.

Walking uses an upright body capsule with radius 0.22 m, cylinder half-height
0.06 m and centre 0.305 m above the root. This approximates the cat's body;
ears, tail and deliberate paw/muzzle interaction contacts are not individually
simulated. Contact tolerances are 3 mm, with a 5 mm endpoint skin for clear
supporting tangents.

The courtyard retains its authored lane bounds and cat seat reservations and
also checks actual solids. Obstructed travel legs use bounded 20 cm navigation
samples and swept, simplified detours. Ordinary walking never climbs an
arbitrary obstacle. Measured furnishing entry routes and elevated quest routes
may add a checked rise before crossing a step, followed by a checked landing.
Intermediate step markers may rise to fit the body volume; final measured
furnishing roots stay unchanged. All solids remain enabled during use, including
the target furnishing and its walls. Leaving reverses the checked entry route.
Use starts on physical arrival, rather than solely from elapsed walking time.

Puzzle props remain static/scripted. Their discrete positions, rolling routes,
switch states, move validation, rewards, undo and save journals still belong to
the existing portable rules. Cats do not push balls or cushions out of logical
slots. Legacy quest endings plan after prop, ball and gate motion completes.
Reduced motion and restoration explicitly project validated supported poses;
ordinary walking never teleports. Scene replacement discards its proxies and
runtime paths along with the scene. Cat-to-cat clearance remains the existing
courtyard lane/seat reservation policy.

The toy-basket scene places Pip on the left, at the existing logical access
cell, facing the cushion. Reused low stone borders enclose the toy area and
form one entrance into the cushion's sliding lane. The central cushion blocks
that entrance physically; north and south parking expose the original pickup
approaches. The borders leave the full slide track, toys and basket recesses
clear. Both saved content versions retain their existing rules and solutions.

Validation includes `paws_cat_collision` (actual cushion and ball assets, large
steps, detours, entry/leave, moved/disabled/destroyed solids and visual previews),
the portable scene suites, actual-asset `paws_release1_final_garden`, and the
generated game's mouse quest and furnishing browser flows. Browser evidence
belongs under `build/evidence/cat-collision/` and remains desktop-scoped.

`paws_basket_contact` loads the actual basket and both Pip assets, verifies that
the unpadded mesh already stops a large walking step, and checks skinned vertices
through the full reaction, actual approach/turns, undo and reduced motion for
both basket content versions. The extended `scripts/tidy-side-access-smoke.cjs`
also records the corrected settled/coin contact in the actual generated game,
alongside the prior toy-placement and save regressions.

Basket repair verified on 15 October 2026: `paws_basket_contact`,
`paws_cat_collision` and `paws_phase6_scene` passed; local and pinned Release
browser builds succeeded. WebGPU and WebGL 2 each passed both basket ending
orders, with coin/reaction screenshots, matching root x=0.893807 m, and no
console/page errors. Reports: `build/evidence/basket-contact/{webgpu,webgl2}`.
The tested local build is `paws-0.1.0-ae1bfc18126e4b73`, Wasm SHA-256
`324093ae8270046045f77254dbfd3d751b55b7be153a28e4aa2518b6ba897164`.
The user's current Chapter 2 save and 60 coins survived the game refresh.
No engine sources, gitlink, model assets or reward rules changed.

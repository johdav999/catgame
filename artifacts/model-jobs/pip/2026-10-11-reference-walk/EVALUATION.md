# Walk evaluation

| Check | Result / evidence |
| --- | --- |
| Reference | Inspected photographic walking poses; authored reconstruction in `references/walk-spec.json`. Not motion capture. |
| Resources | Original rig, mesh, maps, weights and eight other actions preserved by checked GLB merge. 17,854 triangles, two materials, 29 deform bones / 31 engine joints. |
| Timing | 1.2 seconds, 30 fps, 37 authored samples with matching endpoints; 0.36 m stride and 0.30 m/s travel. |
| Planted fore/aft drift | Maximum 0.536 mm over full stance, including wrap, measured on engine-imported skin. |
| Floor contact | Maximum sole-height error 1.012 mm around the 3 mm contact plane; no sampled penetration below -2 mm. |
| Swing clearance | Peak sole height 63.99–64.75 mm across four paws. |
| Lateral drift | Less than 3 mm during planted weight transfer; separately asserted. |
| Source appearance | Twelve phases inspected from five views; r1 PNGs retained. |
| Clean export | All 36 frames rendered in five views; nine actions, 29 bones, normalized weights, two materials. `exports/clean-reimport.json`. |
| WebGPU | All 36 samples captured in five views; moving lane, suspension and reduced motion pass without recorded browser errors. `browser-evidence/walk-report.json`. |
| Garden | Actual generated game earns rewards, places furnishings and traverses the garden with the new walk. See `game-evidence-final/`. |

Contact measurements cover straight constant-speed movement. Existing garden
waypoint turns and entry/exit crossfades do not have dynamic world-space paw IK;
these measurements do not assert zero slip through turns. The simplified canonical
leg chain remains unchanged. No full-body motion-capture or new anatomical rig claim.

The initial native run passed the original asset, starter-scene and garden checks.
Another task subsequently added a stricter settle/rest joint test, which unchanged
r8 settle/rest fails. The dedicated walk target passes; that is not a passed full
asset suite. Physical-device acceptance and other animation revisions are outside
this walk evidence.

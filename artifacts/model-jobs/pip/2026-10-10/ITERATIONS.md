# Inspected correction history

All images below are actual Blender renders, not generated concepts. The base
three-quarter, side and face cameras stayed fixed. Rejected source renders are
retained outside runtime assets.

| Pass | Inspected defect | Correction and observed result |
| --- | --- | --- |
| 1 | `r1-three-quarter`, `r1-face`, `r1-side`: protruding eye disks, polygon-shaped colour variation, weak toe contours | Reduced eye depth/size, softened cheeks, added joined toe forms, reduced sampled high-frequency colour variation. `r2-*` has a calmer coat and less protruding eyes. |
| 2 | `r2-*`: stripe boundaries still inherit vertex interpolation | Unique UV atlas evaluates geometry-aware colour at texels. `r3-three-quarter` has smooth stripe edges, but exposed a colour-space darkening problem. |
| 3 | `r3-rejected-curl`, `r3-rejected-walk`: hard shoulder weight boundaries and a tail-tip spike; coat too dark | Continuous segment-distance weights and explicit sRGB atlas encoding. `r4-three-quarter` restores honey ginger; walking shoulders and the complete tail move together. |
| 4 | `r4-crouch`: rest pose reads as a crouch; 29,560 triangles exceeded allocation | Rebudgeted coat by triangle count rather than polygon count and reduced tiny facial geometry. 14,660 triangles retained silhouette. `r5-rejected-curl` exposed ear/head blending at stronger turns. |
| 5 | `r5-rejected-curl`: face/ear roots receive incompatible head weights; tail hangs awkwardly | Rigid facial head envelope with smooth ear-tip weights; authored tail wraps around the haunch using explicit bone-space poses. `r6-*` and pose renders remove the spikes. |
| 6 | Pose-bound measurement found roughly 10 cm of folded-leg floor penetration, concealed by the review floor | Baked a per-frame pelvis contact correction from the actual skin; root remains fixed. Revision 7 sampled bounds stay at least 3 mm above the contact plane. Source, clean-reimport and runtime snapshots verify the corrected rest pose. |
| 7 | Runtime close view too small; black backdrop distracted from silhouette. Portrait upright tail clipped beside face | 35-degree lens, sage clear colour, bounded asset-scale zoom. Separate same-master 512px portraits lower the tail behind the shoulders. Final browser snapshots and portrait renders inspected. |
| 8 | `r7-rejected-paw-contact`: the contact correction lifted the front paws while preserving the lowest skin point | Reduced knee folding so broad paws support the rested weight. `r8-contact` verifies the grounded paw shapes. |
| 9 | Grounded rest still held its head too high | Lowered neck and nodded head into a sleepy tucked pose; tail remains curled around the haunch. Final source, clean reimport and live renderer inspected again. |

Clean-process import: `renders/reimport-*.png`, `exports/clean-reimport.json`.
Pinned renderer: `build/evidence/pip/` at repository root. The live browser is
flatter and more muted than Cycles; it preserves cream landmarks, stripes, eye
shapes and silhouettes. No equivalence to the concept board's strand fur is claimed.

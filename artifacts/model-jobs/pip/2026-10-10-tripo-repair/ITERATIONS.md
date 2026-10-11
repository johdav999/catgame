# Inspected repair iterations

1. Source inspection: complete standing appearance preserved; incomplete 33-bone source skin confirmed independently in Blender and WebEngine. The source faces diagonally and contains no actions. Retained source-front/side/three-quarter renders.
2. Revision 1: full canonical weights and 1024 maps. Inspected walk, drink and resting renders. Native GLB root/clip/contact checks passed, but face alignment and resting tail appearance needed correction.
3. Revision 2: rotate the source 72 degrees rather than 90; reinforce rigid head influence and reduce ear spill. Inspected source-side and revised drink/rest renders; face alignment improved.
4. Revision 3: attempted centreline fitting of tail geometry. Inspected idle/rest showed pinching and visible folds. Rejected this geometry operation; later revisions preserve the original tail silhouette.
5. Revision 4: continuous resting-tail pose and opaque blink caps. Inspected idle/rest/portrait: open eyelids left a visible line across the iris; closed caps did not cover the whole eye.
6. Revision 5: orient caps to the source eye plane, hide them beneath the surface when open, enlarge closed coverage and add a simple closed-eye crease. Inspected idle/rest showed corrected eyelid visibility. A native sample caught a 5.9 mm settle penetration after hind-contact adaptation.
7. Revision 6: final pelvis contact correction after hind-chain adaptation. Native GLB and FBX root/loop/transition/ground/lifecycle tests passed. Exact GLB canonical bind comparison and every-vertex root-follow test passed. Chrome WebGPU preview passed all nine controls and animated pixel-change checks. Inspected runtime idle and rest screenshots.
8. Revision 7 packaging: preserve all actions with fake users in the packed master; FBX_SCALE_ALL removes the 100x wrapper/local-centimetre mismatch. Normalize FBX stack names. Both formats now pass the exact canonical local-bind comparison (maximum FBX matrix difference 5.96e-8), root-follow checks and full animation tests. Clean-process reopen/reimport verifies packed textures and nine retained actions.

9. Revision 8 timing: set the repair scene explicitly to 30 fps and normalize GLB animation input times to zero, removing Blender's one-frame start offset. Both formats pass explicit canonical-duration assertions as well as the existing pose/ground/loop checks. The packed master and final exports retain all nine actions.

Rejected variants remain explicitly numbered. The accepted delivery is r8. No uninspected render is counted as acceptance.


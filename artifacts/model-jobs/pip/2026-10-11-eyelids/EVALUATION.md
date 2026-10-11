# Evaluation

Accepted scope: Pip skeletal eyelids in the existing desktop WebGPU preview,
exported GLB/FBX and matching portrait renders.

- Open, closing, closed, reopening and open-again captured in
  `browser-evidence/`; closed pose also inspected from both side angles.
- Clean-process Blender GLB/FBX reimports: one material, 20,298 triangles,
  31 deform bones, nine clips, normalized weights and unit lid scales every frame.
- All original 29 bind matrices, names and parents retained; see
  `exports/clean-reimport.json` and `rig-glb.txt` / `rig-fbx.txt`.
- Default engine imports pass for both formats, no raised texture budget.
  `validation-glb.txt` / `validation-fbx.txt` cover clips, root, loops, skin,
  static poses and lifecycle. Every imported vertex follows root motion.
- Existing walk and settle regressions pass: maximum walk stance drift 0.536 mm,
  floor error 1.012 mm, approximately 64 mm paw clearance; settle/rest remain
  above ground and retain their matching transition seam.
- `exports/pip-merged.merge.json` records original runtime hash and all 729
  preserved non-facial channels. Current walk/settle revisions remain in manifest.
- Three portraits rendered from the same packed source with current actions.
- Installed-runtime CTest: 4/4 pass (`native-ctest.log`). Final served-site browser
  smoke passes all nine controls, reduced motion, suspension, rapid selection,
  layouts and HTTP failure/retry, with no console errors. Evidence:
  `browser-evidence/pip-smoke.json`; final blink review uses the served GLB without
  request substitution (`PIP_LIVE=1`). Runtime and served GLB SHA-256 match:
  `f715997ab8f37e10aa09994085aad1099c2f34a2171c48219d836ed357f6c828`.

The blink is an authored idle event; randomized behavioral scheduling was not
added. Full character redesign, a tight sleeping curl, physical mobile performance
and arbitrary extreme facial poses are outside this focused acceptance scope.

# Beach plant contact — 2026-10-21

Perimeter plants previously sampled sand only at their origin and used a fixed
burial depth. Downhill stem bases could remain above the dune. Pepper's composed
trail plants also used puzzle Y=0, while the sand is at Y=-0.02 m.

`beachPlantHeight` samples the sand under transformed vertices in the bottom 5%
of each imported plant. It seats that stem region at least 1 cm into the sand.
Perimeter instances retain any deeper authored burial, and Pepper's components
convert the resulting world height back to their parent space. Meshes, materials,
horizontal placements, puzzle contacts and saved state are unchanged.

Validation:

- The new native contact check failed before the fix on a tall dune plant.
- Starter scene, beach environment and release-one scene suites pass. Contact
  checks cover all six beach layouts and both versions of Pepper's trail through
  their clue/route sequences. Log: `build/evidence/plant-grounding/native.log`.
- Chrome WebGPU and forced WebGL2 rendered the generated game at 1600x1000 and
  390x844, with inspected low-angle views and no page/console errors. Reports and
  screenshots: `build/evidence/plant-grounding/webgpu/` and `webgl2/`.
- Both browser runs fetched Wasm SHA256
  `c345d9a3a50212c32f4e3711e9bd20bbbcaea92936ad1fca8928124314dcc408`.

The isolated preview is `build/web-plants-release/site`, using the existing
`build/engine-water-depth-transition` engine checkout. No engine source or gitlink
was changed. Concurrent Pepper label/camera work is present in the preview;
before/after screenshots are visual evidence, not an isolated pixel comparison.
Browser evidence is desktop Chrome, not physical mobile validation.

# Pepper's shelter and driftwood branch

The beach trail uses two new Tripo exports: an open blue-and-cream striped
canvas shelter and an asymmetric forked driftwood landmark. The branch's low
and high arms remain visibly different. Whole-model rotation supplies the
left/right destination variants, while the existing blue cloth marks the
relevant arm. A smaller copy supplies the branch evidence clue. Striped shelter
choices use the new four-post model; the plain shelter decoy retains its
existing plank assembly.

The models are game-owned assets under `assets/runtime/models/beach/`, with
stable IDs `beach_shelter` and `beach_branch`. The existing importer and asset
preparation path load both. The role selection is confined to
`src/release1/integration/pepper_trail.inc`; collision uses the existing
triangle-mesh queries. No rule, trail answer, content version or save schema
changes are needed.

| Asset | Dimensions X/Y/Z (metres) | Triangles | Materials | Embedded color/normal |
| --- | --- | --- | --- | --- |
| Shelter | 1.4996 / 1.2006 / 0.9001 | 8499 | 1 | 2048 square |
| Branch | 1.0500 / 1.0496 / 0.3007 | 4000 | 1 | 1024 square |

Runtime uses Y up, +Z front and a floor-contact pivot. The shelter centre is
geometrically open at cat height. All four foot minima are within 0.7 mm of the
ground. Cloth anchor placement uses measured fork and support surfaces; the
contact records are in the model job.

## Source and Blender evidence

[Job README](../artifacts/model-jobs/beach-trail-20261020/README.md),
[evaluation](../artifacts/model-jobs/beach-trail-20261020/EVALUATION.md),
[iterations](../artifacts/model-jobs/beach-trail-20261020/ITERATIONS.md),
[provenance](../artifacts/model-jobs/beach-trail-20261020/PROVENANCE.md) and
[manifest](../artifacts/model-jobs/beach-trail-20261020/asset-manifest.json)
retain references, exact prompts, paid task IDs, source/export hashes, review
scripts and measured contracts.

Packed editable masters:
[shelter](../artifacts/model-jobs/beach-trail-20261020/shelter-master.blend),
[branch](../artifacts/model-jobs/beach-trail-20261020/branch-master.blend).
The source masters preserve the original high-resolution meshes and 4096 maps.
Reviewed GLBs:
[shelter](../assets/runtime/models/beach/shelter-v1.glb),
[branch](../assets/runtime/models/beach/branch-v1.glb).

Three review stages covered normalized source orientation, simplified matte
materials, and clean GLB reimport. Reopened masters passed passage/foot/support
checks. Tripo's original generated color and normal maps remain the appearance
source; no external texture library was used.

## Verified runtime

The final [acceptance summary](../build/evidence/beach-trail/summary.json)
records exact build and fetched-GLB hashes. Both renderers used the same
Release site, build `paws-0.1.0-b5eef512709bb6b5`, from
`build/web-water-depth-release/site`. This preview explicitly uses the existing
local water/shadow engine revision `077df1e72364229295dc746768b806531ab37eb4`;
it does not advance the repository's engine pin.

- Native: portable starter and release-one scene tests passed, 2/2, including
  physical arrival, both Pepper content versions and lifecycle regression.
- WebGPU: six-stop v2 trail, full animation, completed with no browser errors.
- WebGL2: four-stop v1 and six-stop v2, larger text/reduced motion, completed
  with no browser errors.
- Both browser paths exercised pointer/keyboard controls, wrong destinations,
  Undo, resized portrait/landscape framing, mid-trail reload and completed-save
  revisit. HTTP response hashes match the reviewed shelter and branch GLBs.

Inspected final captures:
[WebGPU branch](../build/evidence/beach-trail/webgpu/v2-stop-3-ready.png),
[WebGPU shelter](../build/evidence/beach-trail/webgpu/v2-stop-5-ready.png),
[WebGL2 branch](../build/evidence/beach-trail/webgl2/v2-stop-3-ready.png),
[WebGL2 shelter](../build/evidence/beach-trail/webgl2/v2-stop-5-ready.png).
The puzzle apron remains clear and the supports/forks and cloth clues are
visible. These checks establish desktop-browser rendering and interaction,
not physical mobile performance or full-motion WebGL2 acceptance.

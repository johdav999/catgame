# Beach coastal asset pass — 2026-10-19

The coastal sand and four user-supplied Tripo models are integrated: closed
woven picnic baskets, rounded coastal rocks, dune grass and a rope-sided timber
boardwalk. Original downloads are preserved in
`artifacts/model-jobs/beach-coast-20261018/manual/`. Reviewed runtime exports are
in `assets/runtime/models/beach/`. No new Tripo generation requests were needed.

Scenery follows the terrain around every quest's perimeter and has no interaction,
physics or route-support providers. Native tests check transformed model bounds
against all six playable aprons. Terrain flatness, puzzle rules and save identities
are retained. The basket replaces the tall greybox cube with a floor-contact model.
The boardwalk has a small level sand bed outside the play area, padded through
complete heightfield source cells so terrain interpolation cannot swallow a post.

## Extended pier and perimeter planting — 2026-10-19

The boardwalk now comprises four connected 2.4-metre modules: the original
shore approach and three additional sections extending into the sea. All four
share one deck elevation and yaw; the sea sections do not follow the descending
seabed. The original level approach bed remains confined to the shore.

Staggered outer dune clusters and a low landward border add 14 rocks and 34
grass clumps. Each beach scene now uses 26 rocks, 55 grass clumps and four
boardwalk modules, sharing the existing reviewed GLBs and textures. No new
model generation, downloads or Blender-source changes were required.
The complete transformed X/Z bounds of every instance are checked against all
six protected puzzle aprons. Scenery retains no input or collision providers.
Camera framing continues to prioritize the puzzle; orbit and zoom reveal the
longer pier and outer planting.

Validation: all three native scene/terrain suites passed (8,064 integration
checks). Full six-quest Chrome runs passed on WebGPU and forced WebGL2 with no
page/console errors, including real gestures, Undo, restart, reload, garden
travel and the saved ending. Reports: `build/evidence/beach-dressing/webgpu/`
and `webgl2-final/`; native log: `build/evidence/beach-dressing/native.log`.
Both runs fetched Wasm SHA256
`bc520526b76aad93e7f1b8738888c1b32f1b40e40fee16b28f8a6e935bfaed1b`.
Desktop/portrait and orbit views were inspected; the WebGL2 report folder
contains `1-coast-overview.png` and `1-landward-overview.png`. The earlier
`webgl2/` attempt stopped at an unavailable Reset view menu action in the
optional screenshot helper; replacing that helper step with a saved reload
allowed the final run to pass. No runtime fix was needed for that harness error.

| Model | Runtime X/Y/Z metres | Triangles | GLB bytes | Base/normal maps |
| --- | --- | ---: | ---: | --- |
| Basket | 0.78 / 0.48 / 0.78 | 9,610 | 7,581,904 | 2048 square |
| Rock | 1.397 / 0.849 / 1.049 | 3,499 | 2,428,632 | 1024 square |
| Grass | 0.850 / 0.648 / 0.749 | 6,499 | 1,890,096 | 1024 square |
| Boardwalk | 1.40 / 0.88 / 2.40 | 7,976 | 2,594,624 | 1024 square |

Total: 27,584 triangles across four shared meshes; 14,495,256 downloaded bytes,
compared with 37,702,368 original bytes. This is a budget/byte comparison, not a
measured frame-time improvement. Two uncompressed RGBA8 maps per asset with full
mip chains would occupy approximately 74.7 MiB; actual allocations depend on
renderer formats and resource policy.

Blender 3.5.1 review covered front/rear/elevated views, matte material corrections,
restrained normals, geometry simplification, floor-pivot correction, packed-master
reopening in a fresh process and clean GLB reimport. Original 4096-square maps
remain in source GLBs and full-resolution source masters. Supplied Tripo maps are
reused for this focused revision. Submission parameters and commercial entitlement
were not supplied. See runtime `NOTICE.md` and `manifest.json` for provenance.

Editable packed masters, retained scripts, renders, measurements and evaluations:
`artifacts/model-jobs/beach-coast-20261018/review/README.md`.

The lower basket initially exposed the legacy raised channel through its lid.
Presentation now hides a channel only while a basket occupies its cell and
reveals it as the basket slides away. Rules still own action validity. Native
tests check both bounds and actual lid triangles; browser flows check slides,
channel reveal/rotation, Undo and saved-board restoration.

## Material

`assets/runtime/textures/beach/sand-v1.png` is original ImageGen albedo, 1254 by
1254 pixels, 3,645,831 bytes. The existing terrain layers share it at a three-metre
repeat, with separate dry, damp and sage tints and authored roughness. Source
prompt, channel semantics and checksum are in the adjacent `NOTICE.md`.
Runtime preparation loads the texture before scene publication. The accepted
asset manifest includes and verifies its bytes. Playable-apron heights, puzzle contacts,
camera framing and water settings are unchanged by this pass.

## Model references and earlier provider blocker (resolved by manual exports)

Four inspected isolated ImageGen references are retained in
`artifacts/model-input-images/beach-coast-20261018/`, with `prompts.json` and
`manifest.json` recording inputs, dimensions and SHA256 hashes:

- `basket.png`: low closed square wicker basket; continuous lid for centre grabs.
- `rock.png`: rounded warm sandstone boulder for varied perimeter instances.
- `grass.png`: opaque, broad-bladed dune grass clump.
- `boardwalk.png`: short timber deck with four posts and two rope sides.

All derive from `docs/design/beach-adventure-reference-20261018.png`.

The installed Tripo helper uploaded the basket and rock references, but both
image-to-model creation requests returned HTTP 403. The helper suppressed the
response bodies, so the exact provider error code is unknown. No task IDs were
returned. Local job journals are under
`artifacts/model-jobs/beach-coast-20261018/{basket,rock}/job.json`.
Do not blindly resume or resubmit these unknown submissions.

A subsequent read-only account check reported 45 available credits and zero
frozen credits. Usage history returned no entries since 2026-10-18 20:20 UTC,
covering these attempts. These observations suggest a credit/access issue but
do not establish the precise 403 reason. Grass and boardwalk were not submitted.
The user was asked to check/top up the API account; no purchase was made.

On 2026-10-19 the user supplied all four manually generated Tripo GLBs. Review
continued from those files without retrying any unknown provider task. The
automated submission journals remain separate from the manual model provenance.

## Earlier material-only validation (2026-10-18)

Pinned Release browser build succeeded. Native `paws_starter_scene`,
`paws_beach_environment` and `paws_release1_scene` all passed (7,040 integration
scene checks; terrain flatness exercised against all six beach definitions).
First-quest acceptance passed on actual desktop Chrome WebGPU and forced WebGL2,
with zero console/page errors, on the same fetched Wasm SHA256:
`3dfd82c4a737477a95fa1e6c88f61d65d64049ec6ce06bf12cfbd90f6540583a`.

Reports are `build/evidence/beach-pass2/material-webgpu/report.json` and
`material-webgl2/report.json`. Both verify four repeating 1254-square layer
textures and the exact sand PNG hash fetched over HTTP, animated/paused water,
cancelled drag, Undo, save/reload, garden return, portrait interaction and solved
revisit. Desktop and portrait captures were visually inspected: grain stays
subtle with no broad tiling seams; WebGL2 is visibly grainier than WebGPU.
The play area remains flat and free of decoration. These are material-only
checks, not acceptance of the blocked model work or physical mobile performance.

## Model-pass validation (2026-10-19)

Pinned Release build succeeded against engine
`0426d9319de715584a2e34ff9862cd6ba873ecd0`. Native `paws_starter_scene`,
`paws_beach_environment` and `paws_release1_scene` all passed, including 7,452
integration scene checks, all-six-quest scenery clearance, terrain flatness and
basket lid triangle picking. Logs: `build/beach-models-native-final.log`.

Final first-quest and full-story browser evidence lives under
`build/evidence/beach-models/`. First-quest reports record requested actual backend,
four HTTP-fetched GLB hashes/byte counts, actual scene mesh instance counts,
PNG hash, water motion/pause, cancelled drag, Undo, save/reload, garden travel,
portrait interaction and completed revisit. Story runs exercise all six quests,
restart, progression persistence and the four-cat ending.

After the boardwalk-bed correction, complete six-quest runs passed on actual
WebGPU and forced WebGL2 with zero page/console errors, including Undo, restart,
saved-board reload, garden travel and saved/replayed four-cat ending. Final runs:
`contact-story-webgpu/report.json` and `contact-story-webgl2/report.json` under
the evidence folder. Both report the exact final Wasm SHA256:
`094cd63309c307688385f7563f613b7a7666f7ad2b9473b3bb73a325106fd45b`.

The final WebGL2 first-quest/access-boundary probe also passed
(`contact-final-webgl2/report.json`). Earlier first-quest probes passed on both
renderers before the small bed correction. Two final repeated-reload WebGPU
probes hit intermittent Chrome `ERR_NO_BUFFER_SPACE` while loading assets;
their failures are retained in `contact-final-webgpu/` and
`contact-retry-webgpu/`. The latter had already verified all four fetched GLB
hashes and active instance counts before a later garden reload failed. It is
asset-byte evidence, not an overall passing probe. The subsequent fresh final
WebGPU six-quest run passed, including per-quest reloads. The intermittent
loading stress failure remains a scoped limitation, not a model import error.

Desktop and portrait captures, the wider shade-quest deck contact view, and the
final ending were inspected on both renderers. The first quest uses two basket
instances, 12 rocks, 21 grass clumps and one boardwalk, sharing four meshes.
`acceptance-summary.json` links final reports, model hashes, source preservation
and native evidence. `asset-audit-final.json` verifies staged runtime bytes.
These checks do not establish physical mobile performance or compatibility.

# Coastal Tripo models

Basket, rock, dune grass and boardwalk: manually generated in Tripo3D AI by
the user and supplied to this Paws workspace on 2026-10-19. Original unmodified
GLB files are retained in `artifacts/model-jobs/beach-coast-20261018/manual/`.
The manifest records their original byte counts and SHA256 hashes. Task IDs,
exact generation settings and account entitlement were not supplied; commercial
entitlement has not been independently verified.

Visual references: the four original OpenAI ImageGen images and prompt records
in `artifacts/model-input-images/beach-coast-20261018/`, based on
`docs/design/beach-adventure-reference-20261018.png`. Their appearance was
compared with the supplied models. Actual manual Tripo submission parameters
are unknown. No new provider task was submitted during review.

The supplied Tripo base-color and tangent-normal textures were retained as the
appearance sources. They are user-supplied generated maps, not measured scans.
This focused normalization reuses those maps rather than regenerating textures.
Original 4096-square maps remain in the supplied GLBs and normalized source
Blender files. Runtime base-color/normal maps are derived at 2048 for the basket
and 1024 for scenery, without upscaling. Roughness is separately authored as
0.78 for wicker/wood and 0.85 for rock/grass; metallic is zero. Original RM maps
remain in source masters but are not used in the runtime material. Normal maps
retain the glTF/OpenGL tangent convention with restrained strength.

Models use metre units and floor-contact pivots centered in XZ, with Y up in
runtime. Dimensions, actual triangle/vertex counts, material maps, immutable
runtime hashes and source-master links are in `manifest.json`. Review scripts,
packed Blender masters, inspected source/export views and evaluation notes are
under `artifacts/model-jobs/beach-coast-20261018/review/`.

## Driftwood ball channels

Straight and curved channels were generated on 2026-10-20 with the Tripo skill
from original OpenAI ImageGen references, at the user's explicit request.
References and prompts: `artifacts/model-input-images/beach-channels-20261020/`.
Original models, provider task IDs/settings, hashes and provenance sidecars:
`artifacts/model-jobs/beach-channels-20261020/{straight,curve}/`.
Tripo account commercial entitlement remains unverified.

The original generated 4096-pixel base-color and normal maps supply the wood
appearance. Runtime derivatives are 1024 pixels; the packed source masters retain
the originals. Blender fits the meshes to the existing game ports, squares the
ends, welds split vertices, triangulates cut faces and simplifies to approximately
4000 triangles per piece. The small newly cut end faces use a plain pale wood
material to avoid stretched source UVs. Wood roughness is 0.78, metallic zero.
Packed editable masters, clean reimport checks and assembly renders are retained
in the same job directory. See `manifest.json` for actual dimensions and hashes.

## Pepper's shelter and forked driftwood

The striped shelter and forked branch were generated on 2026-10-20 through the
direct Tripo API from original OpenAI ImageGen references at the user's request.
References/prompts: `artifacts/model-input-images/beach-trail-20261020/`.
Source GLBs, provider task IDs/settings and hashed provenance:
`artifacts/model-jobs/beach-trail-20261020/{shelter,branch}/`.
Tripo account commercial entitlement has not been independently verified.

The inspected generated base-color/normal maps are retained as the appearance
source, with 2048-pixel shelter and 1024-pixel branch runtime derivatives.
Packed source masters retain the original 4096-pixel maps. Blender establishes
floor pivots and facing, simplifies geometry, and authors nonmetallic roughness
0.82 and restrained normal strength 0.4. No external photographs or downloaded
third-party texture libraries were used. Runtime manifests retain source,
master, reference and delivered-file hashes. Review scripts, packed editable
masters and source/export renders remain in the job directory.


## Remaining beach puzzle and coastal kit

Generated 2026-10-20 through the direct Tripo API from 15 original OpenAI
ImageGen references at the user's explicit request. References live in
`artifacts/model-input-images/beach-complete-20261020/`; the full prompts,
source GLBs, provider task IDs/settings, hashes and provenance sidecars live in
`artifacts/model-jobs/beach-complete-20261020/`. Tripo account commercial
entitlement has not been independently verified.

The 19 runtime derivatives comprise two driftwood plank lengths, a landing
support, cloth shade panel and post, rope post, three shell shapes, a chime
frame, four picnic mat colours, two rock forms and three grass variants.
Plank lengths and mat colours share their respective generated source models;
loose shells are also used in the hanging chime assembly.

The accepted Tripo base-color and normal maps supply the appearance. Packed
source masters retain the original maps; runtime derivatives use 512 or 1024
pixels. Roughness is authored at 0.82, metallic at zero and normal strength at
0.4. Blender normalizes metre dimensions, cleans and simplifies geometry,
levels walkable centres, and establishes contact pivots. Four mat colours use
explicit glTF baseColorFactor values because Blender 3.5 omitted the linked
Multiply node at export. Editable master node graphs retain those same colours.
No external photographic or third-party texture library content was used.

Source, optimized and clean-import renders, packed editable masters, contact
measurements and export scripts are retained in the job directory. The coastal
manifest records immutable runtime, source, reference and master hashes.

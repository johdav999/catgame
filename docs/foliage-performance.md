# Foliage rendering, 14 October 2026

The garden keeps dense shrubs within two metres of the courtyard and alternates
outer grid cells. Outer clumps use slightly broader footprints, preserving a
continuous planted border with fewer overlapping meshes. Gate, house, puzzle
and foreground-fence clearance remain checked for every chapter size.

The approved supplied foliage is the source for three immutable GLBs:

| Detail | Triangles | Selection threshold | GLB bytes |
| --- | ---: | ---: | ---: |
| Close | 5,996 | Full detail above 240 projected pixels | 925,944 |
| Medium | 2,992 | At most 240 projected pixels | 788,944 |
| Far | 1,466 | At most 90 projected pixels | 695,464 |

WebEngine measures projected bounds in backing pixels and applies its existing
10% hysteresis. LOD selection is enabled by the existing display quality policy.
The chains are attached in `addGardenEnclosure` before scene publication, shared
by all shrubs, and reuse the base mesh's material/texture objects. Opaque shrub
parts opt into the existing instance-reordering hint so the new two-part material
does not break batching. No engine sources or gitlink were changed.

Blender seam welding, conservative occluded-island removal and small crack caps
prepare the solid foliage for back-face culling. Only one fully occluded triangle
was safely removed; most geometry savings come from decimation and fewer shrubs.
The 47 triangles belonging to open leaf fragments remain in a two-sided part at
every level. Original 512px base-color, roughness and normal maps are retained;
no new image generation was needed. The far level stays at 1,466 triangles rather
than forcing the provisional 500-triangle target through disconnected features.
Its larger leaves are intended for small screen coverage.

## Measured comparison

Two runs per build, counterbalanced before/after/after/before, in desktop headless
Chrome 155.0.8059.79 on this PC: WebGPU, 1440×900, DPR 1, Automatic quality,
1x MSAA, one 1024px shadow map, reduced motion, default Chapter 1 garden camera.
Each run used six seconds of warmup and ten seconds of RAF sampling. Matching
Release builds used the configured adjacent WebEngine checkout and a complete,
preserved pre-change site.

| Metric | Before | After |
| --- | ---: | ---: |
| Mean FPS | 31.72 | 48.60 |
| Mean frame interval | 31.53 ms | 20.58 ms |
| GPU total time, mean of two final timing windows | 31.00 ms | 19.97 ms |
| Submitted triangles, including shadows and GUI | 1,939,712 | 1,357,568 |
| Shadow triangles | 404,940 | 404,940 |
| Draw calls | 86 | 92 |
| Camera parts using LOD | 0 | 40 |
| GPU payload estimate | 186.77 MB | 191.13 MB |

FPS improved 53.2%; triangle submissions fell 30.0%. The separate leaf material
and derived geometry cost some fixed memory and six additional draw calls. These
numbers describe this desktop browser comparison; physical mobile performance
has not been measured.

Evidence: [paired FPS report](../build/evidence/foliage-performance-final/report.json),
[runtime visual/asset report](../build/evidence/foliage-visual/report.json),
[garden view](../build/evidence/foliage-performance-final/after-2.png),
[side view](../build/evidence/foliage-visual/garden-side.png).
The smoke checks desktop garden/puzzle, side camera, portrait and landscape,
no console errors, LOD selection and HTTP/hash identity of all three GLBs.

Portable tests against the pin passed: starter scene, garden kit, enclosure
clearance, release-one scene and final garden. Clearance tests cover 7×5, 8×6 and
12×10 courtyards with 133, 146 and 188 shrubs respectively, sharing geometry and
materials across LODs. Browser Release builds passed for both the pin and the
configured adjacent checkout.

## Reproduce

Build `scripts/build.ps1 -Configuration Release`, preserve its complete site,
then build the comparison revision using the same dependency and configuration.
Serve the two sites with `scripts/serve.ps1 -SiteDirectory ... -Port ...`.
Set `BEFORE_URL`, `AFTER_URL`, and optionally `EVIDENCE_DIR`, then run
`node scripts/foliage-fps-compare.cjs`. Keep the viewport, camera and quality
fixed. For visual/asset acceptance, set `PAWS_URL` and run
`node scripts/garden-greenery-smoke.cjs`.

Editable packed masters, preserved approved source, scripts, clean-import renders
and provenance are in the [model job](../artifacts/model-jobs/garden-foliage/2026-10-14-performance/README.md).


## Foliage and paving optimization, 15 October 2026

The original foliage meshes, density, seeded positions/tints, saved identities
and 240/90-pixel detail thresholds are retained. Opaque shrubs are created near
first for the default elevated +Z camera, preserving the original poses and
identities while reducing hidden-leaf shading within compatible batches.

The original 2,498-triangle paving remains the close mesh. Two accepted Blender
derivatives use 600/200 triangles below 180/80 projected backing pixels, with
existing 10% hysteresis. They share the original material/texture resources.
Closed reverse faces are culled. The original near/picking bounds remain
unchanged. Garden-floor slabs still receive shadows but no longer submit buried
geometry into shadow maps. No render-resolution or shadow-quality reduction is
used; Automatic remains 1x, MSAA off, one 1024px nine-tap shadow map.

The largest measured gain comes from the configured adjacent WebEngine's cached
material-pipeline specialization: PBR/unlit/water, alpha handling, optional maps,
terrain layers and reflection clipping compile only their applicable paths.
Opaque ordinary draws compile without discard. A matched static garden capture
before/after this shader change was pixel-identical outside the profiling/FPS
panels. More aggressive foliage detail thresholds were tested and rejected: they
changed leaves without a reliable FPS gain. The final thresholds are unchanged.

### Final measured comparison

Chrome 155.0.8059.79, Intel Gen-12LP, 2187x1244 backing/CSS pixels,
DPR 1, live Chapter 3 garden copy with normal animations and the same default
camera. Two runs per version in before/after/after/before order, each with 14s
warmup and 12s RAF sampling. No builds or other test browsers ran during capture.
UnrealEditor/UnrealEditor-Cmd process absence was checked before and after every
sample. These are desktop measurements on this PC, not physical-mobile evidence.

| Metric | Before | After |
| --- | ---: | ---: |
| Mean FPS | 23.77 | 48.21 |
| Mean frame interval | 42.07 ms | 20.74 ms |
| GPU mean, average of final windows | 40.19 ms | 19.66 ms |
| Submitted triangles, including shadows/GUI | 2,164,624 | 1,629,512 |
| Shadow triangles | 700,132 | 396,576 |
| Draw calls | 95 | 94 |

FPS improved 102.8%. Baseline build:
`paws-0.1.0-ae1bfc18126e4b73`; final build:
`paws-0.1.0-2ca32c28a2203d6b`. The final Wasm SHA-256 is
`273d1951a2cf8107e224c4c6e7c85669ede8a1a674599f0030115c38f944b752`. The reusable renderer change is in the configured
adjacent WebEngine checkout; the engine gitlink remains unchanged. Pinned builds
support the game-side presentation assets but retain their existing shaders.

### Appearance and regression evidence

Matched default, close, side, portrait and landscape game views were inspected.
The paving retains its warm material, grout pattern and rounded silhouette;
close views retain the approved original. Shrub density/coverage and leaf
identity are retained. Both paving companions were reopened from packed masters
and rendered after clean GLB import. All six actual HTTP asset responses (three
foliage meshes, original paving and two companions) matched the accepted hashes.

Five focused native checks against the pin passed: starter scene, garden kit,
enclosure clearance, release-one scene and final garden. Browser garden/puzzle,
portrait/landscape, touch restart, object movement, return and saved-state reload
passed without console errors. The rebuilt engine's material, layered terrain
and planar-water tests passed, covering alpha variants, map edits, skinned and
mirrored geometry, 1x/4x MSAA, clipping, archive restoration and device recovery.

- [Four-run report](../build/evidence/foliage-paving-20261015/final/report.json)
- [Before garden](../build/evidence/foliage-paving-20261015/final/before-1.png)
- [After garden](../build/evidence/foliage-paving-20261015/final/after-2.png)
- [After close view](../build/evidence/foliage-paving-20261015/final/after-close.png)
- [Runtime hashes and responsive views](../build/evidence/foliage-paving-20261015/game-visuals/report.json)
- [Restart/save regression](../build/evidence/foliage-paving-20261015/game-restart/report.json)
- [Material regression](../build/evidence/foliage-paving-20261015/engine-materials/results.json)
- [Planar water regression](../build/evidence/foliage-paving-20261015/engine-water/result.json)
- [Layered terrain regression](../build/evidence/foliage-paving-20261015/engine-terrain/result.json)
- [Editable paving sources and provenance](../artifacts/model-jobs/garden-paving/2026-10-15-lods/README.md)
- [Focused engine patch relative to the pre-task working checkout](../build/evidence/foliage-paving-20261015/engine-specialization.patch)

Reproduce using `scripts/garden-performance-compare.cjs`, `BEFORE_URL`,
`AFTER_URL`, `SAVE_FILE` (an exported Chapter 3 copy), `LIVE=1` and
`EVIDENCE_DIR`. Preserve a complete baseline site; close Unreal and other GPU
benchmarks before comparing. Default mode order is before/after/after/before.

# Provenance and material inputs

Created 10 October 2026 for Paws/CatGame. Geometry, UVs, material setup, toy shapes,
socket cuts and procedural construction scripts are original work authored in
this task using Blender 3.5.1. No downloaded models or engine demo assets were used.

The original style authority is `design.md` and the inspected
`docs/design/paws-style-reference.png`. Pip is an existing project dependency and
is not redistributed in this job. Its exact fit-check hash is in the kit manifest;
the final check used the concurrently delivered `pip-tripo-repaired-r7` asset.
Pip's own manifest/provenance govern its source and rights.

The six-tile, 1536 × 1024 source atlas was generated with OpenAI's built-in
ImageGen during this task. Original provider output filename:
`exec-da133cf3-5413-44b4-8204-39d2a70a8eb6.png`. The unmodified original and exact
prompt are retained under `sources/`; the manifest records SHA-256 hashes.
No third-party texture license, scan provenance, or measured PBR data is asserted.
These are generated stylized colour inputs, governed by the applicable generation
service terms; this record does not independently certify account entitlements.

The script extracts six native 512 × 512 tiles without upscaling. The wood/wicker
UV correction aligns grain with members and follows the curved trough. Tile
repetition is restrained and was inspected on the real assets; this is not a
claim of mathematically seamless source pixels. The fine wicker colour shading
is stylized surface artwork. Structural basket depth, holes and rims are geometry.

| Family | Consumers | Roughness |
| --- | --- | --- |
| Stone | Paving, edge | 0.88 |
| Terracotta | Both pot shells | 0.88 |
| Wood | Bench, fence, channels, switch base, gate | 0.73 |
| Cream cotton | Mouse, basket insert, blocking cushion | 0.91 |
| Wicker | Basket shell and rim bands | 0.84 |
| Sage cotton | Purchasable tufted cushion | 0.91 |

Roughness is independently authored linear scalar data, not inferred from colour.
Neutral OpenGL +Y normals deliberately leave relief to geometry; no invented
height from colour markings. Exporter-generated metallic/roughness images use
glTF channels G=roughness, B=metallic=0. Base colour is sRGB; data maps are linear.
All consumed images are packed into the master and embedded in the GLBs.

Procedural solid-colour exceptions: opaque foliage/flower volumes, painted teal
wood, coral/cream ball bands, toy accents/eyes, ceramic glaze and flat water.
Image textures add little to those deliberately simple surfaces. Water is opaque
satin-coloured geometry, with no unsupported transmission or fluid simulation.

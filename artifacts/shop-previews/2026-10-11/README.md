# Shop preview delivery

`render_previews.py` imports the accepted cushion/bowl GLBs unchanged and renders
transparent 512 × 512 RGBA PNGs in Blender 3.5.1. The two packed `.blend` files
retain review cameras/materials. `manifest.json` records input/output hashes.
Runtime copies are `assets/runtime/models/garden/{cushion,bowl}-preview.png`.
No geometry, material, scale or placement contract was revised.

Both previews were visually inspected on transparency and in the actual shop,
details and portrait/short-landscape game GUI. Cushion fabric/buttons and bowl
rim/water silhouette are readable. The accepted garden kit's project geometry,
ImageGen colour sources and rights record remain the provenance authority:
`artifacts/model-jobs/garden-kit/2026-10-10/PROVENANCE.md`.

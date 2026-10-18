# Garden lawn

This describes the pinned-engine lawn fallback. Terrain-capable local builds
replace those exterior slabs with [hilly garden terrain](garden-terrain.md) and
new ImageGen meadow/earth textures.

The exterior ground slabs use `assets/runtime/textures/garden/grass-lawn-v1.png`,
generated with the built-in ImageGen tool on 2026-10-12. The adjacent provenance
file records the exact prompt and rights. The texture catalogue records its hash.

The browser prepares the texture with the other garden assets before publishing
the scene, then shares a textured cube through GardenMeshes. Each ground slab
uses world-aligned UV scale and offset at two metres per repeat, with a matte PBR
material. Portable fixtures without the texture retain the existing fallback.

Validation: pinned Release browser build passed. `scripts/grass-lawn-smoke.cjs`
checked garden and puzzle at 1440x900, 390x844 and 844x390, with no browser errors,
and verified the HTTP texture bytes against the source SHA-256. Screenshots and
report are in `build/evidence/grass-lawn/`. Desktop and portrait garden screenshots
were visually reviewed. Physical mobile performance was not measured.
The separate production-package audit rejects this existing development site
because it contains `pip-preview.html`; runtime asset hash checks passed before
that development-file gate. This is a validated local preview, not a release package.

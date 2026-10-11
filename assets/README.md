# Paws assets

Only accepted models, textures, sounds, manifests and their provenance/license
notices belong in `runtime/`. Browser builds copy **only** `assets/runtime/` into
`site/assets/`. Put source masters and raw/rejected generations in `artifacts/`.
Phase 1 uses procedural engine primitives, so no imported art is required.

Use stable logical paths; do not reference WebEngine demo fixtures or developer
absolute paths. Loading policy and puzzle data belong to Paws.

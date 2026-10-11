# Provenance

Underlying character and maps: user-supplied `assets/Pip/pip.fbx`, SHA-256
`60d739a7f5eddff7a40d561d434425d541384d0feb152a819b5ac7b3985c4617`.
Original unchanged. Source rights and provider record remain as documented in
`../2026-10-10-tripo-repair/PROVENANCE.md`; this revision makes no new license claim.

Base mesh, canonical rig and action source: repaired r8 Blender master from that
job. New facial skinning, two lower-lid controls, timing and local edge refinement
authored in retained Blender scripts. Original coat/eye UV maps are reused for
identity and seam continuity; no new ImageGen texture was needed for this focused
deformation correction. No new external imagery or engine demo asset is used.

Accepted walk and settle sources are recorded in the runtime manifest's preserved
`animation_revisions`. Their non-facial runtime channels are copied byte-for-byte;
the master and FBX import the corresponding editable action curves. Three new
portraits use this same model. Checksums are in `asset-manifest.json`.

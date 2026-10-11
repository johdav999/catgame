# Provenance

Underlying Pip mesh and coat maps come from user-supplied `assets/Pip/pip.fbx`; source SHA-256 `60d739a7f5eddff7a40d561d434425d541384d0feb152a819b5ac7b3985c4617`. Original provider and rights records remain in `../2026-10-10-tripo-repair/PROVENANCE.md`. This repair makes no additional license claim.

The canonical rig and native-skin eyelid source are retained in `native-source.blend`, copied from the prior 11 October eyelid job. This revision adds fitted opening/closure geometry, smooth facial weighting and a welded thin lid margin. Original coat textures and UVs are reused; the narrow margin uses an authored constant dark-brown material. No new provider imagery, engine demo asset or external model was used.

The accepted walk and settle/rest actions are preserved. All 729 non-facial channels were compared against the then-current runtime with zero sampled difference. The final packed master, GLB, FBX and three portraits agree; their checksums and upstream-master checksum are in `asset-manifest.json`.

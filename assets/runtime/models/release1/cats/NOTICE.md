# Provenance

Source: user's installed Toon Cats pack by Radik Bilalov, identified in the open
Unreal Fab window (displayed as Saved in My Library). Project:
`C:/Users/Johan/source/repos/Unreal/Hansa/MyProject2/MyProject2.uproject`.
Main asset: `/Game/CartoonCats/Cat/Meshes/Mesh_CartoonCat`.

User requested export for CatGame and WebEngine. No model, rig, texture or animation
was AI-generated. Unreal 5.8.3 native exporters produced GLB/FBX/PNG source copies.
Blender 3.5.1 imported the derived starter and packed an editable review master.

The Fab library state is not a substitute for the purchase/license record.
Specific license terms and entitlement tier were not retrieved or independently
verified. Retain the seller's applicable license with any eventual release; these
are local game development assets, not a redistribution of the standalone pack.

The user subsequently requested replacement of Pip, Mabel, Button and Pepper.
The active derivatives are under `assets/runtime/models/release1/cats/toon-v1/`.
They use original numbered coat textures 4, 12, 3 and 8 respectively, the purchased
rig, and mapped/resampled source animation clips. Geometry is scaled 2.27x to
match Paws' established game dimensions. Invalid source normals are repaired;
skin weights are preserved. `runtime-package.json` records exact output hashes
and original animation names. `RUNTIME.md` describes all presentation adaptations.

No original Unreal asset package was saved. Material-slot changes for coat export
occurred only in a disposable commandlet process. Source hashes and delivered
artifact hashes are recorded in asset-manifest.json.

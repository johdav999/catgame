# Pip open-eye rim correction

Accepted revision: `pip-fitted-blink-r21`, 11 October 2026.

The circular artifact came from the dark connected lid margin added in r15,
not a surviving eye cap. Its raised strip was visible at rest and its shared
normals changed the shading of the surrounding skin. A removal-only comparison
confirmed the cause but weakened the closed crease.

The replacement is a tucked microfold with a negligible open width, skinned to
the existing upper/lower lid controls so that it unfolds continuously into the
accepted closure overlap. Original facial normals are retained independently of
the fold. No animation timing, bone transforms, coat maps or gameplay changed.

- [Packed editable master](pip-blink.blend)
- [Runtime GLB](../../../../assets/runtime/models/pip/pip.glb)
- [Matching repaired FBX](../../../../assets/Pip/pip-repaired.fbx)
- [Live WebGPU blink video, three views at quarter speed](browser/blink-three-views-4x-slow.webm)
- [Open front](browser/view-0-open.png), [left](browser/view-1-open.png), [right](browser/view-2-open.png)
- [Closed front](browser/view-0-closed.png)
- [Evaluation](EVALUATION.md), [iterations](ITERATIONS.md), [provenance](PROVENANCE.md)
- [Manifest and hashes](asset-manifest.json)

The original runtime/source are retained as `before.glb`, `before.blend`, and
`previous-runtime/`. `repair.py -- 21`, `export.py -- 21`, `export_fbx.py`, and
`render_portraits.py` reproduce the delivered source/exports/portraits in Blender
3.5.1. Earlier numbered checkpoints are rejected diagnostics.

Measured: 20,762 triangles, 10,499 source vertices, 12,253 imported GLB vertices,
two materials, 31 deform bones including the original 29 canonical bones,
nine clips, maximum four normalized influences. GLB: 4,282,748 bytes.
Source bounds remain approximately 0.400 x 1.254 x 1.053 metres (Z-up);
GLB is Y-up, +Z forward, with the existing ground-centred root.

All four supplied 1024-square coat maps are reused, with no new texture cost.
No new renderer feature, transparent overlay, face mask, or runtime scale trick
is involved. Browser evidence is desktop Chrome WebGPU, not physical mobile QA.

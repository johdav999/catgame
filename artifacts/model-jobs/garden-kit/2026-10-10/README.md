# Production garden kit — prompt 4

Accepted delivery: `garden/kit@1`, revision 7. Twenty-one independently loadable
GLBs cover the full prompt inventory. Geometry, textures and manifests are under
this job; only accepted GLBs, the manifest and notices are staged at
[`assets/runtime/models/garden`](../../../../assets/runtime/models/garden/).

- [Packed editable master](garden-kit-r7.blend): original named construction pieces,
  UVs, modifiers, material nodes and packed textures. Blender 3.5.1.
- [Retained construction/export script](scripts/build_kit.py).
- [Portable maps](maps/), [native ImageGen atlas](sources/material-atlas.png),
  [exact generation prompt](sources/material-atlas.prompt.txt).
- [Manifest and measured budgets](asset-manifest.json).
- [Clean reimport measurements](exports/r7/clean-reimport.json).
- [Source render](renders/r7-kit.png), [reimport](renders/r7-reimport.png),
  [Pip/cushion](renders/r7-pip-cushion.png), [Pip/bowl](renders/r7-pip-bowl.png).
- [Evaluation](EVALUATION.md), [correction history](ITERATIONS.md),
  [provenance](PROVENANCE.md), [runtime contract](../../../../docs/garden-kit.md).

Regenerate from the repository root with an installed Blender executable:

```powershell
blender --background --python-exit-code 1 --python artifacts/model-jobs/garden-kit/2026-10-10/scripts/build_kit.py -- --revision 7
blender --background --python-exit-code 1 --python artifacts/model-jobs/garden-kit/2026-10-10/scripts/verify_export.py -- 7
python artifacts/model-jobs/garden-kit/2026-10-10/scripts/package_delivery.py 7
python scripts/generate-garden-contract.py
```

Inspect regenerated output before promoting a future revision. Earlier source
masters and renders retain rejected iterations; the current script reproduces
the final geometry, not every historical draft. Runtime assembly data is compiled
from the delivered manifest by `generate-garden-contract.py`; `--check` detects drift.

The isolated Paws preview is `/garden-preview.html`. Build/serve commands and
acceptance paths are in the runtime contract. This is the asset-production phase;
quest rules, purchases and furnishing behaviour belong to their gameplay phases.

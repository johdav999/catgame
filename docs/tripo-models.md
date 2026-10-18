# Tripo models

The `$tripo` personal skill has a repository copy at `tools/tripo`. It uses the
ImageGen reference creation followed by direct Tripo image-to-model generation,
saves resumable jobs and provenance,
and never places credentials in browser code.

From this repository, run in your own PowerShell terminal:

```powershell
python tools/tripo/scripts/tripo.py configure --repo .
python tools/tripo/scripts/tripo.py status --repo .
```

The first command prompts invisibly for your Tripo API key and stores it as
Windows DPAPI encrypted `.secrets/tripo.dpapi`. The existing `/.secrets/` Git
ignore covers it, as with ElevenLabs. No key was supplied during skill creation.
Reconfigure to rotate the key or on another Windows login/machine. CI may inject
`TRIPO_API_KEY` from a secret manager. Never include `.secrets` in deployment or
asset bundles.

Defaults: Ultra mesh on, AI Complete off, texture on, 4K quality tier, Remove
Lighting off, PBR on, triangles and 50,000 face limit. The pinned generation and
texture models and individual CLI overrides are documented in
[`tools/tripo/SKILL.md`](../tools/tripo/SKILL.md).

Validate a request without spending credits:

```powershell
python tools/tripo/scripts/tripo.py generate --repo . --image artifacts/model-input-images/pot/reference-v1.png --output artifacts/model-jobs/tripo-pot/model.glb --dry-run
```

First create and inspect the reference with the built-in ImageGen tool, and save
it at the illustrated path with its prompt and provenance. Remove `--dry-run` to
submit the image for a paid model generation. Each generation requires a new
output directory; keep its reference in a separate input folder. Text-to-model
and `--prompt` are unsupported by the skill helper.
An interrupted known task can be resumed with:

```powershell
python tools/tripo/scripts/tripo.py resume --repo . --job artifacts/model-jobs/tripo-pot/job.json
```

No live generation is part of installation. Before runtime integration, follow
the repository art direction and PawModels workflow, inspect the actual output,
and record input rights and applicable provider entitlement.

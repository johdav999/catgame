# Pip settle correction — 11 October 2026

The old settle compressed the neck by 23 cm using a local translation inherited
from the procedural cat. Its leg-fold rotations were then compensated by raising
the entire pelvis to clear the lowest vertex and translating hind lower joints.
This produced the collapsed chest and raised back seen in the user's four views.
Floor-only and endpoint tests passed that anatomically disconnected pose.

The replacement uses a continuous low settle with connected joint rotations,
two-segment leg solving against the supplied mesh's actual toe samples, a lowered
haunch, relaxed forelegs and a tail brought beside the body. The 2.4-second timing,
canonical bind, fixed root and matching four-second rest endpoint are preserved.
The result is a supported low rest, not a tight sleeping curl. Side-roll attempts
were rejected; none of those trials were published.

- Editable animation source: [pip-settle.blend](pip-settle.blend).
- Standalone exported source: [pip-settle.glb](pip-settle.glb).
- Runtime: [pip.glb](../../../../assets/runtime/models/pip/pip.glb).
- [Browser cushion fit](browser/cushion-fit.png), [side](browser/end-side.png),
  [rear](browser/end-rear.png), and six timed transition captures in `browser/`.
- [Evaluation](EVALUATION.md), [iterations](ITERATIONS.md),
  [provenance](PROVENANCE.md), [manifest](asset-manifest.json).

`merge_settle.py` publishes only settle/rest body channels, preserving the current
mesh, materials, rig, eyelid channels and unrelated animations. The editable source
is an animation layer based on the preserved r8 rig; it is not the authoritative
source for concurrent walk or eyelid revisions. Do not replace the full runtime
with its standalone export. `promote.py` checks exact reviewed hashes before writes.

Run `paws_pip_settle_tests <glb>` or CTest `paws_pip_settle`. It checks connected
joints and scale, 60 Hz interpolated skin, contact clearance, frame displacement,
timing and settle/rest seams. The original fails with `Disconnected settle/rest joint`.

The isolated review site is `build/web-settle-review/site`, served on port 8099
during this job. Rebuild normal browser targets to stage the production asset.
Future curled-rest changes must also reauthor the settle endpoint and pass the
same continuity/deformation review. Eyelid fit and true sleeping-curl quality are
separate outstanding asset work; this job does not certify them or mobile hardware.

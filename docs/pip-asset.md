# Pip asset and animation contract

Stable ID `cats/pip@1`; relative URL `assets/models/pip/pip.glb`. Source job:
[`artifacts/model-jobs/pip/2026-10-10-tripo-repair`](../artifacts/model-jobs/pip/2026-10-10-tripo-repair/README.md).
The JSON manifest is authoring/release metadata, not an engine-parsed gameplay
definition. Paws' C++ action table is the runtime contract.

The [11 October settle repair](../artifacts/model-jobs/pip/2026-10-11-settle-fix/README.md)
replaces the settle/rest body channels with connected-joint animation and
skin-measured paw contacts. The old procedural neck translation and disconnected
hind-joint correction are removed. The runtime manifest records this animation
layer separately from concurrent walk/eyelid revisions; merge channels rather than
overwriting the full GLB with an older standalone source export.

`garden_walk` was rebuilt on 11 October from photographic walking references and
explicit contact/swing targets. See the [walk source and evidence](../artifacts/model-jobs/pip/2026-10-11-reference-walk/README.md).
It covers 0.36 m per 1.2-second cycle; `pipWalkSpeed` is 0.30 m/s and drives garden
travel. The isolated preview includes a complete-cycle scrubber and translating
lane. The walk merge preserves other animation revisions, including all eyelid
controls. The eyelid delivery also brings the accepted walk/settle actions into
the packed Blender master and repaired FBX.

The [complete fitted blink repair](../artifacts/model-jobs/pip/2026-10-11-blink-production/README.md)
smooths the facial openings, fits a shared closure surface, blends lid movement
into the cheek and brow, and adds a connected inward-folding margin. All original
coat UVs are retained. Its GLB, FBX, packed master and portraits match. A 41-position
clean-reimport check finds no exposed eye surface at full closure from front/side
directions; WebGPU evidence includes closing and reopening. The preview's Blink
inspection controls expose the complete exported clip for repeatable review.

The [open-eye rim correction](../artifacts/model-jobs/pip/2026-10-11-eye-rim/README.md)
replaces the permanently raised dark margin with a tucked microfold that unfolds
as the lids close. Facial shading normals remain independent of that thin fold,
preventing the circular outline around the open eyes. All 837 animation channels,
including the accepted blink timing, are unchanged. The packed source, GLB, FBX
and portraits use this correction; front and both side views are recorded.

## Geometry and rig

Metres, right-handed. Blender source: Z up, -Y forward. GLB/WebEngine: Y up, +Z
forward, +X right. Origin is the ground-centred root beneath the torso. Rest bounds
are approximately 0.400 m wide, 1.053 m tall including tail, and 1.254 m long.
Use unit actor scale; let the selected exporter handle coordinate/unit conversion.

The current coat is the supplied Tripo mesh, rebuilt from `assets/Pip/pip.fbx`.
Its incomplete source weights were replaced completely. Source UVs and coat
markings are retained, with 1024-square base-color, normal, roughness and metallic
maps. Eyelids deform the original facial skin with its coat UVs; the eye surfaces
remain stationary relative to the head. There are 20,762 triangles, one skinned
mesh and two material slots (coat and the thin welded lid margin). No strand
fur, morph animation, runtime constraints or transparent cards are needed. Every
vertex has up to four normalized anatomical influences; none use the static
mesh-node fallback. The original FBX and previous runtime delivery are preserved.

Canonical bones: `root -> pelvis -> spine -> chest -> neck -> head`;
`ear.L/R` and `blink.L/R` beneath head; `front_upper.L/R -> front_lower.L/R ->
front_paw.L/R` beneath chest; corresponding `hind_*` beneath pelvis; `tail.00`
through `tail.06` beneath pelvis. All 29 original deform bones retain their names,
parents and bind transforms. Two optional head-parented controls,
`blink.lower.L/R`, bring the total to 31 deform bones; importer wrapper nodes
make 33 engine joints in GLB and 34 in FBX. The FBX uses
`FBX_SCALE_ALL` to avoid a 100x wrapper with centimetre-valued child translations,
and its animation stack names are normalized to the action contract. Both formats
pass the pinned importer with default limits. Never use raw imported joint indices
as saved IDs.

Root policy: every action keeps `root` translation/rotation/scale at bind values.
Pelvis lift, gait and contact adjustments are visual only. Paws owns world
translation along garden paths; do not enable root-motion extraction for these
clips. The authored 30 fps samples use linear interpolation and include endpoints.

Supported deformation envelope is the delivered actions and their short blends:
the canonical actions are adapted for this mesh's ground contact, resting tail
and eyelid skin. Arbitrary extreme user poses are outside this tested envelope.
Blink uses translation with unit bone scales, avoiding the old caps' inflation and
near-singular normal transforms. Upper lids do most of the travel and lower lids
rise to meet them along a gently curved seam. The idle closes in 100 ms, holds for
67 ms and reopens over 200 ms, beginning at 2.4 s of the four-second idle loop.
Settle closes gradually and rest holds closed; other clips keep eyes open.
The [fitted eyelid source and evidence](../artifacts/model-jobs/pip/2026-10-11-blink-production/README.md)
include a clean GLB reimport, GLB/FBX importer checks and close-up WebGPU review.
All 729 non-facial animation channels retain identical sampled poses.

## Actions

| Name | Seconds | Loop | Reduced-motion sample | Integration |
| --- | ---: | --- | ---: | --- |
| `relaxed_idle` | 4.0 | Yes | 0 | Resting attention and slow blink |
| `attentive_request` | 2.4 | No | 1.2 | Request head tilt and ear turn |
| `happy_pounce` | 1.2 | No | 0.6 | Small happy hop; chirrup sound belongs to phase 9 |
| `garden_walk` | 1.2 | Yes | 0 | In-place short garden gait; caller moves actor |
| `settle_cushion` | 2.4 | No | 2.4 | Lower and fold onto authored cushion contact plane |
| `curled_rest` | 4.0 | Yes | 0 | Supported low rest, relaxed forelegs, tail beside the haunch |
| `approach_bowl` | 1.8 | No | 1.8 | Head-lowering end of walk-to-bowl sequence |
| `drink` | 2.0 | Yes | 0 | Gentle repeated drinking/head motion |
| `return_idle` | 1.8 | No | 1.8 | Raise head from drinking back to idle |

Settle end matches rest start; approach end matches drink start; drink end matches
return start; return end matches idle start. Portable tests check every joint at
those boundaries and all looping endpoints. `presentPip()` resolves clip names,
crossfades for 0.18 seconds, or seeks/pauses a meaningful static pose. It changes
no puzzle, reward, furnishing or save state. Later gameplay chooses successors;
the inspection harness intentionally holds non-loop endpoints for review.

The repaired resting clip presents a supported low rest, not a tight sleeping curl.
Settle/rest deformation and visual cushion fit were reviewed in the actual WebGPU
garden-kit preview on 11 October; a finished sleeping curl remains a separate
authoring task. The new `paws_pip_settle` regression rejects disconnected joints,
non-anatomical scales, floor penetration, frame pops and a mismatched rest seam.
Any replacement rest must update its matching settle endpoint. Exact bowl contacts
remain separate furnishing work.
Do not reuse the previous model's contact dimensions without checking this mesh.
Align the actor root with the authored contact plane. The bowl approach consists of `garden_walk` on an authored
path, then `approach_bowl`, `drink`, and `return_idle`. Use the head-bound muzzle
anchor measured for this mesh, and verify lip height against the final bowl.

## Runtime integration and validation

`paws_pip` is a portable optional library containing immutable asset validation,
clip lookup, presentation selection and the preview scene. `pip_preview` links it
to the existing pinned renderer and `engine_assets_browser`. The HTTP request
publishes through `Application` at frame boundaries. Failed fetch/decode preserves
the loading surface; Retry cancels the old request and evicts unused failed data.
Commands resolve a generation-checked component handle. Renderer/controller
members are destroyed before Application scenes. Application alone ticks animation.

The preview page's HTML controls are development tooling, not replacement
production game UI. The parallel phase-1/2 main executable is preserved. Phase 5
can link `paws_pip`, prepare `cats/pip@1`, instantiate `SkeletalMeshComponent`,
and use the same action/portrait IDs inside its garden and dialogue flow.

```powershell
./scripts/build.ps1 -Pinned -Configuration Release -EmsdkPath <installed-emsdk> -BuildDirectory build/web-pip-pinned-release
./scripts/serve.ps1 -SiteDirectory build/web-pip-pinned-release/site -Port 8094
# Open http://127.0.0.1:8094/pip-preview.html
node scripts/pip-smoke.cjs
```

Use `PAWS_URL`, `BROWSER_EXECUTABLE`, `NODE_PATH` and `EVIDENCE_DIR` as needed.
Native: build `paws_pip_tests` and `paws_scene_tests`, then run CTest with
`-R 'paws_pip_asset|paws_starter_scene'`. On Windows put the matching compiler
runtime on PATH. Existing local Jolt source can be reused through the ignored
CMake cache's `FETCHCONTENT_SOURCE_DIR_JOLT`; shared source files contain no
machine-specific dependency paths.

Physical mobile and final garden/furnishing acceptance remain phase-7/10 work.

The repair evidence includes native default FBX/GLB importing, exact bind/hierarchy
comparison, a root-follow test over every imported vertex, all nine actions, loop
and transition seams, floor/height envelopes, packed-master reopening, clean
reimport, and actual Chrome WebGPU playback. Browser evidence is retained in the
repair job and `build/evidence/pip-tripo-repair/`.

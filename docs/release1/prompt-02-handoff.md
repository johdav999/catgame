# Prompt 2 — Tripo cats, canonical rigs and animations

## Delivered scope

Button, Mabel and Pepper use the user's supplied FBX geometry, repaired/fitted
skins and original coat UVs/maps. Original identities take precedence over the
swapped source folder labels: `assets/button → Button`, `assets/pepper → Mabel`,
`assets/mabel → Pepper`. Originals and ZIPs are untouched. The user attested a
Tripo3D Pro plan on 11 October 2026; the direct statement is recorded, without
inventing a receipt or account-specific license.

All four cats, including a separate release-one Pip variant, expose 31 canonical
deform bones and 16 named actions. Accepted Pip authority files are unchanged.
New source/output lives only in the task's cat namespaces. No engine change,
gitlink update, shared production registration, commit or publication was made.

Final full bake: **r24**. Probe masters and older revisions are retained rejected
iterations. Runtime outputs are under `assets/runtime/models/release1/cats/`.
The machine-readable handoff is [fragments/cats.json](fragments/cats.json).

| Cat | Triangles | Materials | GLB bytes | Packed master |
| --- | ---: | ---: | ---: | --- |
| Pip | 20,762 | 2 | 5,107,444 | `artifacts/model-jobs/release1-cats/pip/pip-r24.blend` |
| Mabel | 24,109 | 1 | 4,841,824 | `artifacts/model-jobs/release1-cats/mabel/mabel-r24.blend` |
| Button | 25,675 | 1 | 4,891,284 | `artifacts/model-jobs/release1-cats/button/button-r24.blend` |
| Pepper | 24,553 | 1 | 4,909,176 | `artifacts/model-jobs/release1-cats/pepper/pepper-r24.blend` |

Exact hashes, bounds, footprint, material-map identities, portraits, packed
masters, clean-reimport masters, provenance and contact records are in the
fragment and each runtime cat's `manifest.json`. The task allocation is 28k
triangles including fitted lids; it does not alter the shared model contract.

## Interfaces for Prompts 6 and 9

`src/release1/cats/cat_assets.hpp/.cpp` provides `assets`, `actions`, `validate`,
`validateCanonical` and `present`. IDs are `release1/cats/{cat}@1`; site URLs are
`assets/models/release1/cats/{cat}/{cat}.glb`. Include the module in game composition
and register its assets through the application asset owner. `present` selects
by action name, crossfades for normal motion, or seeks/pauses a deliberate static
pose for reduced motion. Clip completion never commits a puzzle/reward.

Every cat has relaxed_idle, attentive_request, happy_pounce, garden_walk,
settle_cushion, curled_rest, approach_bowl, drink, return_idle, scratch, perch_step,
perch_settle, playhouse_enter, playhouse_lookout, picnic_sit and personality_reaction.
Timings, loop flags and static frames are in the header/fragment. Bind names,
parents, local TRS and inverse binds match `canonical-rig.json` within 1e-5.
Do not identify deform bones by importer wrapper-node indices.

All clips are in place. The application owns actor travel, support heights and
orientation. Walk translation is 0.30 m/s (0.36 m per 1.2-second cycle). The
original walk curves are reused; per-mesh eye/weight/ground fitting is baked.
Pip's new variant additionally corrects torso/haunch/bib skinning for curled poses.
The original accepted Pip GLB and master remain byte-identical.

Use `contacts.json` for measured bounds, paw soles/centres, nose/muzzle surfaces
and joint origins at first/mid/last frames. `animation-measurements-runtime.json` supplies
dense per-frame bounds and foot measurements. Coordinates are metres, Y up,
+Z forward. Surface measurements are distinct from bone origins. The retained authoring
`animation-measurements.json` uses Blender Z-up coordinates; use the explicitly
converted `animation-measurements-runtime.json` for game integration.

- **Rest:** Pip/Button use a compact C-shaped sleeping curl. Their extents are
  approximately 0.793 × 0.800 m and 0.757 × 0.796 m. The root is offset from the
  footprint centre. Align the cat's measured `rest_root` anchor with the prop
  support centre, or use `rest_root_from_support`; rotate the offset with the
  actor's yaw. Add the cushion/hammock support height to actor Y. Mabel/Pepper
  retain adapted low-rest poses.
- **Bowl:** the existing bowl water plane is 0.383 m. Each cat's measured lower
  muzzle is approximately 0.392 m at the drink endpoint. Use its individual
  `drink_root_from_water` X/Z offset; do not reuse Pip's old scalar -0.61327 Z
  offset for the new cats. The approach/drink/return endpoints remain connected.
- **Scratch:** align the post face to the measured front-paw contacts; the
  supplied post's reference contact height is 0.62 m. Retain the measured hind
  support plane. The scratch loop alternates the paws.
- **Perch/playhouse:** the clips provide in-place leg/head/settling poses.
  `perch_step` is a lift/placement gesture, not a self-contained staircase
  traversal. Prompt 6 owns supported movement through the supplied approach,
  step (0.20 m), top (0.46 m), entry, lookout and exit anchors. Validate the actual
  combined prop/actor sequence and visible support there. Do not infer completed
  traversal from animation playback. Entrance compatibility was checked against
  the 0.60 m width / 1.10 m height contract.

No shared file patch is required from this asset task. Prompt 9 merges the
fragment, registers the module/URLs and portraits, and verifies the resulting
site's exact bytes. This handoff does not replace Prompt 6's furnishing choreography
or Prompt 9's full-game verification.

## Build and verification

The task-local wrapper at `scripts/release1/cats/CMakeLists.txt` reuses the shared
track harness and pinned bridge, while linking the cat workshop only to paws_pip
and engine facilities. This prevents concurrent garden/UI implementation errors
from becoming cat-preview compile dependencies. Shared CMake files were not
modified by this task.

From the repository, set `$blender`, `$emcmake`, and optionally `$joltSource` to
installed tool/dependency paths (the latter is a local cache, not a different
engine source). See the job README for source authoring/reimport commands.

```powershell
cmake -S scripts/release1/cats -B build/native-release1-cats -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build/native-release1-cats --target release1_track_tests --parallel 4
& $emcmake cmake -S scripts/release1/cats -B build/web-release1-cats -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build/web-release1-cats --target release1_track_site --parallel 4
foreach ($cat in @('pip','mabel','button','pepper')) {
  Copy-Item "artifacts/model-jobs/release1-cats/$cat/exports/$cat-r24.glb" "build/web-release1-cats/site/candidates/$cat.glb"
  Copy-Item "artifacts/model-jobs/release1-cats/$cat/renders/portrait.png" "build/web-release1-cats/site/candidates/$cat-portrait.png"
}
python scripts/release1/cats/validate_exports.py build/web-release1-cats/site/candidates/pip.glb build/web-release1-cats/site/candidates/mabel.glb build/web-release1-cats/site/candidates/button.glb build/web-release1-cats/site/candidates/pepper.glb --report build/evidence/release1/prompt-02/glb-validation.json
python scripts/release1/cats/validate_runtime.py
node scripts/release1/cats/browser-smoke.cjs
python scripts/release1/cats/package_cats.py 24
python scripts/release1/cats/serve_preview.py --port 19612
```

Create `build/web-release1-cats/site/candidates` before first staging. Native
execution needs the selected MinGW compiler's `bin` on PATH. The local run used
`-DFETCHCONTENT_SOURCE_DIR_JOLT=<existing JoltPhysics-5.3.0 cache>` to avoid a new
download. Both build caches resolve WebEngine to the exact pinned checkout,
`3fe7d1dc1dc610c703944803cbb717cd65024ec9`. Use `--fresh` once when migrating an
existing cache from the generic track source directory to this wrapper.

Recorded evidence is under `build/evidence/release1/prompt-02/`:

- Raw GLB: exact canonical binds, action set, zero-based finite samplers, top-four
  normalized skin, supported embedded textures/materials and geometry budget.
- Native: all 64 actions sampled at 60 Hz, including half-keyframe positions;
  finite skin matrices, ground/height envelope, seamless loops and common
  transitions, reduced motion, pause, suspension and scene shutdown.
- Existing pinned portable `paws_starter_scene` test: passed. New workshop
  lifecycle behavior is also covered by the freshly built cat harness.
- Chrome WebGPU: all four actual GLB HTTP requests, 64 action selections,
  animated walk and close-up blink pixel changes, reduced-motion stillness,
  suspension, shadow-free puzzle distance, 390×844 layout and no browser errors.
  Native/browser reports bind these observations to exact payload hashes.
- Fresh-process Blender GLB reimports, face/action views, final portraits and
  [four-cat comparison](../../artifacts/model-jobs/release1-cats/lineup.png).

Job documentation is in `artifacts/model-jobs/release1-cats/README.md`,
`EVALUATION.md`, `ITERATIONS.md`, `PROVENANCE.md` and `asset-manifest.json`.
Task-owned code is under `src/release1/cats`, `tests/release1/cats`,
`scripts/release1/cats` and `web/release1/cats`.

Full four-cat garden composition and furnishing use remain integration work.
Physical mobile, Safari, independent-player acceptance and sustained device
performance are not established by this desktop asset-preview run.

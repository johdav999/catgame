# WebGL2 material shader variants

Published engine commit `619ce5e7b5fc2965754b4fd4ce0c4cf96494f018`, on
`codex/webgl-material-variants-20261023` and based on pin
`9b597da84d4582973d010fa9ba3e328c20181892`, adds two context-owned WebGL2 mesh
programs. RacingGame selects it. Verify each consumer pin independently; the
CatGame pin is unchanged by this task.

Ordinary non-water, non-layered, unmasked draws outside planar reflections use
the simple program. Its GLSL compile-time constants remove water/layer/ribbon
code and mask/reflection discard paths while retaining the same lighting, fog,
texture, tint, tone-map, opacity and transform expressions. Water, layered and
masked materials, planar-reflection draws, ribbons and particles keep the
general program. This is private backend policy: no game-specific shader code,
public API, asset replacement, geometry reduction or resolution change is needed.

Initialize per-view camera and fog state for both programs. Recreate both after
context loss and delete both during normal resource release. Keep program-local
uniform locations associated with the correct bound program. Native core and
WebGPU paths are unchanged.

Profile an actual warmed racing camera and record backend/adapter, viewport,
FPS, frame-time p95, draw/triangle counts and LOD use. Separate GPU backpressure
from CPU encoding: a large encode duration can include a saturated GPU queue.
Bracket diagnostic changes against the restored baseline. Uniform caching and
mipmap generation did not fix the RacingGame reproduction; compile-time material
specialization did. Do not infer causality from one FPS counter sample.

Acceptance must use the actual compiled engine, retain every skyline placement
and asset hash, compare ordinary and special-material views on both backends,
exercise WebGL context loss/restoration and complete driven laps. Native static
asset/placement checks protect source geometry, materials and shared LODs.
Consumer docs/evidence: RacingGame `docs/webgl2-performance-fix.md` and
`build/evidence/webgl2-material-fix/`. Local development-engine evidence is not a
published-pin acceptance result until the engine branch is published and selected.

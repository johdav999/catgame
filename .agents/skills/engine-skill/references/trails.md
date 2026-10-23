# Ribbon trails (2026-10-17)

Read WebEngine docs/trails.md and docs/trails-validation.md, and verify the selected
engine pin. ParticleEmitterDefinition::renderMode defaults Billboard; Ribbon uses
one component-owned bounded ring in trail.cpp, sampled only by scene variable ticks.
Defaults: trailMaxPoints=256 (2-4096), trailLifetime=1 (>0, <=86400),
trailMinDistance=.05 (>=.0001), trailBreakDistance=5 (>=sample distance),
trailTextureLength=1 (>=.0001). All must be finite. At most 256 distance samples per
update; overload starts a strip and records drops. Capacity evicts oldest points;
expiry uses the simulation clock. trailPointCount()/trailPoint(index) inspect the
ring. particles() counts billboards only; droppedParticles() includes trail drops.

Move the source before scene.update(chosenDelta). World trails retain world samples
and birth width scale. Local trails sample the emitter origin in its parent's local
coordinates: emitter-relative movement draws; parent movement carries history.
This differs from local billboards following the emitter itself. Distance uses the
selected space's units. breakTrail() explicitly splits a stroke; teleports beyond
trailBreakDistance and resume also split it. Pause/disable freeze; disabled motion
needs explicit break when a bridge is unwanted. Stop drains sampled points; clear
and restart empty them. Duration/loop/startDelay apply. Rate/bursts/birth shapes,
forces, alignment, rotation and flipbooks do not drive ribbon geometry.

Size is full width and RGBA curves use point age/trailLifetime. Full texture repeats
along accumulated U length; V spans width. SceneFrame carries portable RibbonDraw
segments, FxFrame builds camera-specific six-vertex quads with shared tangent joins
and camera-parallel fallback. Alpha segments sort with meshes/billboards by center;
additive follows. Both renderers provide World depth test, masks, fog and optional
softDistance using the existing opaque-depth capture with its receiver limitations.
WebGPU retains per-view buffers and skips static uploads; GL streams batches.
FrameCounters exposes trailSegments/trailDrawCalls/trailUploadBytes. No GPU types
enter portable APIs. No collision, decals, per-particle subtrails, mesh/ground-aligned
particles, fancy caps or miter joins are delivered.

Editor Ribbon trail preset/group reuses immutable EffectAsset edits, curves,
textures, undo/redo and .wefx. EffectPreview owns a separate scene and optional
source orbit (setTrailMotion / Move trail source checkbox); never move authoring
nodes for preview. Both local/world preview motion moves source children. Archive
v19 retains definitions plus full live ring/sampler; versions 1-18 default Billboard.
Exact undo retains runtime state while fresh handles rebuild; deploy matching host
and worker codecs. Preview motion controls are transient.

Use repo-local .tools/emsdk and .emscripten-version with scripts/build.ps1 Release.
Build native trail_tests, fx_tests, editor_fx_tests, persistence_tests,
render_frame_tests; run CTest trail/fx/editor_fx/persistence/render_frame with native
compiler bin on PATH when needed. Run scripts/trail-smoke.cjs (DPR 1 and 2),
scripts/trail-depth-smoke.cjs, editor-fx-smoke.cjs and fx-milestone-smoke.cjs on the
exact staged host/Wasm with Playwright/pngjs, NODE_PATH, BROWSER_EXECUTABLE,
DEMO_URL and EVIDENCE_DIR. Desktop evidence does not establish physical mobile,
non-Chromium browser acceptance or sustained performance.

Plane alignment and archive v20: see [plane particles](plane-particles.md).

Static mesh particles and current archive v21: see [mesh particles](mesh-particles.md).
Soft-distance fading remains supported on quad/ribbon particles, not Mesh mode.

Explicit SurfaceTrail now accepts supplied world contacts/normals with gaps and
camera-independent tangent-plane geometry. It leaves Ribbon alignment unchanged.
See [surface trails](surface-trails.md) for receiver policy, v28 live-normal
persistence and current writer/validation guidance.

## Direct D3D12 FX milestone 7 (2026-10-22)

Milestone 7 adds direct D3D12 billboard/velocity/plane FX, flipbooks,
ribbons/supplied-contact surface trails, mesh particles and soft opaque-depth fading.
Use existing ParticleEmitterComponent/EffectComponent definitions and FxFrame;
simulation, seeds, transforms, contact policy and archives remain portable. Alpha
meshes/quads/segments share center sorting before additive draws. FX depth-tests
without writes, uses premultiplied blending, alpha fog haze/additive extinction,
HDR/MSAA, masks and ordered views. Mesh particles follow ordinary materials/LOD
and individual direct mesh submissions; they do not cast shadows and require zero
softDistance. General mesh instancing now accompanies the dedicated particle path.

Soft FX shares one opaque World color/depth snapshot with water, taking minimum
4x depth; per-camera perspective/orthographic reconstruction and viewport clamps
isolate views. Underwater runs after World FX, before Overlay/GUI. Positive soft
distance defaults off (zero); active masked soft emitters conservatively allocate
scratch even when empty/paused. Disabled subtrees/absent masks do not. Transparent
surfaces never become soft-depth receivers. Approximate sorting remains.

Direct per-view streams retain numeric/GPU data without scene borrows. Unchanged
paused streams upload nothing; barriers and submission fences protect copies and
retired buffers. Identical host view snapshots retain streams/LOD history; changed
camera identities invalidate them. Preflight FX staging is at most 32 MiB within
the combined 64 MiB arena; meshes/FX share the 40960 draw and 65536 descriptor limits.
Counters expose particles, meshParticles, particleDrawCalls, trailDrawCalls,
trailSegments and changed particleUploadBytes/trailUploadBytes per view. Mesh instancing, BC7 uploads and bounded direct recovery are now delivered.

Launch webengine_fx_windows --backend dawn|d3d12 for deterministic paused presets,
restart, simulation playback, fog/HDR/MSAA; F11 fullscreen, Esc exits. --capture
image.ppm includes retained controls (create parent directory first). Build with
scripts/build-windows.ps1 -Backend Both|Dawn|D3D12; Both remains default. Run
windows-fx-smoke.ps1 for 102 paired captures, then fx/trail/plane_particle/
mesh_particle/material/water/render_frame and desktop_api/desktop_api_d3d12 CTest.
Validate a separate Dawn-free build. Browser suites fx-milestone-smoke,
trail-depth-smoke, plane-particle-smoke, mesh-particle-smoke and surface-trail-smoke
force WebGPU/WebGL2 against fresh pinned local Emscripten outputs. Record native
and browser separately; no other-GPU/mobile/throughput acceptance follows. No
consumer pin, Paws host, public renderer API or archive version changes here.

Read [native FX](windows-fx-rendering.md) and [milestone 7 evidence](windows-d3d12-milestone7-validation.md).

## Direct D3D12 recovery and performance milestone 8 (2026-10-22)

Direct D3D12 now supports bounded device recovery, shared opaque mesh batches
and semantic BC7 mip uploads with mandatory RGBA/GUI fallback. Existing simulation,
resource immutability, scene handles, archive and presentation contracts remain.

Read [recovery/performance contracts](windows-d3d12-recovery-performance.md) and [milestone 8 evidence](windows-d3d12-milestone8-validation.md).

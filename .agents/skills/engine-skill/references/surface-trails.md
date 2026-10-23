# Surface-following trails (development engine, 2026-10-19)

Verify the selected checkout/game pin before using these APIs. Stage 6 introduces
ParticleRenderMode::SurfaceTrail in engine_core, sharing the existing bounded FX
ring, texture, size/RGBA curves and private renderer trail path. It is explicitly
surface-oriented, unlike camera-facing Ribbon. Include
engine/particle_emitter_component.hpp and engine/effect_presets.hpp.

Use surfaceTrailEffect() or an immutable ParticleEmitterDefinition with SurfaceTrail
and ParticleSpace::World. Call submitSurfaceContact(worldPoint, worldNormal) on the
owning thread before each normal scene variable update; last valid submission wins.
No submission means a gap. Do not add a second scene update. Games own tyre slip,
contact/query timing, receiver revision/eligibility and per-wheel emitters. There
are no implicit physics queries, retained receiver handles or terrain subscriptions.
Marks stay in baked world coordinates across receiver movement/terrain edits/page
eviction: applications breakTrail(), stop(true) or replace streams explicitly.

Preset: 512 points, 12 s, .05 m samples, 2 m teleport break, 1 m texture length,
.32 m full width, .015 m normal offset, dark alpha with final-fifth fade. Capacity
2..4096; lifetime >0..86400; sample/repeat >=.0001; break>=sample. Contact/normal
components finite +/-1e6, normal length>=1e-6; normalize on submission. Local space,
particle collision and offsets outside +/-1 m are rejected. Invalid input retains
valid state. Ordinary shared definition validation still applies.

Distance/residual sampling interpolates straight endpoints, normals and birth
times, <=256 new points per update; overflow starts a new stroke and counts drops.
Capacity evicts oldest; ages use scene variable time, pause freezes, stop drains,
stop(true)/play(true) clear, resume breaks. Missing input, opposed normals and
teleports break. Disabled state freezes/hides; break across disabled movement.
trailPointCount()/trailPoint(i) expose the bounded ring with normal/birth/distance.

SceneFrame RibbonDraw.surfaceAligned/beginNormal/endNormal feed FxFrame's
camera-independent tangent-plane sides. Degenerate/reversed sides suppress the
segment. Full-width terrain projection, miter/round caps and receiver reprojection
are absent. At most capacity segments, 2*capacity triangles, 6*capacity 48-byte
vertices per emitter/view; applications bound aggregate emitters/views. Existing
World masks, depth/no-write, alpha/additive, fog, soft fade, sort/upload/recovery
apply on both renderers. Physics is unaffected. Use DecalComponent for discrete marks.

Details -> Particle FX -> Surface trail uses immutable asset history, appearance
and size/RGBA curves plus Trails parameter controls. EffectPreview owns a sloped
receiver and orbit contact fixture with airborne intervals, not source scene or
physics queries. Host framing, move/pause/restart/step/isolate, trail point/drop and
segment/upload diagnostics are transient. Release renderer borrows before teardown.

Archive v28 adds definitions/live point+source normals with ring/clock/residual
continuation; prior trails default +Y. Current shared checkout writer is v29 due to
separate FX composition work; readers include v28. Pending contacts, receivers and
render caches are not saved. Resubmit contact before the next update to continue.
Scenes/application envelopes retain live state; .wefx retains definitions only.
Duplicate and Play reuse shared definitions with independent live state. Deploy
matched host/worker/Wasm. Keep pre-publication allocation/normal/chronology checks.

Use pinned repository .tools/emsdk (6.0.11), README build commands and selected
build's web_assets target to publish dist. Focused native CTest surface_trail plus
trail/fx/editor_fx/persistence/render_frame; explicit surface_trail_wasm_tests runs
in Node. Run scripts/surface-trail-smoke.cjs at DEVICE_SCALE_FACTOR=1 and 2;
scripts/trail-depth-smoke.cjs with SURFACE_TRAIL=1 checks surface pixels, depth,
fog, soft fade, alpha/additive, MSAA, masks, save/open and recovery on both backends.
Use NODE_PATH, BROWSER_EXECUTABLE, DEMO_URL and EVIDENCE_DIR as in build-validation.
Desktop/emulated checks do not establish physical mobile or max-budget performance.
Read selected-engine docs/surface-trails.md and surface-trails-validation.md.

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

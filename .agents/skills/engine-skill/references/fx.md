# Particle FX integration (2026-10-17)

Development WebEngine adds optional `ParticleShape::{Box,Sphere,Disc,Cone,MeshSurface}`,
`radius`, `coneAngle` (radians), `speedMin/Max` and sphere `surfaceOnly`. Shape position
adds to existing box position ranges; speed adds to random velocity. Cone/disc axis
is emitter-local +Y. MeshSurface uses immutable static `emissionMesh` triangle area
and geometric face normals, with a per-emitter derived CDF; no skin/morph/LOD sampling.
Validate before replacing definitions; preserve active state on failure. Publish all
shared definition/mesh/texture resources immutable.

Motion remains CPU-owned in the scene variable tick: optional `drag`, `wind`,
`turbulence` per-axis sinusoidal acceleration amplitude, `turbulenceFrequency`, and
`velocityOverLife`/`forceOverLife` normalized-age `VectorKey` curves. Wind is drag's
target velocity and has no effect at zero drag; velocity curves add displacement
velocity, force curves add acceleration. Fields use selected local/world simulation
space. Constant gravity/drag integrate analytically; curves/gusts use midpoint steps
near 1/60 second capped at 128 per live particle/update. Large updates are approximate;
varying fields are not exactly invariant to frame partition. Restart repeats the seed.

Opt into `ParticleAlignment::Velocity`, `stretch` seconds and `softDistance` world
view-depth units. Per-camera stretch preserves width and adds worldSpeed*stretch to
length; zero/projected-zero velocity uses a stable square. `FxFrame` retains alpha
order and now emits 112-byte WebGPU instances; fallback retains individual draws.
Soft fading uses opaque World depth on both renderers and attenuates alpha/additive
radiance. Clear depth retains opacity; transparent meshes/particles do not supply
soft depth. It shares water's per-view opaque color/depth snapshot and excludes water
requiring scene inputs. No physics or picking state depends on these choices.

Active masked soft emitters conservatively request existing scratch targets, even
when paused/drained. Disabled subtrees/absent masks release canvas-size storage if
water/underwater also do not need it. Views snapshot before transparency with 1x/4x,
perspective/orthographic, resize and recovery support. Soft-only views add capture
and continuation passes plus final 1x presentation; color is captured alongside depth.
HDR/bloom, collision, fluid solvers and GPU simulation remain extensions.
For the delivered FX editor and isolated preview see [FX authoring](fx-authoring.md).

Format 17 archives retain definitions/static mesh references and live turbulence
phases; CDFs rebuild on load. Formats 1–16 keep legacy defaults (Box, Camera, new
motion/fading off). Clone/editor Play use the same codec. Deploy host and worker
together. Read engine `docs/fx.md`, `docs/fx-validation.md`, README FX contracts and
`architecture.md`; verify availability in the consuming game's selected engine pin.
The new playground preset is **Visual effects → Fountain and mist** (`fountainEffect`).

Build via `./scripts/build.ps1 -Configuration Release` using the repo `.tools/emsdk`
and `.emscripten-version` (6.0.11). Build/run CTest `fx`, `scene_archive`, `persistence`,
`water`, `terrain`; Windows native compiler runtime DLLs may require its bin on PATH.
Run existing `scripts/fx-smoke.cjs`, new `scripts/fx-milestone-smoke.cjs` (both forced
backends) and `scripts/water-depth-smoke.cjs`, using Playwright/pngjs, Chrome and
`DEMO_URL` for the exact staged host/Wasm. Check depth fades, alpha/additive, stretch,
1x/4x, ordered views/masks, resize/recovery, archives and public controls. Desktop
Chrome evidence is not physical-mobile or sustained-performance acceptance.

Archive v18 adds optional startDelay; see [authoring](fx-authoring.md).

Ribbon mode and archive v19: see [ribbon trails](trails.md).

Plane alignment and archive v20: see [plane particles](plane-particles.md).

Static mesh particles and current archive v21: see [mesh particles](mesh-particles.md).
Soft-distance fading remains supported on quad/ribbon particles, not Mesh mode.

Particle contacts and projected decals: [integration](fx-contacts-decals.md). Collision is optional; applications own queued-impact consumption and decal lifetime.

Event-driven child effects and managed fading decals: [composition contracts](fx-composition.md) and [validation](fx-composition-validation.md). Use native fx_composition plus FX/persistence/lifecycle regressions, and scripts/fx-composition-smoke.cjs on WebGPU/WebGL2 at DPR 1 and 2. Archive v29 retains nested dependencies and live continuation; events/counters remain transient.

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

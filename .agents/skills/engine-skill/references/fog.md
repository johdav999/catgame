# Fog integration

## Direct D3D12 environment rendering (2026-10-22)

Scene::setSky and setFog atomically validate optional presentation settings.
Direct milestone 3 shares native Dawn/browser sky and fog payloads. Sky supports
gradient, sRGB panorama (U repeat/V clamp), atmosphere/clouds/day-night and inward
sphere/fullscreen geometry; applications explicitly advanceSky on their clock.
World meshes and sky receive height/start fog; additive meshes extinguish without
haze, Background/Overlay and retained GUI are excluded. DesktopView skyEnabled/
fogEnabled are independent. Direct FX now supports billboards, planes, ribbons, surface trails and mesh particles.
RenderQuality::post enables format-checked FP16 scene targets and independent HDR
MSAA resolve, quarter-size bloom and SDR exposure/Reinhard/saturation/contrast/sRGB
composition before GUI. Default off; zero bloom skips extraction. Diagnostics
effectivePost/effectiveMsaaSamples preserve the requested quality distinction.
See [native contracts](windows-d3d12.md), [post controls](lighting-post-processing.md)
and [native/browser validation](windows-d3d12-milestone3-validation.md).

## Direct D3D12 animated rendering (2026-10-22)

Native direct D3D12 milestone 4 now renders final skin/morph palettes from the
existing portable player, layers, blend trees, IK and retargeted clips. Animation
evaluation/clocks, target and movement policy stay with their existing owners.
Four-influence GPU skinning uses structured BoneUniform palettes up to 1024
entries and inverse-transpose normals/tangent axes; cached CPU morphing runs first
in color and shadow passes. Unchanged pose revisions upload nothing. Weak GPU
pose caches and submission fences preserve lifetimes; camera-list changes release
inactive collected references. Culling uses matching bounds and keeps full source
detail. Fog/HDR/MSAA/material alpha/winding remain composable. Browser WebGPU
retains GPU skinning; WebGL2 retains CPU deformation and per-draw vertex uploads.
See [animated rendering](windows-animation-rendering.md) and
[milestone 4 validation](windows-d3d12-milestone4-validation.md).
This does not advance a consumer pin or establish physical mobile acceptance.

## Direct D3D12 water (2026-10-22)

Milestone 6 reuses MaterialShader::Water/WaterSettings, waterParameters,
waterReflection, underwaterView, WaterInteractionParameters and prepareUnderwater.
Direct water supports Gerstner displacement after morph/skin, moving normals,
Fresnel/IBL, depth refraction/foam, mirrored opaque/cutout World captures, reflected
height fog, underwater absorption/scattering/caustics and transient rings/wakes.
Mirrored double-sided PSOs preserve front-face normals independently of culling.
Viewport-clamped/scissored snapshots isolate cameras. At 4x, color averages and
depth takes the nearest sample; 1x loads directly. Underwater runs after completed
World and before Overlay/GUI. HDR inputs stay linear; non-HDR 1x accumulation copies
to the swapchain before GUI. Fence-safe targets shrink to 1x1 when effects are off.
Object constants are now 560 bytes in 768-byte slots aligned to 256 bytes; frame
payloads are 2352 bytes, sharing the existing bounded 64 MiB upload arena.

DesktopHost::setWaterInteractions(span) validates/copies up to 16 existing portable
WaterInteraction values after scene activation or from frame callbacks. Native
water uses Application simulation seconds, independent of GUI presentation; use
that same clock for startTime. Pause/suspension freeze waves, caustics and contact
ages. View edits/recovery retain snapshots; successful scene handoff clears them.
Invalid snapshots preserve previous values. Inputs borrow no nodes/callbacks and
are not archived. Applications own contact detection/emission and reconstitution.
Browser Renderer clocks remain caller-owned. Water surface defaults remain .12 m
amplitude, 3 m wavelength, speed 1; refraction/foam/planar/underwater/caustics are off
by default. Geometry must be sufficiently subdivided; height/displacement never
changes simulation or collision. Rectangular underwater and one reflection plane
per view remain approximations; transparent reflection sources are excluded.

Launch webengine_water_windows --backend dawn|d3d12 for retained water controls,
moving wake/contact emission, pause, HDR/MSAA and --capture image.ppm. F11 toggles
fullscreen; Esc exits; capture parent must exist. The example enables water scene
effects and HDR only as application policy. Run windows-water-smoke.ps1 for paired
63-case native pixels/counters, plus water/material/render_frame/desktop_api tests.
Run water-smoke, water-depth-smoke, water-planar-smoke, water-underwater-smoke,
water-caustics-smoke and water-interactions-smoke against fresh pinned local SDK
artifacts on explicit WebGPU/WebGL2. Report native/browser separately. No physical
mobile, other-GPU, sustained-performance or direct device-recovery claim follows.
Milestone 8 delivers mesh instancing and semantic BC7 uploads.
No fluid simulation, buoyancy, automatic emission or new archive version is added.

Read [native scope](windows-d3d12.md) and [milestone 6 validation](windows-d3d12-milestone6-validation.md).
Verify the consumer pin; skill documentation does not advance a game dependency.

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

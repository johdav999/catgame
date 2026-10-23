# Windows desktop host

Development WebEngine supplies optional `engine_desktop` / `WebEngine::Desktop`
with public `engine/desktop.hpp`. Read selected-engine docs/windows-desktop.md,
docs/windows-native.md and docs/windows-native-validation.md; verify availability
in the chosen checkout. This skill update does not advance CatGame's pin or add
a Paws Windows host.

## Build and validate

```powershell
./scripts/setup-windows.ps1
./scripts/build-windows.ps1 -BuildDir build/windows-desktop
./build/windows-desktop/Release/webengine_windows.exe
./scripts/windows-smoke.ps1 -BuildDir build/windows-desktop -EvidenceDir build/evidence/windows-desktop
./scripts/windows-smoke.ps1 -BuildDir build/windows-desktop -EvidenceDir build/evidence/windows-desktop-glb -Model assets/formula1-red-driving.glb
ctest --test-dir build/windows-desktop -C Release -R '^(desktop_text|desktop_api|application|input|gui)$' --output-on-failure
```

Default BuildDir is build/windows; use a separate directory while an older app is
open. Setup hash-verifies SDL 3.4.18, Dawn v20261016.190333 and Microsoft DXC
v1.9.2609. Windows x64 MSVC Release is covered; default generator is VS18 2026.
Keep SDL3.dll, dxcompiler.dll, dxil.dll, bundled font/notices and the staged
assets/fonts and assets/audio directories beside the executable.
Jolt uses the same dynamic MSVC CRT as the host. Browser/core builds do not require
desktop SDKs. The shared browser build still uses repository-local Emscripten 6.0.11.

## Public API and contracts

DesktopHost owns Application, GuiSystem, SDL window/event/input adapters and
renderer through a pimpl. Public headers/link usage require no SDL/Dawn/private
include directories. One host owns the process SDL event queue, on its creation
thread. `application()` exposes ordinary scene loading/preparation/transitions;
the host reserves Application's scene-binding slot. Register copied DesktopView
camera handles with setViews before start/transition. Candidates validate before
replacement; transitions release outgoing borrows before scene unload, stale
cameras omit safely at presentation, shutdown releases graphics before scenes.

`step(callbacks, optionalMonotonicSeconds)` polls/services platform state, calls
Application::advance once and renders after the application's present callback.
`run(callbacks)` supplies host timestamps. Application alone owns clocks/physics.
Step rejects reentry; callbacks are synchronous and their captures must remain
valid while installed/used. requestClose is safe inside GUI/frame callbacks and
commits cancellation/shutdown at an idle boundary. Native capture callbacks own
their buffers, and shutdown explicitly cancels pending captures.

`bindings()` uses existing InputBindings with stable controlId(DesktopControl::W)
etc.; games choose action meanings/contexts. GUI gets pointer/wheel/key ownership
before gameplay, consumed controls cancel held sources, and GUI drags cannot leak
to scene pointers. Focus loss/minimize cancels GUI capture/focus and component
commands through InputCancelled, suspends clocks, and preserves explicit pause.
setPaused and setInputEnabled provide menu gates. Gamepads rearm from neutral;
sticks use a 15% and triggers a 5% dead zone. Modal navigation stays usable
while driving axes await neutral. Native text/clipboard/SDL preedit integration
is delivered through the same TextInput; native accessibility remains future work.

`setQuality`, setShadowsEnabled, fullscreen/window controls, diagnostics and
asynchronous captureFrame preserve simulation/presentation separation. Desktop
rendering shares WebGPU renderer/water/GUI code and WGSL with the browser through
private WebGpuHost. Desktop submission timing includes native presentation.
Read [detailed desktop API guidance](windows-desktop.md).

## Vehicle menu and evidence

examples/windows_vehicle.cpp and windows_vehicle_app.hpp use the public API and
shared vehicle course. P/Escape/Pad Start or Pause/settings opens a modal menu;
settings apply during pause and survive Resume. Shared Button/Checkbox/Slider/
ScrollPanel controls expose shadows/MSAA/resolution/fullscreen/defaults, with
fixed Resume/Reset/Quit outside scrolling content. Mouse, Tab/Shift+Tab,
Enter/Space and arrows operate the widgets. Quit shuts down safely. GLB drop or
--model stages a validated static preview without replacing the vehicle.

desktop_api_tests is a public-header consumer testing validation, scene unload,
stale cameras, pause, exception/reentry and shutdown/capture cancellation.
webengine_windows_smoke is a separate SDL test driver for the same app, not part
of the interactive executable. Its real hidden-window D3D12 captures and synthetic
SDL input verify driving, menu ownership/settings, keyboard Resume/Quit, small
window scrolling, fixed actions and blur cancellation. PPM/PNG and JSON/hashes
are written under the requested evidence directory. Shared renderer edits still
need both-backend browser acceptance on a frozen matching site. No physical
controller/monitor/fullscreen/installed-IME-UI/accessibility, broad driver, native editor,
installer/distribution or sustained performance acceptance is implied.

## Native single-line text

Use the same GuiSystem TextInput and trusted font. Focus starts OS text input;
blur/hide/minimize/focus loss stop it. Text focus cancels held gameplay sources,
including programmatic focus while running. Native UTF-8 selection supports
mouse drag, Shift arrows/Home/End, Backspace/Delete, Ctrl+A/C/X/V and Ctrl word
navigation/deletion. Clipboard CRLF/LF/CR/Tab normalize to spaces. Native payloads
and resulting edits are bounded to 32 KiB and invalid UTF-8 preserves valid state.
IME preedit replaces saved selection; cancellation restores committed state unless
application edits have changed the revision. Stale queued text is flushed on focus
handoffs. No retained widget pointers or asynchronous callbacks escape the adapter.
Observe composing() in onChange; composition is transient until committed.
Escape cancels preedit first, then follows normal menu policy. Enter submits.
Keep driver names and menu/shortcut policy in application code; names are local to
the vehicle session. D-pad chooses/adjusts controls, A activates, B returns, Start
toggles the menu; driving controls still require neutral after cancellation.

Run desktop_text with desktop clipboard access (it temporarily replaces/restores
clipboard text). Native smoke additionally covers text ownership and a real SDL
virtual gamepad. Browser gui-smoke.cjs and vehicle-smoke.cjs run serially on
WebGPU and forced WebGL2 against fresh staged artifacts. See
[GUI acceptance and limitations](windows-desktop-gui-validation.md). Text is
single-line/codepoint based: no undo, grapheme navigation, bidi/shaping, multiline
or Windows accessibility. Synthetic SDL composition and virtual controllers do
not establish installed IME candidate UI or physical controller acceptance.

## Native platform services

WebEngine::Platform supplies DesktopFiles and DesktopAudioBackend without a
renderer/scene owner; DesktopHost supplies modal showFileDialog. Use
[platform contracts](windows-platform-services.md) and [acceptance](windows-platform-validation.md).
Files default to executable/assets and SDL per-user saves/settings, with explicit
root overrides. Directory loaders copy paths/decoder, run synchronously and publish
through AssetManager. Atomic byte storage preserves old files on failure. Archives
retain schemas/validation/migration; checkpoint/auto-save policy stays in games.
AudioSystem remains application-owned. Its native backend lazily opens SDL output
and supports PCM/WAV loops/rate/gain/stereo positioning; local encoded streams and filter/room DSP are now implemented through the
[Windows audio adapter](windows-audio-environment.md). HRTF remains unsupported. Device callbacks retain only owned PCM/state. Inspect diagnostics
and suspend audio explicitly on host/menu/dialog policy. Runtime voices stay transient.
Native Open/Save is synchronous, owner-thread, idle/GUI-events only; it validates
before cancelling input/suspending clocks and preserves pause with fresh baselines.
Vehicle settings now persist committed names/render/volume; explicit checkpoints
restore at rest across launches. Open model/WAV/checkpoint and Save as use native
dialogs. --assets and --user-dir select explicit roots. Build in windows-services
to keep an earlier executable open. Run 12 focused CTest suites, opt-in native
dialog test and vehicle/GLB smoke. Regress shared audio/vehicle on both browser
backends serially. Physical speakers/IME/controllers and broad drivers need own UAT.

The separate [broader acceptance runner](windows-acceptance.md) now tests native
animation, terrain, water/underwater, particles, lighting/shadows, HDR post and
MSAA with actual framebuffer comparisons. It also uses real resize, fullscreen
and minimize/restore operations and induced Dawn device loss. Its report records
which physical controller, mixed-DPI, sleep/wake and driver cases remain open.


Native scene editing is now available through [DesktopEditor](windows-editor.md);
its specialized-panel limits are explicit. Build scripts/build-windows.ps1 and
launch build/windows/Release/webengine_editor.exe. See [parity acceptance](windows-parity-validation.md).

## Selectable Windows graphics backends (2026-10-22)

DesktopSettings::backend selects DesktopBackend::Dawn (default) or Direct3D12 at
startup; backend() reports it. Milestones 1-8 deliver shared vehicle/native GUI,
resize/borderless fullscreen/captures, static conventional PBR/IBL, five material
maps with semantic mips/wrapping, first eight directional/point/spot lights, four
directional/cascade/spot shadow maps, cutouts, sorted transparency, winding and
checked 4x MSAA. Reuse Material/Texture/Environment, RenderFrame/FxFrame/planShadows.

Milestone 3 reuses Scene::setSky/setFog, SkyBackground/FogSettings, prepareSky/
prepareFog and skySphereMesh. Gradient/panorama/atmosphere/clouds/day-night payloads
render as fullscreen or inward sphere. Only applications call advanceSky; the
renderer never advances clocks. Fog affects static World meshes and sky, respects
per-view flags and additive extinction; Background/Overlay and GUI stay outside.
RenderQuality::post is off by default. Supported FP16 render/blend/sample/load
enables HDR scene rendering, optional quarter-size bloom, exposure/Reinhard/
saturation/contrast/sRGB composition, then GUI. Bloom may cross view boundaries.
Zero bloom skips extraction. This is SDR presentation, not HDR10 monitor output.
hdrPostProcessing reflects queried support; HDR 4x/resolve is checked separately.
Requested quality stays intact; diagnostics effectivePost/effectiveMsaaSamples
report the result. Attachment changes fence old resources and preserve captures.

Milestone 4 consumes existing SkeletalMeshComponent/AnimationComponent,
SkinPose/BoneUniform, SkinVertex and MorphPose without another scheduler or public
renderer API. Four-influence GPU skinning reads up to 1024 structured palette
entries (including identity slot zero); inverse-transpose normals and both tangent
axes retain handedness. Cached CPU morph vertices precede skinning in color and
shadow passes, including alpha modes, normal maps, mirrored placement, fog/HDR/MSAA.
Camera/shadow bounds match deformation; static LODs do not simplify these draws.
Independent players/weights remain per component; clips/layers/trees/IK/retargeting
continue to produce ordinary shared palettes. Applications still apply root motion.
Changed pose revisions copy once to reusable GPU buffers; unchanged poses upload
nothing. Pose caches weakly reference CPU sources and retire buffers behind fences.
Changing view lists clears collected references/LOD history; unbinding releases
collected CPU resources. Each pose cache caps at 4096; staging shares the 64 MiB
frame upload arena with object/view/GUI data. Invalid sizes/indices/nonfinite data
reject before recording, including when culling is disabled. No GPU clip sampling,
dual-quaternion skinning, cloth or new import format is added.

Milestone 5 uses existing MaterialLayers, materialTextures/TextureSemantics,
materialLayerParameters and TerrainComponent meshes without new portable APIs.
Four sRGB albedos, linear RGBA weights and four packed RGB-normal/alpha-height
maps support semantic mips, individual wrapping, global/per-layer UVs, weighted
color/alpha/roughness, active-layer height blending and signed world triplanar
projection. Missing/zero weights select layer zero; missing color is white and
missing relief is flat/zero. Conventional maps are inactive while layered;
scalar metallic/emissive, tint, alpha modes, PBR/IBL, shadows, fog/HDR/MSAA remain.
Static and skin/morph draws share matching layered cutout shadows. Nine-descriptor
tables include layered semantic roles in cache keys; object constants use 448
bytes in 512-byte aligned slots within the existing 64 MiB frame upload bound.
Shared camera/shadow LOD stays independent, including automatic projected-size
selection. Rendering never changes terrain ground/collision, paging or simulation.
Direct counters add layeredDrawCalls, colorTriangles, shadowTriangles and camera/
shadow LOD parts; these are collector/submission counters, not GPU timings.

Direct mesh instancing and semantic BC7 uploads are now supported. Device loss queues bounded reacquisition. No silent
backend fallback/live switching, full renderer parity or speed claim is made.
Keep GPU types private and simulation independent. Verify consumer pins; this
does not advance CatGame's pin or create a Paws Windows host.

Build scripts/build-windows.ps1 -Backend Both|Dawn|D3D12 (Both default); direct-only
excludes Dawn lookup/linkage. Launch webengine_windows or webengine_lighting_windows
with --backend dawn|d3d12. The latter adds material/lighting/environment/post GUI
controls and --capture image.ppm (parent must exist); F11 fullscreen, Escape exits.
Run windows-animation-smoke.ps1, windows-environment-smoke.ps1, windows-lighting-smoke.ps1 and
windows-backend-smoke.ps1 plus native desktop_api/desktop_api_d3d12/render_quality/
adaptive_quality. WEBENGINE_D3D12_VALIDATION=1 requires Windows Graphics Tools.
Also launch webengine_animation_windows --backend dawn|d3d12 for playback/blend/
pose/morph controls and deterministic --capture image.ppm; F11 fullscreen, Esc exits.
Run browser morph-smoke with BACKEND=webgpu|webgl2, skeletal-animation-smoke
(WebGPU palette uploads) and animation-layers-smoke (both). WebGL2 CPU deformation
still uploads vertices per draw. Rerun sky-smoke/sky-weather-smoke/fog-smoke/lighting-post-smoke on forced
WebGPU/WebGL2 against the pinned local Emscripten build. WebGL2 retains basic
lighting, not PBR/IBL. Use fresh pixel/counter evidence and separate native/browser
results; neither establishes physical mobile or unsupported-adapter acceptance.

Read [backend contracts](windows-d3d12.md), [sky](sky.md),
[fog](fog.md) and [milestone 3 evidence](windows-d3d12-milestone3-validation.md).
Earlier milestone evidence remains historical.

Read [animated rendering](windows-animation-rendering.md) and
[milestone 4 evidence](windows-d3d12-milestone4-validation.md).

Launch webengine_terrain_windows --backend dawn|d3d12 for retained layer/relief/
projection and independent camera/shadow LOD controls. --capture image.ppm makes a
paused GUI capture; F11 toggles fullscreen, Esc exits. Run windows-terrain-smoke.ps1
for paired 46-case native pixels and counters, plus terrain/material/render_frame
and desktop_api tests. Run browser terrain-surface-smoke.cjs on fresh explicit
WebGPU/WebGL2 artifacts for layered surfaces, LOD, collision invariance, archive
round trips and recovery. WebGL2 supports shadows with basic lighting, no PBR/IBL.
Read [terrain](terrain.md) and [milestone 5 evidence](windows-d3d12-milestone5-validation.md).

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

Read [water](water.md) and [milestone 6 evidence](windows-d3d12-milestone6-validation.md).

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

Direct milestone 8 adds bounded device recovery, general mesh instancing and
semantic BC7 uploads through existing portable resources. Typed removed/reset/hung/
driver errors queue owner-thread same-backend reacquisition; ordinary invalid data,
budget failures and user exceptions do not trigger it. Release old GPU/swapchain/
readback resources before replacement. Keep scene-scoped camera handles, GUI/font
state, quality/shadows, simulation clock and water snapshots. Resolve handles again;
skip removed cameras. Cancel pending direct captures with captureError. New devices
recheck capabilities and lazily upload all assets/poses/FX. Host unavailability
cancels held input, freezes advancement and preserves Running/Paused without catch-up.

There are three automatic attempts per unstable cycle, bounded backoff starting at
50 ms, and a budget reset after 120 successful presentations. Exhaustion reports an
error; retryGraphics starts a new cycle. Do not reacquire after unbinding/shutdown.
Diagnostics expose generation/deviceRecovery/recoveryAttempts/lastDeviceLoss and
monotonic presentation counts; ready gates use. Opted-in idle simulateGraphicsLoss
can call real ID3D12Device5::RemoveDevice when available. It is not an OS reset.
Dawn retains its existing recovery/capture behavior; strict direct never switches API.

Direct RenderFrame camera/shadow/reflection batches now instance compatible opaque/
masked parts, including material maps/layers/water and shared skin/morph poses.
Different poses/bindings/winding/ranges split batches; sorted transparent meshes
stay individual. allowInstanceReordering defaults false; opt in only when opaque
equal-depth precedence may change. Existing ObjectUniform fields use 256-byte slots
and dirty-range GPU copies, with zero paused object uploads. Per-view streams retain
no nodes and retire behind fences. Instance staging caps at 32 MiB within the
combined 64 MiB arena; draw/descriptor/resource limits still apply. Direct counters
submittedInstances/instancedDrawCalls/maxInstancesPerDraw count mesh submissions
across color/shadow/reflection; FX has separate counters. Do not infer speedup from
draw counts or assume cross-backend clear/pass counters have identical scopes.

RenderQuality.compressedTextures defaults true. Direct queries BC7 linear/sRGB
support and chooses the first supported Texture::compressed variant matching its
TextureSemantic. Validate all variants before publication: 4-aligned base <=4096,
complete halved logical mips through 1x1 and exact 16-byte 4x4 blocks. Device copy
footprints handle block rows/padding/tail mips. Color is BC7 sRGB; data/normal linear.
Keep RGBA pixels for disabled/unsupported compression, archives and other backends.
ETC2/ASTC use RGBA fallback on D3D12. GUI forces a separate RGBA cache identity.
Compression changes fence invalidated texture/material bindings. No runtime encoder,
transcoder, compressed archive payload or new public renderer/texture API is added.

Build scripts/build-windows.ps1 and a separate -Backend D3D12 tree. Run
windows-performance-smoke.ps1 for paired 22-case native pixels, real device removal,
257-to-3 direct color draws, BC7/RGBA selection and retained scenes/clocks/resources.
Run native desktop_recovery/desktop_fallback/desktop_api/desktop_api_d3d12/
instancing/render_quality/material/render_frame; repeat recovery/API in the Dawn-free
build. Regress FX/water/terrain/animation/lighting/vehicle. Browser Release uses
the pinned repository-local SDK; run render-quality-smoke, instancing-smoke and
render-fallback-smoke on fresh frozen outputs, with explicit WebGPU/WebGL2 coverage.
Record platforms separately. Other-GPU, physical mobile, sustained performance and
cross-device timing comparisons remain separate. Consumer pins/Paws hosts do
not change with this skill documentation.

Read [recovery/performance contracts](windows-d3d12-recovery-performance.md) and [milestone 8 evidence](windows-d3d12-milestone8-validation.md).


## Direct D3D12 GPU profiling

Use [GPU profiling](windows-gpu-profiling.md) for the current direct
timestamp/query contract and on-demand DesktopHost::gpuProfileTrace export.
diagnostics().performance includes shared gpu summaries plus gpuProfile nested
frame/view/pass events, inclusive/self times, sampled metadata and rolling 120-sample
statistics. Three fenced readback slots sample at 10 Hz without profiling waits;
scene/view/quality/attachment epochs and device recovery reject stale results.
WEBENGINE_GPU_TIMING=0 disables native queries/readbacks at device creation on
direct and Dawn. Browser WebGPU keeps ?gpuTiming=off; WebGL2 remains unavailable.
GPU clocks are not CPU-correlated. Keep GPU types private and simulation unchanged.
Run scripts/windows-gpu-profile-smoke.ps1 for both native backends plus a Dawn-free
build, and browser performance-smoke, performance-timeline-smoke and
render-fallback-smoke on fresh pinned output. Check actual timing hierarchy, trace
eviction, loss/recovery and exact profiling-on/off captures. No throughput or
other-GPU acceptance follows. Verify the consumer pin before using new APIs.

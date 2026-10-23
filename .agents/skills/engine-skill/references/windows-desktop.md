# Reusable Windows desktop host

Optional `engine_desktop` (`WebEngine::Desktop`) supplies `engine/desktop.hpp`.
It owns an SDL3 window/event dispatcher, input adapters, retained GUI, Application
and a selected private graphics backend. Dawn shares browser WebGPU/WGSL;
Direct3D12 supports conventional PBR/lighting/shadows, skin/morph, sky/fog/HDR/MSAA and vehicle/GUI rendering. Public headers contain no
SDL, HWND or Dawn types. Link the public target; consumers need no private source
or SDK include directories. Windows x64 MSVC Release is currently supported.

## Minimal application

```cpp
#include "engine/desktop.hpp"

int main() {
    engine::DesktopSettings settings;
    settings.title = "My game";
    engine::DesktopHost host(settings);
    auto scene = std::make_unique<engine::Scene>("Level");
    auto& camera = scene->spawn("Camera").addComponent<engine::CameraComponent>();
    camera.transform.position = {0, 2, -8};
    const auto sceneId = host.application().loadScene(std::move(scene));
    const engine::DesktopView view{camera.id()};
    host.setViews(sceneId, {&view, 1});
    host.application().start(sceneId);
    return host.run();
}
```

Link `target_link_libraries(my_game PRIVATE WebEngine::Desktop)` in an engine
build configured with `WEBENGINE_BUILD_WINDOWS=ON`. The public API test consumer
demonstrates this without linking SDL or Dawn directly. The desktop implementation
handles SDL main initialization; application entry points use ordinary C++ `main`.

Build/setup commands and pinned runtime dependencies are in
[the Windows guide](windows-native.md). `scripts/build-windows.ps1 -BuildDir
build/windows-desktop` provides an independent build directory. The equivalent
smoke command takes `-BuildDir build/windows-desktop` too.

## Ownership and frame execution

- One DesktopHost owns the process's SDL event queue and one window. All host,
  Application, GUI and rendering operations occur on its creation thread.
  Concurrent hosts reject before acquiring SDL subsystems. Multiple windows and
  integration with an externally owned SDL event dispatcher are not delivered.
- The host owns Application and its scene-binding slot. Use `application()` for
  loading, preparation, start, transitions, pause and shutdown; do not replace its
  scene-binding callback. `setViews(SceneId, span<DesktopView>)` copies validated
  camera handles/composition settings. Unknown/foreign scenes, invalid cameras and
  malformed view/quality candidates reject before replacing valid state.
- Scene binding resolves handles before activation. Transitions rebind before
  outgoing scene destruction; shutdown releases the renderer before scenes.
  A camera removed during simulation is omitted on the next presentation instead
  of dereferencing an expired pointer. Register views before entering a scene.
- `step(callbacks, optionalTimestamp)` polls SDL events, services graphics work,
  calls Application::advance once and renders after `callbacks.present`. Callers
  supply gameplay input/simulation/camera policy through the existing
  ApplicationFrameCallbacks. Optional timestamps are monotonic seconds for tools
  and deterministic tests; ordinary `run(callbacks)` uses a monotonic host clock.
  Do not advance Application/scene/physics again from these callbacks.
- Step is non-reentrant. Exceptions from user callbacks unwind host guards and
  propagate to the caller. Changing scene view configuration requires an idle
  Application boundary; rendering settings can change before rendering a frame.
  Initial graphics acquisition is asynchronous; diagnostics().ready reports it.
  Initialization failures throw from step with the graphics diagnostic.
- `requestClose()` is safe inside GUI/frame callbacks. The host completes the
  current safe boundary, cancels input and calls Application::shutdown before
  returning false. Destruction clears event callbacks before releasing runtime
  owners. Callback captures must remain valid while installed/used; application
  code owns frame and GUI callback capture lifetimes.

## Input and GUI

`bindings()` returns the existing portable InputBindings. Bind stable
`controlId(DesktopControl::W)` etc. to application-defined actions and install the
usual context stack. There are no default gameplay bindings. Keyboard letters,
arrows, Space, Shift, Escape, Enter, Tab, F11, mouse buttons/wheel and one gamepad
map through public engine control IDs. Unconsumed mouse records include logical
and render-pixel positions in Application's input snapshot. Wheel bindings should
use the displacement flag. Gamepad sticks have a 15% dead zone, triggers 5%; games
own forward/reverse/brake meanings. Reconnection/control cancellation requires
neutral axes and released action buttons before driving resumes.

`gui()` owns the existing retained widget tree and font. The host lays it out in
window logical coordinates, forwards mouse capture/drag, wheel and supported GUI
keys before gameplay mappings, and renders it through the shared GUI renderer.
Tab/Shift+Tab, Enter/Space, arrows and Escape use GuiSystem's existing behavior.
Gamepad D-pad up/down traverses focus, left/right adjusts controls, and south/east
activate/cancel. Modal controller navigation remains available while held driving
axes wait for neutral; Start can always open the application menu. GUI-consumed
actions cancel held gameplay sources; a GUI-owned
mouse drag cannot leak into scene pointer records.

`setEventHandler` receives copied portable control, file-drop, focus, resize,
cancellation and close events. Returning true consumes a control or vetoes a
close request; `guiConsumed` records GUI ownership. Escape/P/F11 behavior is
application policy. `cancelInput`, `setPaused`, `setInputEnabled`, scene boundaries,
focus loss and minimize cancel the relevant input ownership. The synchronous
InputCancelled event lets applications clear held component commands immediately.
Focus/minimize also suspends all Application clocks, cancels GUI capture/focus,
and preserves explicit pause on restore. Input cannot accumulate hidden-time work.

### Native text input

Focusing an existing `TextInput` automatically starts SDL OS text input; blur,
hidden/clipped controls, focus loss, minimize and cancellation stop it. No public
SDL API or second widget system is needed. Applications load the shared GUI font
and create/focus widgets normally. `onChange` and `onSubmit` use existing contracts.
Typing is consumed before application driving shortcuts; event handlers must
respect `guiConsumed`. Text focus also cancels held bindings and gates gameplay
axes, including when focus is acquired programmatically without pausing.
Tab/Shift+Tab moves focus and Enter submits. Escape cancels
an active IME preedit first; a subsequent Escape follows normal GUI/menu policy.

The private adapter accepts UTF-8 text commits, mouse caret/selection, Shift with
Left/Right/Home/End, Backspace/Delete, Ctrl+A/C/X/V, Ctrl+Left/Right and Ctrl with
Backspace/Delete. Selection offsets remain UTF-8 bytes; movement/deletion uses
codepoint boundaries. Word movement uses ASCII whitespace. Clipboard/preedit
payloads and resulting native edits are bounded to 32 KiB; malformed/oversized
input preserves the previous value. Pasted CRLF, CR, LF and Tab become spaces.
AltGr text comes through SDL text events rather than guessed keyboard characters.

SDL composition offsets convert from characters to UTF-8 bytes. Repeated preedit
replaces the same saved selection, commit replaces it once, and cancellation
restores committed text/selection. Programmatic changes to text or selection win
over pending preedit. Switching/losing focus discards queued text for the old
widget. Composition can temporarily invoke `onChange`; inspect `composing()`
before publishing committed application values. The candidate area follows the
textbox/caret in logical window coordinates. OS services stop before window/GUI
destruction, without calling application widget hooks during teardown.

This is single-line editing, without undo/redo, grapheme segmentation, shaping,
bidi or multiline editing. Native Windows accessibility uses
[UI Automation](accessibility-input.md). Automated SDL preedit
tests establish adapter behavior, not acceptance of a physical keyboard layout,
installed IME candidate UI or every Windows input method.

## Presentation and diagnostics

`setQuality` validates a complete RenderQuality candidate and preserves the
existing shared resolution/MSAA/post/LOD contracts. `setShadowsEnabled`,
`setFullscreen`, `setWindowSize`, `size` and `capabilities` provide presentation
control. These never alter physics state. `diagnostics` returns readiness, actual
render dimensions, effective MSAA, frame count and existing render/performance
reports. Render scale affects framebuffer size; input/GUI use logical coordinates.
`retryGraphics` requests the existing bounded recovery mechanism after a graphics
failure. Desktop performance submission timing includes native presentation.

Acceptance tools may opt in through `DesktopSettings::allowGraphicsFaultInjection`
(default false) and call `simulateGraphicsLoss()` at an idle boundary. Disabled
or reentrant calls throw `logic_error`; a renderer that is not ready returns false.
The call cancels input and destroys the actual Dawn device. The existing lost
callback reacquires a device and rebuilds GPU resources from shared CPU assets.
This is an induced device loss, not a Windows driver reset/TDR. Native renderState
JSON includes the selected adapter's vendor/device/description, IDs and software
flag. See [the acceptance runner](windows-acceptance.md).

`size()` returns logical content units, with display scale included in pixelRatio.
The render extent remains subject to RenderQuality resolution limits. Initial
width/height and `setWindowSize` use SDL window coordinates (physical pixels on
Windows). Mouse/wheel positions are converted to logical GUI units, then to
backing pixels for scene pointers. Native IME rectangles convert back to window
coordinates. Display-scale events relayout without advancing simulation.

Native power suspend/resume notifications cancel input and reset frame timing at
an idle boundary, preserving explicit pause. Resume waits for focus and restore
when necessary. The window hook owns no application callbacks and is removed
before the host destroys its window.

`captureFrame(path)` requests one asynchronous PPM readback; poll capturePending
and captureError while stepping. The callback owns its readback buffer/data and
does not borrow application objects. Shutdown cancels pending capture and exposes
its cancellation through captureError. `baseDirectory` resolves bundled files.
Capture/diagnostic APIs are suitable for acceptance tooling, not a claim of a
complete resource/loading/file service.

## Vehicle pause/settings example

`examples/windows_vehicle.cpp` is a public API entry point.
`windows_vehicle_app.hpp` owns course creation, commands, chase-camera policy,
model preview and menu callbacks. It shares `src/vehicle_example.hpp` with the
browser. Click Pause/settings or press P/Escape/Pad Start. The modal menu pauses
simulation, cancels held driving input and exposes shadows, 4x MSAA, resolution,
fullscreen, defaults and a persisted driver-name textbox. Driver names update
the HUD after committed edits; Enter returns focus to Resume. P and R typed into
the textbox cannot trigger menu/reset shortcuts. D-pad chooses/adjusts controls,
A activates, B returns and Start toggles the menu. Resume, Reset car and Quit stay outside the scrolling
settings area. Settings apply during pause and survive Resume. Test coverage
includes 960x540 and 640x360 logical viewports. Escape closes the menu; Quit exits.

SDL/Dawn code and synthetic SDL event injection live in implementation/test files.
`webengine_windows_smoke` uses the same application example with the public host;
the interactive executable contains no test driver.

See [native platform services](windows-platform-services.md) for audio playback,
asset/user directories, atomic archive byte storage and showFileDialog. The example
now retains settings and an explicit checkpoint across launches; it also opens
models/WAV/checkpoints through native pickers. See [GUI acceptance](windows-desktop-gui-validation.md)
and selected-engine `docs/windows-native-validation.md`. Native editor UI,
distribution is now available through [Windows packaging](windows-distribution.md). Publisher signing, clean-VM sign-off and broad driver acceptance remain separate gates. Broader
rendering/window acceptance is tracked separately in [Windows acceptance](windows-acceptance.md).

## Selectable Windows graphics backends (2026-10-22)

DesktopSettings::backend defaults to Automatic (Dawn D3D12 → Vulkan → D3D11).
Explicit choices stay strict; backend() reports the current candidate. Milestones 1-8 deliver shared vehicle/native GUI,
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

Direct mesh instancing and semantic BC7 uploads are now supported. Direct device loss queues bounded reacquisition. See [Automatic Dawn API
fallback](windows-graphics-fallback.md). Full renderer parity and speed comparisons are not established.
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

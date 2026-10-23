# WebEngine contract map

Paths are relative to the selected repository, not the skill. Read only relevant rows, then current headers/implementation. Use `rg --files` and `rg -n` to locate moved files, exact signatures and consumers. This is navigation, not a frozen feature inventory. Header-only names below refer to `include/engine/`.

| Task | Contracts | Source landmarks |
| --- | --- | --- |
| Planetary terrain and space-to-surface visual flight | `docs/planet-terrain.md`, `docs/planet-validation.md` | `planet.hpp`, `src/planet.cpp`, `examples/planet`, `cmake/planet.cmake`, `tests/planet_tests.cpp`, `scripts/planet-smoke.cjs`, `tools/verify_planet.py` |
| Scene authoring, component/hierarchy mutations, multi-selection/scale and isolated Play/Stop | `docs/scene-editor.md`, `docs/scene-editor-structure-validation.md` | `editor.hpp`, `editor_properties.hpp`, `editor_play.hpp`, `src/editor*.cpp`, `web/editor*.js`, `tests/editor*_tests.cpp`, `scripts/editor*-smoke.cjs` |
| Runtime, levels, pause and transitions | `docs/application-lifecycle.md` | `application.hpp`, `src/application.cpp`, `src/main.cpp` |
| Hierarchy, handles, lifecycle and transforms | README sections “Reusable scene objects”, “Actor identity, components and lifecycle”, “Component lifecycle, updates and settings” | `scene.hpp`, `node.hpp`, `actor.hpp`, `component.hpp` |
| Repeated objects/factories | `docs/prefabs.md` | `prefab.hpp`, `prefab_components.hpp` |
| Physical input, bindings, context stacks, virtual controls and contact ownership | README “Mouse, keyboard and mobile input”, `docs/input-validation.md`, `docs/ui-input-integration.md` | `input.hpp`, `web/input.js` |
| Game AI, navmesh revisions, clearance, recovery and perception | `docs/ai.md`, `docs/navmesh.md`, `docs/navigation-updates-validation.md` | `ai.hpp`, `navmesh.hpp`, optional `engine_ai`, `NavigationSource`, `Navigator::navigateSource`, `tests/navmesh_tests.cpp`, `scripts/navmesh-smoke.cjs` |
| AI state machines and graphical editor | `docs/ai-state-machines.md`, `docs/ai-state-machines-validation.md` | `state_machine.hpp`, `editor_ai.hpp`, `src/editor_ai_browser.cpp`, `web/editor-ai.js`, `state_machine_tests`, `scripts/editor-ai-smoke.cjs` |
| Hierarchical AI states | selected engine docs/ai-hierarchy.md and ai-hierarchy-validation.md | AiState parent/initialChild, StateMachine activePath/stateSeconds, transitioned callback, editor hierarchy forms, scripts/editor-ai-hierarchy-smoke.cjs |
| Typed AI event queues | selected engine docs/ai-events.md and ai-events-validation.md | AiEventDefinition/AiEvent/AiEventScope, StateMachine postEvent/currentEvent/eventHistory, AiControllerComponent adapters, editor-ai-events.js, scripts/editor-ai-events-smoke.cjs |
| Actor AI controllers, bindings, scene persistence and live debugging | `docs/ai-controllers.md`, `docs/ai-controllers-validation.md` | `ai_controller.hpp`, `AiBindingRegistry`, `aiControllerCodecs`, `EditorSession::setAiController`, `editor-ai-controller.js`, `ai_controller_tests`, `scripts/editor-ai-controller-smoke.cjs` |
| Typed AI blackboards | selected engine docs/ai-blackboards.md and ai-blackboards-validation.md | ai_value.hpp, StateMachineData::blackboard, AiBinding::sources, AiControllerComponent::setBlackboard, editor-ai-blackboard.js, ai_controller_tests, scripts/editor-ai-blackboard-smoke.cjs |
| Movement, jumping and grounding | `docs/movement.md`, `docs/physics.md` | `character_motor_component.hpp`, `rigid_body_component.hpp`, `collider_component.hpp` |
| Collision, queries and triggers | `docs/physics.md` | `physics.hpp`, `src/physics.cpp` |
| Heightfields, layered materials, seamless LOD, sculpt/paint transactions, source/resident ground, collision and bounded derived streaming, source paging and asynchronous generation, paged brushes and atomic dirty writeback | `docs/terrain.md`, `docs/terrain-detail-validation.md`, `docs/terrain-edit-validation.md`, `docs/terrain-paging-validation.md`, `docs/terrain-page-edit-validation.md` | `terrain.hpp`, `terrain_paging.hpp`, `terrain_browser.hpp`, `material.hpp`, `mesh.hpp`, `render_frame.hpp`, `src/terrain_demo.*`, `web/terrain.js`, CTest `terrain` / `terrain_paging` / `terrain_page_edit`, `scripts/terrain-smoke.cjs`, `scripts/terrain-detail-smoke.cjs`, `scripts/terrain-edit-smoke.cjs`, `scripts/terrain-paging-smoke.cjs`, `scripts/terrain-page-edit-smoke.cjs`, `web/terrain-storage.js`, `web/terrain-worker.js`, `tools/terrain_demo_pages.py` |
| Hinges, sliders, springs and physics grabs | `docs/physics-joints.md` | `physics_joint.hpp`, `physics.hpp` |
| Picking, actions, dragging and rotation | `docs/selection.md` | `interaction_component.hpp`, `selection.hpp`, `manipulation.hpp` |
| Follow/orbit/framing and camera collision | `docs/camera-controllers.md`, README camera contracts | `camera_component.hpp`, `camera_controller.hpp`, `orbit_controller.hpp` |
| Loading, cancellation, cache and preparation | `docs/assets.md` | `assets.hpp`, `asset_browser.hpp`, `fbx_importer.hpp`, `glb_importer.hpp` |
| Worker decode, GPU upload admission and KTX2/fallback delivery | `docs/mobile-assets.md`, `docs/assets.md` | `asset_decode.hpp`, `asset_browser.hpp`, `upload_budget.hpp`, `texture_bundle.hpp`, `src/asset_worker_browser.cpp`, `web/asset-worker.js`, `tools/texture_tool.cpp` |
| FBX/GLB imports and mesh optimization | README “FBX mesh importer”, “GLB mesh importer”, “Imported mesh optimization” | `fbx_importer.hpp`, `glb_importer.hpp`, `mesh_optimizer.hpp` |
| PBR maps, material overrides, IBL and baked graphs | README “Materials and image-based lighting”, `docs/material-graph.md`, `docs/materials-validation.md` | `material.hpp`, `material_graph.hpp`, `environment.hpp`, `texture.hpp`, `mesh_component.hpp` |
| Animated water, interaction ripples/wakes, underwater fog/absorption and caustics, planar reflections, depth-aware refraction/foam and caller-owned clock | `docs/water.md` | `water.hpp`, `MaterialShader::Water`, `MaterialInstance::water`, `Renderer::setWaterTime`, `src/water_demo.*`, `tests/water_tests.cpp`, `scripts/water-smoke.cjs`, `scripts/water-depth-smoke.cjs`, `scripts/water-planar-smoke.cjs`, `scripts/water-underwater-smoke.cjs`, `scripts/water-caustics-smoke.cjs`, `scripts/water-interactions-smoke.cjs` |
| Visible skies, spheres, atmosphere, clouds and day/night | `docs/sky.md` | `sky.hpp`, `Scene::setSky`, `SkyGeometry`, `advanceSky`, `RenderView::skyEnabled`, `tests/sky_tests.cpp`, `scripts/sky-smoke.cjs`, `scripts/sky-weather-smoke.cjs` |
| Directional shadows and lighting | README “Directional shadow maps”, `docs/shadows-validation.md` | `light_component.hpp`, `shadow.hpp`, `render_layer.hpp` |
| Particles, flipbooks and effect presets | `docs/fx.md`, README “Particles, flipbooks and reusable effects”, [integration](fx.md) | `effect_component.hpp`, `effect_presets.hpp`, `particle_emitter_component.hpp`, `fx_frame.hpp` |
| Animation, transitions, markers and root motion | `docs/animation-control.md`, README skeletal animation section | `skeletal_mesh_component.hpp`, `animation_component.hpp`, `animation_controller.hpp` |
| Animation layers and 1D/2D blend trees | `docs/animation-layers.md`, [integration](animation-layers.md) | `AnimationBlendTree`, `AnimationLayer`, `restartLayer`, `animationSubtreeMask` |
| Animation library and graphical controller authoring | `docs/animation-authoring.md`, [integration](animation-authoring.md) | `animation_library.hpp`, `editor_animation.hpp`, `EditorSession::setAnimationLibrary` |
| Morph targets and instance weights | README “Morph targets” | `mesh.hpp`, `mesh_component.hpp`, `tests/morph_tests.cpp`, `scripts/morph-smoke.cjs` |
| HUD, menus, text and accessibility | `docs/gui.md`, `docs/ui-input-integration.md` | `gui.hpp`, `gui_browser.hpp`, `web/gui.js` |
| Music, spatial audio and activation | `docs/audio.md` | `audio.hpp`, `audio_browser.hpp`, `src/audio_demo.cpp`, `web/audio-system.js` |
| Saves, migrations and custom codecs | `docs/persistence.md`, `docs/scene-archives.md` | `persistence.hpp`, `scene_archive.hpp`, `web/storage.js` |
| Ordered camera views, viewport coordinates, visibility masks and layers | README “Reusable scene objects”, “Layered renderer”, `docs/rendering-fallback.md` | `camera_component.hpp`, `render_view.hpp`, `render_layer.hpp`, `scripts/compositing-smoke.cjs` |
| WebGPU/WebGL 2 selection, reduced capabilities and context recovery | `docs/rendering-fallback.md`, `docs/rendering-quality.md` | `renderer.hpp`, `render_quality.hpp`, `src/renderer.cpp`, `src/webgpu_renderer.hpp`, `src/webgl_renderer.hpp` |
| Device capabilities, quality budgets and page lifecycle | `docs/rendering-quality.md` | `render_quality.hpp`, `web/quality.js`, `web/app.js` |
| Culling, instancing, LOD and performance | README performance sections, `docs/mesh-detail-validation.md` | `render_frame.hpp`, `mesh_detail.hpp`, `performance.hpp`, `scripts/performance-smoke.cjs` |

Existing `tests/` and `scripts/*-smoke.cjs` supply usage/regression examples. Inspect setup and assertions rather than copying old suite counts or thresholds. `docs/puzzle-game-readiness.md` can help scope puzzle integration, but recheck its capability claims against source and do not impose puzzle rules on other games.

When evidence matters, follow companion validation documents such as `docs/application-lifecycle-validation.md`, `docs/assets-validation.md`, `docs/prefabs-validation.md`, `docs/selection-validation.md`, `docs/gui-validation.md`, `docs/audio-validation.md`, `docs/scene-archives-validation.md` and `docs/persistence-validation.md`. Their recorded results describe past scoped runs. They are not current validation of a new game or change. Ship stress/instancing investigations are examples and measurements, not game-specific engine policy.

Browser editor workspace docking, resizing and Quick Add: [integration guidance](editor-workspace.md), `docs/scene-editor.md`, `docs/scene-editor-layout-validation.md`, `web/editor-layout.js`, `web/editor-layout.css` and `scripts/editor-layout-smoke.cjs`.

- Scene atmospheric fog: `docs/fog.md`, `docs/fog-validation.md`, `include/engine/fog.hpp`, `Scene::setFog`, `RenderView::fogEnabled`, `src/fog.cpp`, shared `shaders/fog.wgsl` / `fog.glsl`, CTest `fog`, `scripts/fog-smoke.cjs`.

Editor camera flight/framing: `include/engine/orbit_controller.hpp`,
`src/orbit_controller.cpp`, `src/editor_browser.cpp`, `web/editor-camera.js`,
`scripts/editor-camera-smoke.cjs`, `docs/scene-editor-camera-validation.md` and
[workspace integration](editor-workspace.md).

Editor terrain authoring: `src/editor_terrain.cpp`, `include/engine/editor.hpp`,
`web/editor-terrain.js`, `tests/editor_terrain_tests.cpp`,
`scripts/editor-terrain-smoke.cjs`, `docs/scene-editor-terrain-validation.md` and
[workspace integration](editor-workspace.md).

Terrain layer texture assignment: `TerrainAsset::withLayers` in terrain.hpp/.cpp,
`EditorSession::setTerrainLayerTexture` in editor.hpp/editor_terrain.cpp,
`web/editor-terrain.js`, `scripts/editor-terrain-textures-smoke.cjs`,
`docs/scene-editor-terrain-textures-validation.md` and
[integration](editor-terrain-textures.md).

FX authoring: engine `docs/fx-authoring.md`, `editor_fx.cpp`, `effect_preview.hpp`,
`effect_archive.hpp`, `web/editor-fx.js`, and [integration](fx-authoring.md).

Ribbon trails: engine `docs/trails.md`, `trail.cpp`, `fx_frame.hpp`,
`tests/trail_tests.cpp`, `scripts/trail-smoke.cjs`, `scripts/trail-depth-smoke.cjs`
and [integration](trails.md).

Ground/plane particles: engine `docs/plane-particles.md`, `particle_emitter_component.hpp`,
`fx_frame.cpp`, `tests/plane_particle_tests.cpp`, plane browser smoke scripts and
[integration](plane-particles.md).

Mesh particles: engine `docs/mesh-particles.md`, `mesh_particle.cpp`,
`tests/mesh_particle_tests.cpp`, mesh browser smoke scripts and
[integration](mesh-particles.md).


Runtime terrain surface relief: `material.hpp` / material.cpp, mesh.wgsl,
webgpu_renderer.cpp and webgl_renderer.cpp; `docs/terrain.md#surface-relief-and-projection`,
`docs/terrain-surface-validation.md`, `scripts/terrain-surface-smoke.cjs`, CTest
terrain / terrain_paging / material / scene_archive. Archive v23; deploy matched workers.

Particle contacts and projected decals: [integration](fx-contacts-decals.md). Collision is optional; applications own queued-impact consumption and decal lifetime.


Wheeled vehicles and body mechanics: engine docs/vehicles.md and docs/vehicles-validation.md; vehicle_component.hpp, rigid_body_component.hpp, physics.hpp, src/physics_vehicle.inc, vehicle editor properties/preset and archive v24. See [vehicle integration](vehicles.md); CTest vehicle, Node vehicle_wasm_tests and scripts/vehicle-smoke.cjs on both renderers/DPRs.

Driving Stage 2: engine docs/physical-surfaces.md and docs/physical-surfaces-validation.md; physical_surface.hpp, collider_component.hpp, physics.hpp, terrain_paging.hpp; [integration](physical-surfaces.md). CTest physical_surface, Node physical_surface_wasm_tests, scripts/physical-surface-smoke.cjs on both renderers/DPRs.

Driving Stage 3: selected engine docs/driving-controls.md / driving-controls-validation.md; web/gamepad.js, input.js, camera_follow.hpp, camera_controller.hpp and camera_component.hpp. See [integration](driving-controls.md), CTest camera_follow, Node camera_follow_wasm_tests and scripts/driving-controls-smoke.cjs on both backends/DPRs.

Driving playback rate, editor preview and separate tuning persistence: `docs/audio.md`,
`docs/driving-audio.md`, `docs/driving-audio-validation.md`; see [integration](driving-audio.md).

Splines, swept profiles and authored route geometry: `docs/splines-routes.md`,
`docs/splines-routes-validation.md`; APIs `spline.hpp`, `route.hpp`, EditorSession;
see [integration](splines-routes.md).

- Surface-following strips/contact traces: `docs/surface-trails.md`, `docs/surface-trails-validation.md`; [integration](surface-trails.md).

Custom GUI controls and hooks: read [integration](gui-widgets.md) and selected-engine
docs/gui-widgets.md / docs/gui-widgets-validation.md; APIs live in gui.hpp and
browser semantics in gui_browser.cpp / web/gui.js. Demo: ?gui=1&guiSettings=1.

Event-driven child effects and managed fading decals: [composition contracts](fx-composition.md) and [validation](fx-composition-validation.md). Use native fx_composition plus FX/persistence/lifecycle regressions, and scripts/fx-composition-smoke.cjs on WebGPU/WebGL2 at DPR 1 and 2. Archive v29 retains nested dependencies and live continuation; events/counters remain transient.


GUI theme defaults and state styles: [integration](gui-themes.md), selected-engine
docs/gui-themes.md / docs/gui-themes-validation.md, gui.hpp and gui.cpp. Demo:
?gui=1&guiTheme=1; both-renderer/DPR acceptance: scripts/gui-theme-smoke.cjs.


GUI min/max/content sizing, alignment and grid: [integration](gui-layout.md),
selected-engine docs/gui-layout.md / docs/gui-layout-validation.md; gui.hpp,
gui.cpp, tests/gui_tests.cpp and scripts/gui-layout-smoke.cjs. Demo:
?gui=1&guiLayout=1. Keep column-selection policy in applications.

- Spotlights/headlights: `docs/spotlights.md`, `docs/spotlights-validation.md`; [integration](spotlights.md).

General region residency: selected-engine docs/region-streaming.md and
region-streaming-validation.md; region_stream.hpp, PhysicsWorld::bodyBounds,
SceneArchive v31 and EditorSession. See [integration](region-streaming.md).

Shadowed lights, sun cascades, HDR bloom/color grading and both-backend validation: [current lighting contracts](lighting-post-processing.md).

| AI task sequences | selected engine docs/ai-tasks.md and ai-tasks-validation.md | AiTaskSpec/AiTask/AiTaskBinding, controller task ownership, ai_tasks, scripts/editor-ai-tasks-smoke.cjs |

Named animation retarget profiles and clip baking: [integration](animation-retargeting.md); selected-engine docs/animation-retargeting.md and animation-retargeting-validation.md, animation_retarget.hpp, animation_retarget_tests and scripts/animation-retarget-smoke.cjs.

Prefab definition assets/variants/linked updates: [integration](prefab-assets.md), selected-engine docs/prefab-editor.md and prefab-editor-validation.md; prefab_asset.hpp/editor_prefab.hpp.
Detailed physics/animation/AI/loading timing: [integration](profiling-timeline.md), selected-engine docs/profiling-timeline.md and profiling-timeline-validation.md; performance_timeline.hpp.

Adaptive resolution/shadow/detail feedback: [integration](adaptive-quality.md), selected-engine docs/adaptive-quality.md and adaptive-quality-validation.md; adaptive_quality.hpp and RenderQuality::detailScale.

## Native Windows desktop host

Read [Windows distribution](windows-distribution.md) and [package validation](windows-distribution-validation.md) for WebEngine::WindowsRuntime, desktop_runtime.hpp, process-owned logs/dumps, manifest verification, /MT launcher, per-user installer, PE/CRT closure, deterministic ZIP/symbol artifacts and explicit clean-VM/signing gates. Source: src/desktop/desktop_runtime.cpp, desktop_package.cpp, examples/windows_launcher.cpp, tools/package_windows.py, distribution/windows/ and scripts/*windows-package*.ps1.


Read [broader Windows acceptance](windows-acceptance.md) for the opt-in
windows_acceptance target, portable gallery, native power/DPI contracts,
Dawn device-loss injection, pixel evidence and explicit physical-hardware gaps.
Inspect examples/windows_acceptance_scene.hpp, tests/windows_acceptance.cpp,
scripts/windows-acceptance.ps1 and tools/verify_windows_acceptance.py.


Read [Windows integration](windows-native.md), [desktop API](windows-desktop.md),
[platform services](windows-platform-services.md) and [acceptance](windows-platform-validation.md).
Selected-engine docs: windows-desktop.md, windows-native.md,
windows-desktop-gui-validation.md, windows-platform-services.md and
windows-platform-validation.md under docs/. Inspect include/engine/desktop.hpp,
desktop_services.hpp, audio_desktop.hpp, assets.hpp, audio.hpp, persistence.hpp;
src/desktop/{desktop,desktop_files,sdl_audio,sdl_text_input}.cpp;
cmake/windows.cmake; examples/windows_vehicle_app.hpp/windows_vehicle_state.hpp.
Tests: desktop_services, desktop_dialog, desktop_demo_services plus API/text and
Windows vehicle smoke. WebEngine::Platform publicly links core; SDL/Win32/Dawn
implementation details stay private. Browser adapters remain independent. Verify
availability at the consumer pin; this does not add a Paws Windows app.


## Cinematic sequencer

Use [cinematic integration](cinematic-sequencer.md), selected-engine docs/cinematic-sequencer.md and its validation record. Inspect cinematic_sequence.hpp/editor_cinematic.hpp, src/cinematic_sequence.cpp/editor_cinematic.cpp, application cinematic_browser.inc/cinematic_example.hpp and web/cinematic.js. Core owns values/clock; optional editor owns history/clone preview. No scene codec or automatic gameplay event track.


## Replay and ghost recording

Read [replay integration](replay-recording.md), selected-engine docs/replay-recording.md and its validation file. Inspect include/engine/replay.hpp/replay_ghost.hpp and src/replay.cpp/replay_ghost.cpp; shared application policy is examples/driving_replay.hpp. Browser src/replay_browser.inc/web/replay.js and native windows_vehicle_app.hpp use it. No solver replay or new scene codec.

### Windows background asset adapter

Read [Windows background assets](windows-background-assets.md) and
[validation](windows-background-assets-validation.md). Inspect asset_desktop.hpp,
assets.hpp, asset_decode.hpp/asset_browser.hpp, src/desktop/desktop_assets.cpp,
src/asset_decode.cpp, src/asset_worker_browser.cpp, tests/desktop_asset_tests.cpp
and the vehicle application. Worker ownership, admission deadlines, cancellation
without native preemption, joined teardown and owner-only publication are required.



### Native scene workbench and audio parity

Read [workbench](windows-editor.md), [audio](windows-audio-environment.md) and
[separate target acceptance](windows-parity-validation.md). Inspect editor_desktop.hpp,
audio_desktop.hpp and src/desktop/{editor_desktop,audio_stream,sdl_audio}.cpp.
Build webengine_editor/desktop_editor_tests/desktop_audio_environment_tests and
portable audio_dsp tests; use scripts/platform-parity-smoke.cjs for scene exchange.

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

[Accessibility and native contacts](accessibility-input.md): Windows UIA, native
touch/pen mapping, browser cancellation, and separate target/device validation.

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

## Shared authoring frontend (2026-10-22)

Read [native/browser authoring APIs and limits](editor-authoring.md),
[content iteration](content-iteration.md) and [validation commands/evidence](editor-authoring-validation.md).
Link optional engine_editor_gui; keep platform file IO in host adapters.
Run native and Wasm widget contracts, real Windows SDL/Dawn and both forced
browser renderer acceptance, including file exchange and stale-upload rejection.
Do not claim advanced browser graph/curve/cinematic playback parity from these forms.


## Native graphics API fallback (2026-10-22)

See [Automatic D3D12/Vulkan/D3D11 APIs and ownership](windows-graphics-fallback.md)
and [native/browser validation and commands](windows-graphics-fallback-validation.md).
Strict selections do not switch. Keep renderer replacement, diagnostics, runtime
lookup and readback cancellation in the native host, with source/GUI/clocks shared.

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

## Compiled module composition

For reusable code-plugin packaging/composition, read [modules](modules.md), selected
engine `docs/modules.md` and `docs/modules-validation.md`; inspect
`include/engine/modules.hpp`, `cmake/WebEngineModule.cmake`, `examples/modules` and
`tests/modules_tests.cpp`. Verify the consumer pin before adopting development APIs.

## Quest kernel and custom quest handlers

Read [quests](quests.md), selected engine `docs/quests.md` and
`docs/quests-validation.md`; inspect `include/engine/quests`, `src/quests`,
`examples/quests` and `tests/quests_tests.cpp`. M2 provides the portable kernel;
M3 [host integration](quests-host.md) supplies persistence, pause and scene travel;
Paws migration remains M4 onward.


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

- Shared consumer sessions, native GUI rebuilding and pointer adapters: [consumer hosts](consumer-hosts.md), selected engine docs/consumer-hosts.md.


## Multiplayer foundation

Read [multiplayer contracts](multiplayer.md) and [validation](multiplayer-validation.md).
Inspect the selected engine's `include/engine/network.hpp`, `cmake/network.cmake`,
`examples/multiplayer` and `docs/multiplayer.md`. Session state and byte schemas
stay portable; IO stays in adapters; game policy stays in examples/applications.
The M1 Multiplayer test action retains the fixed arena. M2 adds the bounded authored-scene Host/Join workflow below.

## Authored multiplayer scenes (M2)

Read [authored scene contracts](multiplayer-scenes.md) and
[runtime validation](multiplayer-scenes-validation.md). Inspect network_scene.hpp,
network_content.hpp, the scene server/client examples and editor Host/Join callbacks.
The optional scene layer validates a bounded static profile; network core remains
scene-independent, and example movement/transport/publication policy stays in hosts.


## Multiplayer prediction (M3)

Read [prediction contracts](multiplayer-prediction.md) and [runtime evidence](multiplayer-prediction-validation.md). Inspect network_prediction.hpp, scene_shared.hpp (currently MP06) and server fixed-step ownership before extending prediction policy.


## Multiplayer presentation (M4)

Read [presentation contracts](multiplayer-presentation.md) and [validation](multiplayer-presentation-validation.md). Inspect network_presentation.hpp/.cpp and the staged Client buffers before extending visual movement. Authority, prediction and source documents must remain independent.


## Multiplayer replication (M5)

Read [delta/interest contracts](multiplayer-replication.md) and [runtime validation](multiplayer-replication-validation.md). Inspect network.hpp/.cpp and scene_server.cpp before changing baselines or relevance. Global ownership, per-peer selection and presentation remain separate. Both editors launch MP05 clients; saved scenes stay compatible.


## Multiplayer authoring (M6)

Read [authoring contracts](multiplayer-authoring.md) and [validation](multiplayer-authoring-validation.md). Inspect network_authoring.hpp, NetworkScene::replicated, the shared editor property/creation paths and per-kind TransformReplicas before extending ownership or relevance. Current scene schema is MP06; saved envelope formats are unchanged.


## Gameplay-component replication (M7)

Current scene schema is MP07 with WES3 component manifests. Inspect network_components.hpp, network_component_editor.hpp, NetworkScene and application-owned gameplay.hpp. Read [contracts](multiplayer-components.md) and [evidence](multiplayer-components-validation.md). Earlier MP05/MP06 current-status notes are historical.

## Open-path terrain cliffs

For heightfield-to-mesh cliff replacement, paired hole/mesh publication, exact
perimeter seams, material transitions, LOD/collision and browser/Windows acceptance,
read [cliff contracts](terrain-cliffs.md) and [validation](terrain-cliffs-validation.md).
Verify availability in the selected engine checkout; documentation does not
advance a consumer dependency.

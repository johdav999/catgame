# Animation layers and blend trees

Verify these development APIs exist in the selected WebEngine checkout; updating
this skill does not advance a game's engine pin. Read `docs/animation-layers.md`,
`docs/animation-control.md`, `docs/animation-authoring.md` and the composition
validation record before integrating.

`AnimationControllerState::tree` optionally contains `AnimationBlendTree`:
`parameterX`, optional Number `parameterY`, `points` of `{clip,x,y}`, and
`synchronizePhase=true`. It is exclusive with the legacy two-clip `blend`.
1D interpolates neighboring samples and clamps; Cartesian 2D normalizes gradient
bands and clamps to sample bounds. Drive sideways/forward velocity to combine
speed/direction without angle wrapping. Translation/scale use weighted sums;
quaternions use hemisphere-aligned normalized sums. These are flat sample trees,
not recursive graphs. All sources use the longest clip's cycle when synchronized;
otherwise they retain independent seconds. Weight changes preserve phase.

`AnimationControllerDefinition::layers` holds up to 16 ordered `AnimationLayer`
objects: name, motion (clip/blend/tree plus loop/speed), mode Override/Additive,
mask, weight (default1), optional Number weightParameter, optional referenceClip
and referenceTime (default0). Empty mask selects all joints; otherwise one [0,1]
weight per joint. `animationSubtreeMask(skeleton,joint,weight=1)` includes descendants.
Additive poses use bind/reference translation differences, local rotation deltas
and scale ratios (reference scale magnitude >1e-6). Publish definitions immutable.

Each player owns layer clocks/buffers. Layers advance at zero weight, continue over
completed base poses and do not restart during base transitions. Explicit pause,
disable/application pause freezes all. `restartLayer(name)` resets one overlay,
`layerTime(name)` inspects it. One-shot interactions use loop=false and application
weight/restart policy. Seek affects the base only. Manual control removes layers;
detached trees resume the state's fallback clip. Manual crossfades freeze the
composed outgoing pose to avoid a jump. Resource replacement cancels all.

Only the base produces markers/root motion. Tree marker times use the common
unwrapped clock and weighted contributing sources. Layers run after base fades/root
stripping and skip the extracted locomotion joint and ancestors. Gameplay owns
movement/collision/output consumption. [IK](inverse-kinematics.md) runs afterward;
retargeting, layer events, layer state machines and recursive nodes remain separate.

Editor: Details -> Animation library & state machine -> state Blend tree and
Animation layers. Author samples, numeric axes, phase sync, modes, weights,
subtree/custom masks, references and order. Isolated preview exposes live inputs
and Restart layer controls. Existing bounded library history and scene Apply undo
retain full layer state. Same-rig library apply remaps masks by joint identity.
Scene format33 reads v1-31 with composition disabled and v32 with IK off. WEANIM1
uses ANIM3 metadata (reading ANIM1-2), embedding the same scene codec. Save both scene and
library as appropriate. Never mutate published shared assets.

Limits:16 layers,64 points/tree,256 joints; finite coordinates +/-1e6, separation
>=1e-6, distinct numeric axes. Invalid definitions and initial composed transforms
reject before replacing playback. Base transition/output limits still apply.

Validation: native animation_layers, animation_control, animation_library,
animation, root_motion, scene_archive, persistence and editor_content as applicable.
Build explicit `animation_layers_wasm_tests` and run its .js in Node for Wasm ABI
coverage. Build `web_assets` with repository-local pinned Emscripten6.0.11 and serve
that exact dist, including matching decode worker. Run animation-layers-smoke.cjs
and editor-animation-smoke.cjs on WebGPU/WebGL2 (also DEVICE_SCALE_FACTOR=2).
Use selected engine build commands; game consumers keep their own scripts.
Desktop Chromium and Wasm tests do not establish physical mobile acceptance.

[Explicit retarget baking](animation-retargeting.md) produces ordinary destination clips before layers evaluate.

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

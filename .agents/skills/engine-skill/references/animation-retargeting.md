# Animation retargeting

Verify the selected engine contains `animation_retarget.hpp` and
`AnimationLibrary::appendRetargeted`. Read that checkout's
`docs/animation-retargeting.md` / `animation-retargeting-validation.md`.
This development capability does not establish availability in CatGame's pin.

Link engine_core. `AnimationRetargetProfile` has `joints` (named source/target
`AnimationRetargetJoint` records and RetargetTranslation mode), `modelRotation`
(source-model to target-model quaternion, identity default), `translationScale`
(1 default), and `validate(source,target)`.
`matchAnimationJoints` suggests exact names only; unmatched rigs need explicit
mappings. Suggestions may be empty or incompatible, so review before bake.
`estimateRetargetScale` returns the median nonzero mapped segment ratio in model
space, or 1 without usable segments. Unchanged segments can dominate the median.

`retargetAnimations(source,target,profile)` synchronously returns ordinary
AnimationClip samples, retaining name, duration, frame count and markers. Rotations
transfer model-space source motion relative to source bind, align model axes, then
apply target bind orientation and reconstruct local rotations. Bind translation
keeps target local offsets; ScaledDelta transfers rest-relative displacement using
nearest mapped ancestors' bind/animated transforms. Root suggestions default to
ScaledDelta; others Bind. Enable pelvis translation deliberately. Model unit scale
is applied once. Target bind scales are retained.

One-to-one names and corresponding nearest mapped ancestors are required.
Unmapped target joints retain local bind pose and follow parents; unmapped helper
bones may differ between rigs. Fixed positive uniform scales only; source sampled
scale deviations above relative 1e-5 reject during bake. Names are unique, <=256
bytes/no controls; only a root may be unnamed. At most 256 mappings/joints and
64 clips; source and output poses each <=32 MiB. Translation scale finite in
[.0001,10000], orientation quaternion finite/nonzero. Existing skeleton validation
and finite world/inverse checks apply. No semantic mapping, contact correction,
twist redistribution, full-body optimization or runtime retarget streaming.
Use [IK](inverse-kinematics.md) for explicitly authored contact correction.

`library->appendRetargeted(source,profile)` validates total library clip/pose
budgets before baking and returns a new immutable library, retaining prior clip
IDs, controller references and destination inverse binds. Source controller,
IK/layers and library organization are not transferred. Strict `append` and apply
compatibility checks are unchanged. No input mutation, retained pointers,
callbacks, subscriptions or per-frame work. Caller owns immutable inputs during
the synchronous bake; apply on the idle scene owner through existing guards.

Editor: select destination -> Animation library & state machine -> Retarget clips.
Load FBX/GLB/.weanim, review every named joint, translation mode and scale/model
orientation, then Bake. Mapping drafts are separate from library history; Bake
is one library edit and Apply is the existing scene history command. Preview/scrub
uses the isolated Play clone. Invalid source/profile input preserves valid state.
Structurally valid but incompatible profiles remain repairable with Bake disabled.
Close/discard, rebind and invalidation release the staged source; async JS reads
check binding generation and Play gates authoring. Decoded source geometry/images
are released after import. Files <=64 MiB, existing importer budgets apply.

`retargetProfileText` / `parseRetargetProfile` use WERETARGET1, <=256 KiB,
with named pairs, mode, translation scale and model quaternion. Save/Open mapping
profile uses `.weretarget`. Baked poses need no codec change: scene33/ANIM3 remain.
Profiles/provenance stay separate from `.weanim` and scene archives.

Demo: `?scene=retarget&editor=1`, short blue source/tall gold destination with
different forearm lengths/bind orientation. Source proportions and animation are
application example policy, not a production character pipeline.

Validation: native `animation_retarget_tests` (CTest animation_retarget), then
animation_library, animation_layers, animation_control, ik, animation, root_motion,
scene_archive, persistence and editor_content. Use matching MinGW bin on PATH.
Use the repository-local SDK/version in .emscripten-version (currently 6.0.11),
build web_assets and animation_retarget_wasm_tests, then run the latter JS in Node.
Serve the exact staged build and run scripts/animation-retarget-smoke.cjs with
NODE_PATH/BROWSER_EXECUTABLE/DEMO_URL, WebGPU and forced WebGL2 at DPR1 and
DEVICE_SCALE_FACTOR=2. Run editor-animation, animation-layers and ik smoke scripts
serially against the same site. Verify destination pixels, mapping/profile failures,
source isolation, history, library/scene roundtrips and compact controls.
Desktop Chromium results do not establish physical mobile or production-asset
motion quality. Consumer games keep their own build/acceptance commands.

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

# Inverse kinematics

Verify the selected WebEngine checkout has `engine/ik.hpp` and read its
`docs/inverse-kinematics.md`, architecture and validation record before integrating.
These development APIs may be newer than the consumer's pinned engine.

`AnimationControllerDefinition::ik` contains <=32 named `IkConstraint` values.
`mode=TwoBone` (default) uses direct root/middle/tip joint indices, a model-space
position/pole target and optional orientTip (default false). `LookAt` uses root,
forward (joint-local +Z default) and maxAngle (pi radians default), measured from
the incoming animated pose. Constraints apply in vector order after base fades,
root stripping and layers, before skin matrices. Later constraints see earlier
results. Effective weight = constraint weight * target weight; both default1,
range[0,1]. Target enabled defaults true. Unreachable limbs clamp without stretch;
singular poles use the incoming bend plane or deterministic perpendicular.
Look-at is a swing cone, not anatomical hinge/twist limits or a full-body solver.

Publish controller/rig/clip assets immutable. The player copies default targets
and owns independent live state. `setIkTarget(name,target)` / `setIkTargets(span)`
evaluate explicitly even while paused, preserving clocks and queued markers/root
movement; batch several foot updates. Invalid targets/initial solves reject before
publishing visible pose or replacing a valid controller. Constraint rotations may
not touch the root-motion joint or its ancestors, even when disabled. Manual
playback clears IK, manual fades freeze the outgoing solved pose, base controller
transitions retain targets and skeleton replacement cancels control. No callbacks,
scene pointers, workers or new clock are retained. Use scene owner and handles.

`ikTargets()` and `poseBeforeIk()` are borrowed until relevant control/pose updates.
The pre-IK pose is composed/root-stripped and empty without IK. `ikResult()` gives
per-constraint residual (model units for limbs, radians for look) and limited flag.
Diagnostics are transient, never gameplay events. Solver output is presentation;
only base animation produces consumable markers/movement. Ordinary scene ticking
obeys lifecycle pause/disable; explicit target edits can update disabled poses.

`solveIk(skeleton,input,constraints,targets,output)` works standalone on <=256 joints
and uses bounded stack scratch without per-solve heap allocation. Output can alias
input; failure leaves it intact. Positive uniform joint scales, finite targets,
nonzero quaternions and limb segments are required. `ikJointTransform` returns a
model matrix; `ikJointTarget` returns incoming model position/orientation. World
conversion `ikTargetFromWorld(modelToWorld,target)` requires positive uniform affine
placement including all mesh ancestors; shear/reflections/nonuniform scale reject.

Grounding is opt-in: after animation, get an incoming foot target with
`ikJointTarget(rig,player.poseBeforeIk(),footJoint)`, copy the authored pole, and
call `groundIkTarget(physics,modelToWorld,foot,probe,filter)`. Probe defaults:
worldUp/soleUp +Y, above .5, below .8, soleOffset .03 world units, maxSlope pi/3.
It raycasts existing collision; synchronize the scene first. Default filter
ignores sensors; explicit filters are honored unchanged. Exclude the character's
ActorId/bodies explicitly. Miss/steep hit returns nullopt; gameplay chooses weight
release/smoothing. The supplied incoming rotation controls sole alignment/heading.
Use simulation placement and pre-IK input to avoid feedback. Gait/contact locks,
pelvis offsets and moving-platform anchors remain game policy. The player never
queries physics or moves actors/bodies itself.

Editor: select skeletal mesh -> Animation library & state machine -> Inverse
kinematics. Author chains, target/pole, weights, tip quaternions, forward/limit and
order. Graph preview exposes live targets, enable/weight and residuals, including
paused edits. Stop restores source; existing library and scene histories apply.
Demo `?scene=ik&editor=1` / IK character shows animated weight shifting with two
feet initially probed onto sloping pads, hand reach and head aim. Pad movement
requires application re-probing. It is an authored block-rig demo, not a character
asset pipeline. Library apply remaps joint indices by identity on compatible rigs.

IK began in scene33; current scene34 reads1..33 (pre-33 IK empty); WEANIM1 uses ANIM3 and reads1..2.
Scene constraints and live targets persist; libraries retain constraint defaults.
Scratch and diagnostics rebuild.
Native validation: build ik_tests plus animation_layers, animation_control,
animation_library, animation, root_motion, scene_archive, persistence and
editor_content tests. Use matching MinGW bin on PATH on Windows. Browser: use
repository-local Emscripten version in .emscripten-version (6.0.11 currently),
build `web_assets ik_wasm_tests`, run ik_wasm_tests.js in Node, serve that exact
dist with its matching decoder. Run scripts/ik-smoke.cjs serially on WebGPU/WebGL2
at DPR1 and DEVICE_SCALE_FACTOR=2, then relevant animation regressions. Consumer
games retain their own build scripts. This does not establish physical mobile
acceptance or production character-quality foot locking.

[Explicit retarget baking](animation-retargeting.md) transfers compatible source clips before playback; IK remains separate contact/aim adjustment.

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

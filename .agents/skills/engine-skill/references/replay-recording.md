# Pose/event replay and non-colliding ghosts

Read selected-engine docs/replay-recording.md and its validation record. Development
APIs may be newer than a consumer pin. This documentation does not advance CatGame's
gitlink or provide a Paws replay application.

engine/replay.hpp in engine_core owns ReplayData, ReplayFrame (time, JointPose poses,
numeric values, cut), ReplayEvent (time/name/payload), immutable ReplayRecording,
ReplayRecorder, ReplayPlayer and ReplayArchive. New portable APIs contain no DOM,
SDL, Dawn, Jolt or renderer implementation types. This is recorded presentation
and diagnostics, not deterministic physics/input execution or a solver checkpoint.

Schema names pose/value channels plus an application context. First frame time=0,
later times strictly increase. ReplayRecording::create validates before publication.
sample(time) linearly blends position/positive scale/numeric values and uses existing
normalized shortest-arc quaternion interpolation. A cut holds the prior sample
until the cut frame. Wheel rotation exceeding180deg between samples is ambiguous.
replayPose(matrix)/replayTransform(pose) convert positive TRS; reject shear/reflection.
Events are ordered data, never callbacks. eventsBetween(begin,end) selects (begin,end];
inspect data().events for time0 events. Seek pauses and executes no recorded commands.

ReplayRecorder takes an empty schema; capture(frame) and event(event) reject invalid
data before mutation. Budget exhaustion returns false without discarding the admitted
buffer. Events reference already captured time. finish() copies to an immutable
recording, needing two frames. Application chooses capture/retention policy; no ring
buffer, compression, background streaming, media encoder or automatic recording.

ReplayPlayer owns a shared immutable recording; play/pause/stop/seek/advance use an
explicit eligible delta0..0.25s. Speed.125..4, looping=false by default, end pauses,
Play at end restarts. Applications select simulation or presentation time and freeze
on host suspension. No scene clock or second physics step is added.

Limits: 1..32 pose channels, 0..16 numeric channels, 18001 frames, 180000 total pose
samples, 2048 events, duration<=600s, WEGHOST 1 .weghost files<=64MiB. Printable names
256 bytes, channel/event names128, context1024, payload1024. Finite translations
within+/-1e6, positive scale.0001..1000, normalized quaternions, finite telemetry
within+/-1e9. Strict count/value/trailing-data validation and locale-independent
quoted text work on native and Wasm. Files hold no scenes, resources or runtime
handles. Save the course/resources separately and version application context tags.

engine/replay_ghost.hpp exposes ReplayVisual and noncopyable ReplayGhost. Supply
1..31 static mesh resources with optional texture/material overrides. Scene owns a
mesh-only actor, tinted translucent cyan, no shadows/collision/vehicle/rigid bodies.
apply(scene,poses) takes a scene-relative root and per-mesh local poses, validating
all before publication and resolving generation/scoped handles. Removed/foreign
bindings return false. No Scene/Node borrows or callbacks are retained. Explicitly
remove on handoff; scene destruction also releases nodes. Proxy uses existing
transient subtree exclusion and never enters scene/application archive payloads.
No new scene version, resource codec or asset-worker transport is introduced.

examples/driving_replay.hpp is shared browser/Windows application policy. Capture
afterSimulation at up to30Hz from authoritative chassis/current VehicleState wheel
poses plus authored offsets, independently of culling/LOD/render interpolation.
Actual timestamps mean slow frames yield fewer samples, not fabricated catch-up.
Records speed/RPM/gear/commands and command/gear/reset/bug events. Resets cut the next
sample. Limits finish capture; event-budget exhaustion is diagnosed. Course tag plus
geometry/authored-layout fingerprint gates compatible imports; it is not authenticity
or proof the course is unchanged. Changed source geometry/layout needs Clear/rebind.

?scene=replay and the Formula demo's collapsible overlay expose Record/Finish,
Review, scrubbing, speed, event
jumps, .weghost save/load and Race ghost. Windows vehicle HUD uses shared retained
GUI buttons/Slider, event navigation and native Open/Save dialogs. Review pauses live
physics, cancels input, hides live mesh presentation and owns camera follow temporarily.
Close restores masks/camera/prior pause through handles. Race explicitly teleports
the live car to recorded start at rest and runs a non-colliding1x simulation-clock
ghost; slow review speed does not carry into races. Apps own lap validity/scoring.
Record only eligible simulation time; resume motion if paused. Source changes/shutdown
release proxies/owners. Browser source controls become inert; editor/cinematic previews
exclude active recording/visible replay. Close review before source SceneArchive saves;
browser exports guard temporary camera/visibility state. Events are inspected, not executed or sonified.
Skinned/morph capture, scenery/NPC/state recording, dynamic-world rewind and network
ghost leaderboards remain application extensions.

Use README, local Emscripten6.0.11, Node replay_wasm_tests and native replay_tests/CTest
replay. Build webengine web_assets and run scripts/replay-smoke.cjs serially on actual
WebGPU/forcedWebGL2 at DEVICE_SCALE_FACTOR1/2; REPLAY_SCENE=formula covers Formula.
MSVC/Dawn builds add windows_replay_smoke. Run scripts/replay-windows-smoke.ps1 with a
fresh EvidenceDir and optional compatible BrowserRecording. Verify GUI driving capture,
slider/events, live pose/velocity preservation, reset cuts, files/corrupt retention,
non-colliding races,1x reset, suspension, GPU recovery, compact layout, native actual
framebuffer readbacks and shutdown. Regress vehicle/camera/input paths. Desktop/DPR
evidence does not establish physical mobile/controllers, Safari or sustained speed.

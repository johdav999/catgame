# Cinematic sequencer integration

Development engine_core exposes immutable CinematicSequence/CinematicSequenceData,
CinematicTrack/CinematicKey, CinematicSequenceArchive and CinematicPlayer through
engine/cinematic_sequence.hpp. Read selected-engine docs/cinematic-sequencer.md and
its validation record. Consumer pins may lack these APIs; no gitlink is advanced.

Five track kinds: local Transform, Camera pose/lens shots, existing uniquely named
Animation clips, Audio URI clips and EffectComponent windows. Camera shots cannot
overlap; duplicate/conflicting target writers reject. Parent/child poses compose.
Keys interpolate Linear, smoothstep Smooth or Step; Euler rotations take shortest
arcs per axis, not quaternion splines/multiturn curves. Lens uses camera projection
units. Track spans are [start,end); final visual clips can sample at film end.

Assets validate completely before publication: .01..3600s, fps1..120, <=64 tracks,
<=256 keys/track, <=4096 keys total, <=8 simultaneous sound clips, 256-byte names,
4096-byte sound URIs. WECINE 1 files are <=1MiB and contain references, not scenes,
resources, callbacks or runtime checkpoints. Save scene/resources separately.
Core player seek/play/pause/stop/advance uses explicit eligible delta0..0.25s,
optional looping=false, end pauses. Runtime games own binding/completion/handoff.

Optional engine_editor exposes CinematicSequenceEditor (128-edit independent
immutable document history) and CinematicPreview in engine/editor_cinematic.hpp.
Preview clones with SceneArchive, shares immutable assets, owns all mutable nodes
and enabled sound tracks' optional AudioSystem/backend. Source borrowed only at
construction; renderer scene/camera borrows MUST be released before destruction.
Cloning bounds: 10000 nodes/256MiB allocations; custom components need codecs.
Protected nodes, physics-authoritative targets, following cameras, missing enabled
bindings, invalid projections and ambiguous clip names reject before publication.
Disabled bindings need not resolve. Other cloned simulation writers stay disabled.

Absolute silent seek samples poses/animation/shots and reconstructs deterministic
FX on a canonical60Hz grid. FX clips <=10s => <=600 scene variable steps, replaying
only active windows. Motion samples along replay path; static physics synchronized
without solver steps. Existing composition/collision budgets apply; no frame-time
guarantee. Effect graphs <=128 unique assets; external-contact SurfaceTrail rejects.
Forward playback uses the same grid; clip changes/wraps reconstruct. Animation
markers/gameplay root extraction do not run; gaps restore baseline/bind poses.

Play activates preview audio from a gesture; inactive audio device holds its clock.
Silent sequences need no device. Pause/seek/end/Close stop owned voices. Suspension
freezes clock/media and requires explicit resumed Play. Initial looped stream source
offsets wrap after metadata; nonloop offsets must fit source duration. No sample-
accurate mix/offline video export, waveform authoring or automatic acoustic zones.

Browser ?scene=cinematic builds a procedural greeting with all track kinds. Its
scrubbable SVG timeline supports clip/key drag, frame snap, exact fields, target
capture, native undo/redo and .wecin import/export. Cancelled gestures create no
history; invalid/stale loads preserve state. Preview is readonly/isolated, pauses
source, excludes editor Play/FX/region previews and input, then restores source
views through handles before owner teardown. Browser adapters stay application code;
no portable DOM types or new implicit frame phase/archive codec is introduced.

Use README and pinned repository Emscripten6.0.11. Native cinematic_sequence_tests,
CTest cinematic_sequence; build cinematic_sequence_wasm_tests and run with Node.
Build webengine web_assets to publish matched output; run cinematic-sequence-smoke.cjs
serially on WebGPU and forcedWebGL2 at DEVICE_SCALE_FACTOR1/2. Regress audio,
animation_control and fx_composition. Verify source bytes, deterministic pixels,
real audio Play/Pause, loop offsets, GPU recovery, gestures/files/editor isolation,
compact controls and shutdown. Desktop evidence is not physical mobile/native/Safari
or sustained-performance acceptance. Native audio does not yet stream encoded URI
tracks; supply a capable backend or use silent preview.

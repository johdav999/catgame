# Environmental audio and encoded music streams

Development WebEngine adds optional AudioOcclusionSettings, AudioReverbSettings
and immutable URI-only AudioStreamAsset through the existing AudioSystem.
Read selected-engine docs/environmental-audio.md and its validation file. This
documentation does not advance a consumer engine gitlink. Windows output,
streaming and DSP use a [separate adapter](windows-audio-environment.md).

AudioPlayOptions.occlusion=false and reverbSend=0 preserve dry defaults. Enable
system occlusion and each spatial voice explicitly. AudioSystem.update after
synchronized simulation borrows physicsIfCreated without creating a world or
retaining scene pointers. Queries use authoritative listener/emitter poses,
exclude sensors and bound owner actors, and include solid static/dynamic/kinematic
bodies on selected physics layers. Null/missing worlds/listeners clear targets;
missing/disabled/foreign emitters stop normally. Render masks/interpolation/LOD
never drive acoustics. No extra clock or implicit scene synchronization.

Occlusion defaults off, layers0xffff, maxQueriesPerUpdate8 (1..64), blockedGain.35
(0..1), lowPassHz800 (20..20000), transitionSeconds.1 (0..10). Fair round-robin
admission retains unvisited targets; active audio time drives exponential amount
and logarithmic cutoff smoothing. Suspension spends no rays/freezes smoothing.
lastOcclusionQueries()/voiceOcclusion(id) expose diagnostics. This is one ray,
not material transmission, diffraction, portals or multiray partial occlusion.

setReverb validates wet0 (0..1), decaySeconds1.5 (.1..8), preDelaySeconds.02
(0..0.2). Backend rejection preserves accepted configuration. AudioCapabilities
reports occlusionFilter/reverb/streaming; new optional backend methods default
dry/unsupported so custom backends still compile. Browser owns per-voice filter
and post-gain send plus one explicit shared stereo convolution room per device.
IR generation is deterministic, bounded to eight seconds at device sample rate;
replacement may briefly retain both graphs. Settings stage before publication,
do not open a device before playback, and should change only on settings edits.
Rooms replace tails without crossfade; applications select rooms, no auto zones.
Individual stop/mute leaves prior natural tails; master mute/zero master volume
silences both paths, stopAll clears tails and disposal disconnects resources.

playStream(AudioStreamAsset::create(uri), options) shares voice/bus/mute/fade/rate
contracts and the default128 total-voice budget; browser additionally limits eight
streams/device. URI1..4096 bytes, no whitespace/control; browser accepts relative,
HTTP/HTTPS/blob sources. No PCM/duration/browser object lives in the asset. Media
elements/MediaElementAudioSourceNodes provide progressive browser decoding,
metadata preload, optional panning/filter/send. Never decode the whole track to
AudioBuffer/Wasm. Encoded buffering remains browser policy, not a hard memory cap.
Remote seeking needs range-capable hosting and cross-origin Web Audio needs CORS.

streamPosition(id) is source seconds (-1 if unknown/PCM); seekStream validates
nonnegative finite offsets. Initial looped-stream offsets wrap after metadata reveals duration. Nonlooped
initial offsets may then fail if out of range; known invalid seeks preserve cursor. Start success means pending
voice accepted, not decoded/playing. Media errors release the voice and report
device error. Late completions check identity and cannot resurrect stopped voices.
Stop removes handlers/src, pauses, aborts loading and disconnects. Caller owns blob
URL lifetime; revoke after all referencing voices stop. Loop/rate.125..8, no pitch
preservation, existing volume/mutes/fades apply. Explicit gesture resume starts
pending media; suspension pauses media plus context, and return requires a new
gesture. Preserve no-op repeated suspend(false) handling and activation revisions.

?scene=audio hosts a separate src/audio_demo.cpp scene for wall/room controls.
The sample stream is a short synthetic MP3; local long music streams via blob
without upload or PCM import. Stop/replace/error/pagehide revokes owned file URLs.
web/audio-system-panel.js owns browser controls. Runtime audio/settings/streams
remain transient, no scene/archive version or waveform-player payload change.

Terminal host shutdown publishes data-engine=stopped and rejects late renderer
status updates. Terrain polling and its native probe independently guard stopped
applications before borrowing scene data. Observe a post-shutdown window in the
audio smoke to catch late callbacks; do not poll released scene/camera owners.

Use pinned local Emscripten6.0.11. Native audio_tests/CTest audio covers ownership,
query fairness/budgets/layers/sensors/self-exclusions, smoothing/suspension, invalid
retention, unsupported/rejected commands and streams. Build webengine web_assets
audio_wasm_tests; run Node Wasm, then environment-audio-smoke.cjs serially at
DEVICE_SCALE_FACTOR1/2. It measures real muffling spectra and reverb after the dry
analyser window, twenty-minute HTTP range playback without PCM decode, seeking,
mute/rate, suspension, pending cancellation/corrupt-media/blob cleanup and shutdown
on actual WebGPU/forced WebGL2. Run existing audio-system-smoke.cjs, audio-smoke.cjs
and driving-audio-smoke.cjs for affected integrations. Desktop samples/DPR are not
physical speakers/mobile, Safari, native audio or sustained-performance acceptance.

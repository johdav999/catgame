# Beach wave sound effects

## Opening cleanup and volume controls — 22 October 2026

All four supplied variants contained a brief decaying impact before the surf.
The runtime WAVs now silence their first 300 ms and use a 200 ms raised-cosine
fade into the remaining wave. Samples after 500 ms are bit-identical to the
originals. Duration remains 2.55 seconds, preserving the existing wave/foam
timing. Immutable originals and their recorded hashes are retained in
`artifacts/audio-production/sources/beach-waves-original/`; preparation is
repeatable from those originals through `scripts/prepare-beach-audio.py`.

Wave-specific gain is now **0.12**, down from 0.25 (about 6.4 dB quieter), before
the saved Effects percentage. Other effects and saved bus percentages are
unchanged. Music, Ambience and Effects controls show a horizontal fill inside
their existing rectangles. Mouse or finger drags upward increase the value;
downward drags decrease it, with 200 logical pixels spanning the full range.
Arrow keys adjust 1%, Page keys 10%, and Home/End choose the endpoints. A completed
or cancelled drag saves its last value once; the settings panel remains scrollable
outside the value controls. The account launcher hides on Settings so it cannot
cover the narrow-screen Back button.

Validation: all three focused native suites (`paws_audio`, `paws_release1_audio`,
`paws_release1_gui`) pass. `scripts/volume-drag-smoke.cjs` passes on WebGPU and
WebGL2 with real mouse input, Chrome-emulated touch, cancellation, scroll ownership,
keyboard controls, visible bar pixels and save/reload. Reports and inspected
desktop/portrait captures: `build/evidence/audio-controls/`. These are desktop
browser and touch-emulation checks; physical-mobile listening is not asserted.

The real browser audio run also passes: decoded opening silence, nonzero surf
output after the fade, 0.12 wave gain multiplied by Effects, randomized playback,
foam timing, music independence, pause/mute and garden return. Evidence:
`build/evidence/audio-controls/audio-webgpu/report.json`. Final preview build:
`paws-0.1.0-20677581a42d7c6c`; the last change is settings-only CSS, with the same
Wasm and audio content as the passing audio run.

## Beachside Puzzle background music

The beach adventure selects the supplied **Beachside Puzzle** track instead of
the garden playlist. Its single-track playlist repeats after backend completion
and keeps the same voice through beach quest changes. Returning to the garden
stops it and restores Golden Afternoon Garden. Region selection follows the
active scene, so loading another beach quest does not briefly switch music.

The original `assets/runtime/audio/beach/Beachside Puzzle.wav` is retained as a
source master and excluded from deployment. `python scripts/prepare-beach-music.py`
creates `beach/music_beachside_puzzle_01.mp3`: full 172.36-second stereo playback,
48 kHz / 192 kbps, 100 ms entry/exit fades and encoding headroom. The runtime file
is 4,138,028 bytes and retains 66,186,240 PCM bytes at 48 kHz. Preparation hashes,
technical metrics and source permission records are under
`artifacts/audio-production/sources/beachside-puzzle/`.

The existing Music bus uses gain 0.18 before the saved Music slider. Master mute,
pause, host suspension and gesture activation apply. Wave effects continue on
the independent Effects bus with their random order and pauses. Only the current
music track's PCM is retained. The player uses the existing music memory budget
and generation-checked browser decoder; no engine or save format changes apply.

Verification on 19 October 2026: both native audio tests passed. The browser
loaded the hashed MP3, measured nonzero music output alongside random waves,
repeated the track after real backend completion (accelerated 64x for that check),
passed Music volume/mute/pause checks and restored a single garden music voice
on exit. No browser errors occurred. The WAV master was absent from the staged
site. Evidence: `build/evidence/beach-music/report.json`.

## Wave effects

The beach adventure randomly selects from the supplied `waves1.wav` through
`waves4.wav`, excluding the most recently played variant. Each clip plays once
at normal speed, followed by an independently randomized 3.55–4.35-second pause
after backend completion. Variants never overlap or repeat consecutively.
The start-to-start interval is 6.1–6.9 seconds: twice the original interval,
so waves occur half as often. The silent foam cycle also doubles to 6.5 seconds.
The reduced-frequency pass passed native audio/beach environment tests and the
generated browser check. Measured pauses were 3.592–4.291 seconds, giving
6.142–6.841-second intervals between starts; random selection, synchronized
foam, music and playback controls passed with zero browser errors. Evidence:
`build/evidence/beach-wave-frequency-final/report.json`. The earlier frequency
run confirmed the timing but hit a test analyser-cleanup race; the probe now
waits for a fresh wave before measuring its output.
The initial wave begins after audio activation and decoding. Waves use the
Effects bus at gain 0.12 and follow master mute and the saved Effects percentage.

The actual successful `beach_wave_*` playback event now starts the scene's
incoming foam wash. Two soft cove-shaped bands advance toward shore and fade
over the recording's 2.55 seconds; there is no separate sound timer or queued
catch-up. Slightly stronger water displacement and shallow foam make the surf
more visible. The wash uses the same presentation/pause policy as wave audio,
continues on a gentle silent cycle when muted, and is hidden with reduced motion.
Scene-owned component handles discard the effect on navigation; it owns no
puzzle, collision or saved progression state.

Surf verification on 19 October 2026: all four native audio, beach environment,
starter scene and release-one scene tests passed. The generated local-engine
Release site passed real wave playback, same-frame foam starts, pause, mute,
Effects controls and garden return on Chrome WebGPU and forced WebGL2, with
zero browser errors. Captures and reports are under
`build/evidence/beach-surf/webgpu/` and `verified-webgl2/`. These are desktop
browser results; the engine gitlink was not advanced by the surf change.

`GameAudio::beachWaves` owns the transient sequence over the existing engine
audio system. The browser enables it only in an active beach scene, including
beach dialogue/completion. Leaving the beach or replacing a scene cancels the
current wave; pause/page suspension freeze the device and sequence. Muting or
zero Effects volume schedules no new waves, and resuming cannot create a
catch-up burst. The background music playlist remains independent. Garden air
is disabled at the beach, and garden birds remain garden-only.

The four wave files load only after entering the beach. They are stereo 48 kHz,
16-bit PCM, 2.55 seconds each, using 3,916,800 decoded PCM bytes in total at
48 kHz. The existing generation checks reject stale decodes after navigation.
Failed variants are skipped, with no repeated fetch attempts until Enable sound
retries them. If only the previous variant is available, the sequence waits
rather than repeating it. The previous variant survives scene changes; a pending
choice is cleared on navigation. A randomly seeded local generator owns variant
and pause choices. No wave state is stored in progress saves.

`python scripts/prepare-beach-audio.py` records metadata/hashes and registers the
four stable `beach_wave_01`–`04` clip IDs, retaining source masters and preparing
the cleaned runtime samples described above.
The deployment manifest and browser decoder include the `beach/` audio folder.
Source permission/unknown provider and license status are recorded in
`assets/runtime/audio/beach/PROVENANCE.md`; final human approval is not asserted.

Verification: native `paws_audio` exercises random selection/no consecutive
repeats, varied gap bounds, actual backend completion, lazy loading, pause,
mute/Effects and cancellation;
`paws_release1_audio` checks existing cue integration. Run
`node scripts/beach-audio-smoke.cjs` against `PAWS_URL` for real file hashes,
normal-speed browser playback and mixer output, pause/mute/Effects settings,
and return to garden. Evidence is written to `build/evidence/beach-audio/`.

On 19 October 2026 both native audio tests passed. The real browser run passed
the original complete 1–2–3–4–1 sequence (before randomization), nonzero mixer output, pause/mute/Effects and
garden-return checks with zero browser errors. The Release preview is served
at port 8081; its Wasm SHA256 is
`1b4ecf33d2a53fdb4ba2b22183d04f8a7653728fba476e5ffa86ee8b1e1d61ff`.

The subsequent randomization pass passed both native audio tests and a real
browser run with eight waves: 4, 2, 1, 4, 3, 1, 3, 2. Measured pauses ranged
from 0.530 to 0.872 seconds, with no consecutive repeats or browser errors.
Pause, mute, Effects volume and garden return passed again. Current evidence:
`build/evidence/beach-audio-random/report.json`.

# Golden Afternoon Garden playlist

The game plays **Golden Afternoon Garden 1**, then **Golden Afternoon Garden 2**,
and repeats that order. Play, Continue or Enable sound activates the browser audio
device. Music continues through garden, puzzle and ending transitions. Pause and
page suspension freeze playback; returning from host suspension requires a gesture.
The existing Music volume and Sound mute controls apply to the playlist.

The original stereo WAVs remain in `assets/music/`. The second supplied filename is
`Golden Afternoon Garden2 .wav`. `python scripts/prepare-garden-music.py` prepares
full-length 48 kHz stereo, 192 kbps MP3 copies with 100 ms entry/exit fades and
headroom for encoding. Runtime copies are about 4.3 MB each; no WAV masters ship.
The manifest's `playlists` record and `src/music_playlist.hpp` share the ordered
stable clip IDs. Those reuse the existing afternoon/puzzle music slots, so catalog
indices stay compatible. Each track plays once before advancing, regardless of
the legacy single-track loop flags.

`GameAudio` owns the playlist over the pinned engine backend. It retains only the
current music PCM and voice, advances on actual backend completion, skips failed
tracks, and validates replacements before changing playback. Scene changes cancel
pending old-generation decodes and scene effects while preserving background music.
Enable sound retries failed clips. All-failed playlists stay quiet without repeated
fetches. Effects retain their 24 MiB clip limit; music permits 96 MiB per decoded
track and the playlist permits 128 MiB total retained PCM. The supplied tracks use
about 65/67 MiB at 48 kHz. Browser decoder, Web Audio and Wasm transfer copies add
transient memory beyond retained PCM. Physical mobile performance remains untested.

Preparation metrics, hashes and source records are under
`artifacts/audio-production/sources/golden-afternoon/`. The user states that they
created the tracks and authorized game playback. They are marked technically
playable; independent rights verification and final human listening approval are
not asserted.

## Verification on 2026-10-11

The pinned Release browser target built successfully. Native `paws_audio`,
`paws_release1_audio`, `paws_starter_scene` and `paws_release1_scene` passed.
`npm run test:browser:music` exercised the generated game with its actual MP3
files: hash/decode/publication, nonzero music output, garden/puzzle transitions
preserving the same source, track order/repeat, mute/volume, pause and gesture
resume. Track-completion automation used 64x source playback speed; it does not
claim a full-duration listening review. The Chrome run retained at most 74,380,904
PCM bytes and two simultaneous voices, with no decode failures or page exceptions.
Evidence and screenshots are in `build/evidence/garden-music/`.

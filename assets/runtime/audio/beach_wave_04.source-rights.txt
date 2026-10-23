# Beach wave recordings

The user supplied `waves1.wav` through `waves4.wav` in this folder and explicitly
requested their use as alternating beach sound effects on 19 October 2026.
The provider, generation settings and original license were not supplied;
independent ownership/license review and final listening approval are not asserted.

The original supplied PCM is retained under
`artifacts/audio-production/sources/beach-waves-original/`. On 22 October 2026,
the runtime copies were revised at the user's request: the opening 300 ms impact
is silenced and a 200 ms raised-cosine fade leads into the surf. Samples after
500 ms are unchanged and timing is preserved. Each file is stereo, 48 kHz,
16-bit PCM, 2.55 seconds long. Technical metadata and SHA256 hashes are recorded
by `scripts/prepare-beach-audio.py` in the runtime audio catalogue. Runtime
gain is 0.12 (previously 0.25); the Effects control and master mute apply. One variant plays at a
time; the game controls the random pauses and selects a different variant.

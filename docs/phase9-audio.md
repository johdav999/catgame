# Phase 9 audio integration

The game now has audio routing and a real browser backend, but **no final audio
is installed**. This is an integration delivery with an open production gate.
The connected-plugin search found no ElevenLabs or Suno integration. No generation,
source audition, provider rights verification or final journey listening is claimed.

## Production handoff

The [exact production checklist](phase9-audio-production.md) covers 29 ElevenLabs
families / 44 clips and three Suno music deliveries. The runtime manifest is
`assets/runtime/audio/catalog.json`; its 47 entries remain `accepted: false` until
real files pass the [source/import workflow](../artifacts/audio-production/README.md).
There are no silent WAVs or replacement-provider tracks in the deployed tree.

`scripts/prepare-audio-catalog.py` derives the stable manifest, C++ catalog and
checklist from the prompt inventory while retaining existing import metadata.
`scripts/import-audio.py` checks review/rights metadata, original/output hashes,
PCM16 layout, peak <= -2 dBFS, DC offset, length and full-buffer loop boundaries.
It stages only the reviewed WAV and distribution notice, retains a waveform
envelope under `artifacts/audio-production/reviews/`, and distinguishes source
approval from actual-game listening. MP3 originals remain unchanged outside runtime.

## Ownership and event policy

`GameAudio` owns an engine `AudioSystem` and Music, Ambience and Effects buses.
`BrowserGame` owns this service and projects committed gameplay into cues. Scene
replacement/import/restart stops voices and advances an audio generation before
scene destruction. Async decoded candidates with old generations cannot publish.
Music is loaded on demand; short cues preload serially after activation. Pending
one-shots expire after 400 ms and never play later in an unrelated scene.

The host uses the pinned `BrowserAudioBackend` and `EngineAudioDevices.decode`.
Play/Continue/Enable sound issue activation synchronously; trusted later gestures
can resume after page suspension. Visibility alone does not autoplay. Settings
show a readable enable/retry or missing-files explanation. Preferences use the
existing phase-8 snapshot/restore path. Existing 10% volume buttons provide mouse,
touch and keyboard adjustment; there is no engine slider widget in this pin.
Settings remain useful without audio files. Save acknowledgements have no cue.

| Event | Cue |
| --- | --- |
| Title/garden/shop; puzzle; ending | Corresponding Suno track |
| Active environment, sparse bird accents | Garden air and bird variants |
| Dialogue / request | Pip request variants with cooldown |
| New puzzle completion; placed furnishing | Pip happy variants |
| Walk quarter-cycle contacts / happy landing | Paw variants, omitted for reduced motion |
| Cushion settle / bounded rest; bowl drinking | Settle / purr / drink |
| Selection, button activation, panel change | Select, confirm, panel |
| Committed undo/hint, rejected action | Undo, hint, blocked |
| Increased reward wallet; accepted purchase | Coins, purchase |
| Accepted pot/cushion movement, channel rotation | Slide variants, rotate variants |
| Validated ball presentation / arrival or blocked roll | Bounded roll, contact variants |
| Accepted toy pickup / matching placement | Pickup, basket variants |
| Actual switch occupancy changes, including undo/return | Switch down/up and gate open/close |
| Accepted furnishing placement/move | Cushion place or bowl place |

Cat contact timings are presentation markers and never advance rules. Fine
audible contact alignment remains part of final clip audition. Source immutability
and checked cat component handles are preserved. Ordinary hint/move/menu actions
retain the current music. Reduced motion remains fully playable without sound.

## Budgets and loop policy

- At most 12 effects and 14 total voices, one looping ambience bed and one music
  voice. Variant rotation and cooldowns limit repeated cues. Purr is bounded to
  five seconds per request and ball roll to four; scene cancellation is immediate.
- At most 24 MiB PCM per decoded clip and 48 MiB retained in the Paws cache. The
  host budgets decode at the device sample rate because Web Audio resamples.
  Backend AudioBuffers can duplicate retained PCM; decoder and transfer copies
  add transient memory. These counters are not a total-browser memory measure.
- The pinned engine loops the entire buffer. Import pre-trimmed loop WAVs; it
  has no exposed subrange loop points or sample-accurate crossfade scheduling.
  Music fades out over 250 ms, releases, then loads/fades in the next track. No
  overlapping full music decodes are scheduled. Ending plays once per entry.
- Initial music gain is 0.18, ambience 0.2 and effects usually 0.35, multiplied by
  user bus settings; selection/panel/paws are quieter. These are provisional mix
  choices. Final sources, mixed clipping, musical seams and device decode peaks
  still require acceptance. Synthetic low-level signals do not prove those.

## Build and verify

```powershell
./scripts/build.ps1 -Pinned -Configuration Release -BuildDirectory build/web-phase9-pinned-release -EmsdkPath C:/Users/Johan/source/repos/WebEngine/.tools/emsdk
./scripts/serve.ps1 -SiteDirectory build/web-phase9-pinned-release/site -Port 8099
ctest --test-dir build/native -R '^(paws_audio|paws_ui|paws_starter_scene)$' --output-on-failure
python tests/test_audio_import.py
python scripts/import-audio.py --check
npm run test:browser:phase9
```

The browser runner reuses the complete phase-7 journey and intercepts audio
requests with **test-only non-silent signals in memory**. It never writes these
to assets. Reports separate this backend evidence from imported/final audio. A
second context checks the actual empty production catalog. Evidence is under
`build/evidence/phase9/journey/`; exact results are recorded in implementation
status. Physical speakers, Android/iOS and final audio listening remain open.

Before phase 10 can close audio acceptance: obtain all 47 provider files and
rights records; review/import; audition five actual-backend repeats per loop;
listen to the entire three-quest/shop/use/ending journey; record clipping, timing,
decode peaks and physical-device evidence. Do not call this production-ready.

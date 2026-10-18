# Garden ambience

Generated on 11 October 2026 using the personal ElevenLabs skill and
`eleven_text_to_sound_v2`, from four explicitly authorized generation requests.
Original MP3s and the exact request/provenance sidecars remain in
`artifacts/audio-production/sources/garden-ambience/`. The provider reported a
combined character-cost of 325; no monetary price is inferred.

| Runtime file | Duration | Format |
| --- | --- | --- |
| `assets/runtime/audio/amb_garden_air_01.wav` | 24.750249 s, full-buffer loop | Stereo, 44.1 kHz, PCM16 |
| `assets/runtime/audio/amb_garden_bird_01.wav` | 2.48 s | Mono, 44.1 kHz, PCM16 |
| `assets/runtime/audio/amb_garden_bird_02.wav` | 2.48 s | Mono, 44.1 kHz, PCM16 |
| `assets/runtime/audio/amb_garden_bird_03.wav` | 2.48 s | Mono, 44.1 kHz, PCM16 |

The breeze has a circular 250 ms end/start crossfade and complete-buffer loop
boundaries. Birds have 20 ms entry/exit fades. Each export has removed DC offset,
-6 dBFS peak headroom, an immutable content hash and a distribution-status notice.
The four decoded buffers total about 10.42 MiB at a 48 kHz browser device rate.
Detailed measurements are in
`artifacts/audio-production/reviews/garden-ambience/technical-report.json`.

## Playback

The breeze uses the existing Ambience bus and 0.2 clip gain. The three bird calls
rotate through GameAudio, with varying 18–37-second gaps during audible garden
presentation. The countdown freezes during pause, mute, zero ambience volume,
loading and host suspension, and starts fresh after scene replacement. Birds are
preloaded again when a scene transition cancels outstanding decode requests.
There is no catch-up burst or extra ambience voice per frame.

Suspension reaches the backend only on an actual state change. Repeating the
same suspension value every frame would invalidate its asynchronous activation
revision and intermittently prevent gesture resume; regression checks cover this.

Choose Play/Continue, then interact with the loaded garden if the browser requires
a fresh audio gesture after loading. Menu → Settings → Enable sound also resumes
audio. The Ambience volume and master mute controls apply to all four files.

## Verification and acceptance

The catalog marks these real files `playable: true`, `technicalVerified: true`,
and `accepted: false`. This permits local game playback with hash/PCM validation
without claiming a human listening review or verified distribution rights.
The existing final release completeness gate still requires accepted audio.
Account plan/attribution evidence and human listening remain recorded as open.

Native checks cover sparse scheduling, inactive countdowns, long-frame bounds,
cached ambience retention and restarting cancelled preloads, plus the portable
scene regression. The browser smoke serves the actual pinned Release game and
its real WAVs without interception or synthetic fixtures. It observes five loop
traversals, all three naturally scheduled variants, nonzero output signal, memory,
pause/mute and gesture resume. Results belong under
`build/evidence/garden-ambience/summary.json` combines the recorded 131.57-second
playback pass (five traversals and all three variants) with the separate corrected
lifecycle pass in `report.json`. Automated signal checks do not
establish physical speakers/headphones or subjective sound quality.

```powershell
node scripts/garden-ambience-smoke.cjs
# Short lifecycle regression after a resume fix; does not claim five loop checks:
node scripts/garden-ambience-smoke.cjs --lifecycle-only
```

`scripts/prepare-garden-ambience.py` documents and performs the repeatable original
decode/edit/export steps (numpy and ffmpeg). It refuses existing runtime WAVs and
does not make API calls. Do not rerun it merely to regenerate already delivered
files or overwrite later reviewed exports.

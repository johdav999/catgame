# Pip sound delivery

Ten real ElevenLabs effects generated on 2026-10-11 through the installed
`elevenlabs` skill and the repository's encrypted credential. No API key is
included in audio, provenance or the generated browser site.

| Runtime files in `assets/runtime/audio/` | Gameplay trigger |
| --- | --- |
| `pip_request_01.wav`, `pip_request_02.wav` | Pip's request opens; variants alternate |
| `pip_happy_01.wav`, `pip_happy_02.wav` | A requesting Pip's puzzle completes; variants alternate |
| `pip_purr_01.wav` | Pip rests on a cushion/hammock; at most five seconds per use |
| `pip_paws_stone_01.wav` through `_03.wav` | Pip walks; three padded paw variants alternate |
| `pip_settle_01.wav` | Pip enters the cushion pose, once per entry |
| `pip_drink_01.wav` | Pip reaches the bowl's drinking pose, once per use |

Owned, placed cushions and bowls also offer **Invite Pip to rest** and
**Invite Pip to drink** in Shop → furnishing details. The ordinary Use furnishing
control continues to invite Button/Mabel when available. Their contacts do not
trigger Pip's sounds. Busy/reserved furnishings retain their existing checks.

`PipSounds` observes the existing garden simulation without owning actors,
animations, reservations or rules. Movement distance uses the canonical
0.36-metre/1.2-second stride, with four contacts per cycle. No steps occur for
stationary poses, reduced-motion teleporting, paused simulation or long-frame
catch-up. The browser resolves Pip's scene-scoped actor handle for every sample
and resets the projection on scene replacement. Settling starts at cushion entry;
purring starts in the supported resting pose; lapping stops when drinking ends.
Request cues wait for the browser device to resume after scene preparation, and
are dropped on mute or a changed audio generation. Completed rewards remain
the committed owner of happy trills.

All ten sounds use the Effects bus, existing mute/volume controls, variant
cooldowns and voice budgets. Scene cancellation invalidates delayed decodes;
small Pip buffers preload again after replacement. These ten buffers occupy
about 3.4 MiB at a 48 kHz device rate, separate from background music.

## Files and provenance

- Provider downloads and exact prompt/settings/hash sidecars:
  `artifacts/audio-production/sources/pip-sounds/`.
- Runtime mono 44.1 kHz PCM16 WAVs and notices: `assets/runtime/audio/`.
- Per-file source, processing and review records: `assets/runtime/audio/catalog.json`.
- Offline measurements: `artifacts/audio-production/reviews/pip-sounds/technical-report.json`.
- Browser evidence: `build/evidence/pip-sounds/report.json` and screenshots.

`scripts/generate-pip-sounds.py` calls the skill helper once per missing source
and preserves completed downloads. `scripts/prepare-pip-sounds.py` exports WAVs,
removes DC, sets peak headroom to -6 dBFS, fades one-shot boundaries and trims
the provider's minimum 500 ms paw files to a single energy-centred 230 ms contact.
The 9.75-second purr has a circular 250 ms seam blend and full-buffer loop bounds.

Delivery is `playable: true` with recorded technical verification. Human listening,
account distribution rights and final release acceptance remain pending; these
fields are explicitly false/unverified rather than inferred from generation.

## Verification

The portable audio test covers stride contacts, no phantom/catch-up steps,
single settling/drinking cues, pause preservation, reduced-motion rest and reset,
alongside the existing generation, loop, mute, budget and playlist regressions.
The portable scene test checks the existing scene/lifecycle contract.

Run `node scripts/pip-sounds-smoke.cjs` against the generated pinned-engine site.
It exercises three real Pip quests, buys and places the cushion/bowl, observes
all ten decoded provider files and real WebAudio voices, checks per-file nonzero
unclipped signal, pause/mute/resume and other cats' identity separation. It does
not replace audio routes, synthesize fixtures or claim human/physical-device
audition. Desktop headless Chrome does not establish mobile audio acceptance.

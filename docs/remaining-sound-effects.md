# Remaining sound effects

Generated 49 requested ElevenLabs files through the installed `elevenlabs` skill
and the existing encrypted repository credential. No key is stored in source,
provenance or the generated site. The earlier ambience, Pip sounds and supplied
background playlist are preserved.

| Group | Files | Trigger |
| --- | ---: | --- |
| Mabel, Button, Pepper | 6 | Their request opens and their puzzle completes |
| UI and rewards | 9 | Selection, confirmation, panel open/close, undo, hint, blocked action, earned coins, purchase |
| Ball and channels | 9 | Accepted pot slide, channel turn, visible ball travel and actual arrival |
| Toys and cushions | 7 | Pickup, basket placement, fabric slide and correct cushion fit |
| Gates and construction | 8 | Actual switch/gate edges, crate slide, plank fit/connection |
| Garden bells | 4 | Leaf/star/paw strikes, followed by reset after a wrong symbol |
| Water | 2 | A completed connected route flows, then the bowl fills |
| Furnishing interactions | 4 | Soft furnishing or ceramic bowl placement; actual scratching/playhouse use |

Runtime files live in `assets/runtime/audio/`; the nineteen release-one additions
live in its `release1/` subfolder. The catalogue records every source, edit, hash,
PCM measurement and review status. Original MP3 downloads and exact provider
request sidecars are in `artifacts/audio-production/sources/remaining-effects/`.
The generation manifest is `artifacts/audio-production/remaining-effects-jobs.json`.
Offline reports are in `artifacts/audio-production/reviews/remaining-effects/`.

The new exports use mono 44.1 kHz PCM16, removed DC, -6 dBFS peak headroom and
short entry/exit fades. Transient UI/switch/ball sounds are cropped around their
energy peak to the authored contact duration. Ball roll and water flow use a
circular 250 ms seam blend and complete-buffer loop bounds. The three bells
retain provider timbres with pitch alignment to E5 (leaf), G5 (star) and C6 (paw),
so the symbols have distinct sounds. Gentle bell EQ and tail fades keep the
identifying notes clear. The 49 decoded buffers total about 7.61 MiB
at a 48 kHz device rate.

## Runtime policy

Only accepted commands produce material or reward cues. Pressure/gate sounds
follow changed before/after object states, including undo, restart or a cat
releasing its switch. Cushion movement uses fabric rather than terracotta;
ceramic bowl placement has its own cue. A wrong bell still sounds its own
symbol before the gentle reset. Ball arrival follows the visible roll rather
than firing at command dispatch; undo/restart/replacement cancels that receipt.
Furnishing-use cues start at the supported Use phase, after the cat approaches.

The game retains its existing variant rotation, cooldowns, 400 ms decode expiry,
generation rejection, mute, Effects volume and combined 14-voice limit. World
audio pauses with simulation. A second existing engine backend supplies at most
two menu-feedback voices while user-paused; host suspension freezes both devices.
Blocked, hint, undo and purchase feedback take priority over decorative clicks
and panel tails, so fast navigation cannot hide the useful response.
No new engine code or submodule revision is needed. UI sounds own no scene data.

The release-one packaging refresh preserves imported audio metadata rather than
resetting the nineteen authored slots on every build. There are now 63 playable
effects/ambience files plus two supplied playable music tracks. The remaining
legacy ending-music slot is outside this sound-effect request; the supplied
background playlist continues to accompany the ending.

## Verification and review

Native audio, committed-event, UI and portable scene regressions cover paused
menu feedback, mute/host suspension, shared voice limits, switch/gate release,
cushion material selection, wrong-bell feedback and existing lifecycle rules.
Run `npm run test:browser:remaining-effects` against the generated pinned-engine
production site for real provider decoding and playback through the quests,
menus and furnishings. Reports/screenshots are under
`build/evidence/remaining-effects/`.

These files are playable with technical verification. Human source/journey
listening, physical-device acceptance and account distribution/attribution rights
remain explicitly pending. Generation and automated signal checks do not replace
those release reviews.

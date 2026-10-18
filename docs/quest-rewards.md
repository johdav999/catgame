# Quest completion rewards

After the existing quest reaction finishes, a gold paw coin rises above each
requesting cat for 1.2 seconds with a new ElevenLabs coin chime. A coral heart
follows for 2.2 seconds with a new one-shot purr. The old dimmed “Thank you!” card
is replaced by a compact receipt and **Back to garden** button. The shared camera
frames the requesting cats and their rewards. There is no timer or automatic
navigation forcing the player out of the scene.

The authoritative Afternoon reward receipt starts the sequence once. Practice
keeps its cat reaction and return control without pretending to grant more coins
or friendship. Reload does not replay a grant. Undo/restart and navigation cancel
tokens and their sounds. Host suspension and renderer unavailability hold the
timeline. Reduced motion shows stationary coin and heart tokens in sequence.
Sound mute/effect-volume settings continue through the existing audio owner.

Tokens use the engine's Overlay layer with world-space positions so playhouse
roofs and foreground props cannot obscure this feedback.
Both models render at half their original size (0.5 scale), including reduced
motion; the existing animated scale variation is multiplied by the same factor.

`src/quest_rewards.cpp` owns presentation only, using generation-checked actor
handles, shared meshes and an explicit Application presentation delta. It never
changes cat transforms, puzzle state, wallet, friendship or saves. Rendering,
resource import and playback remain WebEngine responsibilities. `GameAudio`
preloads the two short effects after consent and scene-generation changes.
The audio catalogue retains its original 66 slot indices and appends the new
`reward_friendship_01` one-shot; the existing coin slot gets the requested new
delivery. Its former source/edited WAV and catalogue record are retained under
`artifacts/audio-production/quest-rewards/2026-10-15`.

## Delivered assets

The two original references were generated with built-in ImageGen. Their exact
[prompts](../artifacts/model-input-images/quest-rewards/prompts.json) and native
[coin](../artifacts/model-input-images/quest-rewards/coin-reference-v1.png) /
[heart](../artifacts/model-input-images/quest-rewards/heart-reference-v1.png)
pixels are retained. They were submitted to Tripo image-to-model, preserving the
skill defaults (detailed geometry, 50k face request, detailed textures, PBR).
The provider returned UUID task IDs; the existing UUID-compatible helper resumed
the recorded jobs without resubmitting them.

The [Blender processing and review job](../artifacts/model-jobs/quest-rewards/2026-10-15/README.md)
retains editable packed masters, original provider GLBs, normalization/export and
clean-import scripts, measurements and review renders. Runtime pivots are
centered, Y is up and fronts face +Z. Tokens are about 0.46 metres wide and are
presentation surfaces without interaction/collision or shadows. Provider texture
islands were welded before decimation to prevent cracks. Runtime maps are 512px
derivatives of the retained provider originals.

| Asset | Runtime file | Geometry |
| --- | --- | --- |
| Gold paw coin | [reward_coin.glb](../assets/runtime/models/rewards/reward_coin.glb) | 2400 triangles, one material |
| Coral heart | [reward_heart.glb](../assets/runtime/models/rewards/reward_heart.glb) | 1600 triangles, one material |
| Coin chime | [reward_coins_01.wav](../assets/runtime/audio/reward_coins_01.wav) | 1.2 s, mono 44.1 kHz PCM16 |
| Friendship purr | [reward_friendship_01.wav](../assets/runtime/audio/reward_friendship_01.wav) | 2.2 s, mono 44.1 kHz PCM16 |

Both sounds were generated using ElevenLabs Sound Effects v2, with originals,
exact prompts, settings, hashes and cost headers retained beside their delivery
records. Runtime copies remove DC, apply 25 ms entry/exit fades and peak-normalize
to -6 dBFS. Provider account distribution rights and human listening approval
remain unverified; see the model notice and sound rights records. Technical
playability does not silently mark these as rights-approved.

## Reproduce verification

Build and run `paws_quest_rewards_tests` for asset import, sequence ordering,
one-shot cue edges, rise, reduced motion, cancellation, stale handles and multiple
requesters. Pass `build/evidence/quest-rewards/fixtures` as its sole argument to
generate valid production-owner saves for quests 0, 8 and 9. Native scene, GUI and
audio tests cover the surrounding owners and layout contracts.

Build the actual browser target, serve `build/web-local-release/site`, then run:

```powershell
$env:PAWS_URL='http://127.0.0.1:8156/'
node scripts/quest-rewards-smoke.cjs
$env:REWARD_BACKEND='webgl2'
node scripts/quest-rewards-smoke.cjs
```

The script uses trusted puzzle gestures and checks the requesting-cat count,
coin-before-heart order, visible world-space rise, suspension, actual model
requests, real nonzero WebAudio output from both generated effects, once-only
cues, saved rewards, reload, and the portrait four-cat reduced-motion case.
Evidence lives under `build/evidence/quest-rewards`. Build/Wasm identity is checked
against the generated site. Browser desktop/viewport checks do not establish
physical-device performance or human audio audition.

Verified on 15 October 2026: the five selected native suites passed, both browser
targets built, and WebGPU/WebGL 2 each passed single-cat, two-cat and four-cat
portrait/reduced-motion scenarios. Both generated effects reached nonzero actual
WebAudio output in each scenario. The final reports have no page/console errors
and identify Release build `paws-0.1.0-190ec584a889879a`, Wasm SHA-256
`f5b52a46f36fdb997dee9b1e644eefe8201d42d0b206c9e460338b0c6b4ab3ae`.
The runtime manifest/staging audit passed. The engine gitlink was unchanged.

# Prompt 08 handoff — release-one audio

Implemented 11 October 2026. **Catalog/import/event wiring preparation is complete
and verified in an isolated pinned browser harness. Production audio and listening
acceptance remain open.** The production game retains its existing 47-slot audio
registration until Prompt 9 applies the supplied integration changes.

## Delivered paths

- `src/release1/audio/cues.hpp`, `cues.cpp`: committed receipt projection over
  existing GameAudio; scene/attempt/quest/revision rejection, deduplication,
  400 ms event expiry, pause cancellation, music selection and picnic alias.
- `src/release1/audio/preview_main.cpp`, `web/release1/audio/`: isolated browser
  playback/mixer/lifecycle inspection. It loads the actual pinned audio backend;
  it is a development harness, not a replacement GUI or gameplay owner.
- `scripts/release1/audio/prepare.py`: 19-clip inventory and exact provider briefs,
  combined 66-slot preview catalog, generated shared-runtime fixtures and patch.
- `scripts/release1/audio/import_audio.py`: source/rights/review validation,
  immutable accepted imports, PCM16/duration/peak/DC/loop/hash checks, decoded
  budgets, actual measurements and accepted manifest publication.
- `tests/release1/audio/`: original audio regression suite exercised against the
  expanded runtime plus receipt/cancellation tests and six importer test groups.
- `scripts/release1/audio/smoke.cjs`: real browser-backend tests with non-silent
  synthetic signals served only through test routes; also checks the actual
  missing-file catalog without interception.
- `artifacts/audio-jobs/release1/`: source catalog, exact generation/import briefs,
  metadata template, integration/listening instructions and `shared-runtime.patch`.
- `docs/release1/fragments/audio.json`: preparation fragment; zero accepted assets,
  references to 47 existing catalog slots and picnic alias. No fabricated hashes,
  licenses or zero-length runtime placeholders.

`src/release1/audio/runtime/` and `tests/release1/audio/legacy_tests.inc` are
generated fixtures, not a second production runtime. Prompt 9 applies the narrow
shared patch and builds `cues.cpp` with `PAWS_RELEASE1_AUDIO_PRODUCTION=1` instead.
No shared composition, production manifest, build registration, common status
document or engine file was edited by this task. All other dirty/concurrent work,
including new ElevenLabs setup documentation and `.gitignore`, was preserved.
No additional agents, commit, push or publication.

## Catalog and behavior

Keep all 47 MVP clip IDs/order/provider requirements. Add 19 ElevenLabs deliveries
covering the 17 new effect families: two crate variants, two plank variants and
one of every other effect. `picnic_complete`, the eighteenth frozen family,
aliases the existing Suno ending phrase. Reuse garden and puzzle beds across all
three chapters; existing cushion, retrieval, gutter rotation, switches, gates,
paws, resting/drinking, UI and rewards remain shared. No extra chapter tracks.

The runtime patch preserves activation, settings, serial decoding, 12 effects /
14 total voices, 24 MiB per clip, 48 MiB cache, cooldown/variant rotation and
400 ms pending-shot expiry. Water uses the same cue queue and whole-buffer
playback, bounded to four seconds; new revisions, bowl fill, pause and scene
replacement cancel it. Visible symbols and committed rules remain authoritative
when muted. Audio events never grant progress or determine a puzzle result.

Read the [integration README](../../artifacts/audio-jobs/release1/README.md) for
the exact owner seam and event mapping. Shared patch includes the easily missed
BrowserGame hardcoded `index >= 47` publication guard, expanded host URL/count
validation and CMake registration. `git apply --check` passes against this
checkout. The combined preview catalog must be merged/copied by Prompt 9 after
real import validation; it is not installed automatically by this task.

## Verification actually run

Pinned engine: `3fe7d1dc1dc610c703944803cbb717cd65024ec9`, clean.
Emscripten 6.0.11 Release browser harness:
`build/web-release1-audio/site/release1_preview.wasm` SHA-256
`b350ad12be08e7106076e37b3033f51402e841ea6825bfc9385344ccb9af017d`.

- Native `release1_audio`: 1/1 passed, including the prior audio regression suite
  against 66 slots and new receipt/expiry/water/muted-bell/picnic tests.
- Existing portable `paws_audio` and `paws_starter_scene`: 2/2 passed.
- Python importer: 6/6 test groups passed (measured fixture/resampling, immutable
  imports, source/provider/rights metadata, silence/clipping/duration, loop review,
  byte tampering and frozen families/legacy slot preservation).
- Contract/fragment validation and shared patch applicability passed.
- Chrome 154.0.8037.99: actual generated site at nested `/review/index.html`,
  63 serial ambience/effect decodes, all new effect families, rejected stale event,
  four-second water termination, pause cancellation, gesture resume, real
  nonzero mix, master mute, rapid music transitions and no browser exceptions.
- **Synthetic fixture measurements only:** peak five voices, 6,085,544 bytes
  retained PCM, 176,396-byte largest decode, mixed sample peak approximately
  0.01127. These prove backend plumbing, not production budgets or listening.
  Native regressions separately exercise the 12-effect cap and 14-voice ceiling.
- Actual unmodified catalog: zero imported, 66 missing, zero voices and zero WAV
  requests. Final-catalog decoded memory is `null` until actual final clips exist.
  `--require-complete` deliberately fails while files/listening are missing.

Evidence: `build/evidence/release1/prompt-08/` contains `native.txt`,
`baseline-audio-scene.txt`, `import-tests.txt`, `catalog-audit.json`,
`contracts.json`, and `browser/report.json` with screenshots and final Wasm hash.
Initial preview compilation found/fixed an EM_ASM object-literal macro issue and
a revision narrowing conversion. A browser check exposed a test timing race:
Web Audio gain scheduling is now awaited through the next device render quantum.
Final checks above passed after correction. A scope notification was also fixed
to abort host decoding only when audio generation changes, not on every revision.

## Reproduce

Run from repository root; installed compiler tools must be accessible. Supply
the native compiler's bin directory on PATH for its runtime DLLs. Local SDK/Jolt
paths below identify this machine only; no machine path is committed to shared
build configuration.

```powershell
python scripts/release1/audio/prepare.py
python tests/release1/audio/test_import.py
python scripts/release1/audio/import_audio.py --check
python scripts/release1/validate-contract.py --fragment docs/release1/fragments/audio.json
git apply --check artifacts/audio-jobs/release1/shared-runtime.patch

$audioRepo = (Get-Location).Path.Replace('\','/')
$audioSources = "$audioRepo/src/release1/audio/cues.cpp;$audioRepo/src/release1/audio/runtime/game_audio.cpp"
$audioJolt = 'C:/Users/Johan/source/repos/WebEngine/build/dependencies/JoltPhysics-5.3.0'
cmake -S tools/release1-track -B build/native-release1-audio -G Ninja -DCMAKE_BUILD_TYPE=Debug -DPAWS_TRACK=audio "-DPAWS_TRACK_SOURCES=$audioSources" "-DPAWS_TRACK_TEST=$audioRepo/tests/release1/audio/tests.cpp" "-DFETCHCONTENT_SOURCE_DIR_JOLT=$audioJolt"
cmake --build build/native-release1-audio --target release1_track_tests --parallel 4
ctest --test-dir build/native-release1-audio --output-on-failure
ctest --test-dir build/native -R '^(paws_audio|paws_starter_scene)$' --output-on-failure

& C:/Users/Johan/source/repos/WebEngine/.tools/emsdk/upstream/emscripten/emcmake.exe cmake -S tools/release1-track -B build/web-release1-audio -G Ninja -DCMAKE_BUILD_TYPE=Release -DPAWS_TRACK=audio "-DPAWS_TRACK_SOURCES=$audioSources" "-DPAWS_TRACK_BROWSER_MAIN=$audioRepo/src/release1/audio/preview_main.cpp" "-DPAWS_TRACK_WEB=$audioRepo/web/release1/audio" "-DFETCHCONTENT_SOURCE_DIR_JOLT=$audioJolt"
cmake --build build/web-release1-audio --target release1_track_site --parallel 4
node scripts/release1/audio/smoke.cjs
./scripts/serve.ps1 -SiteDirectory build/web-release1-audio/site -Port 19619
```

Smoke owns port 19618 and temporary browser contexts. `BROWSER_EXECUTABLE`,
`SITE_DIR` and `EVIDENCE_DIR` override its Chrome/site/evidence paths. The manual
preview at port 19619 remains silent until accepted imports are supplied.

## Precise remaining inputs and gates

No supplied WAV/MP3/OGG/FLAC/M4A originals were found in assets/artifacts. The
provider plugin search returned no ElevenLabs/Suno matches. A concurrently added
personal ElevenLabs helper was read and checked: `status --repo .` reported
**No API key configured**. No paid API calls or substitute-provider output.

Required: 63 real ElevenLabs effect/ambience clips (44 existing + 19 new), three
real Suno deliveries, exact source/settings/download provenance, actual rights
records and distribution notices, edited WAVs, source/loop reviews and final
browser journey listening. Configure the local ElevenLabs helper or provide
originals; Suno instrumental music still requires its selected-provider inputs.
No skill or approval rule forced a pause: missing authenticated provider inputs,
sources and listening evidence are the dependencies.

After import, audition five actual-backend repeats per loop and all variants;
review bell pitches/balance, mix, pause/background/resume and rapid transitions.
Measure actual resampled PCM/decoder/backend buffers. Prompt 9 must wire the
committed owner events into the ten-quest game and verify the complete journey.
Physical-device output, final music seams, actual rights, full-game listening
and production-audio memory are not established by these automated fixtures.

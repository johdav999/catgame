# Prompt 9 — the production release-one afternoon

Implemented 11 October 2026 in the actual `paws`/`paws_site` targets. This is a
review candidate: the complete afternoon is integrated, while final audio,
independent-player and physical-device release gates remain open. No engine
source, gitlink, commit, push or publication was changed by this integration.

## Delivered composition

The existing `Afternoon` now owns three preserved legacy sessions and seven
new attempts through the Prompt 5 policy/codec. Ten rewards total 200 coins;
eight furnishings cost 190. Pip ends at 5 friendship; Mabel, Button and Pepper
at 3 each. Introductions, both Chapter 2 orders, both bells/playhouse orders,
free hints, undo, restart and reward-free practice use the same authoritative
generation/attempt/revision command boundary. Availability comes from prepared
presentation assets. There is one Application, scene owner and save coordinator.

Production links the delivered rules, progression, cat/prop, garden, retained
GUI and cue modules. Track fixtures, debug entry points, galleries and preview
shells remain development-only. The pinned bridge remains build integration.
The engine is the clean exact pin `3fe7d1dc1dc610c703944803cbb717cd65024ec9`.

All 21 original garden meshes, 30 new prop meshes, original Pip authority and
four separate r24 cat exports retain their actual bytes/notices. Four final
portraits, eight model thumbnails and fonts are registered. The merger/auditor
verifies stable relative URLs, bytes, SHA-256 and rights-record presence. New
cat clips/binds validate before scene activation; cats have independent playback.
The original accepted Pip GLB/master remain unchanged.

Seven new boards project the actual assets, including matching normalized
cushion/socket rotations, supported plank endpoints, validated water surfaces,
independent bells, live simultaneous switches/door and readable spatial clues.
Requesting cats stand on accepted paving. Physical legacy gestures and retained
keyboard/touch object/actions share the owner; new physical taps open that same
action view. The camera fits the retained GUI's unobstructed board rectangle.

The garden reconstructs nine stable placement spots and eight visible uses.
Rest/drink/scratch alignment uses actual cat surface measurements. The final
Button hammock height is measured against curved cloth using all skinned
vertices over 241 samples of the full curl: minimum root Y 0.402113 m; shipped
Y 0.405 m. Its record is bound to the exact Button export. Perch/entry paths,
independent reservations, cancellation and reduced-motion poses remain game
presentation, never progress or physics-derived puzzle legality. Real-time uses
and their images are scoped evidence, not physical-device or aesthetic certification.

After all ten paid requests and one owned furnishing placed, a detached garden
candidate prepares four accepted cats and four seat reservations before publishing
the once-only picnic record. It supplies table/mat when unpurchased. The opening
stages short arrivals at the picnic apron; normal use walks from home. The closing
card leaves the actual gathering visible beside/above it, and friends stay seated
while it is read. Continued play and replay remain available without new rewards.

`PAWS_AFTERNOON 6` preserves supported versions 1–5, legacy attempts/journals,
wallet/purchases/settings and the historical MVP ending. That ending does not
mark the new picnic seen. Import validates a candidate before publication;
explicit pause survives import/reload/background and GPU recovery. The existing
Web Lock, independent temporary tab, five snapshots, coalescing and latest-save
receipt policies remain in `web/saves.js`.

## Corrected integration defects

Registration patches now compile with production owners rather than copied
track fixtures. Preparation retains the outgoing scene/owner on failure or
cancel; delayed publication is rejected. Restart confirmation from Pause
explicitly resumes/rebases the authorized command. Imported pause is preserved
and resumed through Continue. Required props throw on missing assets instead
of becoming stand-ins. Rotated cushions fit the actual logical cells. Final
gathering preparation precedes its persistent record; the closing card no longer
covers the cats. Supported contact height replaces the floating hammock guide.
Generated accepted inputs are dependencies before compilation, and browser
reports bind to the build actually loaded rather than a changing output folder.

## Exact final evidence

Observed production Release, gallery/previews OFF, Chrome 154.0.8037.99:

- Build ID: `paws-0.1.0-074a5b58a29b2231`.
- Wasm SHA-256: `03963903380fb652765f624309489c2069cddbeb2eb4940da90f2147a1512ac9`.
- Content SHA-256: `ac59c2d8cb4ed9015303109432e1dde06b070aebcdbaa7a9714cb7a74a3f4e24`.
- Notices SHA-256: `24e12796be95110624b46f338bb1ebb7b71bfe1608848d67af160d729dcaae32`.

`build/evidence/release1/prompt-09/identity.json` binds final reports, current
source hashes, all payload hashes and native log identity. Final evidence:

- `native-final.txt`: 21/21 portable suites, including actual final-cat use,
  full-loop cloth contact, candidate retention and all ten scene projections.
- `assets.json`: 56 distinct GLBs and 94 hashed runtime deliveries, including
  16 technically playable audio files with explicit pending final reviews.
- `browser-final/report.json`: all ten solutions through retained controls,
  alternate chapter orders, hints/undo/practice, 200/190 economy, eight actual
  purchases/placements/uses, supplied-prop four-cat finale/replay; 1440×900,
  390×844 and 844×390 larger-text/reduced-motion layouts; file recovery,
  second-tab ownership, write failure/retry, reload, composed suspension,
  explicit Pause and actual WebGPU device-destroy/recovery. Zero page errors.
- `faults/report.json`: expected injected HTTP 503, successful retry/cancel,
  actual board mouse picking, emulated touch action, restart cancellation/
  confirmation, interrupted journal restore/undo, purchase cancellation and
  five retained version-6 history records. Injection never touches user storage.
- `uses-final/report.json`: corrected real-time hammock contact, real-time
  four-cat arrivals, closing retention and narrow/short closing layouts on the
  final payload. `uses/report.json` separately records all six animated
  walk/entry/use sequences before the cloth correction on its recorded earlier
  payload; final static uses are also exercised in the complete final journey.

All browser runners own/close isolated contexts and servers. The full journey
serves the actual production site under `/afternoon/` to test relative paths.
Earlier `browser`, `browser-recovery`, prior failed runs and producer previews
are historical; they do not replace the final reports. Automated authored
solutions do not establish independent discovery or a 45–75-minute pace.

## Reproduce and current launch

From the repository root, with configured compiler runtime on PATH, installed
Emscripten 6.0.11 and optional existing Jolt 5.3.0 cache:

```powershell
./scripts/release1/integration/build.ps1 -Native -JoltSourceDir $joltSourceDir
ctest --test-dir build/native-release1-p09 --output-on-failure
./scripts/release1/integration/build.ps1 -EmsdkPath $emsdkPath -JoltSourceDir $joltSourceDir
npm run test:browser:release1
npm run test:browser:release1:faults
$env:EVIDENCE_DIR='build/evidence/release1/prompt-09/uses-final'
$env:PAWS_TEST_USE='Hammock'
npm run test:browser:release1:uses
python scripts/release1/integration/record.py
```

The reproducible current-site launch is:

```powershell
./scripts/serve.ps1 -SiteDirectory build/web-release1-p09-pinned-release/site -Port 19630
```

Open `http://127.0.0.1:19630/`. This is the production game, independent of older
track preview ports. SDK/cache parameters never select a different engine source.

## Remaining release gates

All 66 audio slots and committed cue families are integrated. Concurrently
supplied two music, four ambience and ten Pip deliveries are hash/technically
verified for authorized local playback; their pending rights/listening fields
are preserved. Fifty clips still have no playable delivery. All 66 remain
pending final distribution/source/loop/journey listening acceptance; their
importer/provider brief and source-review records remain authoritative. No
silent/test-tone substitutions or new provider jobs were made by Prompt 9.

The candidate therefore remains incomplete for release. Prompt 10 must close
the remaining final audio, independent-player discovery/pacing, sustained
performance/soak, supported physical-browser/device and package acceptance.
Desktop WebGPU, keyboard and emulated touch/layout checks establish their
recorded scope only; they do not establish Safari or physical mobile support.

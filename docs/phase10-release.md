# Prompt 10 release review

Product: Paws browser 3D puzzle game. Role: a player in a fresh isolated browser
context, without accounts or external services. Environment: local Windows PC,
pinned WebEngine `3fe7d1dc1dc610c703944803cbb717cd65024ec9`, Emscripten 6.0.11
Release, gallery OFF. Automation uses Chrome and retained semantic/canvas input.
No user browser saves are changed. No site was published.

## Delivered corrections

- Settings now offers Automatic, Mobile and Desktop quality. Automatic selects
  Mobile for a display narrower than 700 CSS px, shorter than 500 CSS px, or
  primary coarse input; this is conservative display policy, not a GPU classification.
  Mobile caps at 921,600 pixels/DPR 1 with zero shadow maps. Desktop caps at
  2,073,600 pixels/DPR 1.5 and at most one 1024 shadow map. Both preserve aspect
  and all puzzle/selection identities. Existing lights must opt into shadow casting.
- Quality persists in snapshot version 5; versions 1–4 restore with Automatic.
  Invalid display values fail before import publication.
- Narrow and short HUDs retain coins, friendship and truthful save status.
  Failed saves use error text and remain recoverable through Menu.
- Performance collection now receives measured pre-render Paws application,
  GUI and audio wall time instead of a hardcoded zero.
- Unsupported browsers retain the compatibility/Retry shell without starting
  the Wasm runtime. Supported browsers start after the deferred adapters load.
- Build IDs fingerprint Wasm, runtime JS/HTML/CSS/adapters, content and notices. Package
  validation detects later changes, verifies the recorded pin, model hashes and
  required notices, and copies only the production runtime tree. Preview pages,
  preview Wasm, galleries and source masters are excluded. Archive file hashes,
  sizes, missing audio and remaining gates are recorded in `release-manifest.json`.
- Exact Emscripten and Dawn notices accompany the existing library and asset
  notices; changes to their staged text are rejected by the package validator.

## Review flows and issue ledger

| ID | Severity | Flow / observed baseline | Acceptance / regression | Status |
| --- | --- | --- | --- | --- |
| UAT-10-01 | P2 | Settings claimed quality would arrive later and exposed a disabled control | Switch real profiles, restore the choice, preserve attempt/revision/wallet; portable GUI/save and packaged browser checks | Verified in portable tests and corrected package review |
| UAT-10-02 | P2 | Portrait/short HUD hid coins, friendship and save state | Read status in the actual narrow HUD with larger text; portable semantic check and runtime captures | Verified in portable tests and runtime captures |
| UAT-10-03 | P2 | CPU update measurement was always zero | Publish measured Paws wall time and retain scoped renderer counters | Verified in retained renderer samples |
| UAT-10-04 | P2 | Host/journey labels referred to old phases; staging included developer previews | Payload-derived build/content IDs, notices and verified versioned package under a nested URL | Implemented; package tamper/integrity checks pass |
| UAT-10-05 | P1 gate | All 47 final provider audio clips/rights/listening reviews are missing | Approved ElevenLabs/Suno originals, import and actual-backend mix/loop review | Open external-asset gate; no substituted clips |
| UAT-10-06 | P1 gate | Final cushion pose is supported low rest, not a natural sleeping curl | Correct canonical-rig deformation/contact and inspect source/export/runtime at all furnishing spots | Open art gate from phase 7 |
| UAT-10-07 | P1 gate | No independent first-player or physical-mobile sessions | First-player timing and real supported-device sessions of at least 15 minutes | Open environment/player gate |
| UAT-10-08 | P2 | Missing WebGPU showed Retry but still entered runtime initialization | Retain readable compatibility shell, no runtime start or console exception | Fixed startup gate; corrected package failure review |

The fresh scripted journey exercises meet/accept, blocked roll, three free hints,
undo, ordered quests, 60 coins/3 friendship, both item details, purchase/cancel/
place/move, Pip use, ending, continued play and reload. Guided known-solution
automation cannot validate discovery difficulty or a 15–20-minute first playthrough.
No timers, extra quests, forced waits or artificial puzzle padding were added.

Recovery checks exercise actual file export/import, invalid/future save rejection,
latest-receipt durability, failed writes/retry, five bounded backups, second-tab
isolation, unavailable storage/locking, composed suspension, rapid restart,
cancelled preparation and real WebGPU device destruction/recreation.

Final visual captures cover all three boards in 1440×900, 390×844 and 844×390
with larger text, reduced motion, mute, grayscale and Mobile/shadow-free quality.
Separate isolated contexts exercise missing models, missing fonts and missing
WebGPU. Frame/renderer/audio samples distinguish tracked allocations from the
browser's total process/driver memory. GPU timestamp availability is reported,
never inferred. Zero production audio decode is not an audio-budget acceptance.

## Build, package and rerun

```powershell
./scripts/build.ps1 -Pinned -Configuration Release -BuildDirectory build/web-phase10-pinned-release -EmsdkPath <your-emsdk>
python scripts/package-release.py
# Use the exact printed package directory:
$env:SITE_DIR = 'build/releases/<printed-build-id>/site'
npm run test:browser:phase10
python -m unittest discover -s tests -p test_release_package.py
./scripts/serve.ps1 -SiteDirectory $env:SITE_DIR -Port 8100
```

`EVIDENCE_DIR` defaults to `build/evidence/phase10/`. The combined runner uses
task-local ports 19520/19522/19521. `BROWSER_EXECUTABLE` selects installed Chrome.
`PAWS_SOAK_SECONDS` can shorten a targeted rerun; a shortened run does not replace
the full-duration evidence. Each browser report names the exact site and hashes.

The ZIP contains `site/`, `release-manifest.json` and `HOSTING.md`. Host the whole
site over HTTPS with Wasm MIME, gzip/Brotli and atomic cache invalidation. Runtime
URLs work under one directory prefix. The ordinary entry is `index.html`.
Localhost is the development secure-context exception. Origin changes create a
different save scope; use file export/import to move afternoons.

## Final evidence

Final package and measured results are recorded in `docs/implementation-status.md`.
The original 900-second/42-cycle samples are retained in
`build/evidence/phase10-final/`; the corrected startup shell's complete scripted
journey, recovery and responsive/failure rerun is in
`build/evidence/phase10-startup-final/`; the sealed archive with toolchain notices
is verified in `build/evidence/phase10-sealed/`. Their soak durations are zero. The Wasm,
content and engine hashes match the full-duration run, but that is not a
900-second rerun of the final shell. `scripts/phase10-evidence.py` validates
the matching payloads and summarizes both scopes without merging their pass claims.

The original run saved 48,408 RAF intervals and 955 renderer samples before its
terminal report failed on JavaScript BigInt serialization. Its terminal pass
assertion is unavailable. The reporting rerun then exposed UAT-10-08; both the
serializer and startup gate were corrected. Retained samples establish duration,
budgets and allocation behavior; the corrected package reports establish the
functional/failure checks. No claim relies on the failed original terminal report.

The package is a review candidate while the audio, art, first-player and
physical-device gates remain open. Repeat the default 900-second combined run
when validating a subsequent final-audio build or a physical-device release.

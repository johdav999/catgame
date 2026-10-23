# Paws

The [M7 release and SDK handoff](docs/quest-module-m7.md) integrates the shared engine quest
kernel for all 29 quests on browser and Windows. Version-14 saves retain existing
puzzle journals and preserve pre-migration originals; versions 1–13 remain readable.

The [Loopia games platform](docs/loopia-platform.md) adds shared PHP/MariaDB
accounts, manual cloud saves and game-scoped Stripe purchase tracking for Paws
and future games. Hosting and live payment configuration are separate deployment
steps; no products or prices are enabled by default.

The eleven-quest beach adventure is available as a development preview with
`?devBeach=1`: Play/Continue → Adventures → Start / Continue beach. It includes
eleven quests, story continuation, a saved four-cat picnic ending and
independent progress with garden return; purchases are not enabled. See [the beach milestone](docs/beach-adventure.md) for its
separate test build, access boundary and verification.

A browser 3D puzzle game built in C++20 and WebAssembly on WebEngine. This repository
contains three playable requests: Pip's missing ball, Tidy the toy basket, and
The stuck little gate. Portable C++ rules own legality, ordered unlocks, undo,
restart and once-only rewards. The game uses the Pip and garden production kit,
retained engine GUI, keyboard-equivalent commands and local IndexedDB restoration.
See the [puzzle/save contract](docs/phase6-puzzles.md) and
[GUI contract](docs/gui-integration.md). The authored home garden, first quest,
reward card and reload/practice loop are integrated; see the
[garden/save contract](docs/phase5-garden-loop.md). The shop, furnishing previews,
free moves, coherent purchase saves and once-only closing moment are integrated;
see the [phase-7 contract](docs/phase7-shop-garden.md) for cat-use acceptance limits.
See [implementation status](docs/implementation-status.md) for phase-scoped
acceptance and concurrent later-phase work.

Prompt 10 adds saved display-quality controls, narrow-screen wallet/save status,
payload build IDs, final review tooling and a versioned static-site **review
candidate**. Final audio, curled resting animation, independent pacing playtests
and physical-device acceptance remain open. See [release evidence and hosting](docs/phase10-release.md).

```text
src/                 Paws scene and browser runtime
assets/              Game content and provenance
web/                 Paws browser shell
external/WebEngine/  Pinned engine Git submodule
cmake/               Consumer integration and license staging
scripts/             Windows setup, browser build and local server
tests/               Portable integration checks
docs/                Architecture and validation handoff
```

## First build on Windows

The supplied Golden Afternoon Garden tracks play as an ordered, repeating
background playlist after Play/Continue activates sound. Music volume and mute
are in Settings. See [the music playlist](docs/garden-music.md).

Install Git, Python, CMake 3.22+, and Ninja on PATH. Then:

```powershell
git submodule update --init --recursive
./scripts/setup.ps1
./scripts/build.ps1 -Pinned
./scripts/serve.ps1
```

Open the URL printed by the server. It starts at port 8080 and tries up to 100
consecutive ports if a port is occupied or unavailable. Use `-Port` to choose the
starting port, or `-PortAttempts 1` to require that exact port.
Without arguments, `serve.ps1` selects the newest built
game under `build/*/site` by its `paws.wasm` timestamp and prints the selected folder.
This includes pinned and local builds; use `-SiteDirectory` to choose explicitly.
The local server uses versioned asset URLs and disables caching to prevent an old
JavaScript/Wasm build from surviving a refresh. Existing listeners stay running;
stop or restart the existing server to replace the build on its current port.
After building into a different directory, run `./scripts/serve.ps1 -Restart` to
replace this repository's existing server on port 8080 with the selected build.
An already running server keeps its original site directory until restarted;
refreshing the browser alone cannot switch it to another build. `-Restart` refuses
to stop a listener belonging to another application. Use `-SiteDirectory` to
select a particular built site when restarting.
The default build is Debug; use `-Configuration Release` for release output in
`build/web-pinned-release/site/`. Deploy the entire generated `site/` directory together,
including `paws.js`, `paws.wasm`, assets and licenses. Serve over HTTPS in production.
The pinned renderer supports WebGPU and a reduced-feature WebGL 2 fallback;
localhost is suitable for development. Paws has not yet been accepted on physical
mobile devices.

For a new clone, use `git clone --recurse-submodules <game-repository-url>`.
Source is published at https://github.com/johdav999/catgame.

## Develop alongside WebEngine

You can reuse the SDK already installed in the engine checkout without installing
a second one:

```powershell
./scripts/build.ps1 -Pinned -EmsdkPath C:/Users/Johan/source/repos/WebEngine/.tools/emsdk
./scripts/build.ps1 -WebEngineSourceDir C:/Users/Johan/source/repos/WebEngine -EmsdkPath C:/Users/Johan/source/repos/WebEngine/.tools/emsdk
```

The pinned WebEngine revision is `461ab6dd1769171955254473c2b3623ad3dc615a`
(23 October 2026), published on `codex/quest-module-sdk-release`. It adds the
portable module/quest SDK and shared host integration to the existing sky, terrain
and water APIs. See [M4 delivery and Windows builds](docs/quest-module-m4.md). Build with `-Pinned -Configuration Release` and the installed SDK path
above, then serve `build/web-pinned-release/site` explicitly. The sky
keeps a fixed daytime atmosphere and soft static clouds without changing garden
lighting or introducing a day/night cycle. Browser verification:
`node scripts/daytime-sky-smoke.cjs` with `PAWS_URL` set to that site's server.
To see the sun, drag upward on the garden background beyond the horizon, then
drag sideways to look around. The camera tilts upward without moving underground.
Drag down again or use Reset view to return to the garden. Arrow keys with the
canvas focused also control the view. `node scripts/garden-sun-smoke.cjs` verifies
the visible sun and zoom; set `WEBGL=1` to exercise the fallback renderer.

The pinned engine also supplies [hilly garden terrain](docs/garden-terrain.md),
with ImageGen meadow/earth textures and a flat house/garden apron. The same
pinned Release build enables it.
Verify with `node scripts/garden-terrain-smoke.cjs` against the selected site's
`PAWS_URL`. Texture provenance is bundled beside the runtime PNGs.

For persistent local settings, copy `developer-settings.json.example` to the ignored
`developer-settings.json`. The build script reads it unless `-Pinned` is used.
`-Pinned` always selects the submodule and ignores that file; an explicitly supplied
SDK path is still supported. Local output uses `build/web-local-<configuration>/`.
For direct CMake usage:

```powershell
cmake -S . -B build/native-local -DWEBENGINE_SOURCE_DIR=C:/Users/Johan/source/repos/WebEngine
```

`CMakeUserPresets.json.example` supplies a native preset example; copy it to the
ignored `CMakeUserPresets.json` and adjust paths locally. Shared presets and CI use
the pinned submodule. The [CMake add_subdirectory contract](https://cmake.org/cmake/help/latest/command/add_subdirectory.html)
allows a source checkout and separate binary directory; the integration bridge is
explained in [architecture.md](docs/architecture.md).

## Portable tests

Install a native C++20 toolchain (Visual Studio C++ or GCC/Clang):

```powershell
cmake --preset native
cmake --build --preset native --config Debug
ctest --preset native -C Debug
```

This verifies puzzle rules/content, scene construction, application lifecycle,
camera framing and lazy optional physics. It is not browser or native-rendering acceptance. The first
configure downloads WebEngine's hash-pinned Jolt dependency.

Optional browser acceptance uses Node.js and Playwright. Build and serve the
pinned site, run `npm install` if needed, then `npm run test:browser`. The current
`scripts/phase6-smoke.cjs` completes all three quests using keyboard and mouse,
checks two touch-oriented layouts, reloads, practice rewards and failure/retry
paths. Set `BROWSER_EXECUTABLE` for Chrome/Edge, `PAWS_URL` for a different server
(default `http://127.0.0.1:8096`), and `EVIDENCE_DIR` for another evidence folder
(default `build/evidence/phase6`). The phase-1/2 scripts preserve historical
acceptance flows; the phase-6 runner is the current gameplay entry point.

`npm run test:browser:phase7` serves `build/web-phase7-pinned-release/site` itself
and exercises earn/buy/place/use, invalid occupancy, cancellation, free movement,
reload and ending replay. Set `SITE_DIR`, `EVIDENCE_DIR`, `PAWS_TEST_PORT` or
`BROWSER_EXECUTABLE` to override its build, evidence, port or browser.

The component gallery remains available in a development build configured with
`-DPAWS_GUI_GALLERY=ON`, opened with `?gallery=1`. Keep it OFF for production.
See [implementation status](docs/implementation-status.md) for scoped evidence.

## Engine update workflow

Package a pinned Release build with `npm run package:release`; the packager
defaults to `build/web-phase10-pinned-release/site` and writes a hash-named
directory and ZIP under `build/releases/`. It verifies runtime/model hashes and
notices and excludes inspection pages. Serve its `site/` directory locally.
Run `npm run test:browser:phase10` with `SITE_DIR` set to that packaged `site/`.
The sequential suite runs a fresh guided journey, recovery cases, responsive
visual captures and a 15-minute desktop transition soak. This is automated
desktop evidence, not a first-player or physical-mobile acceptance claim.

Prompt 9 adds the audio runtime and import workflow. **Final audio files are not
installed yet**; the game remains playable quietly. See [audio integration and
acceptance](docs/phase9-audio.md) and the [47-clip production checklist](docs/phase9-audio-production.md).
`npm run test:browser:phase9` checks the generated
`build/web-phase9-pinned-release/site` with isolated test-only audio signals.
These signals are never shipped as game assets or treated as an audible production
review. Music, Ambience, Effects, mute and Enable sound are in Settings.

The [Pip sound delivery](docs/pip-audio.md) adds ten playable ElevenLabs effects:
request meows, happy trills, purring, padded paw steps, cushion settling and
drinking. WAVs are in `assets/runtime/audio/`; original downloads and provenance
are in `artifacts/audio-production/sources/pip-sounds/`. Human listening and
distribution-rights acceptance remain tracked separately from local playback.

The [remaining sound effects](docs/remaining-sound-effects.md) add the other 49
requested cat, UI, puzzle, bell, water and furnishing effects. They play through
actual actions and contacts; menu feedback stays audible while the world pauses.
Run `npm run test:browser:remaining-effects` for the real-file browser journey.

Implement reusable features in WebEngine and game rules here. Test engine changes
and the game using the local override; commit and push the engine changes before
moving the game dependency. Then:

```powershell
git -C external/WebEngine fetch origin
git -C external/WebEngine checkout <tested-published-engine-commit>
./scripts/build.ps1 -Pinned -Configuration Release
# Run portable tests and browser acceptance against the pinned build.
git add external/WebEngine
git commit -m "Update tested WebEngine dependency"
```

Do not use `git submodule update --remote` in reproducible builds. The
[Git submodule guide](https://git-scm.com/book/en/v2/Git-Tools-Submodules) explains
recorded revisions and recursive cloning. Read [AGENTS.md](AGENTS.md) for project
boundaries and [validation.md](docs/validation.md) for current evidence.

The proposed gameplay direction is documented separately in
[paws-concept.md](paws-concept.md) and [architecture.md](architecture.md).

## Pip character preview (phase 3)

The pinned browser build also produces `pip-preview.html`, an isolated inspection
page for Pip's production mesh, nine bone-animated actions, reduced-motion poses,
and three portraits. Open that page on the same generated-site server. The
portable `paws_pip` library can be composed into the main garden in phase 5.
See [Pip's rig/runtime contract](docs/pip-asset.md) and
[the repaired Tripo asset delivery](artifacts/model-jobs/pip/2026-10-10-tripo-repair/README.md).

## Garden kit preview (phase 4)

The pinned site includes `garden-preview.html`, with all 21 garden/puzzle/furnishing
models, explicit gate/switch assemblies, toy/socket fit and Pip furnishing poses.
See the [model contract and launch commands](docs/garden-kit.md) and
[editable asset delivery](artifacts/model-jobs/garden-kit/2026-10-10/README.md).

## Durable saves (phase 8)

The title and pause menus provide Save and recovery, file export and validated
import. The usual save has one writable browser tab, five recovery snapshots and
truthful transaction status. Other tabs play temporarily; unavailable locking or
damaged data uses an independent slot with an export reminder.
See [the save/lifecycle contract](docs/phase8-saves.md) and run
`npm run test:browser:phase8` against `build/web-phase8-pinned-release/site`.

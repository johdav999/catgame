# M4 — published engine and shared Paws hosts

The engine is published at
[`76d17928a5e51bbaac53874867e08a510e5b9c8f`](https://github.com/johdav999/WebEngine/commit/76d17928a5e51bbaac53874867e08a510e5b9c8f)
on `codex/quest-modules-paws-host`. Paws' submodule gitlink selects that revision.
The isolated publication includes M1 compiled modules, M2 quest kernel, M3 host
sample and M4 reusable consumer seams. Unrelated development engine changes,
including the direct D3D12 implementation, were excluded. Windows uses SDL/Dawn.

## Delivered ownership

`src/runtime/game_session.hpp` contains the existing game orchestration extracted
from BrowserGame. The `paws_session` target shares it between the two executables.
`Afternoon` remains the sole quest/progression owner. Puzzle rules, journals,
once-only rewards, historical content decoding and `PAWS_AFTERNOON 12` are
unchanged by M4. The frozen M0 replay corpus still verifies all 29 authored quests.
Paws does not yet use the quest kernel as its progression authority; that is M5.

`RuntimeHost` supplies assets, audio backends, viewport/render binding, GUI,
storage/file actions and synchronous diagnostics. BrowserHost owns Application
and Renderer and retains the existing JS, IndexedDB, recovery and cloud adapter
contracts. WindowsHost uses DesktopHost's single Application and stable GUI.
There is no second native Application or duplicate quest implementation.

Scene preparation and command validation remain in GameSession. Both hosts
stage scene bindings before transition, unload the prior scene, and retain game
progress outside scenes. UI rebuilds occur at idle using GuiSystem::clear; old
widget capture/focus is cancelled. Native pointer events honor GUI ownership and
route volume drags through the shared controls. DesktopHost owns focus/minimize
suspension; rendering recovery resets clocks, and water uses simulation time.

Windows assets are resolved beside the executable. DesktopAudioBackend uses the
same GameAudio policy and catalogue; packaging verifies source hashes and converts
the existing MP3 deliveries to PCM WAV with ffmpeg. Decode/publication is bounded
and generation checked. Decode is currently synchronous on the owner thread;
this is not a sustained performance or human audio approval claim.

Native saves use atomic DesktopFiles writes under a separate user profile. An
exclusive writer lock prevents a second process from overwriting that profile.
The ordinary recovery UI exports/imports the same save text as the browser.
Invalid imports preserve live progress. A malformed startup record is retained
as an exact recovery copy before enabling a fresh session/import; earlier backups
are never overwritten. Browser storage behavior and its namespace are unchanged.
Native saves are not automatically synchronized with browser IndexedDB/cloud.

## Build and launch

Use the pin for shared builds:

```powershell
./scripts/build.ps1 -Pinned -Configuration Release -EmsdkPath ../WebEngine/.tools/emsdk
./scripts/build-windows.ps1 -Pinned -Configuration Release -WindowsDependenciesDirectory (Resolve-Path ../WebEngine/build/dependencies/windows)
./build/windows-pinned-release/package/Release/paws_windows.exe
```

The SDK directory above explicitly reuses an installed adjacent engine SDK. New
machines can run the selected engine's `scripts/setup-windows.ps1` and omit that
override. Native builds require MSVC x64, CMake, Python and ffmpeg on PATH.
Local engine overrides stay in ignored developer settings or explicit command
arguments; native and browser pin/local build directories remain separate.

Distribute the complete native `package/Release` folder: executable, DLLs, assets,
fonts, licenses and `build-info.json`. It launches without a development cwd.
Use `--user-directory <path>` for an independent profile, `--assets <path>` for
an explicit asset root, and `--import <file>` for validated save import. Optional
`--dev-beach` enables the same transient development access as the localhost
browser preview; save files do not grant entitlements.

`cmake/WebEngine.cmake` now contains build options only. The renderer source-list
fallback was removed in favor of the published `WebEngine::Browser` target.

## Verification and evidence

Evidence is under `build/evidence/m4/` (generated, not source-controlled):

- Engine native/Wasm: 71 module, 242 kernel and 49 shared host checks on each;
  matching 344-byte `f840213075084f6a` snapshot. GUI reset/lifetime checks pass both.
- Real engine sample hosts: non-quest plugin on Windows and both browser renderers;
  quest sample save/reload, pause, travel and file interchange across four Windows
  processes and forced WebGPU/WebGL2, with wallet 20 and one scene.
- Paws native regression subset: 8/8 rules, progression, scene, GUI, audio, beach
  story and save tests, repeated against the published pin. Both M0 replay tests
  and all 101 frozen artifacts pass.
- `webgpu-outings` and `webgl2-outings`: all five extended beach quests through real
  scene/GUI input, undo, restart, partial reload, garden travel, sequential story
  continuation and picnic ending save/replay. No browser errors; base rewards
  remain unchanged.
- `pinned-seed` / `pinned-resume`: actual Windows GUI input, minimized/restored
  host lifecycle, partial save in one process, restored quest completion and garden
  travel in another, 20 coins retained and one loaded scene.
- Browser save-recovery suites pass on both renderers, including failed writes,
  retries, pending writes, reload and second-writer protection. Final pinned runs
  are `pinned-webgpu-recovery` / `pinned-webgl2-recovery`.
- `scripts/paws-host-smoke.cjs` uses actual browser file chooser/download and native
  import. Both directions retain completion, coins and friendship on both browser
  renderers. Set `PAWS_URL`, `PAWS_NATIVE`, `PAWS_NATIVE_SAVE` and `EVIDENCE_DIR` to
  select exact builds and isolated evidence/profile directories.
- `pinned-corrupt` preserves the original malformed save before a successful
  import; `pinned-second-writer` confirms read-only status and unchanged save bytes.
- `final-interchange` repeats file interchange and native minimize/restore checks
  after the final input fix. `identity.json` records the tested binaries and served
  browser hash, independently of candidate build outputs.

Fresh pinned output is in `build/web-pinned-m4-release/site` and
`build/windows-pinned-m4/package/Release`. Build metadata identifies the engine
commit and Wasm/executable hashes. Candidate evidence is kept separate from these
fresh pin builds. Native Windows and desktop Chrome acceptance do not establish
physical mobile compatibility or other native OS support.

M5 remains the next step: shadow comparisons and the first save-compatible quest
kernel pilots. No Paws quest catalogue or save schema migration happened in M4.
The engine commit is published; the Paws pin is staged and the Paws source changes
remain local alongside the preserved pre-existing work.

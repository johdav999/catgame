# M7 — pinned release and module SDK handoff

The engine SDK handoff is published at
`c819dbf07ee4f6a0810f8dbd017a441d856f326a` on
`codex/quest-module-sdk-release`. Paws pins that exact revision. The engine
changes add standalone consumer coverage and documentation; the portable kernel
and all 29 Paws quest definitions retain the M6 behavior and version-14 codec.
Versions 1–13 and the exact original-backup policy remain supported.

## Reproducible consumer inputs

Normal builds verify `assets/runtime/manifest.json`, immutable hashes and delivered
rights records with `scripts/verify-runtime-assets.py`. They no longer run offline
asset promotion or depend on ignored art/audio job folders. Accepted generated
bindings remain checked in. Offline authoring still uses its existing reviewed
promotion workflow; a consumer build cannot approve or promote new content.

The release branch contains the M4–M6 migration and the current catalogue's source,
assets and provenance prerequisites. Unrelated working-checkout engine/skill
development is not part of the release. Source and package builds use the exact
submodule; local SDK/dependency caches may be selected through command parameters
or ignored developer settings. No machine-specific paths are committed.

```powershell
git submodule update --init --recursive
./scripts/setup.ps1
./external/WebEngine/scripts/setup-windows.ps1
# Install ffmpeg on PATH for the native reviewed-audio conversion.
./scripts/build.ps1 -Pinned -Configuration Release
./scripts/build-windows.ps1 -Pinned
cmake --build build/windows-pinned-release --config Release --parallel 4
ctest --test-dir build/windows-pinned-release -C Release --output-on-failure
python scripts/package-quest-release.py --site build/web-pinned-release/site --desktop build/windows-pinned-release/package/Release --output build/releases/m7
```

`paws_windows_package` always stages assets, native audio, DLLs, notices and
metadata, including when the executable is already up to date. Browser and
Windows build metadata record game commit, engine pin and dirty-source status.
The packager requires clean committed sources, Release configuration, matching
game/engine identities, equal accepted content manifests and intact payload
hashes. It rejects stale binaries, missing DLLs, private saves/profile files and
unexpected desktop output. It verifies every archived entry, writes a complete
file inventory and `SHA256SUMS`, and never overwrites an existing release folder.
Archive ordering and timestamps are fixed; binary reproducibility still requires
the same compiler, dependencies and conversion tools.

## Release verification matrix

`scripts/verify-quest-release.py` is the full local acceptance entry point. It
requires actual generated site and Windows package directories, native and Wasm
catalogue executables, and a fresh evidence directory. It starts its own bounded
local server, verifies served Wasm identity and runs the browser checks serially
to avoid sharing GPU resources between long catalogue runs.

```powershell
python scripts/verify-quest-release.py --site build/web-pinned-release/site --desktop build/windows-pinned-release/package/Release --native-catalogue build/windows-pinned-release/Release/paws_quest_catalogue_tests.exe --wasm-catalogue build/quest-wasm/paws_quest_catalogue.js --evidence build/evidence/m7-final
```

Build the Wasm catalogue and its five portable regressions using `emcmake` on
`tools/quest-baseline` in a separate tree. Native/Wasm baseline manifests freeze
both old and current quest layouts. The full native suite also covers portable
scene/lifecycle contracts.

| Gate | Required evidence |
| --- | --- |
| SDK external consumers | Generic non-quest module, custom quest binding, independent sessions, retained provider, save/reload, module and quest omission on native/Wasm |
| Quest behavior | Every catalogue solution, negative/stale input, undo, practice, pause, travel, receipts, purchases, placements and independent endings |
| Historical saves | Both frozen corpora and supported versions 1–13; no effects on load; all supplied kernel records cross-checked |
| Rendered browser | Fresh full WebGPU and WebGL2 catalogue runs, mid-attempt reload and one scene after travel |
| Windows distribution | Packaged executable from unrelated cwd, executable-relative assets, real SDL GUI events, minimize/restore, independent profile, full catalogue |
| Native storage ownership | Two actual processes share one profile; the secondary cannot replace the primary save |
| Migration/recovery | Exact v12/v13 backups across launches; browser backup failure/retry/original export, closed database, quota, pending writes and second tab |
| Save interchange | File chooser/download between browser and Windows, all furnishings/placements, wallet, friendship and ending flags |
| Ending presentation | Original garden picnic and later beach ending replay/reload on both browser renderers with existing arrival/lane rules |
| Payload identity | Clean game commit, published engine pin, served Wasm, desktop executable, complete file/archive hashes and native/Wasm trace equality |

The runner writes per-gate logs and `report.json`; packaging writes
`release-manifest.json` and checksums. Reports identify the actual payload and
separate portable, rendered browser and Windows evidence. CI adds the independent
SDK matrix in WebEngine and a pinned Windows/web package workflow in Paws.
Hosted CI status is distinct from local runtime acceptance; inspect each workflow
result rather than assuming a newly published workflow passed.

## SDK usage and extension limits

Read the pinned engine's [module SDK handoff](../external/WebEngine/docs/module-sdk.md),
[module contract](../external/WebEngine/docs/modules.md),
[quest API](../external/WebEngine/docs/quests.md) and
[shared host boundary](../external/WebEngine/docs/consumer-hosts.md).
`examples/modules` is the independent non-quest plugin template.
`examples/quests` builds a separate typed custom-binding module, stages game
rewards before kernel commit and proves independent sessions and receipt-safe
restore. It configures without renderer/physics downloads. CTest runs enabled and
omitted consumers on both native and Node/Wasm.

Plugins are statically selected C++20 code modules. Authored quest packs are
versioned content. Neither implies DLL discovery, executable hot-unloading,
cross-toolchain ABI compatibility or automatic ownership of scenes/storage.
Progress, wallet and save policy remain in the application session; browser and
Windows hosts adapt platform services around the same portable owners.

## Migration, rollback and acceptance scope

See [M6 migration](quest-module-m6.md). Back up/export before changing binaries.
M7 keeps `pre-quest-m6` / `saves/pre-quest-m6.save`, existing M5 backups and the
matching old reader. Old binaries cannot read version 14; rollback restores the
preserved original with the matching old build and excludes newer progress.
Never overwrite the only recoverable save or copy a live writable profile into a
release package.

This is a technical module-migration release for Windows x64 SDL/Dawn and desktop
Chrome WebGPU/WebGL2. Physical mobile, other native hosts, live purchases/cloud
services, final provider listening/rights review and independent creative/pacing
approval are not established by these gates. The asset manifest retains its
existing acceptance limitations; M7 does not turn them into retail-game approval.
Quest-editor authoring is a separate optional follow-up, not unfinished runtime
SDK work.

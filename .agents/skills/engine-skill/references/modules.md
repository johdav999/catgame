# Compiled code modules (M1)

Optional `engine_modules` / `WebEngine::Modules` supplies portable C++20
`ModuleDescriptor`, `validateModules`, typed `ModuleComposition<Registry>` and
shared immutable `ModuleSnapshot<Registry>`. No renderer, GUI, AI, physics, quest
or platform dependency. Root builds opt out with `WEBENGINE_BUILD_MODULES=OFF`;
standalone consumers can add only `WebEngine/modules` without engine dependency
discovery. `webengine_add_module` creates a normal static target/public alias,
include/dependency propagation and Wasm exception settings.

IDs are lowercase namespaced ASCII (letters/digits/underscore/hyphen and dots),
1–128 bytes, with no leading/trailing/repeated dots. Limits: 128 modules, 64
dependencies/provided schemas/required capabilities per module, 64 host
capabilities. Version ranges are inclusive minimum/exclusive maximum. Engine API
means module-composition SDK API 1.0.0, not an engine-wide binary ABI guarantee.
Missing/duplicate/incompatible IDs, schema providers, versions, capabilities and
dependency cycles reject before installers run. Topological ties use lexical IDs.

The application supplies an explicit registration list and a typed builder.
`replace` copies provider callbacks, stages the registry, invokes installers in
dependency order, runs optional final validation and publishes only on success.
It is synchronous, owner-thread only and non-reentrant. Installers must not have
external side effects. Sessions retain the snapshot while using provider-backed
objects. Registry destruction precedes reverse dependency release of provider
captures; replacement does not invalidate retained old snapshots. Capture owners
by value; no borrowed installation locals. Each factory creates independent
session state. No static initializer registration, service locator, event bus,
automatic frame loop, dynamic DLL/Wasm loader or live executable unload exists.

Use `examples/modules` as the external plugin template. Its counter module proves
non-quest reuse and independent sessions. `WEBENGINE_BUILD_MODULE_TESTS=ON` adds
module_tests (native and Node/Wasm); `WEBENGINE_BUILD_MODULE_DEMO=ON` adds the
selected host's rendered sample. Browser module_demo uses Application/renderer;
Windows module_demo_windows uses DesktopHost. No second clock/scene owner.
Run standalone consumer and disabled-module builds on native/Wasm, native module
CTest, Node module_tests.js, scripts/modules-smoke.cjs on forced WebGPU/WebGL2 and
Windows --smoke on Dawn/direct D3D12; tools/verify_module_pixels.py checks captures.
See selected engine docs/modules.md and docs/modules-validation.md for commands,
71-check baseline, actual runtime evidence and limits. The M2 [quest kernel](quests.md) now composes through this foundation.
Verify engine selection: CatGame's older pin does not expose this module; local
engine implementation and sample evidence do not advance its gitlink.


M7 SDK handoff: the published c819dbf07ee4f6a0810f8dbd017a441d856f326a revision
provides docs/module-sdk.md and independent consumers in examples/modules and
examples/quests. The latter builds Example::QuestBindings with pure typed
inventory-fact evaluation and stages the game wallet before kernel commit.
CTest runs generic/custom consumers, 71 module checks, 242 kernel checks and
module/quest omission on Windows native and pinned Emscripten/Node. No dynamic
loading or binary ABI is promised. Paws M7 uses package-quest-release.py to reject
dirty/stale/mismatched web/Windows payloads, and verify-quest-release.py for
catalogues, saves, two-process ownership, file interchange and ending replay.
The native package target is paws_windows_package. Normal consumer builds verify
delivered runtime assets without accessing ignored authoring job folders.

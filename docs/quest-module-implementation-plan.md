# WebEngine quest module and Paws migration plan

Date: 22 October 2026. Status: M0-M7 implemented. Paws pins the published engine, shares its session between browser and Windows, and uses the quest kernel for all 29 quests. Version 14 validates full-catalogue state while retaining save readers for versions 1–13. M7 supplies pinned release packaging, regression automation and the module SDK handoff. See [M0/M1](quest-module-m0-m1.md), [M2](quest-module-m2.md), [M3](quest-module-m3.md), [M4 delivery](quest-module-m4.md), [M5 pilots](quest-module-m5.md) and [M6 cutover](quest-module-m6.md) and [M7 release](quest-module-m7.md).

## Outcome and scope

Deliver an optional portable C++20 `engine_quests` module, a small reusable contract for future compiled code modules, and migrate Paws without losing existing progress or changing puzzle behavior. Deliver the engine feature on browser and native Windows together. Finish the migration with a playable Paws Windows application using the same game session, quest definitions, puzzle rules and save format as the browser application.

Desktop acceptance initially means Windows x64 with the existing WebEngine desktop host. Portable interfaces should permit later macOS/Linux adapters; those platforms are not included in the acceptance claim. Browser acceptance includes forced WebGPU and WebGL2. Cloud synchronization, payments, multiplayer, arbitrary downloaded executable code and hot unloading are outside this plan.

Use compiled modules plus explicitly registered extensions. Load new authored quests as content packs. A quest is data, not a DLL or a new C++ plugin class. Dynamic native/Wasm binary loading is a separate future project requiring its own ABI, packaging and lifetime design.

## Verified starting point

Source inspected in this working checkout on the date above:

- [Paws CMake](../CMakeLists.txt) separates `paws_rules`, `paws_content`, `paws_game`, `paws_ui` and `paws_audio`. The playable `paws` executable is currently in the Emscripten branch; native rule tests do not establish a desktop game.
- [Progression policy](../src/release1/progression/policy.hpp) declares 29 quests: 18 base quests and 11 beach quests. IDs, prerequisites, packs and chapter/requester policy already exist.
- [Afternoon](../src/afternoon.hpp) owns attempts, revisions, the completion/reward ledger and game progression. The first three puzzles use legacy `PawsSession`; later puzzles use [rules::Attempt](../src/release1/rules/puzzles.hpp).
- [Save codec](../src/release1/progression/codec.cpp) writes `PAWS_AFTERNOON 12`, accepts versions 1–12 and replays journals to validate progress and reward consistency. Saved content identities can select historical puzzle definitions. Preserve those paths.
- [BrowserGame](../src/browser_main.cpp) combines portable orchestration with Emscripten input, startup, timing, storage and diagnostics. This needs extraction for a real desktop application.
- The Git tree pins WebEngine `0426d9319de715584a2e34ff9862cd6ba873ecd0`. Ignored developer settings select the adjacent checkout for local development. Keep these build modes separate.
- Adjacent WebEngine source exposes `WebEngine::Desktop`, `DesktopHost`, `WebEngine::Platform`, `DesktopFiles`, native audio and shared retained GUI. Its current instructions require every engine feature to be implemented and verified on both browser and Windows. Its revision/dirty status could not be read through Git in this session; inspect both before implementation and choose a published revision deliberately.

Older proposal documents contain stale platform and quest inventories. Current source and scoped evidence take precedence. The working checkout contains unrelated changes: this plan does not authorize discarding them.

## 1. Reusable code-module architecture

### Module packaging

Proposed targets:

| Target | Responsibility | Dependencies |
| --- | --- | --- |
| `engine_modules` / `WebEngine::Modules` | Descriptors, compatibility checks and validated module composition | Standard C++ only; no quest or platform dependency |
| `engine_quests` / `WebEngine::Quests` | Quest definitions, evaluation, instances, snapshots and validation | Modules and portable utilities only; no renderer, GUI, AI, host or scene requirement |
| `engine_quest_editor` / `WebEngine::QuestEditor` | Optional quest document/history and validation frontend | Quests plus existing editor/GUI facilities |
| Game-owned modules | Puzzle adapters, inventory/dialogue bindings, reward policy and authored content | Only the engine modules they explicitly use |

Use a normal CMake target and public-header contract for each module. Keep implementation dependencies private. Add a small CMake helper for consistent target aliases/options/tests, without moving existing engine systems into a new framework. A game that disables quests must still build without quest/editor dependencies. No module may read another module's private headers or source list.

### Shared descriptor and composition contract

Define a `ModuleDescriptor` with a namespaced ID, module version, required module versions, supported engine API version range, provided extension-schema versions and host capability requirements. Keep module API version, content schema and saved-state schema separate.

The application supplies an explicit module list. Validate duplicate IDs, missing dependencies, incompatible versions and dependency cycles before installation. Use deterministic dependency ordering and return actionable diagnostics. Compile/link dependencies remain CMake's responsibility; the descriptor validates the selected application composition.

Do not use static initializer registration, filesystem plugin scanning, a universal string-keyed service locator or a generic engine-wide event bus. Gameplay libraries can be directly constructed. Modules that register contributions install them through explicit typed registries supplied by the application.

Separate installed definitions/factories from live session instances:

1. Validate descriptors and declared contributions in staging.
2. Construct the immutable registry/catalogue snapshot and publish only on success.
3. Construct game/session runtime objects with explicit typed dependencies.
4. On teardown, cancel owner work and destroy runtime instances before releasing registry captures and their providers.

Registration is owner-thread, non-reentrant and side-effect-free outside its staging builder. If preparation fails, destroy staged registrations in reverse dependency order and retain the old composition. Runtime modules do not automatically acquire a frame loop: the application explicitly calls their update hooks in its existing frame callbacks.

Registry definitions remain immutable while live instances use them. No live executable unload in v1. Async adapters carry generation tokens and publish at owner-thread boundaries; teardown invalidates pending work before destroying captured services. Reuse existing engine lifetime mechanisms where available.

### Reuse beyond quests

Publish an external-module example/template containing CMake, public headers, descriptor, registration, tests, documentation and a web/Windows example composition. Prove the contract with a tiny non-quest sample module (for example a typed counter service used only in the SDK examples), including two independent sessions. Do not build a production inventory/dialogue system merely to test extensibility.

Future inventory, dialogue or achievements modules use the same packaging/composition rules but own their typed APIs and state. Inter-module adapters live in the application or an explicit integration module. Quests need not depend on every future gameplay system.

## 2. Quest runtime contract

### Definitions and state

- Immutable `QuestDefinition`: stable quest/content IDs, prerequisites, stages, objectives, conditions, branches and terminal outcomes.
- Mutable `QuestInstance`: availability/activation/completion/failure status, active stage, objective counters, choices and revision. Separate durable completion from the current puzzle attempt.
- `QuestCatalog`: validated immutable definitions and bindings, shared by sessions.
- `QuestRuntime`: independent per-session state, ordered inputs, pure candidate evaluation and bounded diagnostic records.
- `QuestSnapshot`: explicit versioned portable fields and content identities; no callbacks, pointers, scene handles or transient presentation state.
- `QuestView`: read-only status and progress for game journal/HUD projection. Localized text and visual design remain content/game responsibilities.

Begin with sequential stages, All/Any objective groups, typed fact conditions, filtered occurrence counters, explicit choices, branching and success/failure/cancellation. Prerequisites must form a DAG. Stage graphs are acyclic in v1; retry creates a new activation rather than an unrestricted transition loop. Defer arbitrary scripts, concurrent hierarchical regions and a general visual programming language.

Custom objective/condition handlers register under namespaced, versioned keys. Validate typed configuration and required bindings before activating a pack. Handlers evaluate against immutable input/state and return values or candidate changes; they cannot mutate the wallet, scene or session through hidden callback side effects. Content cannot name arbitrary native functions.

### Events, facts and deterministic processing

Distinguish occurrences (`ItemDelivered`) from current facts (`gate_open=true`). Repeatedly projecting a true fact must not increment an occurrence counter. Paws puzzle results come from accepted rule commands, never animation completion or a rendered mesh position.

Use copied, bounded typed values and stable logical IDs. Inputs carry session generation, source command sequence and activation/attempt identity as appropriate. Deduplicate retries using persisted committed sequence state; reject stale generations/activations. Reject queue overflow visibly instead of silently losing progress. A controlled replay mode rebuilds state without delivering external effects.

Document one owner-thread evaluation boundary after authoritative game commands. Input produced during evaluation is deferred to the next batch; callbacks cannot reenter mutation. Define stable event and branch priority ordering. Only currently active stages consume occurrences; a newly entered stage does not reuse the same occurrence unless a future explicit contract allows it. Initial/current facts may be evaluated on entry without inventing occurrence history.

Expose explicit limits for content size, graph size, payload strings, pending inputs, diagnostic history and per-batch work. Exhaustion returns a structured result and preserves valid state. Choose numerical defaults in the contract milestone using the Paws catalogue and a synthetic large catalogue; freeze those defaults in tests and public documentation.

Quest time comes from supplied simulation time; pause/minimize/hidden suspension does not count by default. Save timer progress in an explicit portable representation. No wall-clock catch-up or render-driven quest progress. Match native/Wasm output for identical input/time traces; do not promise deterministic physics.

### Completion, effects and persistence

The application owns the transaction joining puzzle state, quest progress, inventory/wallet and receipts:

1. Validate the game command and prepare candidate puzzle/game state.
2. Evaluate the quest batch against that candidate; obtain proposed progress and typed effect requests.
3. Validate game-specific effects and deduplicate by stable receipt ID.
4. Publish one coherent session revision, with allocations/fallible work completed before publication.
5. Queue one complete save snapshot; release presentation notifications after logical commit.

An error in preparation leaves every live owner unchanged. A disk/IndexedDB failure leaves a coherent but unsaved session with retry/recovery UI; it must not label the revision durable. Loading never re-emits committed reward effects. External network side effects require their own idempotency/outbox contract and are not included.

For once-only rewards, receipt identity is based on stable quest/reward policy, not attempt number. Content revisions must not accidentally create a new award. Paws practice/undo/restart may change attempt completion without revoking earned progression or granting another reward. Reversible in-attempt stages are rebuilt from validated puzzle state/journal; durable completion remains monotonic under the existing policy.

The module encodes/decodes its own state section; the game owns the enclosing save and atomic storage request. Browser and Windows use identical versioned bytes. Unknown required bindings/content, corrupt input or unsupported schemas reject before replacing the active session. Restoring an unloaded scene does not require retaining scene objects.

## 3. Web and desktop integration

| Shared | Browser adapter | Windows adapter |
| --- | --- | --- |
| Game session and quest runtime | Existing Application/browser host | Existing `DesktopHost` and its Application |
| Content validation and decoding | HTTP/asset manager | `DesktopFiles`/existing asset executor |
| Save codec and coordinator policy | IndexedDB and browser tab ownership | Atomic file replacement and explicit profile writer ownership |
| Intents, journal, scenes and retained GUI | Browser input/text/accessibility | Desktop input/text/accessibility |
| Pause and cancellation policy | Visibility/blur callbacks | Focus/minimize/close callbacks |

Use one Application per host. DesktopHost already owns and advances it; extracting Paws must not create a second Application, clock, physics step or GUI owner. Shared orchestration borrows host services with explicit lifetime. Use native platform services through public targets, without SDL/Win32 types leaking into game headers.

DesktopFiles provides atomic replacement but not cross-process locking. Add a Paws profile single-writer lease or read-only second-instance mode before claiming parity with browser tab isolation. Reuse the game's save sequencing so old async completions cannot mark a newer revision saved. Specify close behavior while a save is pending/failed, preserve recovery/export, and test interruption during writes.

Preserve browser account/entitlement/cloud adapters as browser integration. Desktop migration targets local gameplay and save interchange; do not grant adventure ownership from imported saves or silently claim desktop purchase/cloud support. Existing development beach access remains explicitly a development option.

Use the established desktop renderer path for the baseline Paws port. If the shipped application exposes multiple desktop backends, run the same affected acceptance flow on each exposed backend or explicitly constrain the shipped selection. The quest module itself must not branch on renderer type.

## 4. Staged implementation and acceptance gates

Each engine milestone includes native Windows and Wasm tests plus affected real browser/desktop integration. Publish focused engine changes before advancing the game gitlink. Do not hide changes inside an uncommitted submodule.

| Milestone | Concrete deliverables | Exit gate |
| --- | --- | --- |
| M0 — Freeze contracts and baselines | Inspect current revisions/dirty state; record all quest IDs, content revisions, completion/reward/undo policy; capture existing traces and legacy saves; agree bounded v1 schemas | Baselines cover all 29 current quests and save versions 1–12; proposed API/dependency/lifetime contract reviewed |
| M1 — Reusable module foundation | `WebEngine::Modules`, descriptor validation, staged composition, CMake helper, external-module template and non-quest sample | Duplicate/missing/cyclic/incompatible modules reject without partial publication; disabled-module build works; sample runs in web and Windows with independent instances and safe teardown |
| M2 — Portable quest kernel | Definitions, catalogue validation, stages/objectives/facts/events, typed handlers, bounded evaluation, candidate transaction interface and snapshots | Shared native/Wasm trace suite matches; invalid content, duplicates, stale events, reentry, budget exhaustion and rejected effects leave valid state intact |
| M3 — Engine host vertical slice | Shared small quest-demo controller with browser and Windows entry points; branching, custom handler, completion receipt, save/reload, pause and scene travel | Actual forced WebGPU/WebGL2 and Windows desktop flows complete; browser-to-desktop and reverse save interchange pass; no scene-bound progress or duplicate effects |
| M4 — Publish and establish Paws host seam | Publish tested engine revision; validate then update Paws pin; extract shared orchestration from BrowserGame; add `paws_windows` using DesktopHost/platform services | Existing Paws browser flows unchanged; Windows loads garden, handles shared intents/GUI and saves/reloads; no duplicated Application ownership; baseline native rule suites pass |
| M5 — Save-compatible pilot migration | Read-only shadow comparison; migrate one legacy quest and one multi-stage beach quest; add new save section and conversion | Step-by-step progress/receipts match existing behavior; undo/restart/practice/travel/reload pass on all three runtime paths; migration rejects inconsistencies without replacing saves |
| M6 — Full Paws cutover | All catalogue entries use quest runtime for common orchestration; retain puzzle algorithms and game-specific policy; remove duplicate live quest authority | All 29 quests pass known solutions and negative paths; base/beach independence, endings, purchases, placements and reward presentation preserved; old saves import on web and Windows |
| M7 — Release hardening and module SDK handoff | Build/package automation, regression matrix, migration documentation, diagnostics, pinned consumer evidence, SDK example and required skill updates | Reproducible published engine + pinned Paws builds; clean-process desktop package and actual generated sites pass; save recovery/second writer tests pass; evidence records exact versions and limitations |

M1–M3 establish the reusable engine feature. M4–M7 complete the Paws migration and desktop delivery. A separate optional milestone after M7 adds quest graph authoring to the existing shared editor frontend, with native/web preview, document undo/redo, validation diagnostics and a runtime inspector. Runtime delivery does not depend on building a new editor first.

## 5. Detailed Paws migration

### Preserve ownership and public behavior

Keep `Afternoon` as the authoritative game-session owner during extraction. Add a quest runtime member/adaptor; do not create a second wallet, save coordinator or source of completion truth. Keep `Intent`, `PuzzleView` and `ProgressView` stable initially so existing scene/GUI code and acceptance tools continue to work.

| Current responsibility | Destination |
| --- | --- |
| Quest IDs, content bindings, prerequisite evaluation and common objective orchestration | Definitions/runtime in `WebEngine::Quests`, with Paws-authored catalogue |
| Layouts, move legality, routes, puzzle stages, undo and solution validation | Existing Paws `PawsSession` and `rules::Attempt` |
| Cat requesters, chapter policy, friendship, coins, shop, beach independence and endings | Paws progression policy composed with quest completion |
| Journals, content-history compatibility and legacy save interpretation | Paws codec and puzzle rules |
| Animation, hints, quest rewards, camera and HUD | Existing Paws presentation using read-only views and committed notifications |
| Persistence transport and OS lifecycle | Browser/Windows adapters around shared coordinator/session |

Wrap each puzzle attempt as a typed Paws binding. Initially a quest can observe authoritative puzzle completion as one objective; complex puzzles keep their internal stage algorithm. Expose existing stage facts when they improve quest tracking, without duplicating the puzzle's state machine in the engine. Prove engine branching/custom objectives in the independent demo rather than rewriting Paws puzzles to manufacture a need.

### Shadow comparison and pilots

Run the new runtime in side-effect-free shadow mode while legacy code remains authoritative. Feed only accepted commands and validated current facts; compare availability, durable completion, current attempt status and proposed receipt IDs after each command. Shadow mode never saves independent truth, grants rewards or drives UI.

Pilot Pip's original ball quest to exercise the legacy path, then a multi-stage beach quest such as fresh water to exercise modern attempts, scene travel and pack isolation. Freeze exact IDs/content versions in M0. At authoritative cutover, each quest has exactly one active orchestration path selected by a development migration map; do not let both paths grant effects. Remove the map when all quests migrate, while retaining legacy decoding/replay support.

### Save migration and recovery

Reserve the next Paws save schema at implementation time (version 13 if 12 is still current). Never assume concurrent work has not advanced it.

1. Decode old bytes with the existing version-specific validators into an isolated candidate; preserve the original bytes.
2. Replay the historical puzzle journals against their recorded content versions, preserving supported restart/content-upgrade behavior.
3. Map stable quest IDs, attempts, durable completion, current stage facts and reward receipts into the new quest state. Use an explicit migration table; never infer identity from current array order.
4. Cross-check unlocks, journals, wallet, friendship, purchases, placements, introductions, settings and both endings. Preserve newly appended/default quests according to the old format's policy. Access entitlements remain external.
5. Publish the entire candidate only when validation and required scene preparation succeed. Invalidate old command generations; emit no reward or ending presentation during import.
6. Save the new envelope plus quest state and unchanged puzzle journals as one record. Keep a recoverable original/backup and do not report success before storage acknowledges the matching revision.

For new-format saves, validate quest snapshots against the puzzle journals and game receipts; redundant state must not become an alternate authority. Retain historical content decoders or explicit migrations for all supported saves. Missing content/version mappings produce recovery diagnostics instead of silently resetting attempts.

Rollback must be deliberate: an older binary may reject the new schema. Retain the old save backup/build for rollback; do not promise arbitrary downgrade or overwrite the only recoverable copy. Freeze legacy replay as a compatibility reader after cutover, rather than deleting it with duplicate live progression code.

## 6. Proposed file boundaries

These are intended additions, not existing APIs:

```text
WebEngine/
  include/engine/modules.hpp
  include/engine/quests/{definition,runtime,bindings,snapshot}.hpp
  src/modules.cpp
  src/quests/{definition,runtime,bindings,snapshot}.cpp
  cmake/WebEngineModule.cmake
  examples/modules/                 # external module template + non-quest sample
  examples/quests/                  # shared demo + thin web/Windows hosts
  tests/modules_tests.cpp
  tests/quests/                     # common native/Wasm fixtures and traces
  docs/{modules,quests,quests-validation}.md

CatGame/
  src/quests/{catalog,bindings,migration}.hpp/.cpp
  src/runtime/{paws_runtime,save_coordinator}.hpp/.cpp
  src/browser_main.cpp              # reduced browser host
  src/windows_main.cpp              # DesktopHost composition
  src/platform/                     # game-specific browser/Windows service adapters
  tests/quests/                     # compatibility and all-catalogue traces
  tests/fixtures/quests/            # supported old/new saves, including failures
  scripts/                          # separate pinned web/Windows build and smoke entrypoints
```

Keep the first extraction small: existing Afternoon and puzzle files can remain in place. Use the existing engine consumer bridge only for build integration; publish missing engine consumer targets upstream instead of copying engine source lists. Resolve codec/parser reuse from current engine facilities in M0 before adding dependencies.

## 7. Verification and completion criteria

| Area | Required evidence |
| --- | --- |
| Reusable module contract | Independent external consumer; omitted module; dependency/version errors; staged install failure; capture lifetime; independent sessions; non-quest module reuse |
| Quest behavior | Sequences, All/Any, branches, facts versus occurrences, custom bindings, terminal states, bounds, deterministic ordering, no reused occurrence across stages |
| Transaction safety | Rejected game command/effect leaves state unchanged; duplicate/retried completion; stale input; no reward on load; save failure/retry; controlled replay |
| Persistence | Fixtures for every supported save version; partial/completed/practice attempts; historical content changes; invalid/future/truncated/missing-content saves; identical semantic round trips across web/Windows |
| All Paws content | Enumerate the catalogue in tests rather than hard-code 29 forever; run every solution plus representative illegal actions, undo, restart, travel, pause and reload; verify current endings and base/beach isolation |
| Runtime UI | Actual generated Paws site on forced WebGPU and WebGL2; actual Windows executable; mouse/keyboard, GUI capture, save feedback, recovery/export/import, close/minimize/return |
| Lifecycle | Quest progress across unloaded scenes; late callbacks after transitions; cancelled work; changed session generation; clean shutdown and pending save policy |
| Distribution | Pinned engine revision and SDK; exact build/content IDs; served Wasm identity; packaged desktop assets/licenses; launch without a development cwd; independent writable profile |

Keep headless native logic, Node/Wasm logic, rendered browser and rendered desktop evidence separate. Windows renderer parity and physical mobile acceptance are not established by a quest unit test. Performance evidence should measure bounded processing and observed work with an explicit catalogue/input size, not claim universal device performance.

For each engine implementation milestone update WebEngine architecture/contracts and the installed engine skill plus its repository-local Paws copy, as required by the engine instructions. For Paws cutover update delivered architecture, build instructions and migration/recovery documentation. This proposal alone does not change delivered architecture or require claiming new skill APIs.

Completion means a future game can consume the module through public targets and typed APIs, both hosts run the same quest behavior, every migrated Paws quest retains its rules and progress, and existing supported saves recover without duplicated rewards.

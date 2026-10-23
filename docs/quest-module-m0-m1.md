# Quest module M0/M1 handoff

22 October 2026. M0 baseline and M1 reusable compiled-module foundation are implemented locally. Quest execution, Paws migration and the Paws desktop application remain M2–M7 work in [the implementation plan](quest-module-implementation-plan.md).

## M0 baseline and frozen boundaries

The [baseline catalogue](../tests/fixtures/quest-migration-m0/catalogue.tsv) freezes 29 quests (18 base, 11 beach), stable IDs/content revisions, prerequisites, requester policy and authored solution lengths. [The trace](../tests/fixtures/quest-migration-m0/trace.tsv) records 398 accepted solution commands and undo/restart/practice outcomes. [Fixture documentation](../tests/fixtures/quest-migration-m0/README.md) distinguishes existing legacy saves from synthetic old envelopes. Versions 1–12 decode and round-trip through the current owner. No personal save was read or changed.

The native baseline runner validates all 29 solutions, interrupted/complete/undone saves, durable completion after undo and no repeated coins/friendship on practice. Existing progression regressions also pass. The manifest records 101 fixture/table hashes and source provenance. These checks establish portable rule/save behavior; no new Paws browser or desktop acceptance is claimed in M0.

Retain these contracts for M2:

- Afternoon remains the game-session transaction owner. Puzzle move legality, undo stacks, journal replay, historical content selection, economy, cat friendship and endings remain Paws code.
- Durable earned completion is separate from the current attempt's completed flag. Undo/restart/practice must not revoke receipts or duplicate rewards. Beach progression remains independent of base currency/friendship.
- Quest evaluation stages a candidate and effect requests; only the session commits coherent puzzle, quest and reward state. Load/replay must not emit reward effects.
- Quest prerequisites are acyclic; v1 stage graphs are acyclic, deterministic ordered stages with All/Any objectives, explicit facts/occurrences, choices and terminal outcomes. Retry is a new activation. No arbitrary script evaluation.
- Proposed M2 defaults: at most 1,024 catalogue quests, 64 stages/quest, 128 objectives/quest, 256 queued events/session, 32 copied fields/event, 128-byte IDs and 1,024-byte string values. A batch examines at most 64 events and 4,096 condition/objective evaluations. Oversized content/input rejects; deferred evaluation must not partially publish. Freeze implementation-level allocation/byte budgets with M2 tests; these are design defaults, not delivered quest APIs.
- Reuse existing explicit C++ definitions first. Engine `ApplicationArchive` may later carry versioned module state sections, but Paws keeps its validated text envelope and journals during migration. No parser dependency is added in M0/M1. Content-file decoding/schema and content-history migrations require M2's separately validated bounded loader.
- M1 composes synchronous owner-thread modules. Async IO, clocks, scene handles and cancellation retain existing engine/application owners. No second scheduler or global registry.

## M1 delivered engine API

Adjacent WebEngine adds `WebEngine::Modules`, `ModuleDescriptor`, `validateModules`, typed `ModuleComposition<Registry>` and immutable shared `ModuleSnapshot<Registry>`. The contract includes deterministic dependency ordering, API/version/capability/schema checks, bounded descriptors, non-reentrant staged installation, failed replacement preservation and provider lifetime retention/reverse release.

`webengine_add_module` packages ordinary static C++20 libraries. The standalone external consumer links only the modules library; the non-quest counter plugin creates two independent sessions and proves old snapshots remain valid after replacement. Disabled-module native and Wasm consumers build and run without the module target. No Paws API or quest assumption enters this module system.

Public engine contracts and reproduction are in `docs/modules.md` and `docs/modules-validation.md` in the adjacent WebEngine checkout. Both installed and repository-local engine skills describe these APIs and the consumer-pin limitation.

## Verification

| Surface | Result |
| --- | --- |
| Paws portable baseline and existing progression tests, MSVC Release | Passed; 29 quests, 398 solution commands, all supported schema versions |
| M0 fixture hash verification | 101 artifacts verified |
| Engine module contract tests, MSVC and Node/Wasm | 71 checks passed on each |
| External non-quest consumer, MSVC and Node/Wasm | Passed: independent sessions, retained old provider, rejected replacement |
| Disabled-module consumer, MSVC and Node/Wasm | Both built and ran |
| Chrome 155.0.8059.85, forced WebGPU and WebGL2 | Both passed button-driven state/isolation/rejection/reset and rendered-pixel checks; 1,417 changed scene pixels each; no page errors |
| Actual Windows DesktopHost, Dawn and direct D3D12 | Both passed independent sessions/rejection/reset/pause/shutdown and native capture checks; 3,088 changed scene pixels below GUI each |

Browser sample evidence is `WebEngine/build/evidence/modules/report.json`, with native image comparisons in `native-pixels.json`. The Windows smoke invokes the shared command callbacks programmatically; browser smoke clicks the actual buttons. Native pointer automation, a Paws desktop port, physical mobile, other desktop operating systems and dynamic DLL/Wasm plugins are not claimed.

The browser site is isolated under `WebEngine/build/web-release/module-site`; its Wasm SHA-256 is `2b249625986063021ae994c56b5c9a1684b6f9c502f89ac32a2426d285baac9b`. Windows uses `build/windows/Release/module_demo_windows.exe`. Evidence uses the adjacent engine working checkout based on `f4cae84a2ebef51a2e2449409653f6e2901eb211` with unrelated concurrent changes preserved. This is local working-tree acceptance, not a published engine release.

Paws still pins `0426d9319de715584a2e34ff9862cd6ba873ecd0`. M0/M1 do not advance it or migrate game runtime state. Publish the complete engine milestone deliberately before the later pinned Paws integration. Next is M2's quest kernel, using the M1 module contract and frozen baseline rather than changing Paws policy.

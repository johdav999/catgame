# Quest module M2 delivery

22 October 2026. The portable kernel is implemented and verified locally in the
adjacent WebEngine checkout. It builds on M1's typed module composition. Paws
continues to use its existing quest implementation and pinned engine revision.

## Delivered

- Optional `WebEngine::Quests`, with only `WebEngine::Modules` and standard C++20
  dependencies. It has no browser, desktop, renderer, scene or puzzle dependency.
- Validated immutable catalogues, typed facts/events, All/Any objectives,
  sequential stages, ordered branches, choices, prerequisites and explicit timers.
- Versioned custom condition/objective handlers installed by code modules.
  Catalogues retain registry/provider lifetimes; runtimes own independent state.
- Scoped monotonic events, bounded FIFO input/evaluation, atomic candidate
  transactions, stale/reentrant rejection, and bounded diagnostics.
- Durable earned completion and effect receipts across retry/practice/replay.
  Game policy validates/stages effects before committing the candidate.
- Bounded versioned content/snapshot codecs, validation, restore generation
  invalidation and native/Wasm compatibility.
- Small rendered browser and Windows consumers composing the quest module and
  a custom delivery module, plus focused tests and reproduction documentation.

Engine contracts are in `WebEngine/docs/quests.md`; evidence and commands are in
`WebEngine/docs/quests-validation.md`. Public headers are under
`WebEngine/include/engine/quests`, source under `WebEngine/src/quests`, the
standalone consumer under `WebEngine/examples/quests`, and tests in
`WebEngine/tests/quests_tests.cpp`. Required installed and Paws engine-skill
documentation was refreshed with the new quest reference.

## Verification

| Runtime / check | Result |
| --- | --- |
| Native Windows MSVC and Node/WebAssembly | Same 242 checks pass |
| Bidirectional native/Wasm interchange | Exact intermediate state/effect/trace transcripts and final 344-byte snapshots match; snapshot FNV `f840213075084f6a` |
| Actual generated Chrome site | Forced WebGPU and WebGL2 pass the completion, rejected-reward, snapshot and retry flow; 5375 changed canvas pixels each; no page/host errors |
| Native Windows rendered sample | Dawn/D3D12 and direct D3D12 pass the same controller flow; 4269 changed scene pixels each outside the GUI |
| Module-disabled native and Wasm consumers | Configure, build and run |
| Engine core/scene archive/module/quest regressions | 4/4 CTests pass |
| Paws M0 baseline | 101 artifacts verified unchanged |

Evidence lives in ignored `WebEngine/build/evidence/quests`. The samples prove the
kernel runs through the module system on both platforms. Native sample automation
invokes shared GUI controller callbacks and captures actual rendered frames;
browser automation clicks the generated page's controls.

## Remaining milestones

M3 still adds complete host clock/pause wiring, scene travel and persistent
save/load across browser/process reload. M2's rendered harness uses explicit
simulation deltas and in-memory snapshot round trips; its standalone suite does
file interchange. The kernel provides no filesystem or save coordinator.

M4 onward publishes a deliberately selected engine revision, validates and
advances Paws' pin, establishes its shared session/Windows host, then migrates
quests and old saves against the frozen baseline. No Paws puzzle rules, reward
policy, live saves or engine gitlink changed in M2. Engine changes remain local
and uncommitted alongside preserved existing work; the current Paws pin does not
provide these development APIs.

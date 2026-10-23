# Portable quest kernel

M2 provides optional `WebEngine::Quests` / `engine_quests` in the development
engine. Verify availability in the selected checkout; Paws' pin is unchanged.
Read selected engine `docs/quests.md`, `docs/quests-validation.md` and
`include/engine/quests/{definition,bindings,runtime,snapshot}.hpp` before adoption.

Enable `WEBENGINE_BUILD_MODULES` and `WEBENGINE_BUILD_QUESTS`; add `modules/` then
`quests/` for an external portable-only consumer. Both support native and Wasm.
There is no engine-core/render/UI/platform dependency. Install explicit
`quests::moduleRegistration()` into `ModuleComposition<quests::Bindings>`.
Custom code modules declare a `webengine.quests` dependency and install versioned
typed schemas and pure objective/condition handlers. Duplicate/missing/type/mode
and version errors reject staging. Catalogues retain the immutable registry and
provider captures; each Runtime owns independent progress. Capture owned services.

Definitions contain typed bool/int64/string facts/events, acyclic prerequisites,
sequential reachable stages, All/Any objectives, choices, ordered conditional
branches and completion-effect policy keys. Current objectives recompute; event
objectives accumulate once per accepted occurrence. Built-ins are quest.always,
quest.fact, quest.choice, quest.elapsed and quest.event; see exact parameter
schemas in the engine contract. No cyclic graph, script VM or parallel regions.
The delivery example supplies a custom compiled weight objective through M1.

Use owner-thread batch() stamps and activation-stamped commands; event() creates
runtime/generation/activation/stage-scoped events. post() rejects duplicates,
out-of-order producer sequences, stale/cross-runtime input, payload/schema and
queue/source overflow. Persisted source watermarks apply across quests. Pending
input is not saved. Previously queued events invalidated by a transition/retry
are consumed with diagnostics, without advancing the new stage.

prepare() copies live state and computes a candidate. Fact patches precede timer
advancement, ordered commands, lexical quest settling and the FIFO event prefix.
Callbacks must be deterministic, bounded and side-effect-free; no scene/economy
mutation, wall time, external IO or recursive runtime calls. Evaluation borrows
expire after the callback. A budget failure returns no candidate and preserves
all live state/input; retry rather than silently dropping the batch. Defaults
64 events/4096 calls, maxima 256 events/1,048,576 calls. A callback itself cannot
be preempted. The host supplies Application-derived integer simulation ms and
zero while paused, retaining fractional remainder; no new clock is introduced.

Validate effects and stage the game's other state first. Discard a candidate on
rejection. commit() revalidates identity/state/queue revisions and publishes
without allocation/callbacks; any successful post/commit/restore invalidates old
candidates. Publish other staged game state without throwing on the same owner
thread only after success. The kernel cannot atomically commit an external wallet
or filesystem. Borrowed state/view/trace references expire on commit/restore.

Completion receipts are quest-id/effect-key, independent of attempts/content
revisions. Durable earned completion unlocks prerequisites and survives practice.
Retry cannot reissue a first-completion reward. Replay records earned receipts
without emitting effects; only use with authoritative game-state reconstruction.
Persist quest state and the game's ledger/journal atomically in the outer save.

encode/decodeCatalog and encode/decodeSnapshot use bounded canonical little-endian
v1 binary data (not raw structs). Validate exact content identity, typed state,
stage path/counts, prerequisite/receipt consistency and source watermarks before
restore. Restore stages first, rebases generation/revision, drops queue/diagnostics
and emits no callbacks/effects. Old inputs/candidates reject. FNV fingerprints and
checksums detect mismatch, not tampering; structural snapshot validity is not
proof of legal gameplay. Paws keeps puzzle journals and historical save policy.

Limits: 1024 quests, 64 stages/128 objectives per quest; total 4096 stages,
8192 objectives, 16384 branches, 65536 fields; 256 facts/128 event types;
32 payload/parameter fields; 256 queued events/64 sources/64 commands per batch;
16 choices/branches/effects; 128-byte keys, 1024-byte string values, 16 MiB archives;
1,000,000 objective target and ten simulation years maximum stage time. Exact
additional limits/error behavior are in docs/quests.md. Diagnostics keep 64 entries.

Run examples/quests standalone MSVC and pinned Emscripten builds; enable
WEBENGINE_BUILD_QUEST_TESTS. Compare quest_tests --write/--verify in both directions:
both .qsave and .trace files must match (242 checks). Build optional quest_demo
and quest_demo_windows via WEBENGINE_BUILD_QUEST_DEMO. Run quests-smoke.cjs against
the generated quest-site on forced WebGPU/WebGL2; native --smoke on Dawn/direct
D3D12, verify_quest_pixels.py, plus core/scene_archive/modules regressions. Verify
disabled-module native/Wasm consumers. Evidence is distinct for each platform.

M3 now provides the shared host's Application timing, pause/suspension, scene
travel, persistent saves and actual browser/desktop file interchange. Read
[host integration](quests-host.md) and current engine docs/quests-host-validation.md
for commands; the M2 sample's old explicit-timer UI has been superseded. Paws
session extraction, native host, migration and pin publication remain M4 onward.


M7 SDK handoff: the published 461ab6dd1769171955254473c2b3623ad3dc615a revision
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

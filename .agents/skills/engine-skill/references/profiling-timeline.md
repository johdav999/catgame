# Detailed profiling timeline

Verify performance_timeline.hpp and read selected-engine docs/profiling-timeline.md
and profiling-timeline-validation.md. This development API may be absent from the
consuming game's selected pin. Existing aggregate CPU/GPU metrics remain separate.

engine_core FrameTimeline is optional fixed-storage CPU capture: 120 recent frame
or task records, 256 spans each, 32 nesting levels, about5 MiB. Allocate on the heap.
ProfileSession borrows one recorder on its owner thread. ProfileFrame brackets a
host callback; ProfileScope(ProfileCategory,name,subject) adds nested work. End
scopes before releasing the session/recorder. No scene/assets/callbacks are retained.
Labels/subjects copy bounded UTF-8 prefixes (47/95 bytes). Instrumentation disabled
by default performs no clock reads or per-span allocations. Capture/serialization
have overhead; compare normal performance with capture off.

setRecording(true/false), reset(), recording(), size(), record(index), json(),
traceJson() expose capture. Disabled/paused capture retains history; reset discards
current work/history and invalidates old tokens. Ordinary exceptions close scopes;
mid-scope stop reports truncation. Overflow/depth limits omit spans and expose drop
counters. Optional Clock function supports deterministic tests. No simulation step,
clock, AI budget, renderer policy or archive state is changed.

Categories: Frame, Simulation, Physics, Animation, AI, Loading, Render, Other.
Existing instrumentation covers Application/Scene work, physics reconciliation and
Jolt, animation sampling/layers/IK, AI controllers/states/tasks/navigation/search,
asset processing/import/image decode/worker-result hydration/archive IO/CPU upload,
and browser rendering/collection/queue submission. Add ProfileScope to application
work when needed. Worker compute/network waits/GPU execution are not owner-thread
CPU spans. Jolt scope is the blocking owner call, not individual solver-worker time.

Inclusive time contains children; self time subtracts direct children. Category
totals sum self time without nested double counting. A synchronous scope outside a
frame becomes a task record. Frame interval measures recorded callback starts and
includes scheduling/wait/tasks; long interval with short CPU time does not identify
expensive CPU work. Resume omits the first interval. Host callback scopes include
inspector work, while existing cpuMs aggregation excludes inspector processing.

Browser Performance -> Detailed CPU timeline offers Start/Pause, Open timeline,
Clear, Follow latest, Slowest CPU record, configurable .1..10000 ms budget, selected
record category/search filters, zoom and pinning while capture continues. Red can
mean CPU duration or interval budget excess. Empty filtered records are explicit.
?scene=physics&timeline=1 enables the demo capture; default stays off. Polling/display
is bounded to four updates/sec. GPU metrics continue in the normal performance panel.

Export trace downloads Chrome/Perfetto complete X events with relative microsecond
timestamps/durations, subjects/categories/record IDs and truncation/drop diagnostics.
It represents one owner lane, not automatic cross-worker/GPU clock correlation.
EngineTimeline.snapshot includes records, selected pinned record and definitions.
EnginePerformance.snapshot adds optional timeline data to the normal JSON report.
The playground owns optional recorder/session; engine_profile_control actions are
0 pause/1 record/2 clear/3 trace, engine_profile_snapshot reports. Browser adapters
stay outside portable headers. Capture is transient and changes no archive codec.

Validation: performance_timeline_tests/CTest performance_timeline, existing
performance/application/physics/animation/AI/assets/importer/scene persistence suites.
Use performance_timeline_wasm_tests with local pinned SDK (currently6.0.11), run
its JS in Node, build web_assets and serve that exact site. Run
scripts/performance-timeline-smoke.cjs serially on WebGPU/WebGL2 at DPR1 and
DEVICE_SCALE_FACTOR=2 plus existing performance-smoke.cjs. Exercise real work,
hierarchy/self times, standalone imports, pause/resume, filters/zoom/pinning,
budgets, reset, trace/metrics exports and compact controls. Bounds/exceptions/clock
anomalies and disabled no-clock behaviour have deterministic native coverage.
Desktop browser acceptance is not physical-mobile or sustained-performance proof.


## Direct D3D12 GPU profiling

Use [GPU profiling](windows-gpu-profiling.md) for the current direct
timestamp/query contract and on-demand DesktopHost::gpuProfileTrace export.
diagnostics().performance includes shared gpu summaries plus gpuProfile nested
frame/view/pass events, inclusive/self times, sampled metadata and rolling 120-sample
statistics. Three fenced readback slots sample at 10 Hz without profiling waits;
scene/view/quality/attachment epochs and device recovery reject stale results.
WEBENGINE_GPU_TIMING=0 disables native queries/readbacks at device creation on
direct and Dawn. Browser WebGPU keeps ?gpuTiming=off; WebGL2 remains unavailable.
GPU clocks are not CPU-correlated. Keep GPU types private and simulation unchanged.
Run scripts/windows-gpu-profile-smoke.ps1 for both native backends plus a Dawn-free
build, and browser performance-smoke, performance-timeline-smoke and
render-fallback-smoke on fresh pinned output. Check actual timing hierarchy, trace
eviction, loss/recovery and exact profiling-on/off captures. No throughput or
other-GPU acceptance follows. Verify the consumer pin before using new APIs.

# Puzzle input latency repair

Verified 2026-10-18 against the generated Release game using the configured local
WebEngine checkout. No engine sources or dependency pins were changed.

## Cause

Pointer handlers synchronously select/rotate a piece, project the UI and refresh
movement guidance. `Afternoon::movementOptions` preflights transactions on copies.
The old Afternoon copy constructor encoded the save, constructed all quests and
decoded/replayed every journal. Guidance repeats these copies for reachable
sliding destinations and for objects with no destinations. Legacy sliding
results were subsequently discarded because `projectPuzzle` already owns their
rectangles. Separately, every `rules::quest` lookup rebuilt the entire authored
catalogue, multiplying the cost of read models and guidance.

This work runs inside browser input events, outside the ordinary renderer update
timings. Good FPS between events therefore did not indicate responsive clicks.

## Repair

- Deep-copy live Afternoon/Attempt values, including independent legacy sessions,
  undo stacks, journals, transient suspension/access flags and command stamps.
  Save import/load continues to validate and replay untrusted bytes.
- Skip legacy slide guidance before searching and skip speculative clones for
  sources and rotation-only objects.
- Initialize immutable current/historical quest catalogues once; continue
  returning independent definition values to callers.

## Evidence

`scripts/input-latency-smoke.cjs` uses actual canvas input on the exposed
`channel_d` and `channel_e`, asserts all eight quarter-turns, and times the Wasm
pointer entry point. Four rotations use a mouse and four use touch emulation.
The 100 ms handler budget catches the original stall. It measures synchronous
handler time, not physical input-to-display latency or animation duration.

| Release run | Mean handler | Range |
| --- | ---: | ---: |
| Before, WebGPU | 992 ms | 940–1451 ms |
| Fixed, WebGPU | 6.3 ms | 5.7–7.4 ms |
| Fixed, WebGL2 | 8.5 ms | 5.9–12.6 ms |

Final build: `paws-0.1.0-df2975acbc79ddfd`.
Raw reports: `build/evidence/input-latency/{before,final-webgpu,final-webgl2}/latency.json`.
The baseline run explicitly overrides the latency budget to record the old stall.

Native `paws_release1_progression` and `paws_release1_scene` pass. Added regressions
exercise live snapshot stamps, persistent/transient state, independent mutation,
suspension and legacy/extra-quest Undo. Browser movement-guidance acceptance passes
on WebGPU and WebGL2 for the first three complete quests, including actual
rotation, drags, rejected moves, paused Undo, completion and saved reward
restoration after reload. Reports are in the `final-gestures-webgpu` and
`final-gestures-webgl2` evidence folders beside the latency reports.

Run the browser checks with `PAWS_URL` pointing to the selected generated site;
use `RENDER_BACKEND=webgl2` for the fallback and distinct `EVIDENCE_DIR` folders.
Do not rebuild that site while a browser acceptance run is using it.

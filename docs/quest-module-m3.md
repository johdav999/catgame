# Quest module M3 delivery

22 October 2026. M3 is implemented and verified locally in the adjacent WebEngine
checkout. It adds the complete quest-host vertical slice on top of M1/M2, with
one shared controller used by browser and Windows. Paws' runtime and pin remain
unchanged; its extraction/migration starts in M4.

## Delivered

- Application-driven quest time with fractional millisecond carry, pause,
  suspension and no hidden-time catch-up.
- Persistent Save/Load and startup restoration of quest state, wallet, scene
  location and explicit pause in one bounded versioned envelope.
- Browser localStorage plus file import/export; Windows atomic per-user saves
  plus CLI file interchange using the identical `.qhost` format.
- Travel between Harbour and Orchard with scene destruction/recreation while
  session progress and receipts remain outside the scene.
- Generation-stamped controls and delayed imports; invalid/stale loads, failed
  storage writes and failed scene binding preserve valid state.
- Both quest branches, the module-supplied delivery objective, and once-only
  rewards through completion, reload, practice and cross-platform interchange.

Implementation is in WebEngine `examples/quests/{session,controller,scene}.hpp`,
with browser/Windows adapters, `tests/quest_host_tests.cpp` and
`scripts/quests-host-smoke.cjs`. Read WebEngine `docs/quests-host.md` for ownership,
save format and usage, and `docs/quests-host-validation.md` for reproduction.
The optional portable kernel remains independent of scene, renderer and platform
IO; sample-specific economy/save/controller policy lives in the example.

## Evidence

| Check | Result |
| --- | --- |
| Shared host suite | 49 checks pass on Windows and Node/Wasm |
| Quest kernel suite | 242 checks pass on native and Wasm |
| Existing engine regressions | Application, core, scene archive, modules, quests, host: 6/6 pass |
| Generated browser site | Forced WebGPU and WebGL2 complete persistence, pause, travel, failure and interchange flows; no page/host errors |
| Windows native hosts | Dawn/D3D12 and direct D3D12 pass separate seed, restart and import processes |
| Actual file interchange | Browser mid-timer download resumes/completes on desktop; desktop export uploads back into browser with 20 coins and no duplicate reward |
| Scene lifecycle | One scene after travel/load; zero after native shutdown; stale handles/commands reject |
| Paws baseline | 101 M0 artifacts still match |

Native acceptance uses real DesktopHost/renderers with controlled monotonic
timestamps and shared GUI-controller commands. Browser acceptance clicks the
actual generated page, reloads it, downloads files and uploads native results.
The 500 ms timer freezes over longer paused waits. Screenshots were inspected and
pixel differences verified outside the native GUI. Evidence and artifact hashes
are in ignored `WebEngine/build/evidence/quest-host`.

## Remaining work

M4 must publish a deliberately selected engine revision, verify the consumer
against it and advance the Paws gitlink, then extract the shared Paws session and
add its native host. Later milestones migrate quests and historical saves against
the frozen baseline. This sample does not migrate Paws, provide autosave/cloud
saves, solve concurrent writers or establish physical mobile acceptance.

Engine work remains local and uncommitted alongside preserved unrelated changes.
Both required engine-skill copies and the implementation plan were updated for M3.

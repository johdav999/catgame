# Quest host integration (M3)

The development engine's optional `examples/quests` now supplies the full host
vertical slice on browser and Windows. Read selected engine `docs/quests-host.md`
and `docs/quests-host-validation.md`, then inspect session.hpp, controller.hpp,
scene.hpp and both entry points. Verify the consumer pin before adopting it.
This does not migrate Paws or advance its engine dependency.

The generic quest kernel remains standard C++ plus Modules only. Sample game policy
owns its wallet, current location, explicit pause state and complete save envelope.
Controller borrows the one Application and installs simulate/present callbacks;
Windows uses DesktopHost's Application, browser owns an Application. Feed only
frame.simulation.delta, round incoming seconds to nanosecond precision and carry
fractional milliseconds. Pause/suspension/transition/load rebase through existing
Application APIs; never add a wall clock or catch up hidden time. Browser graphics
unavailability also resets the clock baseline. Explicit pause survives suspension.

Own progress outside scenes. Travel builds a fresh QuestScene, registers the camera,
then commits requestTransition(..., true) at idle. The browser renderer rebinds
before outgoing destruction; native resolves DesktopView component IDs. Projection
resolves scene-scoped handles, never delayed raw pointers. Failed binding/load
preserves the old active scene/session. One scene remains after each travel.
Clear event callbacks before their captures expire, including exception paths.

The shared 32-command queue carries a transient host epoch; pause, travel, retry,
replacement and lifecycle cancellation invalidate old controls. File import captures
epoch before asynchronous reading, then validates it before publication. Runtime
identity/generation/stage checks still protect kernel events. Apply this boundary
to future worker/network callbacks as well; no scene borrow belongs in a save.

Session::save/decode use the sample's <=64-KiB WEQHOST1 envelope: little-endian
location/pause, wallet, IEEE-double fractional-ms remainder, length-prefixed M2
snapshot and FNV checksum. Validate all bytes, exact content/state/receipt identity,
valid metadata and wallet consistency before building the replacement scene/session.
Sample wallet must be 20 iff delivery earned completion, else 0. Checksums are not
authentication; games retain authoritative journal/save policy. These are sample
fields, not a Paws save format or a new mandatory engine save service.

Save writes one snapshot/economy envelope. Browser uses origin-scoped localStorage
key webengine.quest.host.v1 and bounded base64; failed setItem leaves the old value.
File import/export copies bounded bytes through explicitly exported HEAPU8 and
frees temporary Wasm allocations. Windows uses DesktopFiles/atomic replacement in
the QuestHostSample per-user directory, with --user-directory for test isolation.
--import and --export exchange the same .qhost bytes. Startup auto-restores the
last explicit save. Save is manual; import replaces live state but does not persist
until Save. Bad startup data reports an error without overwriting it. No cloud,
cross-tab coordination, autosave or physical power-loss acceptance is implied.

Enable WEBENGINE_BUILD_MODULES/QUESTS/QUEST_DEMO/QUEST_TESTS. Build quest_demo or
quest_demo_windows and quest_host_tests. The latter passes the same 49 checks on
MSVC and Node/Wasm; kernel remains 242 checks. Run Application/core/scene_archive/
modules/quests/quest_host regressions. Serve the generated quest-site and run
scripts/quests-host-smoke.cjs for WebGPU/WebGL2 and native Dawn/direct D3D12,
fresh-process restoration, storage failures, stale imports and real bidirectional
download/upload interchange. Run tools/verify_quest_host.py for native pixels and
artifact hashes. Legacy quests-smoke.cjs runs browser-only and does not prove native
support. Native smoke uses real renderers and controlled monotonic timestamps;
browser uses RAF. Only Windows x64 and desktop Chrome were accepted, not physical
mobile or other desktop OSes. M4 starts consumer publication/host extraction;
Paws quest/save migration remains later work.

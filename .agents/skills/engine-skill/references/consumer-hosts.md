# Consumer host integration and Paws M4

Use WebEngine::Browser for the browser renderer rather than reproducing its source
list in a game. Native hosts use WebEngine::Desktop and its sole Application.
A shared game session borrows Application; platform adapters own IO, audio
backends, rendering and file dialogs. Game quest/save policy remains in the game.

GuiSystem::clear() cancels capture/focus and destroys children at an idle rebuild
boundary, retaining font/theme/scale. Never clear during widget callbacks. Native
PointerPreview can reserve custom widget drags before GUI; Pointer/Wheel report
guiConsumed and logical coordinates afterward. Cancel game drags on InputCancelled.
DesktopHost::setWaterTime(optional<double>) accepts finite 0..1000000 seconds;
use simulation time or reduced-motion zero, and null for host presentation time.
Read selected-engine docs/consumer-hosts.md and verify pin availability.

Paws M4 shares GameSession between browser and Windows adapters. M6 makes all
29 quests authoritative through the Afternoon-owned QuestAdapter, composed as
paws.quests with webengine.quests. Paws owns puzzle legality, rewards and endings;
the kernel owns availability, stages, completion and first-completion receipts.
PAWS_AFTERNOON 14 validates all 29 ID-keyed projections against historical
journals. Versions 1-13 reconstruct in replay mode without reward effects;
version 13's two-pilot table is compatibility input only.
Browser pre-quest-m6 and native saves/pre-quest-m6.save retain exact old bytes
before replacement. Backup failure blocks migration writes. Existing M5 backups
remain available. Native assets remain executable-relative. Read Paws
docs/quest-module-m6.md for migration, rollback and scoped native/Wasm,
WebGPU/WebGL2 and Windows evidence. No engine API, direct-D3D12 or physical
mobile acceptance is added by this consumer migration.


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

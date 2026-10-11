# First quest and home garden

The production flow is Play/Continue → courtyard → Meet Pip → Help Pip → puzzle
→ reward card → courtyard. The current checkout also contains phase 6's two
later requests; they continue to share `Afternoon` as the transaction owner.

`makeHomeGarden` composes accepted phase-3 Pip and phase-4 kit resources. It uses
24 shared paving instances, a low rear fence, stone edges, one bench, restrained
planting and two decorative pots. The puzzle boards use the same immutable
resources. Decorations remain outside the missing-ball board's logical cells.
The home garden displays the returned ball only after the first reward exists.
No purchased furnishing is implied by an empty spot.

Three stable spots reserve 1.10 × 1.35 metre footprints: `bench_corner` at
(-1.65, 0, 0.85), `sunny_corner` at (1.65, 0, 0.85), and `quiet_corner` at
(1.65, 0, -0.85). Both kit furnishings fit; phase 7 owns occupancy, purchase,
placement and Pip's use of them. Quiet code-native ground outlines mark the
spots. Camera framing responds to desktop, portrait and short landscape sizes.

## Transactions and snapshots

The first successful Roll commits completion, the `pip_missing_ball` reward
ledger entry, 20 coins and one friendship through `Afternoon::dispatch`.
The command journal allocates its new entry before gameplay mutation. Undo and
restart affect the current puzzle, while the earned ledger remains intact.
Practice never grants additional coins or friendship. Logical success never
waits for an animation or storage callback.

The version-2 `PAWS_AFTERNOON` codec stores active quest, content IDs, each
attempt/revision identity, command epoch, settings, wallet, friendship and the
three reward records. It also retains bounded accepted command journals, which
rebuild current puzzle state, selected object, hints, pause/garden state and undo
history. Decode validates a separate candidate by replaying the same rules;
balances and reward records must match that history. It rejects unsupported
versions/content, invalid identities, illegal commands, truncation and trailing
data. Version-1 journals remain readable and upgrade on the next write.

Back to garden suspends the current attempt without changing its identity.
Scene publication advances the command revision to reject queued actions;
Restart changes the attempt. Cross-quest activation uses a new command epoch.
Restore preserves the explicit version-2 identities. Runtime actor/component
handles, meshes, renderer state and callbacks are rebuilt rather than saved.

The browser uses WebEngine's `IndexedDBStorage` byte adapter in namespace
`paws-afternoon`, slot `current`. Every accepted command, scene navigation and
settings change writes one coherent snapshot. The adapter copies bytes and
serializes transactions. `Saved` is published only after the latest transaction
receipt; old acknowledgements cannot label a newer snapshot durable. Failed
writes retain the current model and expose Retry. Invalid stored bytes remain
untouched and block writes until a valid candidate can be opened.

This first save path does not yet provide phase 8's multi-tab locking, bounded
backup history, journal compaction or file export/import. Storage read failure
currently exposes recovery and Retry. There is no implicit localStorage fallback.

## Scene and UI lifetime

The active scene remains bound during cancellable asset preparation. The complete
candidate is constructed and validated before renderer binding and Application
activation. The orbit controller and renderer release old scene references before
unloading it. Cancelling drops the pending publication and request interests;
late HTTP completions cannot replace the active scene. Model loading failure has
explicit Retry loading and Back controls.

Pip's accepted dialogue portrait is loaded in the same preparation batch.
Completion immediately presents the actual reward card and starts a bounded
happy action; Application advances skeletal animation once. Reduced motion uses
the authored static happy pose. The completion card remains actionable throughout
the reaction. Ordinary modals pause simulation, while the garden may animate Pip
without advancing the suspended puzzle. Host suspension freezes both.

`PawsUi::enter` clears obsolete modal history at scene publication and completion.
Nested ordinary modals retain focus return. Completion Back/Escape dispatches
garden navigation instead of revealing an obsolete puzzle sheet. All moves use
the retained GUI's shared mouse, touch and keyboard-equivalent command path.

## Repeat verification

```powershell
./scripts/build.ps1 -Pinned -Configuration Release -EmsdkPath <installed-emsdk> -BuildDirectory build/web-phase5-pinned-release
node scripts/phase5-smoke.cjs
cmake --build build/native --target paws_rules_tests paws_scene_tests paws_ui_tests paws_phase5_scene_tests paws_phase6_scene_tests
ctest --test-dir build/native -R '^(paws_rules|paws_starter_scene|paws_ui|paws_phase5_scene|paws_phase6_scene)$' --output-on-failure
./scripts/serve.ps1 -SiteDirectory build/web-phase5-pinned-release/site -Port 19485
```

The browser runner owns and closes its port-19485 server and isolated Chrome
contexts. `SITE_DIR`, `EVIDENCE_DIR` and `BROWSER_EXECUTABLE` override its defaults.
Evidence is in `build/evidence/phase5/`; the final report records the served Wasm
hash, browser, viewports, checks and expected versus unexpected errors. The GUI
gallery is OFF in this production build. Chromium touch emulation does not
establish physical mobile acceptance.

Continue with prompt 6 if its current implementation needs further work, then
prompt 7. Extend the existing `Afternoon` codec and transaction owner for spending,
ownership and placement; preserve version-1/2 migration and the reward ledger.

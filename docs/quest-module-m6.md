# M6 — full Paws quest-kernel cutover

All 29 current quests now use `WebEngine::Quests` for prerequisite availability,
current completion, durable earned completion and first-completion receipts.
`paws.quests` composes with `webengine.quests` through the published compiled
module system. The engine pin remains
`76d17928a5e51bbaac53874867e08a510e5b9c8f`.

## Single live orchestration path

The M5 pilot map and shadow effect type have been removed. Both legacy and modern
puzzle commands stage an `Afternoon` candidate, feed validated puzzle facts to
the kernel, validate its candidate, and publish the game/kernel pair together.
Every live reward-ledger entry comes from kernel earned completion. Paws applies
its existing wallet, friendship, pack, chapter, shop, ending and reward-animation
policy to those receipts. Beach receipts continue to grant no base rewards.

The prior grant and prerequisite evaluation code is retained solely behind the
private save-replay flag. It reconstructs historical journals for validation and
never selects a live quest authority. There is no migration switch per quest.
Public progression completion/unlock views use the kernel; the puzzle algorithms
remain in `PawsSession` and `rules::Attempt`. Existing puzzle stages, authored
solutions, undo history, hints, content upgrades and presentation are retained.

Undo and restart re-evaluate current objectives while preserving earned completion
and receipt identity. Practice cannot issue a new first-completion effect.
Progress remains outside scenes. Account/development access is transient Paws
policy and cannot be obtained from a save. Command generations still fence
paused, imported and departed scenes.

`questKernel.authoritative` reports the catalogue size, with stages keyed by
quest ID. The synchronous interaction diagnostic reports the existing scene-input
guards as `inputBusy`; acceptance waits for ball/clue/reaction presentation using
that read-only value. It adds no command or puzzle behavior.

## Save compatibility

The writer now emits `PAWS_AFTERNOON 14` and `QUEST_KERNEL 2 <quest-count>`.
Each ID-keyed record retains actual content identity, attempt, earned/current
completion, checkpoint, stage and receipt. Record order is irrelevant. The
original puzzle journals, wallet, friendship, settings, purchases, placements,
introductions and ending sections keep their existing meaning.

Versions 1–12 use their supported historical validators and reconstruct kernel
state in replay mode. Version 13 retains its exact two-record pilot validator;
its ID table exists only in that compatibility reader. Version 14 requires one
record for every catalogue entry. In both formats, supplied kernel records must
match journal reconstruction. Missing, duplicate, unknown, contradictory and
future-schema records reject the whole candidate without replacing live state.
Loads emit no completion effects or reward presentation.

Before replacing an older browser save, its exact bytes commit to `pre-quest-m6`
outside rotating history. Windows writes `saves/pre-quest-m6.save`. Failed backup
writes block migration writes, and secondary writers cannot replace the primary
save. Existing M5 backups remain intact. Original export prefers the M6 backup,
with M5 fallback. Older M5/M4 binaries cannot read version 14; rollback uses the
preserved original and the matching old build, excluding later progress.

## Behavior baselines and validation

Before editing the cutover, the already-built M5 executable captured
`tests/fixtures/quest-migration-m5`: all 29 current quests, 396 authored solution
commands and version-13 partial/completed/undo saves. Its manifest records source
and artifact hashes. M6 compares the exact current trace, catalogue and save body,
projecting only the new kernel section/header back to M5's two-record format.
No journal, owner revision or other gameplay field is normalized in this strict
comparison. The independent M0 baseline retains its older layouts and 398-command
trace with its documented historical-layout comparison. Both frozen corpora keep
their original hashes.

The full portable catalogue suite covers every authored command, stale/invalid
input, pause, pack denial, completion undo, restart/practice, travel and save
round-trip. It imports both old corpora, checks every receipt against forgery,
accepts reordered kernel records, and verifies all purchases, placements and
separate MVP/garden/beach endings. Native and Wasm traces compare byte for byte.

Generated acceptance evidence is under `build/evidence/m6/`. The full browser
suite imports each frozen M5 partial through the validated public recovery path,
restarts and solves it with scene gestures and retained GUI, reloads interrupted
attempts and travels back to the garden. The Windows catalogue smoke uses real SDL
GUI input plus shared typed puzzle intents, checks completion undo and practice
for every quest, and retains one scene across travel. In practice it invokes the
shared restart UI action so clue playback resumes normally.

The final portable suite performs 5,205 checks on each target, imports 198 old
save fixtures and executes 396 current authored commands. All 36 native
regressions passed; the five portable regressions also passed on native and
Wasm, including both frozen behavior baselines. The M0 and M5 manifests verify
202 unchanged artifacts.

Browser migration storage checks cover versions 12 and 13 on both renderers:
failed backup leaves current bytes intact, Retry commits the original backup,
and original export remains byte-identical after reload. Storage recovery also
covers a closed IndexedDB connection, quota, queued writes and second-writer
protection. Windows startup migration verifies exact version-12/version-13
backups across two launches. Actual file chooser/download interchange preserves
all eight owned furnishings and placements, coins, friendship and ending flags.

Ending acceptance replays the original ten-quest garden picnic and the later
beach ending from separate validated saves. This retains the existing arrival
and occupied-lane rules: a fully furnished late-game garden can reject a
gathering without changing its saved ending. Both rendered endings, garden
return and reload pass on WebGPU and WebGL2.

Host report paths are `final-webgpu/report.json`, `final-webgl2/report.json`,
`windows-final/catalogue.json`, `migration-storage/report.json`,
`interchange/report.json`, `save-recovery-{webgpu,webgl2}/report.json` and
`endings/report.json`, beneath the M6 evidence directory. `identity.json` records
the verified served Wasm, Windows executable and matching portable trace hashes.
Browser runs resume only against the identical build ID. After interruption,
the first 21 WebGL2 passes were recovered from their retained completion
screenshots and observed run results; the resumed report marks those entries.
Renderer recovery is awaited before input, and an input is retried only if its
revision did not advance.

Historical layout tests now construct version-12 envelopes before changing a
content ID to an old supported layout. A current-format envelope whose kernel
record contradicts its content is deliberately rejected, rather than weakening
the new validator to accommodate an inconsistent synthetic fixture.

The builds reuse `build/web-pinned-m4-release/site` and
`build/windows-pinned-m4/package/Release`; these are incremental build directory
names. Current payload identities and report paths are recorded in the M6
evidence. Standard new builds still use `scripts/build.ps1 -Pinned` and
`scripts/build-windows.ps1 -Pinned`.

Portable verification:

```powershell
cmake --build build/quest-pilots-m5 --config Release --parallel 4
ctest --test-dir build/quest-pilots-m5 -C Release --output-on-failure
python tools/quest-baseline/manifest.py --verify-all
./build/quest-pilots-m5/Release/paws_quest_catalogue.exe build/evidence/m6/native-rules
# Build the same portable targets with emcmake in a separate tree and run
# paws_quest_catalogue.js with Node; compare catalogue.trace hashes.
```

This is Windows x64 and desktop Chrome acceptance, including forced WebGPU and
WebGL2. Physical mobile and other native operating systems remain outside the
verified scope. Paws edits remain local alongside preserved existing work.
M7 remains release hardening and the final module SDK handoff.

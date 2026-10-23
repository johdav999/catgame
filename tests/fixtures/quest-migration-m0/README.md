# M0 quest migration baseline

Captured 22 October 2026 from the current Paws working sources using `tools/quest-baseline`. This is a compatibility baseline, not a runtime migration.

- `catalogue.tsv`: all 29 quest IDs, content revisions, packs, chapter/requester policy, prerequisite IDs and solution lengths.
- `trace.tsv`: 398 accepted authored solution commands plus selection, undo, restart and practice-completion checkpoints. Values include current puzzle completion, durable reward completion, wallet, chapter and each cat's friendship.
- Per-quest `partial`, `completed` and `undo` saves: full version-12 envelopes, including cumulative progress in preceding quests. These are generated fixtures, not personal saves.
- `legacy-v1` through `legacy-v5`: copies of the existing released-format regression fixtures, preserving their earned progress/purchases/settings.
- `synthetic-fresh-v6` through `synthetic-fresh-v12`: generated fresh envelopes for each supported structural schema. They are not saves captured from historical executables. Progressed historical-content coverage additionally remains in the existing progression tests.
- `manifest.json`: fixture SHA-256 identities and the gameplay source hashes used for the capture.

Verify without changing fixtures:

```powershell
cmake -S tools/quest-baseline -B build/quest-baseline-m0 -G "Visual Studio 18 2026" -A x64
cmake --build build/quest-baseline-m0 --config Release --parallel 4
ctest --test-dir build/quest-baseline-m0 -C Release --output-on-failure
python tools/quest-baseline/manifest.py --verify
```

`paws_quest_baseline --verify` reproduces outcomes and compares exact fixture bytes. Future migration tests may require a semantic comparison for the new writer, while retaining these original bytes as import inputs. Do not regenerate to conceal changed behavior. Explicit baseline recapture uses `--write` followed by `manifest.py --write` and requires review of every changed expectation.

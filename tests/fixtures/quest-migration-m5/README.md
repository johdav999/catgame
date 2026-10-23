# Frozen M5 behavior before M6

Captured on 22 October 2026 using the already-built M5 `paws_quest_baseline`
executable, before editing the M6 cutover. `manifest.json` records source and
artifact hashes. These are generated compatibility fixtures, not personal saves.

The current catalogue has 29 quests and 396 authored solution commands. This
capture includes the existing v2 windbreak and shell-encore layouts. The older M0
capture retains its original layouts, 398 commands and unchanged artifact hashes.

Per-quest partial/completed/undo saves use version 13. Legacy v1-v5 and synthetic
fresh v6-v12 envelopes retain the older readers. Exact comparison with M6 removes
only its additive full-catalogue kernel records, projecting the two M5 records
and header back to version 13. All other bytes, including journal contents and
owner revisions, must match this capture. The outcome and catalogue TSVs compare
without normalization. Do not regenerate this corpus to conceal a regression.

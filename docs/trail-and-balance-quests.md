# Scent trail and balance quests

Two optional chapter-three requests extend the catalogue from ten to twelve.
They target 6–10 minutes of first-time play through deduction and rearrangement,
without a timer or forced delay. This is a design target; automated solution
playback does not establish human pacing.

## Follow the scent

`pepper_scent_trail` unlocks after Pepper's garden bells. Six consecutive trail
stops each provide three independent observations: paw prints, snagged fluff and
catnip scent. Inspect all three, then compare three destinations. Only one fits
all evidence. A wrong trail retains discoveries without advancing the journal.
Undo restores the previous stop with its evidence. Eighteen observations and six
trail choices form the supplied solution. Sound and colour carry no exclusive clue.

Click the paw mat, mouse and herb pot to inspect evidence, or use the named HUD
buttons. Click a destination pot or its Follow button. On narrow screens, the
evidence buttons become destination buttons once all clues are found. Existing
kit meshes serve as markers; destination descriptions and evidence are textual.

## Pip's wobbly perch

`pip_wobbly_perch` unlocks after the playhouse request. Four setups use cushions
weighted 1, 2, 3 and 4. Drag them onto separate beam positions; each cushion may
return to its own storage mat. Paw marks identify weight; beam positions run
left to right 1, 2, 3, 5, 6, 7. Storage starts left to right with weights 1, 2, 3, 4.

| Setup | Support position | Pip's load | Required cushion torque |
| --- | --- | --- | --- |
| 1 | 4 | None | 0 |
| 2 | 3 | Weight 2, two steps right | -4 |
| 3 | 5 | Weight 2, three steps left | +6 |
| 4 | 4 | Weight 3, three steps right | -9 |

The check sums `weight * (position - support)` and Pip's signed load. All four
cushions must occupy distinct beam positions and the total must equal zero.
Tap the paw mat or Test balance to commit a stable setup; the next returns the
cushions to storage. The supplied solution has sixteen placements and four
tests. Other zero-torque arrangements also work. Beam tilt is visual feedback;
physics and animation do not decide success.

## Ownership and saves

Both families live in the portable release-one rules module. `Afternoon` remains
the progression and reward owner. Each request awards 20 coins and one friendship
for its requester once. Undo, restart and practice cannot duplicate rewards.
Both requests are optional for the original ten-request picnic ending, retaining
existing eligibility.

Version-seven saves write twelve ID-keyed attempts. Version-six imports retain
their ten attempts and initialize two fresh optional attempts. Versions one
through five keep the legacy import path. New semantic verbs `inspect`,
`follow_trail` and `test_balance` replay through the normal rules processor.
Stages, evidence and arrangements reconstruct from journals, including Undo and
restart. No separate browser state owner is introduced.

Scenes compose existing approved meshes. No generated model, runtime asset
replacement, engine change or engine gitlink update is needed.

## Verification

Build/run `paws_trail_balance_tests`. A directory argument writes isolated browser
fixtures. With the C++ runtime DLLs on PATH:

```powershell
./build/native/paws_trail_balance_tests.exe build/evidence/trail-balance/fixtures
./scripts/build.ps1 -Pinned -Configuration Release
./scripts/serve.ps1 -SiteDirectory build/web-pinned-release/site -Port 8168
node scripts/trail-balance-smoke.cjs
```

The pinned engine currently fails renderer initialization when WebGPU is
disabled. For the configured development engine's WebGL 2 path, build with
`./scripts/build.ps1 -Configuration Release`, serve `build/web-local-release/site`
on a separate port, set `PAWS_URL` to that site and `QUEST_BACKEND=webgl2`, then
run the same browser script. Keep these two engine acceptance scopes distinct.

Browser tests use isolated IndexedDB saves and real input for inspections, wrong
choices, cancelled/occupied drops, Undo, restart, full solutions, midway reload
and completed-reward restoration. Reports and screenshots are under
`build/evidence/trail-balance/<backend>/`. Portrait checks establish browser
layout only; physical-device acceptance and human pacing remain unverified.

### Recorded acceptance, 17 October 2026

- Six focused native suites passed: `paws_trail_balance`, `paws_release1_rules`,
  `paws_release1_progression`, `paws_release1_gui`, `paws_phase6_scene` and
  `paws_release1_scene`. The scene suite also plays all twelve owner traces.
- Pinned Release build `paws-0.1.0-8112c19d0a34a93f`: both WebGPU quests passed,
  including 390×844 target bounds and a portrait drag. Report:
  `build/evidence/trail-balance/final-webgpu/report.json`.
- Configured local-engine Release build `paws-0.1.0-15632ab8dc366338`: both WebGL 2
  quests passed with the same checks. Report:
  `build/evidence/trail-balance/final-local-webgl2/report.json`.
- Both successful browser runs reported no page/console errors. Their reports
  retain exact Wasm hashes. The original engine gitlink is unchanged.

# Cat action timing audit — 17 October 2026

Audited all 29 clips in each selected toon-v2 cat against the catalogue and the
garden/quest presentation consumers. Meshes, coats and animation assets remain
unchanged; these fixes synchronize the game with the imported animation.

## Corrections

| Action | Finding and correction |
| --- | --- |
| Personality reaction | A fixed 2.4 s timer cut off the 3 s clip. Completion now reads the selected clip duration. |
| Button's bed ending | The old 4–5.8 s spin replaced the new clip midway through lying down. Toon cats now play the complete authored knead/lie/sleep sequence. |
| Mouse pickup | The old paw nudge and 2.6 s attachment belonged to a different animation. The new mouth pickup attaches at the composed clip's 2 s midpoint, before lifting. |
| Gate toy pickup | Attachment moved from 1 s to the 1.8 s lower/lift join. |
| Bell | Response begins at the new paw extension, 1.3 s instead of 1.9 s. |
| Playhouse entry/exit | A 1.8 s crouch cycle was stretched across a 1.6 m path despite a measured 0.99 m stride. Playback now follows actual travelled distance, including reverse exit; blocked movement does not advance the feet. |
| Sleeping guest | Waking uses the authored `rest_leave` action instead of playing the settling action backwards. |
| Walking sounds | The distance scheduler retained the old 0.36 m stride. It now uses the selected rig's 1.035 m stride. |

Quest-ending duration is read from the selected clip. The actual exported sunny
and drinking endings are already 5 s (the import script overrides its defaults
from the previous catalogue); they were not truncated. Ball and basket contacts,
attention, cushion/bowl transitions, scratch, perch, picnic sits and stationary
idle/rest/use loops retain their existing timing. Preview-only clips have no
additional gameplay completion clock. The previous 2.3 s walking/turning fix is
retained. Presentation completion never grants rewards or changes saved progress.

## Verification

- `build/evidence/cat-action-timing/clip-audit.json`: exact selected asset hashes
  and all 116 clip durations compared with catalogue values.
- Native scene tests: all ten quest traces, exact mouse/bell cue boundaries,
  full clip durations, uninterrupted bed pose, pause/restart and save invariance.
- Native final garden tests: all eight furnishing entry/use/leave/return paths,
  all four full reactions, sleeping/waking, reduced motion and collision checks.
- Native gait tests: actual skinned planted contacts for walk and crawl;
  audio test: bounded contacts over the new stride.
- Actual browser quest evidence: `build/evidence/cat-action-timing/webgpu/`
  and `webgl2/`, including pause, reaction, settled and completed captures.

This is timing acceptance, not a new mesh/contact-placement or physical-device
performance acceptance.

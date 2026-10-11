# Inspected correction history

All revisions are one coherent kit, reviewed against the Paws reference board.
The retained `renders/rN-*` images and packed `garden-kit-rN.blend` checkpoints
show the actual work. Logs are under `logs/` (ignored by Git, retained locally).

| Review | Observation | Correction and next evidence |
| --- | --- | --- |
| r1 silhouette/materials | All inventory present; troughs, thick pot rims, cushion volumes and toy recesses read. Packed UVs ran wood grain across bench slats; mouse socket did not describe its ears/tail well. | Authored member/curved-route UVs and revised mouse outline; r2 kit shows aligned grain and recognizable recess. |
| r2 material/assembly | Grain improved; oversized review floor showed distracting mottling and edge piece overlapped bench. Native import found rotated-object bounds were overestimated. | Tiled paving review, separate edge placement, evaluated-vertex bounds and degenerate cleanup in r3. |
| r3 clean import and Pip fit | Actual toy-mesh tests found six mouse-tail vertices hitting its rim. Pip's original muzzle stayed above a floor bowl. Cushion dimensions supported the actual resting pose. | Added tail clearance and raised ceramic pedestal; r4 source/reimport and Pip/bowl close-up show contact. Native socket tests then pass. |
| r4 source/export comparison | Widened tail cut plus pre-boolean bevel left a dark wedge at basket rear. | Apply a small bevel after all socket cuts; r5 removes wedge without filling recesses. |
| r5/r6 renderer comparison | Original component exports caused 333 colour draws in the full preview. | Join evaluated export copies by material; source stays editable. Exact final served bytes show 53 draws per pass before the concurrent two-material Pip update. |
| r6/current Pip | HTTP checksum validation detected a concurrent Pip replacement; current drinking pose moved the muzzle upward 23 mm. The earlier local server was also serving stale bytes. | Dedicated absolute-path server, hash every HTTP GLB, raise waterline 23 mm, retain rim clearance; final r7 fit checks use current Pip. Increase preview shadow normal bias to suppress grazing-surface acne. |

Final source, all-side clean import and Paws screenshots are assessed in
`EVALUATION.md`. Earlier screenshots intentionally remain as correction evidence;
they are not the final delivered appearance or current Pip identity.

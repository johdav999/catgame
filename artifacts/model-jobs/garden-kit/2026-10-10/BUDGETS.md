# Measured asset budgets

All triangles include modeled rims, cloth seams, foliage and modifier results.
Vertices are exported vertices including UV/normal splits, not editable mesh vertices.

| Asset | Triangles | Vertices | Materials / primitives | GLB bytes |
| --- | ---: | ---: | ---: | ---: |
| paving.glb | 752 | 1,536 | 1 / 1 | 476,772 |
| edge.glb | 188 | 384 | 1 / 1 | 436,476 |
| fence.glb | 2,068 | 4,224 | 1 / 1 | 454,132 |
| planting.glb | 4,716 | 3,744 | 4 / 4 | 152,172 |
| bench.glb | 2,632 | 5,376 | 2 / 2 | 495,320 |
| pot_round.glb | 5,612 | 4,537 | 6 / 6 | 535,420 |
| pot_fluted.glb | 5,612 | 4,535 | 6 / 6 | 535,356 |
| channel_straight.glb | 572 | 288 | 1 / 1 | 319,104 |
| channel_curve.glb | 2,780 | 1,658 | 1 / 1 | 376,200 |
| ball.glb | 960 | 666 | 2 / 2 | 29,272 |
| mouse.glb | 1,836 | 1,407 | 3 / 3 | 508,468 |
| fish.glb | 984 | 955 | 4 / 4 | 40,552 |
| basket.glb | 4,552 | 7,267 | 3 / 3 | 1,081,276 |
| blocking_cushion.glb | 3,524 | 2,460 | 2 / 2 | 551,312 |
| cushion.glb | 3,852 | 2,559 | 3 / 3 | 556,964 |
| switch_base.glb | 188 | 384 | 1 / 1 | 319,948 |
| switch_plate.glb | 380 | 572 | 2 / 2 | 22,788 |
| gate_frame.glb | 824 | 1,108 | 2 / 2 | 347,880 |
| gate_leaf.glb | 2,068 | 4,224 | 3 / 3 | 456,004 |
| bowl.glb | 1,440 | 1,283 | 2 / 2 | 51,848 |
| bowl_water.glb | 48 | 49 | 1 / 1 | 3,072 |

Total: 45,588 triangles, 49,216 vertices, 7,750,336 GLB bytes.

Six 512-square base-colour maps: 6 MiB of unique RGBA8 colour pixels before mips.
Embedded self-contained GLBs duplicate some maps. Do not assume cross-file GPU
deduplication. Actual renderer counters including Pip and review paving are
in browser-evidence/garden-smoke.json; total process/Wasm heaps are not measured.

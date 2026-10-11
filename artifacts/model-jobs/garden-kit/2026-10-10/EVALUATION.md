# Final evaluation — revision 7

Prompt 4's complete model inventory is delivered and inspected in Blender and
the actual pinned Paws WebGPU renderer. The detailed [iteration record](ITERATIONS.md)
retains more than three inspected correction cycles. [Budgets](BUDGETS.md),
[manifest](asset-manifest.json) and [build/content hashes](validation-build.json)
identify this exact acceptance set.

| Check | Evidence and result |
| --- | --- |
| Silhouettes and construction | `renders/r7-kit.png`, `r7-top.png`: open troughs, continuous curved route, rounded pot rims, two pot variants, recognizable ball/mouse/fish and matching cut recesses, separate cushion identities, gate openings, raised ceramic bowl. |
| Materials | Native 512-square generated stone/clay/wood/cloth/wicker inputs visibly feed delivered materials. Member and curved-route UVs follow grain. Soft cushion shape and major basket relief are geometry. Opaque leaf groups stay readable. |
| Packed source | Clean Blender process reopened `garden-kit-r7.blend` and verified consumed images are packed. Named construction pieces and modifiers remain editable; only evaluated export copies are joined. |
| Clean GLB import | `exports/r7/clean-reimport.json`: all 21 GLBs independently imported, exact triangle counts, finite vertices, UVs and metre bounds within 1 mm of the manifest. Front three-quarter, rear, side and top renders inspected; no missing backs or filled route/socket openings. |
| Portable geometry checks | `paws_garden_kit`: actual imported triangle surfaces verify straight/curved route floor continuity, ball width, bowl interior, all three toy silhouettes inside basket recesses, switch travel and clear opened gate. No mouse-tail intersections after correction. |
| Pip proportions | `renders/r7-pip-cushion.png`, `r7-pip-bowl.png` and browser furnishings views use the current `pip-tripo-repaired-r7`. Cushion supports the resting body. Raised bowl/water align with the head-lowering pose; numeric muzzle reference is about 21 mm above water. Full approach/settle/drink gameplay transitions remain phase 7. |
| Runtime | `browser-evidence/garden-smoke.json`: Chrome 154.0.8037.99 on Windows, actual generated Paws site. Every HTTP GLB hash matches delivered files. Normal page has zero console errors. Failed basket HTTP request produces failure state; retry successfully loads the kit. |
| Layouts and controls | Captured 1440×900, 390×844 and 844×390 CSS viewports; overview and close inspections, desktop and shadow-free light, keyboard Enter, rapid latest-pose selection and suspension boundary. Immediate inspection poses provide reduced motion. No horizontal page overflow. |
| Lifecycle/build | Pinned Release browser build passes. Portable garden and starter-scene tests pass 2/2. Manifest-generated C++ drift check passes. No engine source or pin change. |

The final full review scene, including all assets, 30 additional paving instances
and the current two-material Pip, reports **86,002 visible triangles, 54 colour
draws and 54 shadow draws** (108 total draws and 172,004 submitted triangles across
both passes). One instanced paving draw is active. The older 333-draw export was
rejected and consolidated; its screenshots are iteration evidence only.

Final counters: 33,556,240 tracked CPU asset bytes, 39,276,812 GPU texture bytes,
49,220,152 total tracked GPU payload bytes. These are engine counters, not total
browser/process memory. Self-contained GLBs embed some duplicate images. The kit
itself contains 45,588 triangles and 7,750,336 GLB bytes. The short automated run's
frame interval is approximately 20 ms; it does not establish sustained performance
or a CPU/GPU bottleneck. No physical phone, Safari or thermal test was performed.

The raised bowl is an intentional fit decision for the existing cat clip. Water
is a simple opaque surface, foliage is opaque geometry, normals are neutral and
roughness is independently authored. Preview poses verify interfaces and appearance;
they do not implement rewards, shopping, game progression or fluid/rigid-body rules.

No phase-4 tool or asset blocker remains. Later gameplay acceptance and physical
device checks retain their own phase scope. Concurrent Pip and puzzle work was
preserved and is identified separately in repository implementation status.

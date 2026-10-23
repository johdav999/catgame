# Beach driftwood ball channels

The beach ball quest uses two dedicated Tripo meshes: `beach_channel_straight`
and `beach_channel_curve`. The presentation adapter selects these only for beach
objects whose role is `channel`. The garden channels and water gutters retain
their existing assets. Puzzle definitions, saves, input, rotation logic, covered
channel visibility and the rolling route are unchanged.

The models share the existing Y-up metre contract: a floor-contact pivot at the
cell rotation centre, straight endpoints X ±0.5 m, and a curved radius of 0.5 m
joining (-0.5, 0.06, 0) to (0, 0.06, -0.5). Generated meshes were normalized,
their ends squared and their rolling floors fitted to the authored 0.06 m height.
Both contain 4,000 triangles, two materials and 1024-pixel color/normal maps.
Combined GLB transfer size is 5,571,456 bytes. Source 4K maps and provider meshes
are retained outside runtime.

The new assets append to the coastal manifest and use its existing asynchronous
preparation, import, relative-URL and scene ownership paths. The accepted-assets
staging step includes their exact bytes in the generated game site.
No engine change or submodule pin update is part of this pass.

## Sources and reproducibility

- [Job README and editable masters](../artifacts/model-jobs/beach-channels-20261020/README.md)
- [Measured asset manifest](../artifacts/model-jobs/beach-channels-20261020/asset-manifest.json)
- [ImageGen references and exact prompts](../artifacts/model-input-images/beach-channels-20261020/prompts.md)
- [Review iterations](../artifacts/model-jobs/beach-channels-20261020/ITERATIONS.md)
- [Provenance and Tripo task IDs](../artifacts/model-jobs/beach-channels-20261020/PROVENANCE.md)
- [Runtime rights notice](../assets/runtime/models/beach/NOTICE.md)

Run retained `review.py` stages 1, 2 and 3 in Blender, inspecting each stage.
`check_masters.py` independently reopens packed source masters.
`publish.py` promotes reviewed exports and preserves existing coastal entries.

## Validation — 20 October 2026

Blender 3.5.1: inspected source, fitted meshes, independent GLB reimports,
end-face correction and a joined straight/curve/straight assembly. Both exports
have UVs, finite geometry and no boundary cracks after welding import splits.
101 centreline samples per model measure maximum rolling-floor errors of
1.29 mm / 2.91 mm. Both editable masters reopen with their maps packed.

Native `paws_starter_scene` and `paws_release1_scene` passed (2/2, 34.55 s).
The scene test asserts that the beach actually selects these models and retains
its prior grounding, basket selection and scene/lifecycle checks.

The browser build uses the existing explicitly selected
`build/engine-water-depth-transition` engine in `build/web-water-depth-release`,
preserving prior local water/shadow fixes. Native tests use the pinned engine.
Actual browser runs use `scripts/beach-adventure-smoke.cjs` against port 8081,
with evidence under `build/evidence/beach-channels/{webgpu,webgl2}/`.
Each run verifies exact downloaded model hashes, live instances, rotations,
basket dragging, undo/cancellation, save/reload, garden return, portrait input
and the completed ball route. `beach-channels-connected.png` records the joined
route before the ball is released. Both actual WebGPU and forced WebGL2 runs
passed with no browser errors; connected-route screenshots were visually
inspected on both backends. Both reports confirm HTTP 200 and exact manifest
hashes for the two GLBs. The current port-8081 preview serves build prefix
`/__paws_build_73753c56933223b8/`.

This is desktop browser and Blender acceptance; no physical-mobile performance
or commercial license entitlement is established.

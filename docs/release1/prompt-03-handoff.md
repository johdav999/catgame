# Prompt 03 handoff — props, furnishings and pinned preview

Implemented 11 October 2026. Delivered **30 new GLBs**, packed editable Blender
masters, parameterized construction scripts, approved ImageGen material reuse,
eight actual-model thumbnails, namespaced manifest/catalog/assembly fragments,
portable import/fit tests and an isolated pinned browser preview.

The asset-level implementation is verified. Final cushion-board composition
remains dependent on Prompt 4's missing socket coordinates; final cat contact
animation belongs to Prompt 6. Neither dependency is disguised as completed
production integration.

## Owned paths

- `artifacts/model-jobs/release1-props/2026-10-11/`: `props-r6.blend`, revision
  exports/renders, geometry/verification scripts, copied original ImageGen atlas
  and exact prompt, portable maps, provenance, iterations, evaluation, budgets,
  material inventory, raw logs and archived actual-browser evidence.
- `assets/runtime/models/release1/props/`: 30 GLBs, provenance, manifest and
  eight 512-square shop thumbnails with identity manifest.
- `docs/release1/fragments/props.json`: stable assets, legacy reuse identities,
  eight catalog records, role bindings, anchors, assemblies and pending gates.
- `src/release1/props/{prop_assets.hpp,prop_assets.cpp,manifest.inc,preview_main.cpp}`:
  isolated portable composition and browser host executable.
- `tests/release1/props/prop_tests.cpp`, `scripts/release1/props/` and
  `web/release1/props/`: fit/lifecycle tests, packaging/build/evidence tooling and
  inspection-only shell. `build/native-release1-p03`,
  `build/web-release1-p03-pinned-release`, `build/evidence/release1/prompt-03`
  are task-specific output directories.

Pre-existing shared MVP/Prompt 1 changes were preserved. A concurrently created
`artifacts/model-jobs/cat-tripo-inputs/` directory was left untouched. No agents
were spawned and no messages were sent to other chats. The engine submodule
remains clean at `3fe7d1dc1dc610c703944803cbb717cd65024ec9`; adjacent-engine
development changes were inspected read-only and preserved.

## Accepted asset IDs and reuse

All new IDs have prefix `release1/props/` and suffix `@1`:

| Family | Names |
| --- | --- |
| Retrieval / route | `crate`, `plank_1m`, `plank_2m`, `platform`, `plank_socket` |
| Fitting | `cushion_square`, `cushion_bar`, `cushion_elbow`, `socket_square`, `socket_bar`, `socket_elbow` |
| Bells | `bell_leaf`, `bell_star`, `bell_paw`, `bell_support` |
| Water | `rain_barrel`, `water_straight`, `water_curve`, `water_support`, `water_drop` |
| Furnishings / playhouse | `scratching_post`, `sunny_perch`, `hammock`, `playhouse_frame`, `playhouse_door`, `low_picnic_table`, `picnic_mat` |
| Picnic clues | `shade_clue`, `adjacency_clue`, `quiet_clue` |

Reuse the accepted bench/mouse, cushion, bowl/water, channels, switches, ball,
toys, gates, ground and edges at their original IDs/bytes. The fragment references
all 21 old kit identities. Required reach/drink/switch anchor aliases are additive
sidecar overrides; the accepted files are unchanged. New geometry totals 52,674
triangles and 13,118,440 GLB bytes. All individual assets stay below 12,000
triangles; texture sizes are at most 512 square. See the detailed budget table
for material-slot exceptions and measured texture duplication.

## Interfaces and dependencies

- Metres, Y up, +Z forward; anchors include local position and XYZ degrees.
  Mesh-node hierarchy is never assumed to instantiate actors. Door, bell and
  switch transforms remain independent checked actor handles.
- Preserve the 1 m route grid, 0.26 m trough and local y=0.06 floor. The water
  preview uses a supported 0.40 m elevation and a falling ribbon to the unchanged
  y=0.383 bowl surface. Use those explicit transforms; do not sink the bowl.
- Planks: exact 1/2 m length, 0.50 m width, y=0.16 support/port plane. The socket
  is a separate guide, not an extra layer under the walking surface.
- Cushion origin is the lower-left occupied-cell boundary, 0.25 m grid. Rotate
  by quarter turns about that origin and apply the same normalization translation
  to cushion/socket. Edge inset is 0.016 m, socket floor y=0.036. True edge inset
  clears the elbow's concavity; a bounding-box scale does not. The supplied
  `cushion_basket` arrangement is an **inspection layout**, not Prompt 4's board.
  Before integration read the rules handoff and place the independent socket
  modules at its actual origins/rotations; test the final occupied arrangement.
- Playhouse: 0.76 m clear width, 1.13 m height above y=0.07 floor; footprint fits
  1.50 x 1.60 m. Reserve the open door sweep outside that footprint. Switch plates
  retain y=0.09/0.055 states; artwork does not evaluate the held-switch condition.
- `contactEnvelopes` records perch deck/step, hammock sag, entrance and picnic
  guidance. Anchors are presentation inputs, not proof that final cat animations
  match. Prompt 6 must apply measured Mabel/Button/Pepper contact overrides,
  supported transitions and body reservations; hammock curl and four-cat mat
  occupancy need particular attention. Existing Pip was not changed.

## Fresh validation

- Blender 3.5.1: six retained source passes; actual inspected correction cycles
  and clean reimport comparison. All 30 GLBs independently reimported with matching
  bounds/triangles, UVs, finite geometry and packed images. Front/rear/side/top
  kit and assembled front/rear renders; eight actual-model shop renders.
- Native `release1_props`: passed. All 51 new/reused assets import without
  warnings; all three cushions have zero sampled vertex/rim collisions; plank
  surfaces, bench/door clearances and furnishing footprints pass. Rapid assembly
  reset, scene shutdown and stale bindings pass. Existing `paws_starter_scene`
  also passed in its configured native tree.
- Pinned Emscripten 6.0.11 Release preview built/staged. Chrome actual generated
  site: 51 HTTP GLBs match delivered hashes; 41 individual/assembly views,
  changed geometry for door/plates/bells/cushions, shadow-free and narrow
  portrait/landscape captures, keyboard activation, rapid pose changes,
  suspension boundary and injected HTTP failure/retry passed. Zero normal-page
  console/runtime errors. Exact browser version, payload hashes and scoped
  performance/resource counters are in `validation.json` and browser report.
- Frozen-contract/fragment validator and task-local record script pass. Every
  new quest role resolves, all assembly dependencies exist, catalog IDs/prices
  match the frozen eight entries, thumbnails and GLB identities verify.

Early defects were corrected, not waived: hanging-pivot review placement,
playhouse roof/entrance dimensions, Boolean winding and concave cushion inset.
The initial native source compile used a nonexistent vector-length helper; this
was corrected to the verified dot-product API before tests ran. Initial sandbox
toolchain execution required approved access to installed Ninja/compiler tools.

Desktop automation and narrow viewport capture do not establish physical mobile,
Safari, final cat animation, independent player or sustained game performance
acceptance. The original approved generated-texture rights record is retained;
no invented provider license or new generation claim is made.

## Reproduce

Run from the repository root. These SDK/Jolt locations describe this machine;
the build script takes parameters and always compiles the pinned engine.

```powershell
$job='artifacts/model-jobs/release1-props/2026-10-11'
& 'C:/Program Files/Blender Foundation/Blender 3.5/blender.exe' --background --python-exit-code 1 --python "$job/scripts/build.py" -- --revision 6
python scripts/release1/props/package.py 6
& 'C:/Program Files/Blender Foundation/Blender 3.5/blender.exe' --background --python-exit-code 1 --python "$job/scripts/verify.py" -- 6
./scripts/release1/props/build.ps1 -Native -JoltSourceDir C:/Users/Johan/source/repos/WebEngine/build/dependencies/JoltPhysics-5.3.0
# Put the configured CMAKE_CXX_COMPILER directory on PATH for GCC runtime DLLs.
ctest --test-dir build/native-release1-p03 --output-on-failure
ctest --test-dir build/native -R paws_starter_scene --output-on-failure
./scripts/release1/props/build.ps1 -EmsdkPath C:/Users/Johan/source/repos/WebEngine/.tools/emsdk -JoltSourceDir C:/Users/Johan/source/repos/WebEngine/build/dependencies/JoltPhysics-5.3.0
node scripts/release1/props/smoke.cjs
python scripts/release1/validate-contract.py --fragment docs/release1/fragments/props.json
python scripts/release1/props/record.py
python scripts/serve-site.py --directory C:/Users/Johan/source/repos/CatGame/build/web-release1-p03-pinned-release/site --port 19613
# Open http://127.0.0.1:19613/
```

The smoke runner owns its local server/profile and closes both. Blender uses
background processes without closing any interactive Blender session.

## Prompt 9 integration patch

No shared-file patch was applied. Integration should:

1. Merge `docs/release1/fragments/props.json` into the production asset catalog
   using its exact IDs/URLs/hashes/bytes, reuse references and provenance. Preserve
   eight existing frozen item prices/IDs; map the new thumbnail URLs.
2. Bind role assets and explicit assembly transforms in the garden/puzzle owner.
   Apply the final Prompt 4 socket coordinates, and Prompt 6 cat-contact overrides.
   Runtime anchors are application data; the engine does not read this JSON.
3. Reuse `assets()` and the generated asset specifications if useful. The
   `makePreview`/`pose` code, HTML inspector and neutral floor are inspection-only
   and must not replace production scene composition or game GUI.
4. Stage the accepted namespaced runtime directory through the existing asset
   pipeline. Exclude task preview binaries/shells from the production package.
   Re-run final puzzle/furnishing journeys after shared composition is wired.

No common implementation-status rewrite, shared manifest update, root build
registration, engine change, commit, push or publication was performed.

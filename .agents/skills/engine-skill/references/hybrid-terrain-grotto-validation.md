# Mesh-shaped cuts, enlarged grotto and basin validation

2026-10-23, local Windows desktop with repository Emscripten 6.0.11.
Evidence: `build/evidence/terrain-mesh-holes/`.

The last room now scales width, height and length by cbrt(10), about 2.1544,
for approximately ten times its former volume. It retains the −8 m walking
floor, a narrower connecting tunnel and a shared closed rear dome. A dry rock
basin is locally sculpted into the same floor, reaching −8.94 m beside the
unchanged centreline. Both hosts provide Lower grotto and Grotto basin views.

The demo also uses the new portable mesh-contact-rim hole preparation before
seam construction, preserving separate manual and fixed-cliff masks.

Nine focused native CTests passed: terrain_mesh_holes, terrain_mesh, terrain_lake,
terrain_cliff, hybrid_cave_route, hybrid_terrain, terrain_lod_archive, terrain_seam
and terrain_hole_lod. The route suite covers the default entrance and eight
width/depth/height extrema, room dimensions, closed source topology, full motor
descent/ascent, basin collision, dry centreline and saved/restored geometry.
The archive suite retains exact base/derived geometry, locks, repeated save/load,
invalid-load preservation and v35 mesh compatibility. The mesh-hole suite also
passed as Node/Wasm: concavity, conservative cells, islands/disjoint openings,
transforms, nonuniform sloping terrain, ownership, attribute splits/T-junctions,
source-only assets, patches, collision and atomic rejection.

Browser Chrome 155.0.8059.85 passed scripts/hybrid-terrain-smoke.cjs on forced
WebGPU and WebGL2. Both runs exercised real controls, paint/fill, undo/redo,
cancellation, entrance editing, material blending, camera/shadow LOD, device
recovery, storage save/load, fresh-page reload, corrupt-save preservation,
post-load editing, room/basin views and full curved descent/ascent. Basin-view
commands verify the full-detail bed and dry centreline collider. The saved scene
retains the enlarged room, and LOD regenerates from saved recipes.

Native Dawn and direct D3D12 host acceptance is being recorded in this task;
its final outcomes and image checks are added below when complete.

Two integration defects were repaired during validation: signed cliff distance
now remains continuous around endpoint bends, allowing bounded refinement;
seam checks distinguish intentionally painted holes beside the cliff from solid
collar samples. Neither repair changes ownership of manual holes.

The hole operation expects clean authored contact rims, before seam/sculpt
modifiers. It is conservative heightfield-cell rasterization, not exact curved
Boolean excavation or closed-volume intersection. The lower bowl contains no
water; the separate Grand Chamber pond remains. No physical-mobile or sustained
large-world performance acceptance is claimed.

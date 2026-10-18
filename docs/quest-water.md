# Quest water presentation

The drinking route uses WebEngine's water shader for all five gutter surfaces,
the brief bowl inlet and the filled drinking bowl. The picnic bowl and purchased
garden drinking bowls use the same shared water materials. Imported meshes and
materials remain unchanged: `src/water_surface.hpp` supplies component overrides.

Water remains dry until the rules accept the route. The existing ending reveals
gutters in route order, briefly shows the inlet and fills the bowl before Mabel
drinks. Undo and restart hide the water again. Picnic bowls start filled and
follow their bowl actor when moved. Bowl water uses the authored 0.383-unit fill
anchor; gutters sit at 0.08 above their support so the surface clears the wooden
trough floor. Underside faces are culled to avoid overlapping faces in the
flattened bowl mesh.

Waves in bowls are 0.0015 units high; gutter/inlet geometry stays fixed while
ripple normals animate. Reflection is restrained to preserve puzzle readability.
Refraction, foam and planar capture stay disabled for these tiny surfaces.
The browser supplies Application simulation seconds once per frame. Pause and
host suspension freeze the clock; reduced motion sets it to zero. Shading never
decides flow, completion, movement, collision, rewards or saved progression.

The pinned WebEngine revision and configured development checkout supply `water.hpp`,
`MaterialShader::Water` and `Renderer::setWaterTime`. CMake enables
`PAWS_WATER_SHADER` for both. The engine pin is now
`0426d9319de715584a2e34ff9862cd6ba873ecd0` (18 October 2026).

## Verification

Build the browser with `./scripts/build.ps1 -Pinned -Configuration Release` and
the installed SDK path. Run the portable `paws_starter_scene`,
`paws_phase6_scene`, `paws_release1_scene` and `paws_release1_final_garden` tests
for scene and transition regressions.

The `paws_release1_scene_tests` executable accepts an optional output directory
and writes valid production-owner `water.save` and `picnic.save` fixtures there.
For browser acceptance, generate them in `build/evidence/quest-water/fixtures`,
serve the actual generated site, then run:

```powershell
$env:PAWS_URL = 'http://127.0.0.1:8156/'
node scripts/quest-water-smoke.cjs
$env:WATER_BACKEND = 'webgl2'
node scripts/quest-water-smoke.cjs
```

The fallback check rejects initial WebGPU adapter acquisition while leaving the
existing Paws startup policy intact. Reports and screenshots are written under
`build/evidence/quest-water/webgpu` and `webgl2`. The explicit
`Module._paws_water_snapshot()` diagnostic reports the water clock and active
shader surfaces. Checks cover dry rejection, accepted flow, visible water pixels,
pause/suspension, WebGPU device recovery, Undo, restart, saved rewards, fresh
practice, picnic bowls, narrow layouts and reduced motion. Completed saves
return home; choosing Practice intentionally starts a new dry attempt. Native
scene coverage also verifies filled completed-scene reconstruction directly.

These are desktop Chromium checks, including viewport emulation, rather than
physical mobile acceptance.

Verified on 15 October 2026: both browser reports passed with no console or page
errors against Release build `paws-0.1.0-9a44b96e2452f084` (Wasm SHA-256
`c6d000f81a251f5afb9014d06e5ebd033919c0596499ead69c80c63f3f04a418`).
The native water-capable integration run passed 298645 checks. The four selected
pinned-engine scene suites and both development/pinned browser builds passed.

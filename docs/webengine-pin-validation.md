# WebEngine pin update — 18 October 2026

CatGame advances `external/WebEngine` from
`3fe7d1dc1dc610c703944803cbb717cd65024ec9` to
`0426d9319de715584a2e34ff9862cd6ba873ecd0` (upstream `main`,
“Expand terrain, decal, FX, and asset support”). The engine working tree is clean.
`docs/release1/contract.json` carries the same reviewed dependency revision;
the production build and staged-asset audit now read that value instead of
maintaining separate hard-coded hashes. Historical handoffs and asset provenance
retain the engine versions used for their original validation.

The new pin supplies the sky, terrain, water and specialized material pipelines
already used by CatGame's local-engine builds. CMake enables those existing game
integrations for pinned builds. The temporary browser packaging bridge still
compiles upstream renderer sources without copying or modifying engine code.

Two audio test backends implement the new `AudioBackend::playbackRate` interface.
The existing GUI layout matrix also exposed negative development-board image
sizes in a short landscape viewport with larger text. Cell padding now scales
with small cells, and cells too small for their fixed text inset omit the image
while retaining their accessible description. Production board presentation uses
the existing 3D scene.

## Reproduction

```powershell
./scripts/build.ps1 -Pinned -Configuration Release -EmsdkPath C:/Users/Johan/source/repos/WebEngine/.tools/emsdk -Jobs 2
cmake --preset native
cmake --build --preset native --config Debug --parallel 2
ctest --preset native -C Debug --output-on-failure
```

Use the native compiler's matching runtime directory on `PATH`. On this machine,
omitting the configured MinGW `bin` directory loads incompatible runtime DLLs and
causes Windows error `0xc0000139` before tests can execute. Browser checks use the
actual generated `build/web-pinned-release/site`, served explicitly with
`scripts/serve.ps1 -SiteDirectory build/web-pinned-release/site`.

Current run evidence is retained under `build/evidence/webengine-pin/` and the
`build/webengine-pin-*.log` files.

## Validation results

- All 33 native CTests passed, including scene/lifecycle, rules/progression,
  save migration, collision, terrain, audio, GUI viewport/text-size matrices,
  furnishing use and four-cat garden behavior. The full Debug suite took 307.92 s.
  Log: `build/webengine-pin-native-final.log`. The production scene executable
  additionally passed 7,040 checks and regenerated the water/picnic save fixtures.
- `scripts/movement-guidance-smoke.cjs` completed all ten main quests on WebGPU
  and WebGL 2 with zero browser errors. Both runs verified real mouse gestures,
  legal destinations, cancellation, Undo, pause, invalid-move rejection,
  completion, return to garden and saved once-only rewards after reload.
  Reports: `build/evidence/webengine-pin/{webgpu,webgl2}/movement/report.json`.
  These runs used the initial updated-engine build
  `paws-0.1.0-11016ac7e1874564`, Wasm SHA-256
  `4f9b9df240d1fa0832b9129181dd37b4ea88b8e158402b601d9b95ea79496706`.
  The subsequent final rebuild includes the development reference-board sizing
  repair described above; focused browser checks record that final identity.
- The production asset audit passed for 177 accepted assets, including 86 GLBs,
  with matching staged hashes and rights records. Nine stale generated preview
  files from the reused output folder were preserved under
  `build/preserved-previews-webengine-pin/`, outside the deployable site.
  Report: `build/evidence/webengine-pin/asset-audit.json`.
- On the final rebuilt payload, `garden-terrain-smoke.cjs`,
  `garden-sun-smoke.cjs` and `quest-water-smoke.cjs` all passed on WebGPU and
  WebGL 2 with zero browser errors. Each report's build identity matches the
  final site below. The terrain runs each passed 12 checks, including 64 active
  chunks, no fallback slabs, frustum coverage, texture hashes, real puzzle
  interaction, restart, reload, portrait and landscape layouts. Sun checks found
  visible sun pixels through orbit/zoom and restored ordinary framing. Water
  checks covered dry-route rejection, animated pixel changes, completion,
  restart, saved rewards, picnic bowls, pause/suspension, reduced motion and
  WebGPU device recovery. Reports and screenshots are under
  `build/evidence/webengine-pin/{webgpu,webgl2}/{terrain,sun,water}/`;
  `final-summary.json` validates all six report identities.
  The first parallel WebGL 2 feature run encountered local-server connection
  refusals; its log is retained as `build/webengine-pin-webgl2-first-attempt.log`.
  The serial rerun completed every feature suite successfully.

Final Release site: `build/web-pinned-release/site`.

```text
buildId: paws-0.1.0-22121f70143cfd65
enginePin: 0426d9319de715584a2e34ff9862cd6ba873ecd0
wasmSha256: 036ac3f01f4d6530bc443353f17a2e1bc1a0a0f033451122c552e7a116b6f8e5
contentSha256: bbe171dac004224eef4530c201dc1dbd97e75d7a8a5bac6ba5b4863ed2de9ae5
noticesSha256: 24e12796be95110624b46f338bb1ebb7b71bfe1608848d67af160d729dcaae32
```

## Scope

These checks establish desktop Chrome behavior and Chromium viewport emulation.
They do not establish physical Android/iOS performance, Safari compatibility,
production hosting, or the separate audio rights/listening acceptance gates.

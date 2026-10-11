# Evaluation

Engine pin: 3fe7d1dc1dc610c703944803cbb717cd65024ec9. Blender: 3.5.1. Desktop browser: Chrome 154.0.8037.99, Windows, WebGPU.

| Check | Result | Evidence |
| --- | --- | --- |
| Source weight defect | Replaced all weights; zero unweighted vertices in clean FBX and GLB imports | exports/clean-reimport.json |
| Canonical bone hierarchy | All 29 original bones, same parents | rig-glb.txt, rig-fbx.txt |
| Canonical local bind pose | 28 non-root local matrices exact in GLB; maximum FBX difference 5.96e-8; root-follow also verified | rig-glb.txt, rig-fbx.txt |
| Root-follow | All 10,407 imported vertices follow a 0.1 m skeletal-root displacement; max distance error 3.73e-8 m | rig-glb.txt, rig-fbx.txt |
| Anatomical bindings | Every limb, paw, head, ear, eyelid and tail bone has positive weights; no rigid mesh-node fallback | rig-glb.txt, rig-fbx.txt |
| Texture imports | Default fully textured FBX and GLB imports pass with no missing-map warnings or raised limits | validation-fbx.txt, validation-glb.txt |
| Skin and bind geometry | Maximum bind geometry error 1.33e-7 m (GLB), 3.21e-7 m (FBX) | rig-glb.txt, rig-fbx.txt |
| Clip contract | Nine named actions at their exact canonical durations, in-place root, loop endpoints, transition boundaries, floor/height envelope, reduced motion and lifecycle pass | validation-glb.txt, validation-fbx.txt |
| Source/export integrity | Packed master retains all maps and nine actions; clean-process imports have 29 bones, two materials, at most four normalized influences | exports/clean-reimport.json |
| Browser playback | Actual generated Paws site; nine controls, visible skin movement, suspension, static poses, rapid selection, shadow modes, three layouts, failed-load retry | browser-evidence/pip-smoke.json |
| Runtime appearance | Inspected desktop idle and lowered rest; source coat identity and separate limbs retained | browser-evidence/desktop.png, browser-evidence/static-5.png |
| Model budget | 17,854 triangles, two materials; 1024-square coat maps and 64-square eyelid map | asset-manifest.json |
| Further acceptance | Rest is a compact crouch, not a fully authored cushion curl; final furniture contacts and physical mobile remain untested | docs/pip-asset.md |

The GLB is exercised on the actual WebGPU renderer. The FBX is imported and animated through the pinned portable C++ pipeline, with exact canonical bind/skin tests. These are scoped compatibility checks, not a measured mobile-performance claim.


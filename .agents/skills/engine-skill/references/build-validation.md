# Build and validation

Run from the selected repository root. Follow current README, `.emscripten-version`, CMake options and scripts if they differ from these landmarks.

## Windows browser build

```powershell
# Setup only when the local SDK is absent or mismatched.
./scripts/setup.ps1
./scripts/build.ps1 -Configuration Release
./scripts/serve.ps1 -Port 8080
```

Use `.tools/emsdk`; the inspected pin is Emscripten 6.0.11. Its wrappers are `upstream/emscripten/emcc.exe` and `emcmake.exe`. The browser target links `--use-port=emdawnwebgpu` and GLES3/WebGL 2 with `-sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2`. Verify the current pin instead of silently switching to a global SDK/version. Setup and first configuration may download dependencies. Linux/macOS setup and build commands remain in README.

Builds publish into `dist/`. Separate build directories can still publish the same `dist/`; coordinate shared-checkout browser builds. For Ninja `failed recompaction: Permission denied`, check lock ownership and prefer a fresh build tree without cleaning unrelated work:

```powershell
& ./.tools/emsdk/upstream/emscripten/emcmake.exe cmake -S . -B build/web-game-release -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build/web-game-release
```

Check listeners before serving; reuse the correct server or choose a free port. Keep servers managed and stop only task-owned processes. `serve.ps1` serves the repository `dist/`; for an isolated staged site, use `python scripts/serve.py --port 8081 --directory C:/absolute/path/to/site`. Verify that site contains the intended `index.html`, JS and Wasm, avoiding stale outputs/404s. Localhost works for development; deployment uses HTTPS. Publish the whole site with `application/wasm`, including worker wrapper/module/Wasm when used, and inspect hosting/CSP/CORS requirements. The current isolated-worker pipeline needs no cross-origin isolation.

Browser automatic mode supports WebGPU and WebGL 2. Use the playground's `?renderer=webgpu` and `?renderer=webgl2` to exercise each backend; these query options are host policy, while reusable initialization uses `RenderBackendPreference`. Test reduced fallback capabilities, unavailable-backend failure and context/device recovery when affected. Do not treat fallback as a guarantee of browser/device acceptance.

## Portable native checks

```powershell
cmake -S . -B build/native -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build/native
ctest --test-dir build/native --output-on-failure
```

Reuse a valid configured build; use a separate directory for generator/configuration conflicts. For narrow changes, select meaningful tests with `ctest -R` after checking registered names. Add regression coverage when it checks behaviour/invariants, not implementation wording. This native configuration tests portable core/GUI contracts, not browser rendering or a native game backend.

Inspect `ctest --test-dir build/native -N`/current CMake before selecting names. Relevant registered suites include `mobile_assets`, `render_quality`, `assets`, `application`, `selection`, `manipulation`, `physics_joints`, `movement`, `animation_control`, `root_motion`, `prefab`, `persistence` and `camera_controller`. For missing compiler runtime DLLs (for example exit `0xc0000139`), identify the configured compiler and put its matching runtime `bin` directory on PATH rather than mixing toolchains.

## Browser and gameplay evidence

Browser scripts use Node, Playwright/pngjs and a supported Chrome/Edge executable. Discover installed dependencies rather than assuming example paths exist. Follow each script's setup and test-fixture requirements:

```powershell
$env:NODE_PATH = 'path/to/test/node_modules'
$env:BROWSER_EXECUTABLE = 'C:/Program Files/Google/Chrome/Application/chrome.exe'
$env:DEMO_URL = 'http://localhost:8080'
node scripts/browser-smoke.cjs
```

Choose relevant scripts: `application-smoke.cjs`, `input-smoke.cjs`, `ui-input-smoke.cjs`, `gui-smoke.cjs`, `physics-smoke.cjs`, `spatial-query-smoke.cjs`, `physics-joints-smoke.cjs`, `skeletal-animation-smoke.cjs`, `assets-smoke.cjs`, `audio-system-smoke.cjs`, `persistence-smoke.cjs`, `render-quality-smoke.cjs`. Verify current files; some build explicit Wasm targets or host their own runners.

For the newer pipelines use `render-fallback-smoke.cjs` (backend acquisition/capabilities/recovery), `mobile-assets-render-smoke.cjs` (upload admission/progress on both backends), `texture-tool-smoke.cjs` (native mip/package tool) and worker coverage in `assets-smoke.cjs`. Set `ASSET_TEST_SITE` to the chosen generated `asset-tests` directory when that runner requires it. Inspect explicit CMake targets/options for portable Wasm checks, including `asset_browser_tests`, `spatial_query_tests`, `physics_joint_wasm_tests`, `animation_control_wasm_tests` and optional persistence/movement runners.

Rendering-specific changes can use `compositing-smoke.cjs`, `shadow-smoke.cjs`, `material-smoke.cjs`, `material-graph-smoke.cjs`, `glb-import-smoke.cjs`, `morph-smoke.cjs`, `fx-smoke.cjs`, `instancing-smoke.cjs`, `frustum-culling-smoke.cjs`, `mesh-optimization-smoke.cjs`, `mesh-detail-smoke.cjs`, `mesh-detail-quality-smoke.cjs`, `cache-upload-smoke.cjs`, `frame-reuse-smoke.cjs` or `performance-smoke.cjs`. Check each suite's expected backend/fixture and env vars; WebGPU-specific counts and pixel expectations do not establish fallback parity.

Play the game itself: start/load, perform its representative mechanic, observe feedback/state changes, complete/fail and restart/transition. Exercise affected input ownership, resize and page suspension; verify save/reload and callback reconstruction when delivered. Playground smoke checks alone cannot prove a new game's behaviour. Capture console/GPU errors and useful canvas screenshots/results under `build/evidence/` or supported script destinations.

For performance claims compare matching Release builds, browser/GPU, viewport, scene/camera and quality. Distinguish cold loads, warm CPU work, GPU timing and frame intervals; note unavailable counters and scoped memory estimates. Desktop browser evidence/mobile emulation cannot establish physical Android/iOS controls, audible output, browser compatibility or sustained mobile performance. Report unavailable required environments as remaining limits.

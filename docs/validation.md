# Setup validation

Verified locally on 10 October 2026. The game has no configured remote, and these
setup changes have not been committed or pushed.

## Dependency and boundaries

- WebEngine submodule URL: `https://github.com/johdav999/WebEngine.git`.
- Recorded gitlink: `3fe7d1dc1dc610c703944803cbb717cd65024ec9`.
- The submodule working tree is clean. The adjacent development checkout was used
  only as a source override; this task made no changes in that checkout.
- `build/fresh-config` configured the pinned engine and downloaded/hash-verified
  Jolt through WebEngine's FetchContent definition without a local source override.
- Game output stays inside each Paws build directory; engine demo publication and
  engine test executables are excluded from normal Paws builds.

## Executed checks

| Check | Result | Evidence |
| --- | --- | --- |
| Pinned native build, GCC 16.1 | Passed | `build/native/paws_scene_tests.exe` |
| Native scene/lifecycle/camera test | 1/1 passed | `build/native/Testing/Temporary/LastTest.log` |
| Pinned browser Release, Emscripten 6.0.11 | Passed | `build/web-pinned-release/site/` |
| Adjacent local-engine browser Release | Passed | `build/web-local-release/site/` |
| Chrome browser smoke, pinned engine | Passed, zero console/runtime errors | `build/evidence/pinned/browser-smoke.json` |
| Chrome browser smoke, local engine | Passed, zero console/runtime errors | `build/evidence/local-engine/browser-smoke.json` |
| Fresh pinned configuration | Passed | `build/fresh-config/CMakeCache.txt` |

Browser checks confirmed seven visible meshes, non-flat rendered pixels, pointer
orbit, camera reset, zoom buttons, keyboard orbit, and a responsive 390 px layout.
Desktop and narrow-layout screenshots are next to each browser report. The desktop
starter-room screenshot was also visually inspected.

The compiler and Ninja were taken from the installed native toolchain. Browser
builds reused the existing SDK at `WebEngine/.tools/emsdk`. The full builds used the
existing Jolt 5.3.0 source cache to avoid duplicating downloads; the independent
fresh configuration subsequently verified the normal FetchContent path.
On this machine the matching native compiler runtime directory must precede other
GCC runtime directories on PATH while running the native test. Using the compiler
shell or Visual Studio's standard toolchain setup avoids mixing runtimes.

## Remaining acceptance

The CI workflow was authored but has not run on GitHub. Windows Chrome/headless
WebGPU and narrow desktop layout do not establish physical Android/iPhone, Safari,
Firefox, touch-device, accessibility or sustained performance acceptance. No
gameplay rules, authored assets, progress saves or audio have been implemented.

The consumer-side renderer packaging is a temporary bridge. Move the reusable
browser target/options and consumer-safe paths into WebEngine in a separate tested
engine change, then advance the game gitlink after publishing that commit. The
current setup is buildable using the existing published pin and needs no
unpublished engine commit.

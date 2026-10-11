---
name: engine-skill
description: Build and extend playable browser games using Johan's WebEngine C++20/WebAssembly engine. Use for game mechanics, levels, input, cameras, interactions, physics, assets, HUD, audio, saves and game integration in WebEngine, or engine extensions needed by those games. Applies to this repository's engine, not unrelated web game frameworks.
---

# Engine Skill

Turn game requests into playable WebEngine applications that compose existing systems. Keep game rules in application code; extract reusable mechanisms when a concrete mechanic needs them.

## Establish current contracts

For Paws, resolve WebEngine through the pinned `external/WebEngine` submodule by default, or an explicitly configured local checkout in ignored `developer-settings.json` / `CMakeUserPresets.json`. Follow the game repository's `AGENTS.md`, `README.md` and `docs/architecture.md` for dependency selection, consumer builds and runtime ownership. Engine contract paths below are relative to that selected engine checkout; game builds and acceptance run from Paws using its own scripts and generated site.

This repository-local copy was refreshed on 2026-10-11 from the installed `engine-skill` maintained by WebEngine at `C:/Users/Johan/.codex/skills/engine-skill`. On other machines, resolve that installed skill through the skill catalog. When refreshing, compare the complete skill folder, including supporting references, and retain this Paws dependency guidance. The source skill covers uncommitted development features beyond the currently pinned `3fe7d1dc1dc610c703944803cbb717cd65024ec9`; verify availability in the selected checkout before using an API. Refreshing skill documentation does not advance the engine gitlink.

Read applicable `AGENTS.md`, then `architecture.md` before planning or changing engine APIs, platform/build boundaries or game/demo behaviour. Inspect `git status --short` and preserve unrelated work. Read relevant README sections and feature documents, then verify exact signatures and implementations in `include/engine/`, `src/` and `web/`. Select documents through [the contract map](references/contract-map.md).

Current source and contracts take precedence over this guide's API landmarks. Roadmaps/readiness reviews can lag implementation: verify claims before scheduling an existing system for reimplementation. Do not treat intended extensions as delivered features or expand a game request to implement unrelated services.

The supporting references were checked against the working checkout on 2026-10-11, including its uncommitted features. Recheck current contracts when using them. Read [feature integration details](references/feature-integration.md) when composing rendering backends, mobile loading, movement, precise interaction, animation or saved behaviour.

## Implement a coherent game

Infer mechanics, controls and target devices from the request. Resolve routine choices independently; ask only when missing information materially affects the result. For a new game, deliver a small playable loop with clear controls, feedback and appropriate completion/failure/restart behaviour. For an existing game, extend the requested behaviour within its established loop.

Keep levels, goals, scoring, progression, inventory, hints, move legality and undo policy in application/example code. Inspect `src/main.cpp` and CMake targets before choosing integration. Existing demo placement has documented exceptions; avoid copying those into new generic modules or reorganizing unrelated demos. Prefer a clear game-specific source boundary reusing the engine and browser host.

Compose only the systems the game needs:

- Own scenes through `Application`; assemble actors/components through `Scene::spawn<T>()`, `Actor::addComponent<T>()` and the owned hierarchy. Use `Prefab` for repeated configured hierarchies when useful.
- Map physical input to portable actions/contexts, then gameplay commands. Route GUI, camera and scene contacts explicitly so one gesture has one owner. Cancel capture/held commands on blur, context changes, transitions and target loss; keep DOM/key/touch details in adapters.
- Use character motors for command-driven movement, physics queries/sensors for collision/triggers, and selection/actions for interaction. Choose transform manipulation for eligible non-physics actors or joint/grab commands for physics actors. Avoid competing transform writers; collision layers are distinct from render masks.
- Give each camera one writer; evaluate camera controllers after simulation and before rendering. Presentation poses, GUI focus and view selection must not determine authoritative gameplay state.
- Share immutable mesh/material/texture/skeleton/clip assets; keep transforms, overrides and playback state per instance. Gate transitions on asset preparation and retain the active scene on failure. Verify current decode/upload capabilities before assuming loading is stall-free or GPU-prewarmed.
- Use the browser `Renderer` facade and reported `RenderCapabilities`. Automatic initialization prefers WebGPU and can fall back to WebGL 2 before canvas acquisition; the fallback has reduced lighting/features. Service recovery at host frame boundaries even while the renderer is not ready. Keep backend resource types private.
- For larger content, use the optional isolated `browserDecodedAssetLoader`, per-frame `UploadBudget` and offline KTX2/fallback texture tooling where appropriate. These mechanisms are implemented; CPU asset readiness does not mean every draw is GPU-ready. Loading-screen and eviction policies remain application choices.
- Compose retained GUI, audio and persistence as the loop requires. Preserve browser audio activation/resume handling. Save game data with stable saved identities and custom codecs; reconstruct executable behaviour after loading.

## Preserve runtime invariants

`Application::advance()` owns input snapshots, variable scene updates, fixed physics steps and presentation clocks. Do not run scene/animation updates again inside its callbacks. RAF timestamps enter as seconds. Use simulation time for gameplay, fixed ticks for fixed-step logic and an explicit input/presentation time policy for menus, cameras and audio. Pause and host suspension differ: preserve explicit pause through page visibility changes and avoid hidden-time catch-up. Standalone schedulers must deliberately follow the scene update contract.

Use `Scene::synchronize()` for lifecycle/registration reconciliation without ticks; `Scene::update(0)` still performs a variable tick. `fixedUpdate()` does not advance animation clocks. Opt game actor/component tick hooks in through their supported flags. Preserve bounded fixed-step catch-up and the application transition/preparation boundary before input snapshots.

The scene hierarchy owns nodes uniquely. Query pointers/references are borrowed. Delayed work should retain scene-scoped, generation-checked `ActorId`/`ComponentId` handles and resolve them before access. Destruction excludes targets immediately and defers removal until guarded traversal ends. Detach/reparent only at allowed idle boundaries; preserve cache invalidation and handle `KeepWorld` failure. Protected skeletal implementation children cannot be removed independently.

Declare lifetimes for renderer/camera/scene references, controllers, subscriptions and async interests. Release borrowed presentation references before scene destruction; dispose browser adapters before portable owners. Reject stale completions targeting replacement scenes. Validate candidates before publication so malformed data or failed construction preserves valid state.

Simulation positions/collision/game rules remain independent of render interpolation, culling and LOD. Consume animation markers/root motion explicitly; gameplay owns movement/collision policy. Physics saves are not exact solver rewind. Built-in animation controller definitions/state, markers and pending output are archived. Runtime handles, executable callbacks, input bindings, GUI trees, audio voices, camera behaviours and physics joints are not automatically restored: verify codecs and rebuild runtime behaviour intentionally. Reject unknown custom component types rather than silently degrading them.

Keep DOM, Emscripten, WebGPU and solver implementation types out of new portable interfaces. Update `architecture.md` in the same change if responsibilities, ownership, frame execution, dependencies, platform boundaries or extension status change. Keep affected feature contracts consistent with delivered code.

## Validate and hand off

Follow [build and validation guidance](references/build-validation.md). Use focused native checks for portable logic and actual browser play/smoke checks for controls, adapters and rendering. Exercise the requested loop and affected restart, transition and cancellation paths. Documentation-only changes need content/link checks rather than runtime builds.

Report delivered behaviour, launch instructions, checks actually run and material limits. For substantial phased work, leave useful evidence and a repository-local continuation note. Previous evidence is not a new run; core-only native builds are not desktop games; mobile emulation is not physical-device acceptance; fewer uploads/draws do not establish FPS improvement. Deployment and external actions stay within the user's authorized scope.

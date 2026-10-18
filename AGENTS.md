# Paws repository instructions

Paws is a browser 3D puzzle game built in C++20 on WebEngine. Read `README.md` and
`docs/architecture.md` before changing build boundaries, runtime ownership or game
structure. The initial scene is a greybox integration fixture; the proposed
gameplay mechanics have not been implemented yet.

Read [paws-concept.md](paws-concept.md) and the root
[architecture.md](architecture.md) for the proposed gameplay direction before
implementing puzzles, quests or progression. Read [design.md](design.md) before
creating or changing visuals, 3D assets, materials, lighting or UI. Follow its art
direction and reference board. Distinguish these proposals from the delivered
integration foundation described in [docs/architecture.md](docs/architecture.md).

## Required skills

- Always use the `tripo` skill when generating new 3D models. Its installed
  entry point is [Tripo](C:/Users/Johan/.codex/skills/tripo/SKILL.md); on other
  machines, resolve the installed `tripo` skill from the available skill catalog.
  Follow its ImageGen-reference-first workflow, then use `pawmodels` for asset
  review, normalization, export and runtime integration.
- Use the `pawmodels` skill when creating or revising 3D cats, garden props,
  furnishings or puzzle models. Its installed entry point is
  [PawModels](C:/Users/Johan/.codex/skills/pawmodels/SKILL.md); on other machines,
  resolve the installed `pawmodels` skill from the available skill catalog.
  Follow `design.md` alongside its modeling, export and runtime verification
  workflow.
- Use the repository-local [engine-skill](.agents/skills/engine-skill/SKILL.md)
  when implementing gameplay or integrating or extending WebEngine. This copy
  includes its supporting references. Resolve the engine through this repo's
  pinned `external/WebEngine` dependency or an explicitly configured local
  checkout, following the dependency rules below.

## Implementation rules

- Keep cat behavior, puzzle rules, levels, UI and save progression in this repo.
  Reusable scene, rendering, physics, input and resource mechanisms belong in
  WebEngine. Reuse its APIs and read its `AGENTS.md`, architecture and feature
  contracts before changing engine code.
- The default dependency is the exact Git submodule revision at
  `external/WebEngine`. Shared builds and CI must use it. Local engine paths belong
  in ignored `developer-settings.json` or `CMakeUserPresets.json`, never committed
  machine paths. Keep pinned and local build directories separate.
- Preserve unrelated changes in both repos. Prefer developing engine changes in
  the adjacent engine checkout, then commit/push them there and deliberately
  update the game gitlink. Verify the game against the pin before advancing it.
  Never hide an engine change in an uncommitted submodule working tree.
- `cmake/WebEngine.cmake` is a temporary consumer packaging bridge for the current
  engine. It must contain build integration only. Do not fork engine sources,
  duplicate its core source list, or link the playground/main into Paws. Replace
  the renderer fallback when upstream publishes a consumer browser target.
- Keep browser callbacks in the host/runtime adapter. Use `Application` clocks
  and lifecycle, generation-checked handles for delayed scene references, and
  release borrowed renderer/controller references before scene destruction.
- Imported assets need provenance and license records. Do not reuse engine demo
  assets as accidental game dependencies.
- Run the portable scene test for changes to scene/lifecycle contracts and build
  the browser target for browser changes. Browser acceptance must exercise the
  actual generated game site. Record scoped evidence; native tests and emulation
  do not establish physical mobile performance or compatibility.
- Do not spawn additional agents unless explicitly requested by the user.

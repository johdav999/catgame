# WebEngine contract map

Paths are relative to the selected repository, not the skill. Read only relevant rows, then current headers/implementation. Use `rg --files` and `rg -n` to locate moved files, exact signatures and consumers. This is navigation, not a frozen feature inventory. Header-only names below refer to `include/engine/`.

| Task | Contracts | Source landmarks |
| --- | --- | --- |
| Runtime, levels, pause and transitions | `docs/application-lifecycle.md` | `application.hpp`, `src/application.cpp`, `src/main.cpp` |
| Hierarchy, handles, lifecycle and transforms | README sections “Reusable scene objects”, “Actor identity, components and lifecycle”, “Component lifecycle, updates and settings” | `scene.hpp`, `node.hpp`, `actor.hpp`, `component.hpp` |
| Repeated objects/factories | `docs/prefabs.md` | `prefab.hpp`, `prefab_components.hpp` |
| Physical input, bindings, context stacks, virtual controls and contact ownership | README “Mouse, keyboard and mobile input”, `docs/input-validation.md`, `docs/ui-input-integration.md` | `input.hpp`, `web/input.js` |
| Movement, jumping and grounding | `docs/movement.md`, `docs/physics.md` | `character_motor_component.hpp`, `rigid_body_component.hpp`, `collider_component.hpp` |
| Collision, queries and triggers | `docs/physics.md` | `physics.hpp`, `src/physics.cpp` |
| Hinges, sliders, springs and physics grabs | `docs/physics-joints.md` | `physics_joint.hpp`, `physics.hpp` |
| Picking, actions, dragging and rotation | `docs/selection.md` | `interaction_component.hpp`, `selection.hpp`, `manipulation.hpp` |
| Follow/orbit/framing and camera collision | `docs/camera-controllers.md`, README camera contracts | `camera_component.hpp`, `camera_controller.hpp`, `orbit_controller.hpp` |
| Loading, cancellation, cache and preparation | `docs/assets.md` | `assets.hpp`, `asset_browser.hpp`, `fbx_importer.hpp`, `glb_importer.hpp` |
| Worker decode, GPU upload admission and KTX2/fallback delivery | `docs/mobile-assets.md`, `docs/assets.md` | `asset_decode.hpp`, `asset_browser.hpp`, `upload_budget.hpp`, `texture_bundle.hpp`, `src/asset_worker_browser.cpp`, `web/asset-worker.js`, `tools/texture_tool.cpp` |
| FBX/GLB imports and mesh optimization | README “FBX mesh importer”, “GLB mesh importer”, “Imported mesh optimization” | `fbx_importer.hpp`, `glb_importer.hpp`, `mesh_optimizer.hpp` |
| PBR maps, material overrides, IBL and baked graphs | README “Materials and image-based lighting”, `docs/material-graph.md`, `docs/materials-validation.md` | `material.hpp`, `material_graph.hpp`, `environment.hpp`, `texture.hpp`, `mesh_component.hpp` |
| Directional shadows and lighting | README “Directional shadow maps”, `docs/shadows-validation.md` | `light_component.hpp`, `shadow.hpp`, `render_layer.hpp` |
| Particles, flipbooks and effect presets | README “Particles, flipbooks and reusable effects” | `effect_component.hpp`, `effect_presets.hpp`, `particle_emitter_component.hpp`, `fx_frame.hpp` |
| Animation, transitions, markers and root motion | `docs/animation-control.md`, README skeletal animation section | `skeletal_mesh_component.hpp`, `animation_component.hpp`, `animation_controller.hpp` |
| Morph targets and instance weights | README “Morph targets” | `mesh.hpp`, `mesh_component.hpp`, `tests/morph_tests.cpp`, `scripts/morph-smoke.cjs` |
| HUD, menus, text and accessibility | `docs/gui.md`, `docs/ui-input-integration.md` | `gui.hpp`, `gui_browser.hpp`, `web/gui.js` |
| Music, spatial audio and activation | `docs/audio.md` | `audio.hpp`, `audio_browser.hpp`, `src/audio_demo.cpp`, `web/audio-system.js` |
| Saves, migrations and custom codecs | `docs/persistence.md`, `docs/scene-archives.md` | `persistence.hpp`, `scene_archive.hpp`, `web/storage.js` |
| Ordered camera views, viewport coordinates, visibility masks and layers | README “Reusable scene objects”, “Layered renderer”, `docs/rendering-fallback.md` | `camera_component.hpp`, `render_view.hpp`, `render_layer.hpp`, `scripts/compositing-smoke.cjs` |
| WebGPU/WebGL 2 selection, reduced capabilities and context recovery | `docs/rendering-fallback.md`, `docs/rendering-quality.md` | `renderer.hpp`, `render_quality.hpp`, `src/renderer.cpp`, `src/webgpu_renderer.hpp`, `src/webgl_renderer.hpp` |
| Device capabilities, quality budgets and page lifecycle | `docs/rendering-quality.md` | `render_quality.hpp`, `web/quality.js`, `web/app.js` |
| Culling, instancing, LOD and performance | README performance sections, `docs/mesh-detail-validation.md` | `render_frame.hpp`, `mesh_detail.hpp`, `performance.hpp`, `scripts/performance-smoke.cjs` |

Existing `tests/` and `scripts/*-smoke.cjs` supply usage/regression examples. Inspect setup and assertions rather than copying old suite counts or thresholds. `docs/puzzle-game-readiness.md` can help scope puzzle integration, but recheck its capability claims against source and do not impose puzzle rules on other games.

When evidence matters, follow companion validation documents such as `docs/application-lifecycle-validation.md`, `docs/assets-validation.md`, `docs/prefabs-validation.md`, `docs/selection-validation.md`, `docs/gui-validation.md`, `docs/audio-validation.md`, `docs/scene-archives-validation.md` and `docs/persistence-validation.md`. Their recorded results describe past scoped runs. They are not current validation of a new game or change. Ship stress/instancing investigations are examples and measurements, not game-specific engine policy.

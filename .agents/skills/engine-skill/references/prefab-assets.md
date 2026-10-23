# Prefab assets and variants

Verify the selected engine contains prefab_asset.hpp/editor_prefab.hpp and read its
docs/prefab-editor.md, prefab-editor-validation.md and scene-archives.md. Development
documentation does not advance the consuming game pin.

engine_core PrefabAsset owns an immutable detached built-in actor definition,
identity/name/positive uint32 revision and optional immutable base. capture(actor,
name, optionalId, revision=1, optionalBase), create(detachedActor,...), definition(),
id/name/revision/base, and instantiate(optionalOldBindings) compose existing node
and shared immutable asset representations. Instantiation returns detached ownership
with an inert direct-actor PrefabInstanceComponent retaining the asset and stable
definition-key -> instance saved-ID bindings. No global registry, callbacks or IO.
PrefabAssetArchive::save/load uses WEPREFAB1 around a checksummed scene carrier.

The existing Prefab/std::any ComponentFactoryRegistry API remains the C++ factory
path. PrefabAsset supports existing built-in archive types; it does not serialize
arbitrary factories, callbacks or custom game subclasses. Nested linked instances
must be unpacked before capture. No automatic runtime propagation or file watcher.

engine_editor PrefabAssetEditor uses a private Scene and EditorSession (32 history
entries); session() exposes ordinary typed property/transform/component authoring,
snapshot(name) returns the next immutable revision. mergePrefab(previous,local,
incoming,registry) and rebasePrefab(variant,newBase,registry) stage three-way merges.
EditorSession supplies linkPrefab, placePrefab, selectedPrefabRoot, updatePrefabs
(asset span, optional revertSelected), and unpackPrefab. Scene publication is one
grouped undo command with stale state/parent guards; saved IDs survive while
runtime handles are recreated. Re-resolve borrowed nodes after updates.

Local inspector properties, separate TRS axes, tags, mesh/texture bindings and
per-slot materials win over incoming values. Root placement always stays instance
owned. Ordinary actor child additions/deletions/reparenting/order are merged.
Upstream removal of locally edited nodes, type/key/parent conflicts, required child
changes and unexposed configuration overrides reject before publication. Component
child structure overrides require unpacking. Camera-containing updates reject to
protect borrowed RenderView lifetimes; unpack removes only the link in place.
Play gates authoring and uses an isolated clone.

Scene archive v34 adds the inert instance kind/shared asset resources; v1-33 remain
readable. Template node IDs have separate scopes while immutable resources share
the existing resource table across templates/bases/live instances. Duplication
remaps instance bindings; scenes retain applied revisions and variant bases.
Unpublished workbench documents and history are separate. Source links and derived
variants are explicit metadata, not global mutable assets.

Browser: Details -> Prefab assets -> Create from selected actor / Open prefab editor.
Edit definition nodes, save/open .weprefab, Create variant, Place instance, then
Update linked instances & variants. Known descendants rebase before grouped scene
publication. Select a live instance to inspect differences, revert (retaining root
placement), or unpack. Close stashes edits in a session catalog; rebind clears it.
Save files for unplaced definitions. Async reads check binding generation and
workbench/editor lifetime. Invalid input preserves prior valid state.

Bounds: 512 definition nodes, depth64, 512 bindings, eight base ancestors, cycle/ID
validation; names 1-256 bytes without newline/NUL; browser catalog64 assets. Files
<=64 MiB, archive allocations <=128 MiB and normal resource limits. Core cloning
uses existing idle/hierarchy and lifecycle contracts; creating visuals does not
initialize physics. Authoring updates containing unknown/runtime configuration
differences may require reset/unpack rather than destructive replacement.

Demo ?scene=prefabs&editor=1 uses linked lanterns; policy stays in prefab_example.hpp.
Native prefab_asset_tests/CTest prefab_asset plus prefab/editor/editor_content/
editor_structure/scene_archive/persistence/application. Build prefab_asset_wasm_tests
with the repository-local SDK/version, run its JS in Node, then web_assets and
scripts/prefab-editor-smoke.cjs serially on WebGPU/WebGL2 at DPR1 and
DEVICE_SCALE_FACTOR=2. Verify real propagation pixels, source isolation, independent
history, overrides, variants, file failure/roundtrip, scene reload and compact actions.
Consumer games keep their own build workflow and pin selection. Desktop Chromium
does not establish physical mobile acceptance.

# Native Windows scene workbench

Build with `./scripts/build-windows.ps1`, then run
`./build/windows/Release/webengine_editor.exe [scene.wescene]`.
This is a native SDL/Dawn window using the shared C++ editor commands and retained
GUI; it does not require a browser or web server. Keep the staged assets and
SDL/DXC DLLs beside the executable. The player ZIP remains a player distribution;
the editor executable is a separate developer build target.

## Workflow

- Scene Outline selects actors/components, including disabled nodes. It shows up
  to 2048 nodes; deeper indentation is capped for readability.
- Click a mesh in the viewport to select its bounds. Drag with the left mouse to
  orbit; Frame selection and Zoom buttons navigate the authoring camera.
- Details edits local position, Euler rotation in degrees and scale as one atomic
  transform command. Enter or Apply commits all nine fields. Invalid values keep
  the previous scene. Registered component properties use their existing types,
  ranges and validation; choice fields show numbered choices and booleans accept
  `true`/`false`. Finish IME composition before committing.
- Cube/Sphere/Plane create an object through EditorSession; Duplicate, Add light/
  camera/collider/body and Remove component reuse existing history commands.
  Remove component deliberately does not delete actors. Undo/Redo retain up to
  128 commands. Structural/physics restrictions are the portable editor's rules.
- Import GLB loads a bounded 32 MiB embedded mesh file and places it through the
  existing editor command. Companion-image IO and a new import worker are not
  implied by this workbench action.
- New/Open and file drop validate a complete candidate. Dirty documents retain
  their source/history until Save, explicit Discard or Cancel. Saving writes an
  atomic `.wescene` archive; current assets and opaque application metadata remain
  in it. The browser editor reads/writes the same archive. The workspace viewport
  is temporary: serialization restores the authored viewport, then returns to
  the native layout. Camera navigation changes the saved camera pose.
- Play clones the document using SceneArchive and the configured component codecs,
  activates it in the host's existing Application, and disables authoring. Stop
  rebinds the retained source before unloading the runtime. There is one scheduler;
  Play never advances the source. Runtime motion cannot become an authoring edit.
- Window close protects dirty documents. Explicit `DesktopHost::requestClose()` is
  a programmatic force-close; pending editor transactions are cancelled before
  host shutdown releases scenes. F11 toggles fullscreen outside text input.

Use a desktop workspace of at least 1100 x 700 logical units. Outline and Details
scroll vertically. Native authoring is not the complete browser interface:
the [shared authoring frontend](editor-authoring.md) adds terrain stamps and
AI/animation/prefab/FX/cinematic forms and overviews. Advanced browser graph/curve/
timeline dragging, material graphs, viewport terrain strokes, docking, transform
gizmos, asset library browsing, autosave and arbitrary actor deletion remain extensions. Existing authored assets/components survive compatible
scene archives and supported properties remain editable. These are explicit
remaining parity gaps, not new implementations of the browser panels.

## Embedding and ownership

Link optional `WebEngine::DesktopEditor` (`engine_editor_desktop`) and include
`engine/editor_desktop.hpp`. Construct `DesktopEditor(host, document, codecs)` with
an unused DesktopHost, then call `editor.step(timestamp)` or `editor.run()` instead
of advancing the host separately. Default codecs include built-in AI controllers;
applications can supply their own existing ComponentCodecs. Custom callbacks and
registries must outlive their use. Failure to clone unsupported components retains
the source; no fallback silently drops application behaviour.

The host owns the source and Play scenes; the workbench owns their EditorSession,
assets, metadata, camera controller and bounded (32-command) UI queue. Camera views
are scene-scoped handles; raw RenderViews exist only during synchronous archive
operations. Removed authoring cameras are replaced with a new editor camera at
an idle boundary. GUI callbacks enqueue work; commands publish outside frame/GUI
traversal. Destroy the workbench before the host. It cancels input, clears handlers
and widget callbacks, and hides its retained GUI subtree, which is released with
the host. Do not shut down, replace or independently advance its Application while
the workbench is alive; requestClose is the supported programmatic exit.

See [parity validation](windows-parity-validation.md), [desktop host](windows-desktop.md)
and selected-engine `docs/scene-editor.md`.


## Multiplayer test launcher (M1)

With WEBENGINE_BUILD_MULTIPLAYER_DEMO=ON, the endpoint field and Multiplayer test
action launch the isolated arena in a separate native process. DesktopEditor owns
only an optional setMultiplayerLauncher callback; openMultiplayerTest runs at idle
and rejects Play/pending-document transitions. Keep callback captures alive and
clear them before releasing host state. Source documents/history are retained.
The browser editor launches the same arena in a separate tab. No arbitrary-scene
replication or collaborative editing is implied. See [multiplayer](multiplayer.md).

## Authored multiplayer Host/Join (M2)

Host selected actor and Join authored scene use the optional synchronous
setMultiplayerSceneLauncher callback. It borrows source, views, assets and selected
saved ID (empty for Join) at idle; captures must outlive the callback. The sample
serializes a bounded static scene and launches multiplayer_scene_windows with a
temporary package consumed by that client. Source bytes/history/selection remain
in the editor; Play, pending transitions and active gestures reject publication.
Browser uses the same validator and a same-origin expiring handoff to a separate
tab. General component replication and collaborative editing remain future work.
See [authored scene contracts](multiplayer-scenes.md) and
[validation](multiplayer-scenes-validation.md).


## Multiplayer prediction (M3)

Authored-scene Host/Join now launches MP03 clients with automatic local prediction and server reconciliation on Dawn and direct D3D12. Authoring documents/history remain retained. See [M3 contracts](multiplayer-prediction.md) and [evidence](multiplayer-prediction-validation.md).


## Multiplayer presentation (M4)

Authored-scene Host/Join automatically enables remote interpolation and local positional correction smoothing on Dawn and direct D3D12. Buffers reset on session changes and source bytes/history stay isolated. See [contracts](multiplayer-presentation.md) and [evidence](multiplayer-presentation-validation.md).


## Multiplayer replication (M5)

Authored-scene Host/Join now uses MP05 per-peer deltas, periodic full packets, baseline recovery and distance interest on Dawn/direct D3D12. Rebuild server/clients together; source bytes/history and saved packages stay compatible. See [contracts](multiplayer-replication.md) and [evidence](multiplayer-replication-validation.md).


## Multiplayer authoring (M6)

Add replication settings uses the shared registry/history for root mesh actors. Host/Join launches MP06 clients with server-owned actor replication and read-only Diagnostics. Preserve source documents and borrowed scene/view lifetimes. See [contracts](multiplayer-authoring.md) and [evidence](multiplayer-authoring-validation.md). Native authoring tests use EditorSession APIs plus file reopen, not field pointer/keyboard input.


## Gameplay-component replication (M7)

DesktopEditor retains optional property registries and registered factories alongside codecs. The shared EditorSession support preserves custom creation/history/duplication/reopen/Play. Host/Join applies authoritative fields on ordinary components and displays Diagnostics. Native creation/property acceptance is API-driven, not native field pointer/keyboard testing. See [contracts](multiplayer-components.md) and [evidence](multiplayer-components-validation.md).

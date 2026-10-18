# Cat quest interaction

Click or touch a garden cat with an available request to enter its puzzle directly. If a cat has several requests, start its first unlocked unfinished request in authored order. Completed requests remain available for practice through Requests & friends.

Newly unlocked Mabel and Button (chapter two), then Pepper (chapter three), walk
in one at a time through the open right-hand garden gate. Each requester carries
the existing exclamation mark during entry. No Meet/Welcome panel is shown.
The host records the existing introduction intent when a cat reaches home, or
when the player clicks its request during entry, and saves through the ordinary
coordinator. Saved residents load at home; an interrupted unsaved arrival starts
again at the gate. Pause/suspension preserve the route. Reduced motion places
new residents directly at home and records the same introduction.

An independently rotating Tripo-generated honey exclamation mark floats above each requester. It follows the cat and disappears when no unlocked unfinished request remains. Reduced motion stops rotation. Movement beyond 12 CSS pixels, pointer cancellation, blur and release off the cat prevent activation. Cat and marker handles remain scoped to the owned scene; availability comes from the existing authoritative progression projection.

Runtime asset: `assets/runtime/models/garden/quest_marker.glb`, 3,000 triangles, one material, 84,348 bytes, approximately 0.55 metres tall. Native provider output, editable Blender master, reference prompt, normalization script, provenance and inspection history are linked from `artifacts/model-jobs/quest-marker/2026-10-12-tripo/README.md`.

Arrival verification, 15 October 2026: the pinned Release site at
`build/web-pinned-release/site` passed `scripts/cat-arrival-smoke.cjs` in isolated
Chrome storage. Mabel and Button enter in sequence, pause preserves travel,
automatic introductions survive reload, all four chapter-two quests complete
with mouse input, and a portrait touch on entering Pepper opens her request.
Reduced motion places and saves residents directly in a short landscape view.
The served Wasm SHA matches the generated file; no console or page errors.
Evidence and build identity: `build/evidence/cat-arrivals/report.json` and six
screenshots. Native `paws_starter_scene`, `paws_release1_gui` and
`paws_release1_garden` passed 3/3; arrival coverage includes a centred swept gate
crossing and saved residents without repeated entry. Browser sizes were
1600x1000, 390x844 and 844x390; touch was emulated, not a physical-device run.

Verified on 12 October 2026 against pinned WebEngine `3fe7d1dc1dc610c703944803cbb717cd65024ec9`:

- Browser build: `build/web-quest-cats-pinned-release/site`.
- Native starter scene, release-one GUI, final garden and garden tests: 4/4 passed.
- `scripts/cat-quest-smoke.cjs`: all ten requests start through actual cat clicks/taps, complete through retained GUI actions and return to the garden. All markers disappear after completion.
- Desktop 1600x1000, portrait 390x844 and landscape 844x390, including mouse and emulated touch, cancelled/dragged taps, reduced motion and exact served GLB checksum. No page errors.
- Evidence: `build/evidence/cat-quests/report.json`, screenshots and `asset-audit.json`.

Run the browser check with `PAWS_URL` pointing to an isolated generated-game server. Physical-device performance and provider release rights are not established by these checks. Existing audio release gaps are unchanged.

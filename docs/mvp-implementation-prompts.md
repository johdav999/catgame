# Paws MVP implementation prompts

Implement **Pip's first afternoon** in ten ordered phases. The finished MVP is a relaxed browser game with one garden, Pip, three handcrafted puzzles, coin and friendship rewards, two purchasable furnishings, and local progress restoration. Aim for a first playthrough of roughly 15–20 minutes; validate that duration through playtests.

Run one numbered prompt at a time from the CatGame repository. Each prompt tells a new implementation chat to read the shared instructions and production briefs in this file. Update `docs/implementation-status.md` after every phase so the next chat can continue from repository evidence.

The game loop is: meet Pip, accept a request, solve a puzzle, return to the garden, purchase and place a furnishing, and see Pip use it. Completing all three quests and placing at least one purchased furnishing unlocks a short closing moment. The garden remains playable afterward.

## Shared instructions for every phase

- Read `AGENTS.md`, `README.md`, `docs/architecture.md`, `paws-concept.md`, `architecture.md`, `design.md`, this file, and the latest implementation status. Inspect both game and engine Git status and preserve unrelated staged and unstaged work. The current documented foundation is a greybox scene; verify what later phases have actually delivered before changing it.
- Use the repository-local `.agents/skills/engine-skill/SKILL.md` for gameplay, GUI, audio, runtime and engine integration. Use the installed `pawmodels` skill for 3D cats, props and furnishings, and its built-in ImageGen/headless Blender workflow. Read the imagegen skill when generating image inputs. Inspect `docs/design/paws-style-reference.png` when working on appearance. Use the current engine contracts and source instead of assuming proposed systems already exist.
- Keep puzzle logic, quests, economy, cat behaviour, GUI composition and content in Paws. Keep reusable rendering, resource, input and platform mechanisms in WebEngine. Reuse engine facilities, including interaction, GUI, assets, animation, audio and persistence. Add only a concrete missing capability needed by this MVP.
- Shared builds use `external/WebEngine` at its recorded Git revision. Local overrides belong in ignored developer settings and separate build directories. Preserve the packaging bridge until a supported upstream browser target is available. Engine changes follow the repository's separate engine commit/push and deliberate gitlink update workflow; preserve existing work and record validation against the final pin.
- Use `Application` clocks and lifecycle, one authoritative puzzle model, stable saved IDs, and generation-checked scene handles. Browser callbacks belong in the host adapter. Reject stale loading, animation, input and save completions after restart, restore or scene replacement. Release borrowed renderer/controller references before destroying scenes.
- Implement mouse, touch and keyboard-equivalent commands. Use tap/select plus explicit actions as the baseline. Route each gesture to one owner. No puzzle requires hover, exact dragging, sound, a timer or rapid reactions. Include undo, progressive free hints, restart, pause, back to garden, and reduced motion.
- Keep source masters, raw generations and rejected iterations under `artifacts/` outside the deployed `assets/` tree. Stage only accepted runtime assets and their license notices. Record stable asset IDs, hashes, provider/source, prompts, modifications and rights records. Avoid machine-specific paths in game content.
- Run meaningful portable rule tests for changed rules, the portable scene test for changed lifecycle contracts, and a pinned browser build for browser changes. Exercise the actual generated Paws site. Record build/content IDs, browser/device/viewport, screenshots, console errors, and checks actually performed. Physical-device evidence is required for physical-device claims.
- Finish the phase's implementation and corrections, then update `docs/implementation-status.md` with delivered behaviour, changed files, launch commands, evidence paths, unresolved blockers and the next phase. Report missing tools or external assets precisely and continue independent work. Mark incomplete assets or acceptance honestly. Do not spawn additional agents.

## MVP content and economy

| Content | Fixed MVP choice |
| --- | --- |
| Character | Pip, a ginger tabby with cream muzzle/chest and upright curved tail |
| Garden | One small courtyard with sandstone paving, a bench, planting, a low boundary and three furnishing spots |
| Quest order | `pip_missing_ball`, `pip_tidy_toys`, `pip_stuck_gate` |
| Quest reward | Each quest grants 20 coins and 1 friendship, once per save |
| Shop | Opens after the first quest; cushion costs 30 coins and bowl costs 25 |
| Friendship | Show three earned friendship steps, with one affectionate reaction at the final step |
| Purchases | One cushion and one bowl per save; both remain affordable from the total 60 quest coins |
| Placement | Three authored spots with compatible footprints; furnishings can be moved afterward |
| Ending | All three quests completed and at least one purchased furnishing placed; show once, with optional replay from the menu |
| Save | Current attempt, completion/reward records, wallet, friendship, purchases, placements and settings |

Keep the content to these three quests and two shop items. Additional cats, quest catalogues, freeform construction, accounts, multiplayer, cloud saves, daily tasks and offline installation can follow the MVP.

## Prompt 1 Foundation and a playable puzzle prototype

```text
Implement phase 1 of Pip's first afternoon. Read docs/mvp-implementation-prompts.md and follow its shared instructions, MVP scope and handoff policy. Use the repository-local engine-skill.

Inspect the delivered game and pinned engine APIs. Evolve the existing targets into portable rules/content, application ownership and presentation boundaries only as needed; preserve the working consumer bridge. Establish PawsSession, GameState, stable quest/object/attempt IDs, typed commands, revisions and scene bindings. Puzzle rules must be portable C++ with no renderer or browser dependency.

Implement a playable greybox version of Pip's missing ball: two sliding pots, straight and curved channel segments, a source ball and a destination. Author a small nontrivial layout with a known solution. Sliding uses authored slots and occupancy; rotation uses discrete orientations; an explicit Roll action evaluates a finite route. A failed roll reports the blockage and leaves the ball available at its source. A successful roll reaches the destination. Simulation physics must not determine correctness.

Support selection, legal-action feedback, undo, restart and completion using temporary functional controls. Add tests for illegal moves, occupancy, route termination/cycles, known solution replay, undo and stale attempt commands. Define a versioned content schema sufficient for this slice and known-solution validation; do not create unused objective types.

Establish runtime-only asset staging, relative asset URLs and separate source-master storage. Show readable WebGPU/startup failures and retain working page suspension/recovery behaviour.

Acceptance: build against the pin, solve the greybox puzzle in the generated browser game, restart it, verify blocked moves and undo, and capture evidence. Leave docs/implementation-status.md ready for phase 2.
```

## Prompt 2 Production GUI and responsive interaction

```text
Implement phase 2 of Pip's first afternoon. Read docs/mvp-implementation-prompts.md and follow its shared instructions and GUI production brief. Use the repository-local engine-skill and inspect the actual design reference image.

Generate and implement the production GUI as reusable engine-GUI components with original shipped vector/code-native icons, locally bundled licensed fonts, layout tokens and final readable copy. Use Fraunces Semibold for headings and Nunito Sans for controls/body text. Build cream panels, teal actions, visible focus, readable disabled states and clear selection. Keep exact text and controls editable in code; generated screenshots are appearance references only. Use HTML for browser startup and recovery.

Deliver title/continue, garden HUD, Pip dialogue, quest objective/action tray, expandable hints, pause/settings, restart confirmation, completion/reward card, shop/details, furnishing placement, ending and save/recovery surfaces. Wire delivered functionality now. Give later-phase widgets explicit integration contracts and exercise them in a development-only component gallery without presenting fake purchases or saves as working gameplay.

Implement keyboard focus and semantic object/action access, modal capture and focus return, touch targets of at least 44 logical pixels, portrait/landscape reflow, larger text, reduced motion, sound-independent information and safe-area handling. Selected objects expose useful names and legal actions. GUI gestures must not also orbit the camera or commit puzzle moves.

Acceptance: inspect the actual game and component gallery at 1440x900, 390x844 and 844x390, including larger text. Capture all important control states, check contrast and clipping, solve the existing puzzle using keyboard-equivalent actions, and correct observed defects. Update the handoff for phase 3.
```

## Prompt 3 Production Pip model rig portraits and animation

```text
Implement phase 3 of Pip's first afternoon. Read docs/mvp-implementation-prompts.md and follow its shared instructions, Pip art direction and 3D production brief. Use pawmodels, built-in ImageGen for suitable material inputs, headless Blender, and engine-skill for runtime integration.

Create the production Pip asset: an appealing ginger tabby with cream muzzle/chest, broad paws, substantial curved tail, readable face and softly sculpted coat masses. Establish one canonical metre-scale rig with stable bone names, rest pose, deformation limits, documented forward axis and in-place root policy. Choose supported bones/materials rather than relying on unsupported morph animation or strand fur.

Generate and integrate UV-aligned coat colour inputs; author portable roughness/normal inputs independently. Build the actual mesh, UVs, materials and rig in a retained Blender script and editable packed master. Produce actions for relaxed idle/slow blink, attentive request/head tilt, happy pounce/chirrup, short garden walk, settle into a cushion, curled rest, approach/lower head to bowl, drink and return to idle. Reuse compatible pose transitions and keep logical progress independent of clips. Reduced motion uses readable static poses.

Render Pip portraits from this same final model for dialogue and shop/ending presentation so markings remain consistent. Verify deformation and loop seams at paws, shoulders, hips, tail, ears and face.

Complete at least three inspected render/correction cycles, clean-process GLB reimport and actual pinned Paws playback checks. Deliver the packed .blend, scripts, textures, GLB/clips, portraits, manifest, provenance and evaluation records. Inspect both close-ups and ordinary puzzle distance in shadow-free and desktop lighting. Do not accept a disconnected primitive approximation as the final cat. Update the handoff for phase 4.
```

## Prompt 4 Production garden puzzle and furnishing models

```text
Implement phase 4 of Pip's first afternoon. Read docs/mvp-implementation-prompts.md and follow its shared instructions, model inventory and 3D production brief. Use pawmodels, built-in ImageGen material inputs, headless Blender and engine-skill for loading and composition.

Generate the complete production 3D kit listed in the inventory: courtyard paving/edge, low fence and grouped planting, bench, two terracotta pot variants, straight and curved wooden channels, coral ball, toy mouse, toy fish, basket with three recognizable matching recesses, blocking cushion, pressure-switch assembly, hinged gate assembly, purchasable cushion and ceramic drinking bowl with a simple water surface. Reuse compatible assets across quests. Give the blocking cushion an appearance distinct from the purchased garden cushion.

Use real geometry for channels, lips, rims, toy silhouettes, basket clearances, soft cushion volume and gate openings. Establish shared scale, footprints, floor-contact/hinge pivots, route ports, placement anchors and cat-use anchors. Preserve interfaces across visual variants. The ball socket in the toy basket must fit the same ball mesh. Export independently moving gate/switch parts separately and explicitly compose them from manifest transforms; do not assume GLB node hierarchy or Blender empties become runtime actors.

Generate accepted stone, terracotta, wood, cloth and wicker base-colour inputs and integrate them into portable PBR materials. Share material families, bound texture sizes and keep all source files outside staged runtime assets.

Complete three inspected render/correction cycles covering silhouette, materials and export/runtime comparison. Verify connected channel assemblies, gate clearance, toy/socket fit and Pip/furnishing proportions. Deliver editable masters, retained scripts, portable maps, GLBs, manifests, provenance, iteration records and measured asset budgets. Load and inspect the kit in the actual Paws renderer at wide and portrait puzzle distance. Update the handoff for phase 5.
```

## Prompt 5 Complete the first production quest and garden loop

```text
Implement phase 5 of Pip's first afternoon. Read docs/mvp-implementation-prompts.md and follow its shared instructions and fixed MVP content/economy. Use engine-skill and the accepted phase 3/4 assets.

Compose the production garden with Pip, restrained planting, sandstone paving, bench, low boundary and three authored furnishing spots. Replace the quest-1 greybox presentation with the production asset kit. Author camera framing for desktop, portrait and short landscape. Keep interactive targets visible and decorations outside logical puzzle footprints.

Deliver the complete flow: Play/Continue, garden, select Pip, short dialogue, prepare/load puzzle, select/move/rotate/Roll, undo/hints, solve, Pip's happy reaction, actual reward card and return to garden. Include three progressive hints and a known solution. Invalid actions explain their reason. Back to garden suspends the attempt; restart creates a fresh attempt. Rapid actions, reduced motion and transitions must project the latest committed state correctly.

Implement the first working local state codec and IndexedDB save path for attempt state, first-quest completion, the reward ledger, coins, friendship and settings. Grant 20 coins and 1 friendship in the same model transaction as first completion, and persist that coherent snapshot. Show Saved only after storage completion. Replaying a completed quest is practice and grants no extra rewards.

Prepare scenes before publication, retain the active scene on asset failure and invalidate stale callbacks. Provide readable loading/failure/retry GUI.

Acceptance: a fresh player can complete quest 1, return to the production garden, reload with the same progress and wallet, and replay without duplicate rewards. Exercise an interrupted attempt and an asset-load failure. Capture the real game at all three layouts and update the handoff for phase 6.
```

## Prompt 6 Implement the other two reusable puzzles

```text
Implement phase 6 of Pip's first afternoon. Read docs/mvp-implementation-prompts.md and follow its shared instructions, quest order and economy. Use engine-skill and the existing production asset kit.

Add pip_tidy_toys, based on concept quest 6: move a blocking cushion to expose access, then collect/place the ball, mouse and fish into their distinctly shaped basket recesses. Wrong shapes and occupied/blocked targets reject without changing state. Provide explicit pickup/place actions and legal-target previews; precise dragging is optional.

Add pip_stuck_gate, based on concept quest 7: rearrange the route, roll the ball onto a pressure switch, open the linked gate and retrieve the toy mouse behind it. The gate remains open only while the switch is occupied. Retrieving the mouse requires the gate/reachability condition. Undo updates ball, switch, gate and collectible state together. These are discrete rules with animated presentation, not physics-dependent solutions.

Reuse versioned quest/layout definitions, role binding, occupancy, routes, sockets, switches, collection, command handling, hint stages, undo/restart, rewards and save projection. Shared meshes are immutable; each quest has its own saved attempt and object state. Keep dialogue short and Pip's personality consistent. Unlock quests in the defined order and grant each 20 coins plus 1 friendship once.

Author a solvable layout, three useful progressive hints and a replayable known solution for each quest. Test blocked retrieval, compatibility, switch/gate changes, undo, restart, loading cancellation, independent attempts and reward idempotency.

Acceptance: complete all three puzzles in the actual game using mouse and keyboard-equivalent actions, exercise touch-oriented layout/controls, reload mid-puzzle, and verify 60 total coins and 3 friendship from one completion of each. Update the handoff for phase 7.
```

## Prompt 7 Shop furnishing placement and the finished afternoon

```text
Implement phase 7 of Pip's first afternoon. Read docs/mvp-implementation-prompts.md and follow its shared instructions and fixed economy. Use engine-skill and accepted production furnishings/animations.

Open the shop after quest 1. Offer exactly one garden cushion for 30 coins and one drinking bowl for 25 coins, with final preview imagery, names, prices, locked/available/owned states, details and an explicit Buy action. After all quests, 60 earned coins allow both purchases, leaving 5 coins. Reject insufficient funds, repeated/duplicate purchases and stale commands atomically.

Implement furnishing placement into the three authored garden spots. Show the actual model, footprint and valid/invalid explanation, with explicit Place/Cancel. Check compatibility and occupancy. Allow moving owned furnishings later without charging again. Cancelling placement leaves ownership intact and preserves the previous placement. Persist wallet, ownership and placements coherently.

Make Pip approach a placed cushion, settle and curl up, or approach the bowl and drink. Use authored short paths/anchors appropriate to this fixed garden. Keep collision and pose readability believable without implementing a general open-world cat AI system. Bound repetitions and keep Pip from hiding puzzle entry controls. Reduced motion presents equivalent readable poses.

When all three quests are complete and at least one purchased furnishing is placed, show a brief affectionate closing moment with Pip, a compact 'A lovely first afternoon' card and Continue in garden. Record the ending once; offer replay through the menu. Use contextual lines rather than long dialogue.

Acceptance: complete earn/buy/place/use, cancel and move a furnishing, try invalid/repeated purchases, reload, and confirm consistent coins/items/placements and no automatic repeated ending. Capture production GUI and cat-use evidence. Update the handoff for phase 8.
```

## Prompt 8 Durable saves recovery and browser lifecycle

```text
Implement phase 8 of Pip's first afternoon. Read docs/mvp-implementation-prompts.md and follow its shared instructions and architecture.md persistence/lifecycle contracts. Use engine-skill. Harden the delivered MVP save and restore path rather than adding a parallel persistence system.

Persist explicit portable fields for schema/content version, save/session revision, quest attempts, undo policy, completion/reward ledger, wallet, friendship, purchases, placements, ending status and settings. Reconstruct actors, callbacks, GUI and audio from validated data. Preserve original bytes on malformed, future-version or migration failures. Document whether undo history is restored; at minimum preserve the current committed puzzle state.

Coalesce ordinary move saves and immediately snapshot completion, purchase, restart and placement transactions. Use bounded valid snapshot history. Handle out-of-order completions and failed writes without marking unsaved revisions durable. Display Saving/Saved/Retry accurately. Implement export/import with explicit file UI, validate imports before publication and preserve the current game on failure.

Enforce one writable session per save using supported browser locking; otherwise isolate an independent slot. Explain a second-tab session readably. Support a clearly identified temporary session when persistent storage is unavailable, with export access.

Exercise explicit pause, hidden-page suspension, background/return, resize, GPU recovery, rapid restart/scene replacement and asset cancellation. Preserve user pause/settings, clear held input and prevent hidden-time catch-up or duplicate rewards.

Acceptance: test mid-attempt and completed reloads, wallet/ledger consistency, purchases/placement, corrupted/future saves, storage failure/retry, export/import and two-tab isolation. Use the actual generated game for browser checks and record unresolved device-specific evidence honestly. Update the handoff for phase 9.
```

## Prompt 9 ElevenLabs and Suno audio production and integration

```text
Implement phase 9 of Pip's first afternoon. Read docs/mvp-implementation-prompts.md and follow its shared instructions, ElevenLabs sound inventory, Suno music briefs and audio delivery requirements. Use engine-skill and the pinned engine audio contracts.

Produce or ingest the listed effects, cat vocalizations and ambience through ElevenLabs, and the listed instrumental music through Suno. Use connected provider tooling only when available and authorized. If generation must be performed by the user in those services, prepare an exact production checklist with stable filenames, briefs and import destinations, then implement and validate independent audio integration. Report missing final files; silent placeholders are not accepted as generated final audio. Do not generate music through another provider.

Audition each source, retain the original download, trim/clean the accepted clip, create verified loop boundaries where needed, and produce supported runtime encodings. Record provider/source IDs, prompts, settings, generation dates, rights records and processing. Bind every cue to the inventory's event. Use variations for repeated interactions, conservative volumes, cooldowns, bounded voices and no noisy save notifications.

Implement Music, Ambience and Effects controls plus master mute. Activate/resume audio synchronously from the Play/Enable sound gesture, handle rejection readably, suspend/resume with page lifecycle, cancel scene-owned voices on unload, and prevent stacked loops after restart. Crossfade only within the measured decode/voice budget and supported backend. Keep music quieter than informative effects. Text and visual feedback remain sufficient when muted.

Acceptance: inspect waveforms/loop metadata, actually listen to the complete three-quest/shop journey, verify starts, loops, transitions, mute/sliders, background/return and reload, and check clipping/voice/decode budgets. Record generated, imported, wired and audibly verified assets separately. Update the handoff for phase 10.
```

## Prompt 10 Final production polish acceptance and release package

```text
Implement phase 10 of Pip's first afternoon. Read docs/mvp-implementation-prompts.md and follow its shared instructions and all production briefs. Use engine-skill, pawmodels for model corrections, and the polish-uat-loop skill for evidence-led review.

Review a fresh 15–20-minute first-player journey: start, meet Pip, understand each puzzle, use hints/undo, complete all quests, receive 60 coins/3 friendship, inspect both shop items, purchase/place one, see Pip use it, view the ending and continue. Adjust unclear layouts, copy, interaction feedback and pacing based on observed defects. Keep the fixed MVP scope.

Inspect final GUI and art in wide/portrait/short-landscape views, larger text, reduced motion, muted audio, grayscale and shadow-free quality. Verify readable cat identity, route endpoints, toy/socket shapes, legal targets, deformation, transitions, focus/capture and matching source/export/runtime appearance. Fix production defects rather than accepting a mockup or uninspected generated asset.

Run the appropriate portable rules/content/lifecycle tests and pinned Release browser build. Exercise save failures, import/export, asset failures, GPU recovery and repeated scene transitions. Measure startup transfer, frame-time distributions, tracked memory, draws/triangles and audio decode peaks against the provisional architecture budgets. Test at least 15 minutes where physical devices are available; report exact supported browser/OS/device/build combinations. Desktop automation or emulation does not close physical-mobile acceptance.

Prepare a complete versioned static-site release package with runtime assets, notices, correct relative URLs, content/build IDs and hosting instructions. Verify the packaged site locally; publish only when explicitly requested. Keep developer galleries/inspectors out of normal release navigation.

Finish docs/implementation-status.md with the actual delivered scope, final launch/package paths, evidence, known limitations and remaining device or external-asset gates. Do not call the MVP production-ready while required final GUI, models, audio or acceptance remain incomplete.
```

## GUI production brief

Generate a coherent implemented interface, including layout, fonts, icons, states and assets. Use the existing `design.md` tokens rather than inventing a second visual system. Ordinary play should keep the garden/puzzle dominant.

| Surface | Required production result |
| --- | --- |
| Startup and title | Paws wordmark, Play/Continue, real loading status, readable compatibility/retry surface |
| Garden | Compact coin/friendship/menu display, Pip quest marker, contextual Shop and furnishing access |
| Dialogue | Portrait rendered from final Pip, name, one short request, visible Accept/Close |
| Puzzle | Short objective, selected-object name, legal Move/Rotate/Roll/Pick up/Place actions, Undo, Hint and reset view |
| Hints | Three authored steps, progressively exposed in a readable expandable sheet |
| Pause and settings | Continue, restart confirmation, Back to garden, music/ambience/effects, mute, motion, text size and quality |
| Completion | Accurate earned rewards, readable Pip reaction, Back to garden; replay does not display a new grant |
| Shop | Two final item previews, details, price and availability/ownership state; explicit purchase |
| Placement | Actual furnishing preview and footprint, valid/invalid feedback, Place/Cancel |
| Ending | Short Pip moment, compact closing card, Continue in garden and optional later replay |
| Saving and recovery | Truthful Saving/Saved, durable failure and Retry, export/import, temporary/second-tab explanation |

Include default, hover, pressed, keyboard focus, selected, disabled, invalid, loading and failure states. Use the design guide's 48 px standard controls with a 44 × 44 px minimum, at least 8 px separation, readable body text, and functional contrast. Bundle OFL font records and test Å, Ä and Ö. Icons use an original rounded 24-unit family with text labels where useful; platform emoji are not shipped controls.

Generate GUI structure and icons in editable code/vector form. Cat portraits and shop thumbnails can be raster renders from accepted 3D assets. ImageGen may supply useful original decorative illustration if needed, but do not flatten panels, text or controls into images. Final acceptance is against runtime screenshots and interaction, including portrait reflow, keyboard access, modal focus and reduced motion.

## 3D model inventory and production brief

| Asset group | Models or parts to deliver | Gameplay interfaces |
| --- | --- | --- |
| Pip | One skinned cat, canonical rig, required actions and consistent portraits | Root/facing, paw contact, resting/drinking poses and anchor alignment |
| Courtyard | Sandstone ground/paving and rounded perimeter/step pieces | Ground plane and furnishing/puzzle bounds |
| Boundary and planting | Low fence pieces, grouped opaque hedge/leaf clumps and small flowers | Unobtrusive silhouettes outside interactive targets |
| Bench | One softly bevelled painted-wood bench | Floor contact and plausible cat scale |
| Terracotta pots | Two readable variants with shared slide-compatible bases | Logical footprint, floor pivot and permitted slide axis |
| Channels | Straight and quarter-turn curved wooden troughs | Compatible width/height/ports and rotation centre |
| Ball | Coral toy ball with cream band | Radius, channel clearance and basket fit |
| Toy mouse | Rounded mouse with recognizable ears and tail | Pickup origin and matching basket clearance |
| Toy fish | Rounded cloth fish with distinct tail | Pickup origin and matching basket clearance |
| Basket | Wicker basket/base with ball, mouse and fish recesses | Three recognizable compatible socket transforms |
| Blocking cushion | One movable fabric cushion with distinct appearance | Slide footprint, pivot and selection proxy |
| Pressure switch | Fixed base and independently moving plate | Ball rest point, travel axis and pressed/unpressed geometry |
| Garden gate | Posts/frame and independent hinged leaf | Hinge, closed/open transforms and visible passage clearance |
| Shop cushion | Attractive tufted cushion and optional simple base | Placement footprint, Pip approach/settle/rest anchors |
| Shop bowl | Glazed ceramic bowl and simple water surface | Placement footprint, Pip approach/drink anchors |

Production means editable assets that survive export and appear correctly in Paws. Preserve `.blend` masters, scripts, ImageGen originals/prompts, portable maps, runtime GLBs, manifests, provenance, evaluations and correction records. Inspect blockout, detailed materials and clean export/runtime comparison through at least three actual render–inspect–correct cycles for each new asset or coherent kit. A still render alone does not verify cat animation or runtime import.

Use metres and the existing engine axis convention after verifying it. Document measured bounds, pivots, route/contact/hinge/socket/use anchors and material/mesh/texture counts. Because the current importer does not reconstruct mesh-node actor hierarchy, independently moving parts require separate exports and explicit application composition. Preserve the canonical rig and supported in-place clips.

Start with a provisional Pip budget of about 12,000–18,000 visible triangles and one or two material slots; small toys about 300–1,500 triangles, individual props about 500–3,000, and a whole gate/basket/bench assembly about 2,000–6,000. These are authoring allocations to validate, not measured performance. Prefer shared 512–1024 texture maps; use 2048 only where a measured close-up need warrants it. The whole visible scene must stay within the architecture's provisional mobile 150,000-triangle/100-draw target and loading/memory targets. Measure skinned rendering and do not assume static-mesh instancing applies to Pip.

Use matte clay, wood and cloth; restrained glazed highlights; smooth coat masses; soft bevels; quiet broad texture detail; opaque grouped foliage; one warm sun with cool fill; and readable contact without requiring shadow maps. Detailed fur strands, unsupported materials and tiny foliage alpha layers are outside the baseline.

## ElevenLabs sound generation list

Generate these sound families using ElevenLabs Sound Effects. Cats use natural nonverbal sounds; dialogue remains on screen. Each row supplies a generation description, target delivered duration and event binding. Generate repeated-action variants independently with the same brief, then select contrasting but coherent takes.

The current ElevenLabs API accepts 0.5–30 seconds per generation and supports a looping option on its v2 sound-effects model. For short UI clips, generate at least 0.5 seconds and edit to the delivered duration below. Its general sound-effects documentation lists MP3 and non-looping WAV output; retain the best source actually available to the account rather than assuming every looping export is lossless. See the [API contract](https://elevenlabs.io/docs/api-reference/text-to-sound-effects/convert) and [sound-effects guide](https://elevenlabs.io/docs/overview/capabilities/sound-effects).

All one-shot descriptions imply an isolated clean recording with no speech, music, hiss or long reverberation. Use soft natural textures suitable for the small cozy garden. Avoid exaggerated cartoon squeaks, electronic alarms or loud celebration stacks.

| Stable sound ID | Generation description | Delivered duration and variants | Event |
| --- | --- | --- | --- |
| `amb_garden_air` | Gentle late-afternoon breeze softly moving garden leaves, steady quiet air, no birds, people, traffic or distinct gusts; seamless ambience | 20–30 s loop; 1 | Garden and quieter puzzle ambience |
| `amb_garden_bird` | One small songbird giving a short relaxed phrase from a nearby garden, quiet air around it, no other sounds | 1–3 s; 3 | Sparse randomized garden accent |
| `pip_request` | A young friendly domestic cat giving one soft curious requesting meow, natural and close, no distress | 0.7–1.5 s; 2 | Select Pip or accept his request; cooldown |
| `pip_happy` | A contented domestic cat making one brief pleased chirrup or trill, warm and natural | 0.5–1 s; 2 | Completed puzzle or first furnishing use |
| `pip_purr` | Gentle close domestic-cat purring with even natural texture, quiet breathing, no meowing; seamless loop | 8–12 s loop; 1 | Resting on cushion, bounded/faded |
| `pip_paws_stone` | One soft padded cat paw step on smooth sandstone, a tiny natural contact with no shoes or claw scraping | 0.15–0.3 s; 3 | Walk/pounce contact markers |
| `pip_settle` | Small cat settling into a cotton cushion, one quiet fabric compression and soft rustle | 0.5–1 s; 1 | Settle animation contact |
| `pip_drink` | A domestic cat quietly lapping water from a small ceramic bowl, close gentle rhythmic laps, no bowl rattle | 2–4 s; 1 | Short drinking action |
| `ui_select` | Tiny warm dry wooden tick with a soft rounded transient, delicate and satisfying | 0.08–0.15 s; 2 | Selection or focus activation, not every focus move |
| `ui_confirm` | One small rounded wooden pluck, warm and restrained, a slightly brighter confirmation than a tick | 0.12–0.25 s; 1 | Ordinary button activation |
| `ui_panel` | Very soft short paper movement with a gentle air swish, natural stationery texture | 0.2–0.4 s; 1 | Open/close a panel; quiet |
| `ui_undo` | Soft descending pair of muted wooden taps, quick and reassuring | 0.25–0.45 s; 1 | Committed undo |
| `ui_hint` | A tiny clear ceramic bell note with a warm gentle tail, friendly and understated | 0.4–0.8 s; 1 | Reveal next hint |
| `ui_blocked` | Quiet low wooden knock with a soft ending, mild feedback without alarm or buzzer | 0.15–0.3 s; 1 | Rejected move plus explanation |
| `reward_coins` | A few small coins gently settling into a cloth pouch, light metallic contacts followed by a soft fabric stop | 0.6–1 s; 1 | Actual once-only quest reward |
| `shop_purchase` | A single small coin placed on wood followed by a soft pleased wooden chime, restrained and close | 0.5–0.8 s; 1 | Successful committed purchase |
| `pot_slide` | A small heavy terracotta flowerpot sliding briefly across sandstone and coming to a soft stop, ceramic grit without harsh screech | 0.4–0.7 s; 3 | Accepted pot slide |
| `channel_rotate` | Short wooden trough turning on a small pivot, gentle wood friction and a neat soft stop | 0.3–0.6 s; 2 | Accepted channel rotation |
| `ball_roll` | A small smooth toy ball rolling along a shallow wooden trough, light rounded rolling texture, no music | 1–2 s loopable segment; 1 | Validated roll presentation, duration bounded |
| `ball_contact` | A light toy ball making one gentle rounded contact against wood or stone, small and close | 0.1–0.25 s; 3 | Ball stop/arrival/block contact |
| `toy_pickup` | Small soft cloth toy lifted from stone, brief fabric brush with a tiny gentle contact | 0.2–0.4 s; 2 | Accepted toy pickup/retrieval |
| `basket_place` | Small cloth or smooth toy settling into a fitted wicker basket recess, soft wicker touch and light contact | 0.25–0.5 s; 2 | Accepted matching placement |
| `cushion_move` | Cotton cushion sliding a short distance across stone, soft fabric scrape and rounded stop | 0.4–0.7 s; 2 | Blocking cushion slide |
| `switch_down` | Small wooden pressure plate depressing with a quiet tactile click and soft wooden stop | 0.15–0.3 s; 1 | Switch becomes occupied |
| `switch_up` | Small wooden pressure plate gently lifting with a light released click, softer than the down sound | 0.15–0.3 s; 1 | Switch becomes unoccupied |
| `gate_open` | Small garden gate opening on a well-kept hinge, subtle wood movement and very slight natural creak, no squeal | 0.6–1 s; 1 | Gate opens |
| `gate_close` | Small wooden garden gate closing gently onto its stop, quiet wood movement and a padded final knock | 0.5–0.9 s; 1 | Gate closes |
| `furnishing_place` | A small padded garden furnishing gently set onto stone, soft grounded thump and short fabric contact | 0.3–0.6 s; 1 | Cushion placement/move commit |
| `bowl_place` | Small ceramic pet bowl carefully placed onto sandstone, delicate ceramic contact with no ringing crash | 0.2–0.5 s; 1 | Bowl placement/move commit |

This inventory contains 29 sound families and 44 selected clips when all listed variants are retained. Generate additional candidates only to replace flawed takes. Reuse `ui_confirm` for Accept/Continue/Place and `ui_panel` for menu transitions. Quiet save status has no sound. The short musical ending comes from Suno rather than stacking multiple reward effects.

## Suno music generation list

Use Suno Custom mode with Instrumental enabled and leave lyrics empty. Suno documents instrumental creation in its [Custom mode guide](https://help.suno.com/en/articles/3726721) and current available download formats in its [download guide](https://help.suno.com/en/articles/13926081). Keep the best original download available to the account. The following tempos, keys and durations are creative targets; inspect the actual generated music and edit its usable sections.

### Garden afternoon

- Stable ID: `music_garden_afternoon`.
- Use: title, main garden, shop and furnishing placement.
- Generate a 2–3 minute instrumental source; deliver a carefully edited 60–90 second loop, with a separate short opening if useful.
- Style description: “Cozy acoustic chamber music for a sunlit miniature garden with a friendly ginger cat. Around 78 BPM, warm D major, gentle felt piano, soft fingerpicked acoustic guitar, a few light pizzicato strings and occasional delicate wooden mallet notes. Intimate, affectionate and unhurried. A small memorable two-bar motif with spacious variations, light dynamics and plenty of silence between phrases. Simple repeatable harmony, restrained arrangement, unobtrusive background game music.”
- Exclude: vocals, humming, spoken words, percussion drops, big drums, dramatic crescendos, bright piercing bells and cinematic trailer effects.
- Acceptance: remains pleasant over repeated garden/shop use, leaves room for cat and interaction sounds, and has an inspected unobtrusive loop seam.

### Little garden puzzles

- Stable ID: `music_garden_puzzles`.
- Use: all three puzzle scenes, with uninterrupted playback through ordinary hints and moves.
- Generate a 2–3 minute instrumental source; deliver a 60–90 second edited loop.
- Style description: “Sparse gentle acoustic puzzle-game underscore related to a warm D-major garden theme. Around 74 BPM, lightly muted felt piano, soft plucked nylon strings, a few low warm pizzicato notes and delicate wooden mallet touches. Curious and thoughtful with no tension or urgency. Tiny phrases separated by space, stable harmony, very subtle variation, even quiet dynamics. Instrumental background for calmly arranging toys and wooden channels.”
- Exclude: vocals, humming, metronomic ticking, suspense drones, escalating intensity, busy arpeggios, drums, sharp high notes and a big ending.
- Acceptance: supports concentration for every puzzle, shares the garden theme's tone and palette, and loops without an obvious repeated climax.

### Pip feels at home

- Stable ID: `music_pip_at_home`.
- Use: the final afternoon moment only; ordinary quest completion uses Pip's reaction and the reward sound.
- Deliver a 6–10 second non-looping cue. Prefer an edited resolving phrase from the accepted garden theme. If that phrase is unsuitable, generate a matching source in Suno and edit it to length.
- Style description: “A tiny affectionate acoustic resolution for a cozy garden game. Warm D major, soft felt piano and a few rounded nylon-guitar or pizzicato notes, a brief echo of a gentle two-bar garden motif, a warm final chord and a short natural decay. Quiet satisfaction as a ginger cat curls up on its new cushion. Intimate and restrained.”
- Exclude: vocals, applause, fanfare, cymbals, big orchestral swells and dramatic emotion.
- Acceptance: resolves naturally, does not force a long wait, and fades back to the garden theme without a volume jump.

Generation does not establish seamless loops or exact musical timing. Edit final tracks in an audio tool, retain the project/edit instructions, record exact loop boundaries and audition at least five consecutive repeats through the actual game backend. Store sample/frame boundaries or the backend's supported equivalent. Check musical continuity, codec padding, clicks and drift. Use a supported scheduled seam/crossfade when a raw repeated buffer is insufficient; do not infer gapless playback from a filename or provider setting.

## Audio delivery and mixing

Keep originals and edit projects under `artifacts/audio-production/`. Put only accepted supported runtime files and notices under `assets/audio/`, with stable IDs in the content manifest. Retain provider asset IDs/links, creation settings/date, prompt, available use-rights record and transformations for each final file. Commercial packaging requires the actual generation/account rights record; this prompt pack does not assert a provider license on the user's behalf.

Use mono for short dry effects and positional cat sounds where appropriate; reserve stereo for music and ambience when it adds useful space. Preserve source quality and export runtime formats supported by the verified browser backend. Converting a lossy source to WAV does not restore lost detail. Trim silence, remove DC offsets/clicks, preserve natural transients and match related takes. Start with a conservative final peak ceiling near −2 dBFS, then verify the mixed game has headroom and no clipping; do not normalize every effect to equal perceived prominence.

Control master mute and independent Music, Ambience and Effects volumes. Cat vocalizations use Effects. Begin with at most 12 active effects voices, at most one ambience bed, and one music track except a short bounded transition; tune these provisional limits from measurements. Avoid persistent decoded buffers for every full-length source track. A 90-second stereo buffer at 48 kHz with 32-bit float samples is about 33 MiB before other overhead, so music duration, preload policy and crossfade overlap materially affect the mobile memory target.

Activate audio from a real user gesture and show a readable Enable sound action if needed. Suspend/resume with the host lifecycle and preserve the user's mute/volume choices. Stop/fade scene-owned voices on transition and reject stale cue events. Rewards, purchases, gate state and puzzle progress are decided by committed gameplay, never by audio completion. Every sound-dependent event also has readable visual feedback.

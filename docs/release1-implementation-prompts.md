# Paws release one — ten implementation prompts

Release one is a complete cozy afternoon in one garden: four cats, ten handcrafted puzzles, eight meaningful furnishings, three chapters and a garden-picnic ending. Target 45–75 minutes for a first playthrough, subject to independent playtesting. Extend the working MVP; do not rebuild its delivered systems.

This file is the release-one brief. MVP-only limits in `docs/mvp-implementation-prompts.md` describe the previous milestone. Current source and acceptance records take precedence over old foundation descriptions.

## How to run these prompts

Run Prompt 1 first. Then use separate chats for independent tasks. Copy the whole numbered prompt; each incorporates the shared instructions and fixed scope here.

| Wave | Prompts | Dependencies |
| --- | --- | --- |
| A | 1 — contracts and baseline | Working MVP |
| B, parallel | 2 — Tripo cats; 3 — Astra prop production; 4 — puzzle rules/content; 8 — audio | 1 |
| C | 5 — progression and durable saves | 4 |
| D, parallel | 6 — garden/cat integration; 7 — GUI | 6 needs 2, 3 and 5; 7 needs 4 and 5 |
| E | 9 — complete game integration | 2–8 |
| F | 10 — polish, acceptance and release package | 9 |

Prompt 2 contains three separate Tripo paste-in briefs. They are inputs inside one cat task, so the release still has exactly ten numbered prompts. You generate those meshes in Tripo; the cat task prepares intake contracts and subsequently repairs/rigs/exports your supplied results. Pip is reused. Select Astra in the chat running Prompt 3; that prompt owns non-cat geometry production through PawModels. Model selection is a user action, not an instruction to silently change another chat's model.

Parallel chats must use separate asset job directories, evidence directories and build directories. In a shared checkout, each task owns the paths allocated in Prompt 1. Do not edit another active task's files. Keep shared manifest, build registration, browser composition and final documentation merges with Prompt 9. If an unavoidable shared change is needed, record the exact patch in the task handoff and let the integration task apply it. Separate worktrees are optional if you explicitly request them; they are not assumed.

Each task writes its own `docs/release1/prompt-NN-handoff.md`, rather than concurrently updating the common implementation-status file. Handoffs contain changed paths, exact commands, evidence, interface changes, unresolved dependencies and patches awaiting integration. Do not spawn agents or send messages to other chats without explicit user authorization.

## Shared instructions incorporated by every prompt

- Read this entire brief, `AGENTS.md`, `README.md`, `docs/architecture.md`, `architecture.md`, `paws-concept.md`, `design.md`, `docs/implementation-status.md`, and the dependency handoffs. Inspect current source and Git status in both repositories. Preserve unrelated work and accepted Pip revisions.
- Use `.agents/skills/engine-skill/SKILL.md` for game/runtime integration and the installed `pawmodels` skill for 3D work. Inspect the actual design reference image when working on appearance. Tripo is the user's selected provider for new cat meshes; use PawModels for their subsequent canonical-rig repair, materials/export validation and runtime inspection.
- Keep rules, cat behavior, progression, content and GUI composition in Paws. Use the exact pinned `external/WebEngine` for shared builds; verify APIs in that checkout. Follow the adjacent-engine development and deliberate pin-update workflow only for a concrete necessary engine change. Never hide changes in an uncommitted submodule or advance the pin to resolve an assumption.
- Reuse the authoritative session, revision/attempt checks, reward ledger, undo/restart, retained GUI, Application clocks, asset preparation and save coordinator. Browser callbacks stay in the host; delayed scene references use checked handles. Logic must not depend on visual poses, sound, frame rate or physics simulation.
- Mouse, touch and keyboard-equivalent commands reach the same rules. Keep free progressive hints, undo, restart, pause, reduced motion, larger text and sound-independent clues. No timers, precise dragging, grinding or penalties for hints.
- Sources/raw generations/rejected assets stay under `artifacts/`; accepted runtime files belong under `assets/runtime/`. Keep stable IDs, versioned content, hashes, byte lengths, provenance and actual rights records. Do not invent licenses or provider outputs.
- Asset-production chats deliver namespaced manifest/catalog fragments and isolated preview wiring. They must not overwrite the shared production manifest or accepted assets. Build/evidence paths use a task-specific suffix. An isolated preview must actually load the exported asset in the pinned renderer; Blender renders alone do not close runtime acceptance.
- Run meaningful tests for changed rules and save invariants, portable scene tests for lifecycle changes, pinned browser builds for browser work, and checks against the actual generated site. State what was observed and what remains unverified. Physical-device and independent-player acceptance cannot be inferred from automation/emulation.
- Missing user meshes or production audio block their dependent acceptance, not independent preparation. Continue useful work, retain precise input requirements and label incomplete output. Do not promote stand-ins as finished content. Do not publish, commit or push the game as part of these prompts unless separately authorized.
- Finish implementation and correction within the task scope; do not stop at a plan. Write the task handoff. Only Prompts 1, 9 and 10 reconcile the common implementation-status document.

## Fixed content and progression

Preserve the three existing quest IDs and their completed attempts/reward records. Proposed new IDs below are frozen by Prompt 1 after checking collisions. The two-switch quest features Pip and Pepper; it does not add a fifth cat.

| Chapter | Quest | Requester | Main capability |
| --- | --- | --- | --- |
| 1 | `pip_missing_ball` — Pip's missing ball | Pip | Sliding and routing, existing |
| 1 | `pip_tidy_toys` — Tidy the toy basket | Pip | Matching/retrieval, existing |
| 1 | `pip_stuck_gate` — The stuck little gate | Pip | Weighted switch and gate, existing |
| 2 | `button_mouse_bench` — Mouse under the bench | Button | Ordered sliding and restricted retrieval reach |
| 2 | `mabel_sunny_spot` — The sunny spot | Mabel | Plank sockets and traversable connected route |
| 2 | `button_cushion_bed` — A bed for Button | Button | Three cushion shapes fitted without overlap |
| 2 | `mabel_drinking_route` — A drink after playing | Mabel | Rotated gutter route and bowl fill |
| 3 | `pepper_garden_bells` — The garden bells | Pepper | Symbol sequence, readable without sound |
| 3 | `pip_pepper_playhouse` — Two kittens, one doorway | Pip and Pepper | Two switches held simultaneously |
| 3 | `garden_picnic` — The garden picnic | All four | Shade, adjacency and quiet-zone placement clues |

Chapter 2 opens after Chapter 1; its four requests can be solved in any order. Chapter 3 opens after Chapter 2; bells and playhouse can be solved in either order, then picnic opens. Introduce characters through short dialogue, not long cutscenes. Each quest awards 20 coins and one friendship point to its requester, once. The joint quest awards one point to each participant; picnic awards one point to each cat. Define per-cat milestones from this distribution without requiring equal quest counts.

The eight catalog entries below cost 190 coins in total; ten quests earn 200. No furnishing purchase is required to access an unsolved quest: puzzle-local props are supplied. Retain current cushion/bowl IDs, prices and ownership. Unlock the remaining entries at suitable related quest milestones, with both picnic items available before the finale. Purchases and garden placement can continue after the ending.

| Furnishing | Price | Visible use |
| --- | ---: | --- |
| Existing cushion/bed | 30 | Button curls up; other cats may rest |
| Existing drinking bowl | 25 | Mabel drinks |
| Scratching post | 20 | Pip scratches/stretches |
| Sunny perch | 20 | Mabel climbs/settles through an authored supported sequence |
| Hammock | 25 | Button rests |
| Playhouse | 30 | Pip and Pepper enter/look out |
| Low picnic table | 20 | Cats gather around it |
| Picnic mat | 20 | Pepper and companions sit/rest |

Use authored compatible placement spots, not unrestricted construction. Distinguish temporary puzzle arrangements from owned garden furnishings. Finale eligibility is all ten requests completed and at least one owned furnishing placed. The finale must render with supplied celebration props even if the player has not bought picnic furnishings. Store the release-one ending separately from the old MVP ending; an old ending flag must not suppress it.

Defer extra gardens, procedural content, accounts/cloud saves, multiplayer, breeding, daily chores, freeform building, day/night simulation, PWA/offline installation and live events. Desktop browser is the initial supported target; mobile support is added only for accepted physical combinations. Preserve responsive/touch functionality regardless.

## Prompt 1 — Establish the release-one contracts

```text
Implement release-one Prompt 1 in docs/release1-implementation-prompts.md. Follow that file's shared instructions and fixed scope. Use the repository-local engine-skill.

Audit the working MVP against current source and evidence. Preserve all delivered systems. Record open production audio, natural sleeping-curl, independent-player and physical-device gates without treating the previous documentation as a new acceptance run.

Create docs/release1/contracts.md and a compact machine-readable release-one contract under docs/release1/. Freeze the ten quest IDs, four cat IDs, eight item IDs/prices, progression/reward rules, placement roles, required model anchors, canonical rig/action contract, audio cue names, portable command/read-model interfaces and manifest fragment format. Preserve existing IDs and save compatibility. Identify exact source owners and filenames for each later task. Do not create a second state owner or implement all later features here.

Allocate disjoint task-owned directories/files for the parallel work: cat intake/exports, prop kit, release-one rules/content, progression/save codec, garden presentation, GUI views and audio. Shared composition files and manifest registration remain integration-owned. Later code modules must be compilable through task-local harnesses or isolated optional targets until registration is merged. Document the concrete test/build entry point for each track; do not promise independence if existing monolithic files prevent it. Extract only the narrow seams needed to make ownership workable, preserving behavior and covering changed lifecycle contracts.

Produce a baseline pinned Release build and run the current portable and affected browser checks. Record payload identity and scoped evidence. Document migrations and the MVP-to-release-one ending policy. Update docs/implementation-status.md and write docs/release1/prompt-01-handoff.md. Acceptance: the MVP still works and the other tasks have concrete, nonconflicting contracts and paths.
```

## Prompt 2 — Tripo cat intake, canonical rig and character production

```text
Implement release-one Prompt 2 in docs/release1-implementation-prompts.md after Prompt 1. Follow shared instructions/contracts. Use pawmodels and engine-skill for export/runtime verification. The user will generate Mabel, Button and Pepper meshes in Tripo3D AI using the three briefs below. Do not substitute another geometry provider or regenerate accepted Pip.

Prepare three intake folders under artifacts/model-jobs/release1-cats/, export requirements and identity checklists now. Use accepted Pip views and design.md as the visual scale/style authority. Ask for missing mesh files only when needed; do not invent their arrival or treat generated references as delivered meshes. Prefer textured GLB plus original source/export and provider rights information; retain any supplied FBX. The supplied meshes may be unrigged or have incompatible rigs.

Once available, inspect each Tripo result. Repair topology, UV/material problems and unsuitable proportions while preserving accepted identity. Fit/skin to the current canonical Pip skeleton: preserve names, hierarchy, rest/bind transforms, units and clip compatibility. Do not accept Tripo autorigging as the runtime contract. Adapt weights and contact poses per mesh. Reuse the latest accepted Pip animation source, preserving eye/blink/walk repairs. Deliver required common actions plus sparse personality reactions, scratching, supported perch/playhouse poses and a genuinely natural sleeping curl for Button and Pip. Keep world movement application-owned and clips in place. Use supported bone animation, not unsupported morph tracks or fur shaders.

Generate consistent portraits from the final models; produce packed editable Blender masters, retained repair/animation scripts, inspected GLBs, material maps, manifests, provenance and correction logs. Perform PawModels render/inspect/correct cycles, clean-process reimport, joint/deformation/contact review and an isolated pinned Paws preview at game distance, including shadow-free and reduced-motion views. New runtime assets live in the task namespace, not shared production registration. Hand over canonical actions and exact use/contact anchors to Prompt 6.

Acceptance: all four cats keep consistent identity, animate correctly and load in the pinned renderer. If meshes are not supplied, finish intake/reference/contracts and report this acceptance dependency explicitly. Write docs/release1/prompt-02-handoff.md.

```

### Tripo input: Mabel

Attach accepted Pip front/side/three-quarter views as style references when Tripo supports reference input. These three provider briefs are part of Prompt 2, not extra implementation tasks.

```text
Create one full-body stylized 3D cat for a cozy garden puzzle game, matching the supplied Pip reference's softly sculpted anatomy, broad paws and readable face. Mabel is a cool grey tabby with a broad cream chest and muzzle, subtle darker tabby stripes, a relaxed substantial tail and a calm, composed expression. Four legs, connected natural joints, rounded ears, clean visible facial planes. Neutral standing quadruped pose, paws on one flat plane, legs slightly separated for rigging, tail clear of the body. Keep realistic cat structure within the reference's stylization. Matte short coat represented by solid mesh and texture, simple eyes. One cat only; no collar, clothing, props, base, scenery, text, baked cast shadows, strand fur or transparent fur cards. Complete front/side/back geometry suitable for later canonical skeletal rigging; preserve clear shoulder, hip and paw volumes. Deliver the textured mesh in GLB if available.

```

### Tripo input: Button

```text
Create one full-body stylized 3D cat matching the supplied Pip reference and the same cozy garden asset family. Button has a charcoal coat, cream muzzle and cream paws, a gently compact body and a soft shy expression. Dark surfaces must retain readable muzzle, cheeks, limbs and ears; avoid a featureless black silhouette. Substantial tail suitable for curling beside the body. Use the same basic anatomical proportions and scale family as Pip, with modest personality differences. Neutral standing quadruped pose for rigging, four paws on one plane, legs slightly separated, tail clear of the body. Button's eventual curled sleeping silhouette will be animated later; do not generate a permanently curled mesh. Matte sculpted short coat, connected natural anatomy, clean eyes and facial planes. One cat only; no clothing, collar, props, base, scenery, text, baked shadows, strand fur or fur cards. Complete riggable geometry and textured GLB if available.

```

### Tripo input: Pepper

```text
Create one full-body stylized 3D cat matching the supplied Pip reference's anatomy, proportions, softly rounded shapes and cozy garden presentation. Pepper has a warm cream coat with charcoal patches, one distinctive asymmetrical charcoal marking surrounding the LEFT eye and extending toward the LEFT ear, and a cream RIGHT eye region. Preserve this marking consistently from front, sides and back. Bright attentive expression, rounded ears, broad paws and a substantial gently curved tail. Neutral standing quadruped pose for later rigging, all paws on a flat plane, legs slightly separated, tail clear of the body. Matte short coat represented by solid mesh and textures; connected natural shoulder/hip anatomy and simple readable eyes. One cat only; no collar, clothing, props, base, scenery, text, baked cast shadows, strand fur or transparent cards. Complete riggable geometry and textured GLB if available.
```

## Prompt 3 — Astra and PawModels: non-cat release-one model kit

```text
Implement release-one Prompt 3 in docs/release1-implementation-prompts.md after Prompt 1. This is the separate non-cat 3D production task intended to run in an Astra chat. Follow shared instructions/contracts. Use pawmodels, built-in ImageGen for suitable surface inputs, headless Blender and engine-skill for isolated pinned runtime inspection. Do not generate cats; Prompt 2 owns them.

Inspect the accepted garden kit and reuse it wherever it meets the new role. Create only missing or materially inadequate assets: movable crates, retrieval bench clearance pieces, short planks/platforms/perch, three distinct fitting cushion shapes and their basket, symbol-marked hanging bells and support, rain barrel/gutters/bowl water surface, two-switch playhouse parts, picnic puzzle beds/bowls/clue symbols and garden picnic furnishings. Extend the eight-item furnishing catalog with scratching post, sunny perch, hammock, playhouse, low picnic table and picnic mat; reuse the cushion and bowl. Reuse existing ball, toys, channels, switches, gates and garden ground/edge meshes.

Confirm the asset list and dimensions against Prompt 1's puzzle and furnishing contracts before geometry production. Puzzle footprint/socket/route/hinge/paw-contact anchors must match logical interfaces. Preserve channel port heights and widths, plank connection clearances, cushion fits and playhouse entrance clearance. Bells must have visibly distinct symbols; water needs a simple supported surface/ribbon, not fluid simulation. Independently moving gate/switch/bell parts export separately with assembly transforms because imported node hierarchy is not automatically game actors.

Build editable metre-scale Blender sources with reusable parameters and portable materials aligned with design.md. Reuse approved maps when appropriate; generate suitable missing wood/fabric/stone color inputs and author roughness/normal channels separately. Inspect blockout, detailed materials and clean reimport/runtime comparison through at least three actual render/correction cycles for new substantial models. Review at normal game distance, neutral lighting and shadow-free quality. Measure triangles, material/texture counts, bounds, pivots, anchors, file sizes and budgets.

Deliver masters/scripts/maps/GLBs, assembly/anchor manifest fragments, shop thumbnails and complete provenance/evaluation/iteration records under the task-owned paths. Show the actual exported kit in an isolated pinned Paws preview; do not integrate shared game progression or overwrite the production manifest. Write docs/release1/prompt-03-handoff.md with accepted IDs and unresolved defects. Acceptance: every new role has an inspected compatible asset, not merely a concept or Blender-only render.
```

## Prompt 4 — Seven new puzzles and reusable portable rules

```text
Implement release-one Prompt 4 in docs/release1-implementation-prompts.md after Prompt 1. Follow shared instructions/contracts and use engine-skill. This task can run alongside cat, prop and audio production. Own only the allocated rules/content modules and task-local harness; shared registration is deferred to Prompt 9.

Preserve the existing three puzzles. Implement the seven new requests in the fixed table with immutable definitions and independent mutable attempts. Reuse slide/rotate/socket/route/switch/retrieval capabilities where they fit. Add only the required restricted retrieval reach, compatible plank connections, multi-cell shape occupancy, bounded symbol sequence and shade/adjacency/quiet-zone placement predicates. Use deterministic discrete logical state. Validate simultaneous switches on the same committed state; do not latch an earlier press. Water and cat traversal are presentation of a validated route. Picnic clues must be simultaneously satisfiable and readable without color or sound.

Author small nontrivial layouts with known solution traces, three progressive free hints, useful object/action names, portrait/landscape framing requirements and descriptive blocked-action reasons. Shape fitting needs legal rotations and clear destination choices without pixel-perfect dragging. Bells show the picture sequence and reset/retry behavior visibly. Do not add procedural generation or a generic scripting engine. Keep the current typed content approach unless a concrete contract requires otherwise.

Cover solvability, invalid content/reference rejection, occupancy, route termination, held-switch conditions, sequence errors/reset, spatial clues, undo/restart and stale command rejection with meaningful native tests. Keep rewards/progression outside the puzzle owner. Provide read models and a task-local playable debug harness using clearly labeled temporary visuals; it must not replace accepted production content. New assets are addressed by stable roles so rules do not wait for mesh production.

Write docs/release1/prompt-04-handoff.md with layouts, traces, interfaces and integration patch. Acceptance: all ten layouts validate and the seven new puzzles can be completed, undone and restarted through the portable command path.
```

## Prompt 5 — Chapters, friendship, economy and save migration

```text
Implement release-one Prompt 5 in docs/release1-implementation-prompts.md after Prompts 1 and 4. Follow shared instructions/contracts and use engine-skill. Extend the existing authoritative session and save design through the allocated module/seam; do not create a competing wallet or persistence coordinator.

Implement the fixed three-chapter unlock graph, per-cat friendship records/milestones, introductions, ten once-only quest rewards and the eight-item 190-coin catalog. Preserve the existing quest/item IDs and 20-coin rewards. Validate prerequisite graphs and ensure puzzle access never requires a purchase. Show unavailable versus locked versus owned states accurately. Free movement of owned furnishings remains supported, with authored footprint/spot compatibility.

Migrate actual supported MVP saves to release one: retain old attempts, journals, coins/reward identities, friendship, cushion/bowl ownership/placement, settings and recovery data; introduce new cats/quests without retroactive grants. Define the mapping from old Pip friendship explicitly. Preserve the historical MVP ending and add an independent release-one picnic ending record. Older saves must not receive duplicate rewards or accidentally skip the new finale. Unknown future/corrupt content is rejected before publication with original bytes preserved.

Keep completion, friendship, rewards, purchases, placements and ending eligibility atomic in one revision. Preserve normal-tab ownership, independent temporary slots, five recovery snapshots, latest-receipt Saved semantics and export/import. Extend the existing codec/validation rather than serializing raw engine handles. Interrupted attempts must restore independently across quests.

Test old-version fixtures, roundtrip/replay, stale/repeated actions, reward idempotence, affordability/unlocks, failed transactions, invalid references/placements and separate ending identities. Deliver the read-model/intent contract needed by Prompts 6 and 7. Write docs/release1/prompt-05-handoff.md with the migration table and shared integration patch.
```

## Prompt 6 — Living garden, four cats and furnishing use

```text
Implement release-one Prompt 6 in docs/release1-implementation-prompts.md after Prompts 2, 3 and 5. Follow shared instructions/contracts and use engine-skill; use pawmodels for any in-scope asset/contact correction. Own allocated garden/cat presentation modules. Leave shared GUI/browser registration to Prompt 9.

Extend the existing courtyard into one coherent garden with a few visually unlocked areas and authored compatible furnishing spots. Show the four cats as progression introduces them. Provide short approach/attention/reaction sequences, clear request entry points and sparse personality behavior. Reuse immutable assets; independent cats keep their own playback state. This is an authored garden, not an autonomous life simulation.

Implement all eight visible furnishing uses from the fixed table. Match repaired meshes to supplied contact/perch/entry anchors and keep paws/body supported through transitions. Schedule safe authored paths and simple bounded spot reservations to avoid cats occupying the same furnishing/contact space. Cats must not obscure puzzle targets or block the player's quest access. Purchases/placements remain authoritative in Prompt 5; animations never grant rewards or change occupancy rules.

Prepare assets before activation, retain the valid outgoing scene on failure and reject stale loading/use callbacks. Use Application clocks for pause/hidden-page behavior, release borrowed scene references correctly, and support static reduced-motion alternatives. Inspect actual walk/blend/contact/curl/drink/scratch/perch/playhouse playback at ordinary game distance and close-up in desktop and shadow-free profiles. Restore/rebuild the garden from saved state without repeating first-time rewards or introductions.

Deliver an isolated garden harness and scoped browser evidence for four cats, every furnishing, moving/cancelling a placement during use, pause/resume, rapid scene changes and reload. Record asset defects accurately; do not relabel a crouch as a sleeping curl. Write docs/release1/prompt-06-handoff.md.
```

## Prompt 7 — Release-one GUI, dialogue and accessible puzzle presentation

```text
Implement release-one Prompt 7 in docs/release1-implementation-prompts.md after Prompts 4 and 5. Follow shared instructions/contracts, engine-skill and design.md. This can run alongside Prompt 6 using the allocated GUI/view modules and gallery harness.

Extend existing retained engine GUI rather than redesigning the working MVP. Present four cat identities and short dialogue, chapter progress, available requests, per-cat friendship milestones, all ten quest objectives/actions/hints, eight shop entries/details and compatible placement controls. Keep the garden and puzzle board dominant. Use clear names and actual typed command/read-model contracts; do not encode duplicate legality in UI or show synthetic purchases as working gameplay.

Provide visible symbol clues for bells, compatible shapes/rotations for cushion fitting, route connections and blocked reasons, held-switch states, and picnic shade/adjacency/quiet-zone clues. Every operation must have a keyboard-equivalent object/action path. No clue requires audio, hover, red/green discrimination or precise dragging. Maintain modal focus/return, cancellation, one gesture owner, larger text, reduced motion, save status and readable loading/failure feedback.

Create dialogue and completion copy for each quest and a short four-cat picnic closing presentation with continued play/replay. Preserve existing save/recovery and quality controls. Use real model portraits/thumbnails if available; before the asset handoff, use explicitly labeled development references only in the isolated harness.

Inspect the actual retained GUI at 1440x900, 390x844 and 844x390 with larger text and keyboard navigation. Check clipping, touch targets, state labels, contrast, focus and unobstructed legal targets. Complete representative new puzzle families through real intents in the harness. Provide shared registration patches and write docs/release1/prompt-07-handoff.md.
```

## Prompt 8 — Production audio and release-one sound catalog

```text
Implement release-one Prompt 8 in docs/release1-implementation-prompts.md after Prompt 1. Follow shared instructions/contracts and use engine-skill. This task can run alongside assets and rules; own only the allocated audio/catalog/import paths.

Inspect the existing audio runtime and 47-clip MVP production checklist. Reuse the activation, mixing, voice limits, loading budgets and settings rather than building another audio subsystem. Consolidate shared effects and extend only for the new interactions: crate/plank/cushion movement, bells, water flow/fill, extra cat personality cues, scratching/playhouse use and the picnic finale. Music should support the three chapters without a unique track for every puzzle; prioritize a coherent modest sound set.

Produce a cue/source manifest and exact generation/import briefs, including loops, variants, target duration, loudness/headroom, licenses and consumers. Retain the existing selected-provider requirements where still applicable. Generate or import final assets using actually available authorized providers or supplied originals; record real rights/provenance. If those inputs are unavailable, finish importer/wiring/catalog preparation and report the precise missing inputs. Do not substitute silent/test tones as production audio or claim listening acceptance.

Wire stable action/event cue contracts without editing another active task's shared composition. Trigger only committed actions, use cooldowns and cancel expired/stale effects across scene transitions. Bells remain fully solvable with visible symbols when muted. Keep Music/Ambience/Effects, mute, Enable sound and gesture resume behavior. Reuse budgets from current architecture; measure decoded final audio, not a zero-byte placeholder catalog.

Audibly inspect actual browser-backend playback, loop seams, mix balance, pause/background/resume and rapid transitions when final inputs exist. Deliver runtime files, provenance, manifest fragment, measured memory/voice evidence and docs/release1/prompt-08-handoff.md. Clearly separate complete wiring from final asset/listening acceptance.
```

## Prompt 9 — Integrate the complete release-one afternoon

```text
Implement release-one Prompt 9 in docs/release1-implementation-prompts.md after Prompts 2–8. Read all handoffs and follow shared instructions/contracts. Use engine-skill and pawmodels only for necessary asset integration/corrections.

Merge task-local modules, manifest fragments, build registration and composition patches into the actual production Paws target. Resolve contract mismatches at their owner boundaries, preserve current accepted assets and remove development stand-ins from production staging. Verify every stable asset URL/hash, relative-path loading and notices. Integrate all ten puzzle scenes and their actual final assets, four cats, chapters, friendship, eight purchases, garden placement/use, audio and responsive retained GUI.

Make the intended fresh-player journey coherent: meet Pip, complete the first chapter, meet Mabel/Button, solve chapter two in either order, introduce Pepper, finish bells/playhouse, solve the picnic, then show the once-only four-cat gathering after an owned furnishing is placed. Supply finale props regardless of picnic-item purchases. Keep shopping/moving/replay available afterward. Existing MVP saves resume with preserved purchases and rewards and a new release-one finale still ahead. Practice never grants new coins or friendship.

Validate all ten authored known-solution traces through the same production command path and exercise actual mouse/keyboard input plus touch-oriented layouts. Test alternate chapter orders, hint/undo/restart, interrupted quests, failed asset preparation, purchase/place cancellation, save failures/retry, export/import, second-tab ownership, reload, explicit pause/background return and renderer recovery. Check each furnishing's visible use and every puzzle family's actual feedback. Do not confuse scripted solutions with independent pacing acceptance.

Build against the exact final engine pin and run portable rules/scene/save checks and production-browser journeys. Fix integration defects rather than leaving a list for the next task. Missing final meshes/audio leave the candidate incomplete; do not erase those gates. Reconcile docs/architecture.md, relevant feature contracts and docs/implementation-status.md with delivered behavior. Write docs/release1/prompt-09-handoff.md with one reproducible current-site launch and exact evidence identities.
```

## Prompt 10 — First-player polish, acceptance and release package

```text
Implement release-one Prompt 10 in docs/release1-implementation-prompts.md after Prompt 9. Follow shared instructions/contracts and use engine-skill. Apply the polish-uat-loop skill for evidence-led user-flow review; use pawmodels if an observed asset defect needs correction.

Review the exact generated full game as a fresh player and as a migrated MVP player. Correct onboarding, confusing actions/clues, pacing, repetitive layouts, camera framing, contact/animation defects, dialogue, shop affordances and ending flow. Target 45–75 minutes without forced waits or grinding. Independent first-player sessions must measure actual discovery, hint usage and completion; automation cannot establish this. Prepare the concrete playtest script and capture form if human sessions are unavailable, and leave that gate open.

Close final audio/rights/listening and natural sleeping-curl acceptance on the shipped assets. Review keyboard-equivalent completion, larger text, reduced motion, mute, grayscale, shadow-free quality and narrow/short layouts. Perform a full-duration soak on the final package, with repeated puzzle/garden/shop/restore transitions and audio enabled. Measure realistic startup transfer, frame timing, memory/resource retention and audio budgets. Retest after material fixes; earlier payload evidence is historical.

Define the supported desktop browser/device matrix from actual evidence. Test supported physical combinations for at least 15 minutes, including audio, storage, suspend/resume and recovery. Advertise mobile support only for physically accepted combinations; keep untested combinations explicit and preserve the compatibility shell. Do not imply a general accessibility certification from scoped checks.

Produce a versioned pinned Release static-site package with consistent build/content IDs, required notices, correct asset hashes, excluded development previews, hosting instructions, save-migration notes, support matrix, known issues and a short release checklist. Verify the packaged site under a nested URL with production-style compression and repeat the full final journey/recovery checks against that exact payload. Prepare publication materials locally; do not publish without separate authorization.

Update README.md, docs/implementation-status.md and docs/release1/prompt-10-handoff.md. Mark production-ready only when required asset, player and supported-device gates actually pass. Otherwise deliver a clearly identified review candidate, completed independent work and precise remaining gates. Report the concrete release artifact and evidence, not only a proposal.
```

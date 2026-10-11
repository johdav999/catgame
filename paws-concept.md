I’d make the quests **small problems in the garden**, each taking roughly two to five minutes. Their layouts should be handcrafted, while their objects, behaviours, and visual assets come from a shared kit.

| # | Quest | What the player does | Reusable objects and mechanics |
|---|---|---|---|
| 1 | **Pip’s missing ball** | Move two pots and rotate a curved channel to roll his ball back to him. | Sliding obstacles, rotating channels, destination |
| 2 | **Mouse under the bench** | Slide crates in the right order to make space for a paw to retrieve a toy mouse. | Sliding obstacles, restricted paw reach, collectible |
| 3 | **The sunny spot** | Arrange short planks between platforms so Mabel can reach her favourite sunny perch. | Placement sockets, connected route, cat traversal |
| 4 | **A bed for Button** | Fit three differently shaped cushions into a little basket, leaving room for Button to curl up. | Shape fitting, placement sockets, completion pose |
| 5 | **The garden bells** | Tap hanging bells in the sequence shown by a picture clue. Pepper responds to the finished melody. | Ordered interactions, symbols, sound feedback |
| 6 | **Tidy the toy basket** | Match three scattered toys to their shaped compartments, moving a blocking cushion first. | Collectibles, matching sockets, sliding obstacle |
| 7 | **The stuck little gate** | Guide a ball onto a switch that holds the gate open, then retrieve the toy behind it. | Channels, weighted switch, linked gate, collectible |
| 8 | **A drink after playing** | Rotate wooden gutter sections to connect a rain barrel to an empty drinking bowl. | Connected route, rotating channels, fill animation |
| 9 | **Two kittens, one doorway** | Place suitable objects on two switches simultaneously to open a playhouse for the kittens. | Placement, weighted switches, combined conditions |
| 10 | **The garden picnic** | Arrange beds and bowls using clues: Mabel wants shade, Pip wants to sit beside Pepper, and Button wants a quiet corner. | Placement sockets, spatial relationships, multiple conditions |

Every quest awards coins and friendship. Selected milestones unlock related shop items: the playhouse, a nicer drinking bowl, a hammock, or a picnic set. Once purchased, those items become usable furnishings in the main garden.

**For the reusable system, I’d separate four responsibilities.** These are proposed additions, built on the engine’s existing actor/component and prefab support.

| Part | What it contains | What gets reused |
|---|---|---|
| **Quest definition** | Requesting character, dialogue, prerequisites, stages, objectives, hints, rewards | Objective types and quest structure |
| **Quest instance** | Current stage, progress, target bindings, puzzle state, reward status | The runtime that advances any quest |
| **Puzzle object prefab** | Components describing interaction, movement, connections, and logical state | Working objects across many layouts |
| **Presentation definition** | Models, materials, portraits, icons, animations, sounds, camera framing | The visual and audio kit |

A quest definition remains immutable. Each active quest gets its own mutable instance, so solving one puzzle cannot change another puzzle’s definition or shared assets.

**Objects gain capabilities through components.** A flowerpot, for example, could be an actor containing the existing `MeshComponent`, plus proposed interaction and sliding components.

| Proposed component | Responsibility |
|---|---|
| `InteractionComponent` | Exposes permitted actions such as push, turn, pick up, or place |
| `SlideComponent` | Moves between valid positions along a configured axis |
| `RotateComponent` | Turns between authored orientations |
| `SocketComponent` | Accepts objects with compatible shapes or capabilities |
| `RouteSegmentComponent` | Defines connection points and traversal rules |
| `SwitchComponent` | Maintains an activated state based on an occupant or interaction |
| `LinkedStateComponent` | Connects a switch’s state to a gate or another mechanism |
| `CollectibleComponent` | Records successful retrieval |
| `SequenceComponent` | Checks a configured sequence of successful interactions |

The puzzle controller validates a move and commits its logical result. Visual components then animate that result. This keeps puzzle correctness independent of animation speed, graphics quality, and camera position.

For the first version, sliding and rolling should use discrete, predictable rules. Water can follow a validated route with an animated ribbon and bowl-fill effect; it does not require fluid simulation.

**Quest objectives should refer to roles, rather than particular models.** A definition might say:

```text
Quest: pip_missing_ball
Character: pip
Layout: porch_ball_01

Roles:
  requested_item -> toy_ball_01
  destination   -> return_socket_01

Objective:
  requested_item occupies destination

Presentation:
  garden kit: terracotta
  item appearance: red_ball
  success reaction: happy_pounce

Reward:
  coins: 20
  friendship: 1
```

The runtime binds those authored object IDs to the existing scene-scoped actor/component handles. Another quest can reuse the same objective with a different layout, character, and compatible item.

Start with a small set of typed objective checks: **object in socket, item collected, route connected, state activated, sequence completed, and spatial relationship satisfied**. Combine them using **All**, **Any**, or ordered stages. Each objective explicitly chooses whether success stays recorded or must still be true—for example, both switches must remain pressed together.

**Graphics should be reusable at three levels.**

- **Shared assets:** meshes, textures, materials, cat skeletons, animation clips, sounds, and effects are loaded once and shared.
- **Visual variants:** compatible props use material changes or interchangeable visual children. A sliding obstacle might appear as a pot, crate, or cushion, provided its authored footprint and interaction rules remain valid.
- **Scene arrangements:** each puzzle layout places those prefabs, binds their roles, and supplies camera framing, lighting, and decorative surroundings.

A useful initial art kit would include one cat rig with coat variants; a ball and toy mouse; pots and crates; straight and curved channels; planks and platforms; switches and a gate; baskets, cushions, and bowls. Reuse paw gestures, happy reactions, interaction highlights, and success effects across quests.

Give important objects clear silhouettes and symbols as well as colour differences. Decorative scenery should leave the puzzle’s usable objects easy to recognise on a small screen.

**In WebEngine, I’d keep the quest runtime and puzzle components in an application-owned module initially.** Register the components through the existing `ComponentFactoryRegistry` and construct objects with the existing [prefab system](C:/Users/Johan/source/repos/WebEngine/docs/prefabs.md). Cat personalities, shop prices, friendship rules, and quest content belong to the game. Generic capabilities can later become optional reusable modules when another application needs them.

One application-owned quest service should process committed puzzle actions. Scene components retain safe handles; queued work includes the quest instance and attempt identity so restarting a puzzle rejects old actions. Unloading a quest cancels its loading requests and presentation work.

Save stable quest/object identities, puzzle state, friendship, purchases, and reward records together. Reward grants must be idempotent so reopening a completed quest cannot award its coins twice. The engine’s existing persistence can carry that state, but file-based quest definitions need an application schema and loader; prefab factory registration alone does not provide serialization.

I’d prototype quests **1, 6, and 7** first. Together they exercise sliding, routing, matching, switches, rewards, and visual reuse without requiring the entire quest catalogue.
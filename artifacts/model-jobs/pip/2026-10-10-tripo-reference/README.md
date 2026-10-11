# Pip Tripo reference

Created 10 October 2026 using OpenAI built-in ImageGen, followed by one fur simplification edit. Input appearance reference: docs/design/paws-style-reference.png, with design.md and docs/mvp-implementation-prompts.md as the design authority. AI-generated output; no third-party asset license asserted.

Upload pip-tripo-reference.png as a single image-to-3D reference. This is a design reference, not a mesh, rig, skin-weight file, animation, or accepted production portrait. Image inspection confirms a complete standing cat, four visible separate paws, smooth silhouette and tail clear of the torso. Tripo generation and rigging have not been executed or verified.

Generate the mesh, inspect that legs/tail are not fused, and use the quadruped rigging workflow. Validate skin deformation at shoulders, hips, paws and tail. Preserve one final skeleton and bind pose as the canonical rig; render consistent portraits from that final model. Auto-rigging does not establish compatibility with Paws automatically.

Required later actions from the project brief: relaxed idle/slow blink, attentive request/head tilt, happy pounce/chirrup, short garden walk, settle into cushion, curled rest, approach/lower head to bowl, drink and return to idle. These have not been created by this image task.

Tripo guidance: https://www.tripo3d.ai/blog/prepare-creature-mesh-before-auto-rigging

## Initial prompt

Use case: stylized-concept.
Create ONE high-resolution standalone full-body 3D character reference image for image-to-3D generation in Tripo. Subject: Pip, the ginger-and-cream domestic cat from the supplied Paws style board. Use the board only for the cat's identity and warm softly sculpted art direction; remove all garden scenery, UI, typography, other characters and objects.

Show exactly ONE cat, once, centered on a plain light neutral gray studio background. Entire ears, all four paws and entire tail visible with generous margins. Landscape 4:3 composition. Near-orthographic front three-quarter view, camera at chest height, facing toward image left, enough side view to clearly see the torso length and hips, and enough front view to see the appealing face. No perspective exaggeration.

Pip has natural quadruped feline anatomy, gently enlarged round head, triangular ears with muted pink inner ears, short cream muzzle, small dusty pink nose, amber eyes with natural dark pupils, warm ginger coat with simple broad darker orange tabby markings, a clear forehead M marking, symmetric cheek stripes, cream chin and chest bib, cream toes, and a substantial ginger ringed tail with a cream tip. Rounded body, broad soft paws, identifiable shoulders, hips, elbows and hocks. Friendly calm closed-mouth expression. Eyes expressive but not enormous or glassy.

RIGGING REFERENCE POSE: stand neutrally on all FOUR paws with body horizontal, head facing along the body without tilt, legs gently spaced apart and straight but anatomically natural, belly well clear of the ground. All four separate limbs and paws must be individually readable in silhouette and clearly attach to the correct shoulders or hips; choose camera and mild natural stance width so near and far legs do not visually merge. Tail rises behind the rump with a gentle open curve, clearly separated from back, body and legs by background space; no tight curl and no tail touching the body except its base. No sitting, crouching, jumping, raised paw, crossed legs, anthropomorphic human T-pose or accessories.

Render as a polished stylized game character sculpt: cohesive smooth fur masses, subtle broad cheek/chest tufts, matte nonmetallic surface, coat stripes as flat color markings. No individual fur strands, long whiskers, hair cards, wires, disconnected parts, visible skeleton or topology overlay. Soft even neutral studio lighting, readable underbelly and all limb joints, only minimal soft contact shadow, no pedestal or modeled floor, no dramatic shadows or baked ambient occlusion. Sharp whole subject, no depth of field. No text, labels, watermark, montage, turnaround sheet or portrait inset. The only subject is one complete standing cat suitable for subsequent quadruped skeleton binding.

## Final edit prompt

Revise this exact Pip cat reference. Preserve the identity, coat marking locations, colors, proportions, standing four-legged neutral pose, all four separately visible paws, open upright tail silhouette, camera, framing and plain gray studio background. Change ONLY the fur treatment: eliminate every individual hair strand, fuzzy surface and fine strand edge. Replace with smooth matte continuous sculpted surfaces, like an expertly modeled stylized game character with painted ginger tabby markings. Use only a few broad rounded sculpted cheek and chest lobes. The body, legs and tail should have smooth clean silhouettes, with no noisy fuzz. Remove long thin whiskers completely; small muzzle dots suffice. No hair cards, no furry groom, no dense tiny carved fur grooves. This should read as a beautiful smooth shaded 3D game mesh, not a furry plush or realistic fur render. Preserve all four limbs and anatomy. Keep the whole cat sharp and isolated, no text or extra objects.


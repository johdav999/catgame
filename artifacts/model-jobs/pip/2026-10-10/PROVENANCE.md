# Provenance and material records

All geometry, UV layout, rig, weights, actions and portrait cameras were authored
for this Paws request using the retained Blender Python sources. No downloaded cat
mesh, stock animation or engine demo asset is included. Appearance follows the
repository's `design.md` and `docs/design/paws-style-reference.png`.

The ginger surface input was generated with the built-in OpenAI ImageGen tool
on 10 October 2026 in this task. Its original returned image is retained at
`sources/ginger-color-imagegen.png`; native dimensions and SHA-256 are recorded
in `asset-manifest.json`. The generator returned a flat ginger colour swatch,
not a cat photograph, anatomical atlas or measured material scan.

Prompt (new-image generation, no input image):

> Create a square 1024x1024 seamless base-color texture swatch for Pip, an appealing softly sculpted ginger tabby cat in a cozy miniature garden 3D game. This is a material input, NOT a cat picture. Fill entire frame with uniform warm golden ginger short coat color (#D69A50 family), very subtle gently directional tiny short fur color wisps, restrained broad natural tonal variation. Texture physically covers 0.35 metre square. Flat albedo artwork, no anatomy, no eyes, no stripes (tabby stripes and cream landmarks will be authored as separate geometry-aware masks), no shadows, no lighting gradients, no highlights, no ambient occlusion, no perspective, no frame, no text. Opposite edges tile continuously. Soft painterly tactile surface, low contrast, not noisy or photorealistic.

Transformation: the original colour input supplies 15% of the coat's colour
variation in an object-space mapping. Explicit tabby stripes, forehead markings,
cream muzzle/bib/toes and facial colours are evaluated into the unique 1024-square
UV atlas. Atlas edges receive eight-pixel dilation. A linear-to-sRGB correction
was inspected against the original material after revision 3 darkened on export.
No directional light or cast shadow is baked into albedo.

Material exceptions: eyes, nose, ear interiors and mouth use authored flat
colours within that same atlas; a generated image adds no useful information to
these tiny shapes. Roughness is independently authored uniform 0.86 and metallic
is zero. The +Y tangent normal input is neutral: the requested soft coat relief
comes from geometry, not fake stripe-height or strand-fur shading. Export packs
roughness/metallic according to glTF conventions. Portable source maps remain
in `textures/`; embedded copies are consumed by the shipped GLB.

Rights record: project-specific authored work and AI-generated source from this
user-requested workflow. No third-party artwork license is asserted or required
by an imported stock asset. The service did not expose a separate generation
license document or account entitlement record; none has been fabricated. The
generation record and original image are retained for the project's release review.

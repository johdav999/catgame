# Terrain texture provenance

Created 15 October 2026 for Paws using OpenAI's built-in ImageGen tool. These
are original AI-generated outputs from text prompts, with no supplied external
images or third-party texture inputs. No third-party asset license is asserted.
The PNGs are copied unchanged from the generated outputs, including metadata.

`meadow-v1.png` source: `exec-a0d80f5e-499e-4ac2-89c4-b669ca3400c6.png`.
`earth-v1.png` source: `exec-0b37ece1-23e6-42a1-a843-12a8c6a98c1a.png`.

Intended use: repeating albedo layers on WebEngine terrain, with a game-authored
linear blend-weight texture. These are base colors, not height or normal maps.
The adjacent garden texture catalogue records runtime byte counts and SHA-256.

## Exact meadow prompt

Create a seamless tileable square terrain base-color texture for the Paws cosy garden game. Closely cropped overhead orthographic scan of a continuous short meadow grass surface: subdued sage and moss greens, dense fine short blades softly grouped, occasional tiny dry straw, very subtle natural organic color variation. Softly stylized tactile storybook 3D material, grounded and believable, matte. Quiet low contrast to support cream house and terracotta garden props. No flowers, clover clusters, rocks, objects, paths, large patches, borders, letters, text, perspective, directional lighting, shadows, highlights or ambient occlusion. Entire image is grass. Albedo only under perfectly even diffuse lighting. All four edges must tile seamlessly, equal brightness at edges. 1024x1024 texture. Save for terrain-meadow-v1.

## Exact earth prompt

Create a square seamless tileable terrain albedo texture: a quiet weathered sandy limestone and warm dry earth surface for a cosy stylized 3D garden's distant rolling hills. Fine cream-beige sandstone grit mixed with muted warm brown loam, scattered small rounded pale stone flecks, broad soft natural mottling with restrained contrast. Pure top-down orthographic flat surface scan, all edges seamless and equally lit. Softly sculpted tactile storybook realism, matte, neutral even diffuse lighting. Base-color only, no shadows or directional light or highlights or ambient occlusion. No grass tufts, plants, large rocks, objects, footprints, paths, cracks with black lines, borders, text or labels. Entire image continuous fine ground material. Intended as second material blended subtly into sage meadow terrain. 1024x1024. Save for terrain-earth-v1.

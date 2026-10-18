# Flowerpot weight tags

Original Paws UI artwork implementing the approved ImageGen reference in
`reference.png`. Cream labels, evergreen rounded rims, a balance-weight icon and
large Nunito Sans Semibold numerals identify weights 1–4. Tags tilt toward the
normal elevated puzzle camera and retain a short wooden stem above each pot.

Regenerate the four PNGs and embedded resources with
`python scripts/generate-weight-tags.py` (Pillow required). The font is the
repository's existing licensed `assets/runtime/fonts/NunitoSans-Semibold.ttf`.
The PNGs are embedded in `weight_tags.inc` and decoded through WebEngine's
texture API. They follow the pot's existing transform during dragging and
placement; puzzle rules and save data do not depend on the artwork.

`reference.png` was generated with the built-in ImageGen tool from the user's
game screenshot on 18 October 2026. Runtime textures are original deterministic
UI artwork; no new imported 3D asset is required.

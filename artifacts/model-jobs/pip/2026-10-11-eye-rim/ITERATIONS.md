# Inspected correction iterations

1. **r16, removal isolation:** deleting only the added margin removed the ring.
   Closure still occluded the eyes, but the closed crease became weak. Retained
   as a diagnostic reference, not promoted.
2. **r17-r19, recessed full-width strip:** tested a recessed strip with blended
   upper/lower weights, then a skin-facing transition. Front outline improved,
   but side fragments remained; the wider tucked ribbon was rejected.
3. **r20, unfolding microfold:** tucked open geometry and mixed existing lid
   influences preserved the accepted crease while removing the raised loop.
   Pixel comparison still found shading changes caused by averaging the fold
   normals into facial skin.
4. **r21, independent facial normals:** retained the facial surface normals while
   the interior fold uses its own normals. Inspected three-view open/mid/closed
   runtime captures, source and clean-export renders. The open rim regression,
   full closure sweep, exact animation preservation and native regressions pass.

Only r21 is promoted. All existing facial skin positions/weights and authored
animation curves are preserved; changes are confined to the narrow added seal
and its shading interface.

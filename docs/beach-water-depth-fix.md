# Distant water colour cutoff — 2026-10-19

The low-angle beach view exposed a hard horizontal transition between two
water colours. This was not a terrain texture seam: the seabed crosses the
80 m camera far plane while water in front remains visible. Both shaders
previously returned ordinary surface shading immediately for clear opaque
depth; pixels just inside the boundary still used absorption/refraction.
Those two results differ, producing the line.

The isolated engine fix fades refraction and depth foam to surface shading
between 75% and 98% of the capture's reconstructed linear view-distance range.
Nearshore depth effects and foreground rejection remain. No beach geometry,
camera framing, material settings or puzzle state changed for this fix.

Engine branch: `codex/water-depth-transition`, commit `bddec8b`, based on the
existing Paws pin `0426d9319de715584a2e34ff9862cd6ba873ecd0`. Source checkout:
`build/engine-water-depth-transition/`. The reusable WGSL and GLSL changes,
engine architecture/water notes and validation record are committed there.
Both installed and repository engine-skill copies document the changed contract.

Publishing the branch was rejected by automatic approval review: exporting
repository code to the remote requires specific user authorization. No push
occurred and the game gitlink remains unchanged. The local preview uses the
explicit local-engine build `build/web-water-depth-release/site`; after push
approval, publish the branch, fetch the commit into `external/WebEngine`,
advance the gitlink deliberately, rebuild with `-Pinned`, and rerun the focused
water probe. The adjacent engine checkout's unrelated changes were preserved.

## Evidence

`scripts/beach-water-seam-smoke.cjs` captures actual Chrome WebGPU and forced
WebGL2 at elevations 0.1, 0.2, 0.3 and 0.5 radians. `ASSERT_SEAM_FIXED=1` checks
the averaged open-water strip at the reproduced cutoff. Maximum adjacent-row
channel jumps fell from 15.47 to 2.59 (WebGPU) and 12.10 to 3.57 (WebGL2), below
the threshold of 8. Before/after images were inspected and both renderers had
zero page/console errors.

Artifacts: `build/evidence/beach-water-seam/`, with `before-*` and `candidate-*`
folders. The isolated candidate Wasm SHA256 is
`e245a6b3a1d45fd2670a63eb56c2440a1d95847854208dbb8d7f7232094448ce`.
That initial candidate was built before the shader changes were committed, so
its metadata identifies the checkout's base revision. Subsequent local builds
identify engine commit `bddec8b`; the game's gitlink remains unchanged. The
actual engine source is the isolated checkout above.
These are desktop browser checks, not physical-mobile acceptance.

The complete first-quest acceptance probe also passed on both renderers
(`gameplay-webgpu/report.json`, `gameplay-webgl2/report.json`): animated and
paused water, shoreline/refraction configuration, exact loaded coastal model
bytes, cancelled drag, Undo, portrait solve, saved-board reload, garden return
and access restoration. Both reported zero page/console errors and fetched the
same candidate Wasm on every reload. The local preview at port 8080 now serves
this candidate; the published/pinned update remains pending approval above.

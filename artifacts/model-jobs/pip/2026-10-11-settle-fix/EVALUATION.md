# Evaluation

| Check | Evidence and outcome |
| --- | --- |
| Root cause reproduced | Original r8 Blender rest render reproduces the user's collapsed neck and high rump; original GLB fails the new connected-joint regression. |
| Authoring | r9 front/side/rear/top inspected. Neck length and bind preserved; forelegs relax forward and haunch lowers. Low rest rather than tight curl. |
| Export | Packed master saved and GLB clean-reimported; `clean-reimport.json`: 17,854 triangles, 29 bones, 2 materials, 9 clips, max 4 weights. `clean-*.png` inspected. |
| Native final merged asset | Settle floor minimum 0.000253428 m; rest floor 0.00299699 m; maximum 60 Hz vertex step 0.0249002 m. Joint connectivity, scales, timing, seam and ground checks pass. |
| Existing regressions | Full Pip asset test (all 9 actions, root, loops, skin, reduced motion/lifecycle) and portable scene test pass. |
| Runtime | Pinned WebGPU viewer built; Chrome 154.0.8037.99. Six observed transition times from 0.2 to 2.4 seconds; four endpoint orbit views and matching rest. No console/page errors. `browser/review.json`. |
| Cushion | Real garden-kit preview loaded the final GLB on the authored cushion. Body, forepaws and tail fit the cushion footprint; `browser/cushion-fit.png` inspected. This is visual contact evidence, not a cloth simulation. |
| Preservation | Channel-level merge asserts untouched mesh/material/skin/node data and all other animations. Current eyelid channels are retained. |
| Remaining limits | Low rest is not a tightly curled sleeping pose. Original eyelid cap fit is separate work. Physical mobile and broad game interaction acceptance are not established here. |

The initial isolated site copy stopped at a locked unrelated garden file, causing
404s. Required viewer files were staged explicitly and the successful browser
review was rerun. Browser video capture was unavailable because Playwright's
FFmpeg package is absent; the retained evidence is timed rendered frames, not video.

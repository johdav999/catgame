# Inspected iterations

1. Original repair: inflated orange caps and flattened-sphere closure were rejected
   by the user. Removed them while preserving the original head/eye surface.
2. Revisions 1–14: fitted shells, replacement socket rings and retopology trials.
   Rejected visible open-pose seams, material patches and poor side fit.
3. Revisions 15–19: original facial skin and eye islands improved continuity.
   Scale-based closure looked usable in Blender but produced severe jagged
   shading in WebGPU. Rejected against the actual renderer.
4. Revisions 20–22: two additional lower-lid controls allowed pure translation
   with unit scales. WebGPU shading improved; a remaining added rim was visible
   with open eyes and was rejected.
5. Revision 23: removed the rim, retained source UVs and coincident open-pose
   coordinates/normals. Rest appearance matched original; oblique closed views
   still exposed a narrow eye sliver.
6. Revision 24: small 2 mm closure overlap seals the aperture. Inspected actual
   WebGPU open, half, closed, reopening and side views; reimported both formats,
   verified original rig binds and preserved walk/settle data. Accepted delivery.

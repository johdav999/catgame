"""Archive final browser evidence and exact build/content hashes after acceptance."""
import hashlib,json,shutil
from pathlib import Path
JOB=Path(__file__).resolve().parents[1];ROOT=JOB.parents[3]
evidence=JOB/'browser-evidence';evidence.mkdir(exist_ok=True)
for p in (ROOT/'build/evidence/garden').glob('*'):
    if p.name!='failure.png':shutil.copy2(p,evidence/p.name)
paths=['assets/runtime/models/garden/asset-manifest.json','assets/runtime/models/pip/pip.glb','build/web-garden-pinned-release/site/garden_preview.js','build/web-garden-pinned-release/site/garden_preview.wasm','src/garden_kit.cpp','src/garden_manifest.inc','src/garden_preview_main.cpp','web/garden-preview.js','scripts/garden-smoke.cjs']
report={'engine_pin':'3fe7d1dc1dc610c703944803cbb717cd65024ec9','content':'garden/kit@1 revision 7','date':'2026-10-10','pip_revision':'pip-tripo-repaired-r7','browser':'Chrome 154.0.8037.99','viewport_css':[[1440,900],[390,844],[844,390]],'device':'Windows desktop; emulated viewport sizes, not physical mobile','files':[]}
for s in paths:
    p=ROOT/s;report['files'].append({'path':s,'bytes':p.stat().st_size,'sha256':hashlib.sha256(p.read_bytes()).hexdigest()})
(JOB/'validation-build.json').write_text(json.dumps(report,indent=2)+'\n')
m=json.loads((JOB/'asset-manifest.json').read_text());rows=['# Measured asset budgets','','All triangles include modeled rims, cloth seams, foliage and modifier results.','Vertices are exported vertices including UV/normal splits, not editable mesh vertices.','','| Asset | Triangles | Vertices | Materials / primitives | GLB bytes |','| --- | ---: | ---: | ---: | ---: |']
for a in m['assets']:rows.append(f"| {a['file']} | {a['triangles']:,} | {a['vertices']:,} | {a['material_count']} / {a['primitives']} | {a['bytes']:,} |")
rows+=['',f"Total: {m['totals']['triangles']:,} triangles, {m['totals']['vertices']:,} vertices, {m['totals']['bytes']:,} GLB bytes.",'','Six 512-square base-colour maps: 6 MiB of unique RGBA8 colour pixels before mips.','Embedded self-contained GLBs duplicate some maps. Do not assume cross-file GPU','deduplication. Actual renderer counters including Pip and review paving are','in browser-evidence/garden-smoke.json; total process/Wasm heaps are not measured.']
(JOB/'BUDGETS.md').write_text('\n'.join(rows)+'\n')
print('Archived evidence and hashes for',len(paths),'files')

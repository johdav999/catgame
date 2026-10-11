"""Promote the inspected kit revision and record reproducible asset budgets."""
from pathlib import Path
import hashlib,json,struct,shutil,sys
JOB=Path(__file__).resolve().parents[1];ROOT=JOB.parents[3]
revision=int(sys.argv[1]) if len(sys.argv)>1 else 7
source=JOB/'exports'/f'r{revision}';target=ROOT/'assets/runtime/models/garden'
manifest=json.loads((source/'asset-manifest.json').read_text())
def digest(p):return hashlib.sha256(p.read_bytes()).hexdigest()
manifest.update({'schema':'paws-garden-kit/1','source':str((JOB/f'garden-kit-r{revision}.blend').relative_to(ROOT)).replace('\\','/'),'source_sha256':digest(JOB/f'garden-kit-r{revision}.blend'),'engine_pin':'3fe7d1dc1dc610c703944803cbb717cd65024ec9','pip_fit_sha256':digest(ROOT/'assets/runtime/models/pip/pip.glb'),'normal_convention':'OpenGL +Y; neutral maps, relief is geometry','rights':'Original code-authored models with OpenAI built-in ImageGen surface colour. No third-party texture license asserted. See NOTICE.md.','texture_edge_max':512})
for a in manifest['assets']:
    p=source/a['file'];b=p.read_bytes();n=struct.unpack_from('<I',b,12)[0];g=json.loads(b[20:20+n]);a['vertices']=sum(g['accessors'][prim['attributes']['POSITION']]['count'] for mesh in g['meshes'] for prim in mesh['primitives']);a['material_count']=len(g.get('materials',[]));a['primitives']=sum(len(m['primitives']) for m in g['meshes']);a['sha256']=digest(p);a['bytes']=len(b);shutil.copy2(p,target/a['file'])
manifest['totals']={field:sum(a[field] for a in manifest['assets']) for field in ('vertices','triangles','bytes','primitives')}
manifest['source_maps']=[{'file':str(p.relative_to(JOB)).replace('\\','/'),'bytes':p.stat().st_size,'sha256':digest(p)} for p in sorted((JOB/'maps').glob('*.png'))]
manifest['generated_color_input']={'file':'sources/material-atlas.png','sha256':digest(JOB/'sources/material-atlas.png'),'native_dimensions':[1536,1024],'tiles':[512,512],'prompt':'sources/material-atlas.prompt.txt','families':['stone','terracotta','wood','cloth','wicker','sage_cloth']}
for p in (target/'asset-manifest.json',JOB/'asset-manifest.json'):p.write_text(json.dumps(manifest,indent=2)+'\n')
print(json.dumps(manifest['totals']))

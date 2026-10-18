"""Record supplied inputs and frozen authorities without modifying either."""
import hashlib,json,shutil
from pathlib import Path
ROOT=Path(__file__).resolve().parents[3]
JOB=ROOT/'artifacts/model-jobs/release1-cats'
MAPPING={'button':'button','mabel':'pepper','pepper':'mabel'}
def stamp(path):
    data=path.read_bytes()
    return {'path':path.relative_to(ROOT).as_posix(),'bytes':len(data),'sha256':hashlib.sha256(data).hexdigest()}
def main():
    contract=json.loads((ROOT/'docs/release1/contract.json').read_text())
    metadata=json.loads((ROOT/contract['rig']['metadata']).read_text())
    authority=ROOT/metadata['source_master']
    records={'schema':'paws-cat-intake/1','date':'2026-10-11','identityDecision':
             'User requested original looks and intent: grey tabby is Mabel; left-eye patch is Pepper.',
             'sourceMapping':MAPPING,'authority':{'glb':stamp(ROOT/contract['rig']['authority']),
             'master':stamp(authority),'rigSnapshot':stamp(ROOT/contract['rig']['bindSnapshot'])},'cats':{}}
    JOB.mkdir(parents=True,exist_ok=True)
    for cat,source in MAPPING.items():
        folder=JOB/cat
        for name in ['intake','references','renders','exports','textures','logs']:(folder/name).mkdir(parents=True,exist_ok=True)
        # Each immutable original stays in the user's folder; inventory retains
        # exact original paths even where supplied labels were reversed.
        records['cats'][cat]={'sourceFolder':'assets/'+source,'files':[stamp(p) for p in sorted((ROOT/'assets'/source).rglob('*')) if p.is_file()],
            'reference':stamp(ROOT/f'artifacts/model-jobs/cat-tripo-inputs/2026-10-11/{cat}.png'),
            'provider':'Tripo3D, user supplied FBX export','rightsStatus':'User attests Tripo3D Pro plan on 2026-10-11',
            'rightsEvidence':{'kind':'direct user statement','date':'2026-10-11','text':'i have a pro plan of tripo3dai'}}
        shutil.copy2(ROOT/f'artifacts/model-jobs/cat-tripo-inputs/2026-10-11/{cat}.png',folder/'references'/f'{cat}-tripo-input.png')
        (folder/'intake/README.md').write_text(f'''# {cat.title()} intake

Received 11 October 2026 from `{records['cats'][cat]['sourceFolder']}`.
The FBX and all five supplied maps are retained untouched at their original paths.
The source ZIP is also retained in `assets/`. `../references/{cat}-tripo-input.png`
is a reference image, not geometry or evidence of the hidden side.

Use `../../intake.json` for the exact source-to-character mapping and hashes.
The files originally labelled Mabel/Pepper were reversed. The user explicitly
requested the original visual identities on 11 October 2026.

## Identity inspection

- Four connected legs, broad paws, full substantial tail, softly rounded face.
- {'Charcoal coat, cream muzzle, bib and paws; readable face in shade.' if cat=='button' else 'Cool grey tabby, cream muzzle/chest and subtle darker stripes.' if cat=='mabel' else 'Cream coat and charcoal patches; dark anatomical LEFT eye/ear, cream RIGHT eye region.'}
- Compare front, both sides, back, elevated puzzle view and final portraits.
- Provider rig is absent. Canonical rig fitting and skinning are required.

## Additional exports if supplied later

Prefer a textured binary GLB with embedded PNG/JPEG maps, plus original source,
FBX and provider export. Keep neutral quadruped geometry and separated feet.
The user has confirmed a Tripo3D Pro plan; that direct statement is recorded in
PROVENANCE.md. No receipt or account-specific terms were supplied. Preserve any
later rights documentation alongside this record. Do not replace original bytes.

The initial source-inspected.blend and source-inventory.json retain the original
folder labels. For Mabel/Pepper, select the source through the mapping above; the
final character-named masters, portraits and exports use the corrected identity.
''',encoding='utf-8')
    (JOB/'intake.json').write_text(json.dumps(records,indent=2)+'\n')
    rig=dict(contract['rig']);rig.update(schema='paws-release1-cat-rig/1',authorityFiles=records['authority'],
        sourceAxes={'up':'+Z','forward':'-Y'},runtimeAxes={'up':'+Y','forward':'+Z'},
        constraints='31 canonical deform bones; no bind edits, root motion, morph tracks, runtime IK or nonunit animated scales',
        personalityAction='personality_reaction',productionRegistration='Prompt 9 only')
    (JOB/'rig-contract.json').write_text(json.dumps(rig,indent=2)+'\n')
    shutil.copy2(ROOT/contract['rig']['bindSnapshot'],JOB/'canonical-rig.json')
    shutil.copy2(ROOT/'docs/design/paws-style-reference.png',JOB/'paws-style-reference.png')
    (JOB/'PROVENANCE.md').write_text('''# Cat production provenance

The user supplied three Tripo3D FBXs, original ZIPs and companion base-colour,
normal, roughness, metallic and packed RM maps on 11 October 2026. Exact original
paths, byte lengths and SHA-256 hashes are in [intake.json](intake.json).
No geometry provider was invoked by this task. The existing ImageGen reference
images and their original prompt record are retained in
`../cat-tripo-inputs/2026-10-11/provenance.md`.

The user confirmed original character identities: `assets/pepper` supplies Mabel,
`assets/mabel` supplies Pepper, and `assets/button` supplies Button. Original
directories are unchanged. Mesh UVs and coat designs are retained. Portable maps
are derived at 1024 square; metallic is set to zero for a matte nonmetallic coat.
The supplied normal map uses reduced strength after visual inspection.

Rig and base animations derive from the repository's accepted Pip master named
in `rig-contract.json`, including the current walk, settle and eyelid revisions.
Task scripts author new skin weights, mesh-specific eye closures/contact poses
and additional actions. Final portraits must be rendered from those models.

Pip's existing source and rights record remains in
`assets/runtime/models/pip/NOTICE.md` and its manifest, including the accepted
walk/eye revision provenance. These notices are not replaced by this task.

The user stated on 11 October 2026: "i have a pro plan of tripo3dai".
This direct statement records the declared provider plan for the supplied assets.
No receipt, generation date or account-specific terms were supplied. The record
does not invent a third-party license or independently verified entitlement.
''',encoding='utf-8')
    print('Recorded three source inventories, identity mapping and canonical authority hashes')
if __name__=='__main__':main()

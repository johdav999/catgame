"""Validate frozen release contracts against the actual MVP and task ownership.

Run from any directory; no third-party dependencies. Optional fragments are
checked against actual runtime bytes without merging production registration.
"""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess

ROOT = Path(__file__).resolve().parents[2]


def require(value, message):
    if not value:
        raise ValueError(message)


def validate(contract):
    quests = {q['id']: q for q in contract['quests']}
    cats = {c['id']: c for c in contract['cats']}
    items = {i['id']: i for i in contract['items']}
    require(len(quests) == len(contract['quests']) == 10, 'Ten unique quests required')
    require(len(cats) == len(contract['cats']) == 4, 'Four unique cats required')
    require(len(items) == len(contract['items']) == 8, 'Eight unique items required')
    require(len({q['contentId'] for q in quests.values()}) == 10, 'Content IDs collide')
    visiting, visited = set(), set()

    def visit(key):
        require(key in quests, f'Unknown prerequisite: {key}')
        require(key not in visiting, f'Cyclic progression: {key}')
        if key in visited:
            return
        visiting.add(key)
        for dependency in quests[key]['requires']:
            visit(dependency)
        visiting.remove(key)
        visited.add(key)

    distribution = dict.fromkeys(cats, 0)
    for q in quests.values():
        visit(q['id'])
        require(q['requesters'] and len(set(q['requesters'])) == len(q['requesters']), 'Duplicate requester')
        for cat in q['requesters']:
            require(cat in cats, f'Unknown cat {cat}')
            distribution[cat] += 1
    for cat, total in distribution.items():
        spec = cats[cat]
        require(total == spec['maximumFriendship'], f'Friendship distribution mismatch: {cat}')
        require(spec['milestones'] == sorted(set(spec['milestones'])) and
                spec['milestones'][0] > 0 and spec['milestones'][-1] == total, 'Invalid milestones')
    require(sum(i['price'] for i in items.values()) == 190, 'Catalog cost must be 190')
    require(contract['rewards']['coinsPerQuest'] * len(quests) == 200, 'Reward total must be 200')
    require(not contract['rewards']['purchaseRequiredForQuest'], 'Purchase-gated quest')
    content = (ROOT / 'src/content.cpp').read_text()
    for quest, content_id in [('pip_missing_ball', 'missing-ball-v1'),
                              ('pip_tidy_toys', 'tidy-toys-v1'),
                              ('pip_stuck_gate', 'stuck-gate-v1')]:
        require(quests[quest]['contentId'] == content_id and
                f'"{quest}"' in content and f'"{content_id}"' in content, 'Legacy quest drift')
    furnishing = (ROOT / 'src/furnishings.hpp').read_text()
    for key, price in [('garden_cushion', 30), ('drinking_bowl', 25)]:
        require(items[key]['price'] == price, 'Legacy price drift')
        require(re.search(r'\{"' + key + r'"[^\n]*,' + str(price) + r',', furnishing), 'Legacy source drift')
    for spot in contract['placement']['legacySpots']:
        require(f'"{spot}"' in furnishing, 'Legacy spot drift')
    allowed_roles = set()
    for roles in contract['placement']['legacyRoles'].values():
        allowed_roles.update(roles)
    for spot in contract['placement']['newSpots']:
        allowed_roles.add(spot['role'])
    for item in items.values():
        require(set(item['spotRoles']) <= allowed_roles, 'Unknown placement role')
        require(item['unlock'] in quests or item['unlock'] == 'chapter2_complete', 'Unknown item unlock')
    pin = subprocess.check_output(['git', '-C', str(ROOT / 'external/WebEngine'), 'rev-parse', 'HEAD'], text=True).strip()
    require(pin == contract['enginePin'], 'Unexpected engine pin')
    rig = json.loads((ROOT / contract['rig']['metadata']).read_text())
    require(set(contract['rig']['commonActions']) <= set(rig['clips']), 'Canonical action drift')
    bind = json.loads((ROOT / contract['rig']['bindSnapshot']).read_text())
    require(bind['sourceSha256'] == hashlib.sha256((ROOT / bind['source']).read_bytes()).hexdigest(),
            'Canonical authority changed; review the numerical bind contract')
    require(len(bind['joints']) == 31 and len({j['name'] for j in bind['joints']}) == 31,
            'Invalid canonical joint snapshot')
    require(set(j['name'] for j in bind['joints']) == set(rig['bones']), 'Canonical bone drift')
    paths = []
    for track in contract['tracks']:
        for field in ['source', 'tests', 'scripts', 'web', 'jobs', 'runtime', 'fragment']:
            if field in track:
                path = Path(track[field])
                require(not path.is_absolute() and '..' not in path.parts, 'Nonportable ownership path')
                for owner, previous in paths:
                    require(not (path == previous or previous in path.parents or path in previous.parents),
                            f'Ownership overlap: {owner}/{track["name"]}: {path}')
                paths.append((track['name'], path))
    return {'schema': contract['schema'], 'quests': 10, 'cats': 4, 'items': 8,
            'friendship': distribution, 'catalogueCost': 190, 'enginePin': pin,
            'disjointTracks': len(contract['tracks'])}


def validate_fragment(path, contract):
    fragment = json.loads(path.read_text())
    for field in contract['manifestFragment']['requiredFields']:
        require(field in fragment, f'Missing fragment field: {field}')
    require(fragment['schema'] == 'paws-release1-fragment/1', 'Unsupported fragment schema')
    track = next((t for t in contract['tracks'] if t['prompt'] == fragment['task'] and 'runtime' in t), None)
    require(track is not None, 'Unknown asset track')
    require(fragment['namespace'] == 'release1/' + track['name'], 'Incorrect fragment namespace')
    ids = set()
    for asset in fragment['assets']:
        for field in contract['manifestFragment']['assetRequiredFields']:
            require(field in asset, f'Missing asset field: {field}')
        require(asset['id'].startswith('release1/' + track['name'] + '/') and
                asset['id'] not in ids, 'Colliding/non-namespaced asset ID')
        ids.add(asset['id'])
        url = asset['url']
        require(url.startswith('assets/') and '\\' not in url and
                '..' not in Path(url).parts, 'Invalid runtime URL')
        target = ROOT / 'assets/runtime' / url[len('assets/'):]
        require(target.resolve().is_relative_to((ROOT / track['runtime']).resolve()), 'Asset outside track')
        data = target.read_bytes()
        require(asset['bytes'] == len(data) and asset['sha256'] == hashlib.sha256(data).hexdigest(), 'Asset byte identity mismatch')
        rights = ROOT / asset['rightsRecord']
        require(rights.resolve().is_relative_to(ROOT) and rights.is_file(), 'Missing actual rights record')
        for anchor in asset['anchors'].values():
            require(len(anchor['position']) == len(anchor['rotationDegrees']) == 3, 'Invalid anchor transform')
    return {'fragment': str(path), 'assets': len(ids)}


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--fragment', type=Path, action='append', default=[])
    args = parser.parse_args()
    contract = json.loads((ROOT / 'docs/release1/contract.json').read_text())
    report = validate(contract)
    report['fragments'] = [validate_fragment(path, contract) for path in args.fragment]
    print(json.dumps(report, indent=2))

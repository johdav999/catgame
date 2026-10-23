"""Verify delivered runtime bytes without requiring offline art-job workspaces."""
import argparse
import hashlib
import json
from pathlib import Path, PurePosixPath

ROOT = Path(__file__).resolve().parents[1]


def runtime_path(root, url):
    parts = PurePosixPath(url).parts
    if not url.startswith('assets/') or '\\' in url or '..' in parts or ':' in url:
        raise ValueError(f'Invalid runtime URL: {url}')
    path = (root / url[7:]).resolve()
    if not path.is_relative_to(root.resolve()) or not path.is_file():
        raise ValueError(f'Missing runtime file: {url}')
    return path


def verify(runtime):
    manifest = json.loads((runtime / 'manifest.json').read_text(encoding='utf-8'))
    if manifest['schemaVersion'] != 3:
        raise ValueError('Unsupported runtime manifest')
    ids, urls = set(), set()
    for record in manifest['assets']:
        if record['id'] in ids or record['url'] in urls:
            raise ValueError(f'Duplicate runtime asset: {record["id"]}')
        ids.add(record['id']); urls.add(record['url'])
        path = runtime_path(runtime, record['url'])
        if path.stat().st_size != record['bytes'] or hashlib.sha256(path.read_bytes()).hexdigest() != record['sha256']:
            raise ValueError(f'Runtime bytes changed: {record["url"]}')
        runtime_path(runtime, record['rightsRecord'])
    for catalogue in manifest['assetCatalogues']:
        runtime_path(runtime, 'assets/' + catalogue)
    audio = json.loads((runtime / 'audio/catalog.json').read_text(encoding='utf-8'))
    for clip in audio['clips']:
        playable = clip.get('accepted') or (clip.get('playable') and clip.get('review', {}).get('technicalVerified'))
        if playable and 'assets/audio/' + clip['file'] not in urls:
            raise ValueError(f'Playable audio missing from accepted manifest: {clip["id"]}')
    return {'assets': len(ids), 'audioSlots': len(audio['clips']),
            'missingAcceptance': manifest.get('missingAcceptance', [])}


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--runtime', type=Path, default=ROOT / 'assets/runtime')
    args = parser.parse_args()
    print(json.dumps(verify(args.runtime), indent=2))

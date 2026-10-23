"""Package a clean, matching pinned web/Windows quest release with file hashes."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess
import zipfile

ROOT = Path(__file__).resolve().parents[1]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def git(*args, cwd=ROOT):
    return subprocess.check_output(['git', *args], cwd=cwd, text=True).strip()


def inventory(directory):
    result = {}
    for path in sorted(directory.rglob('*')):
        if not path.is_file():
            continue
        relative = path.relative_to(directory)
        if path.is_symlink() or any(p.lower() in {'saves', 'profiles', '.secrets'} for p in relative.parts):
            raise ValueError(f'Private or linked file in package: {relative}')
        result[relative.as_posix()] = {'sha256': sha(path), 'bytes': path.stat().st_size}
    return result


def validate(site, desktop):
    if git('status', '--porcelain', '--untracked-files=no'):
        raise ValueError('Commit the candidate before packaging')
    revision = git('rev-parse', 'HEAD')
    pin = git('ls-tree', 'HEAD', 'external/WebEngine').split()[2]
    engine = ROOT / 'external/WebEngine'
    if git('rev-parse', 'HEAD', cwd=engine) != pin or git('status', '--porcelain', cwd=engine):
        raise ValueError('Engine checkout differs from the clean committed pin')
    spec = importlib.util.spec_from_file_location('site_package', ROOT / 'scripts/package-release.py')
    module = importlib.util.module_from_spec(spec); spec.loader.exec_module(module)
    web = module.validate(site)
    windows = json.loads((desktop / 'build-info.json').read_text())
    for metadata in [web, windows]:
        if metadata.get('sourceDirty') is not False or metadata.get('gameRevision') != revision:
            raise ValueError('Payload was not built from this clean game revision')
        if metadata['enginePin'] != pin or metadata['configuration'] != 'Release':
            raise ValueError('Payload engine/configuration mismatch')
    if windows['saveFormat'] != 14 or sha(desktop / 'paws_windows.exe') != windows['executableSha256']:
        raise ValueError('Windows executable or save identity mismatch')
    if sha(site / 'assets/manifest.json') != windows['manifestSha256'] or sha(desktop / 'assets/manifest.json') != windows['manifestSha256']:
        raise ValueError('The two hosts do not use the same accepted content manifest')
    for required in ['SDL3.dll', 'dxcompiler.dll', 'dxil.dll', 'licenses/SDL3.txt', 'licenses/Dawn.txt', 'licenses/DXC-MIT.txt']:
        if not (desktop / required).is_file():
            raise ValueError(f'Windows dependency missing: {required}')
    for path in desktop.iterdir():
        if path.name not in {'paws_windows.exe', 'SDL3.dll', 'dxcompiler.dll', 'dxil.dll', 'assets', 'licenses', 'build-info.json'}:
            raise ValueError(f'Unexpected desktop package file: {path.name}')
    return web, windows


def archive(directory, target, files):
    with zipfile.ZipFile(target, 'x', compression=zipfile.ZIP_DEFLATED, compresslevel=6) as output:
        for name in files:
            entry = zipfile.ZipInfo(name, (2026, 1, 1, 0, 0, 0))
            entry.compress_type = zipfile.ZIP_DEFLATED; entry.external_attr = 0o100644 << 16
            output.writestr(entry, (directory / name).read_bytes())
    with zipfile.ZipFile(target) as package:
        if set(package.namelist()) != set(files):
            raise ValueError('Archive inventory mismatch')
        for name, expected in files.items():
            if hashlib.sha256(package.read(name)).hexdigest() != expected['sha256']:
                raise ValueError(f'Archive bytes changed: {name}')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--site', type=Path, required=True)
    parser.add_argument('--desktop', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--verify-only', action='store_true')
    args = parser.parse_args()
    web, windows = validate(args.site, args.desktop)
    if args.verify_only:
        print(json.dumps({'verified': True, 'gameRevision': web['gameRevision'], 'enginePin': web['enginePin']}))
    else:
        args.output.mkdir(parents=True, exist_ok=False)
        manifest = {'schema': 1, 'milestone': 'M7', 'gameRevision': web['gameRevision'],
                    'enginePin': web['enginePin'], 'web': web, 'windows': windows, 'packages': {},
                    'scope': 'Windows x64 SDL/Dawn and desktop Chrome WebGPU/WebGL2; module migration candidate.',
                    'contentAcceptance': json.loads((args.site / 'assets/manifest.json').read_text()).get('missingAcceptance', [])}
        for kind, directory in [('web', args.site), ('windows-x64', args.desktop)]:
            files = inventory(directory)
            target = args.output / f'paws-{kind}-{web["gameRevision"][:12]}.zip'
            archive(directory, target, files)
            manifest['packages'][kind] = {'archive': target.name, 'sha256': sha(target),
                                          'bytes': target.stat().st_size, 'files': files}
        text = json.dumps(manifest, indent=2) + '\n'
        (args.output / 'release-manifest.json').write_text(text)
        sums = ''.join(f'{v["sha256"]}  {v["archive"]}\n' for v in manifest['packages'].values())
        sums += f'{sha(args.output / "release-manifest.json")}  release-manifest.json\n'
        (args.output / 'SHA256SUMS').write_text(sums)
        print(json.dumps({'output': str(args.output), 'gameRevision': web['gameRevision'], 'enginePin': web['enginePin']}))

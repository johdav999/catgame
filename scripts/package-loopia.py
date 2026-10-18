"""Create a reviewable multi-game Loopia deployment folder without credentials."""
import argparse
import shutil
import runpy
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--site', type=Path, required=True)
    parser.add_argument('--output', type=Path, default=ROOT / 'build/loopia-platform')
    args = parser.parse_args()
    site = args.site.resolve()
    if not (site / 'paws.wasm').is_file() or not (site / 'cloud.js').is_file():
        parser.error('Build the current Paws browser site first')
    args.output.mkdir(parents=True, exist_ok=False)
    shutil.copytree(ROOT / 'platform/public', args.output / 'public_html')
    private = args.output / 'private'
    private.mkdir()
    for name in ('app.php', 'config.example.php'):
        shutil.copy2(ROOT / 'platform/private' / name, private / name)
    game = args.output / 'public_html/games/paws'
    game.mkdir(parents=True)
    # Reuse the production release allowlist; do not ship stale preview targets
    # that may also exist in a developer's CMake site directory.
    runtime_files = runpy.run_path(str(ROOT / 'scripts/package-release.py'))['FILES']
    for name in runtime_files:
        shutil.copy2(site / name, game / name)
    for name in ('assets', 'licenses'):
        shutil.copytree(site / name, game / name)
    shutil.copy2(ROOT / 'platform/schema.sql', args.output / 'schema.sql')
    shutil.copy2(ROOT / 'docs/loopia-platform.md', args.output / 'DEPLOYMENT.md')
    print(f'Deployment candidate: {args.output.resolve()} (no credentials included)')


if __name__ == '__main__':
    main()

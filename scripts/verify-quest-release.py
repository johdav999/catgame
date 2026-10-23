"""Run the M7 matrix against one clean pinned site and packaged Windows host."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import time
import urllib.request

ROOT = Path(__file__).resolve().parents[1]
HIDDEN = subprocess.CREATE_NO_WINDOW if os.name == 'nt' else 0


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def wait_for(predicate, seconds=90):
    end = time.monotonic() + seconds
    while time.monotonic() < end:
        if predicate():
            return
        time.sleep(.2)
    raise TimeoutError('Release acceptance wait expired')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--site', type=Path, required=True)
    parser.add_argument('--desktop', type=Path, required=True)
    parser.add_argument('--native-catalogue', type=Path, required=True)
    parser.add_argument('--wasm-catalogue', type=Path, required=True)
    parser.add_argument('--node', default='node')
    parser.add_argument('--evidence', type=Path, required=True)
    parser.add_argument('--port', type=int, default=8157)
    args = parser.parse_args()
    site, desktop, out = args.site.resolve(), args.desktop.resolve(), args.evidence.resolve()
    out.mkdir(parents=True, exist_ok=False)
    cwd = out / 'unrelated-working-directory'; cwd.mkdir()
    report = {'schema': 1, 'milestone': 'M7', 'gates': []}
    environment = dict(os.environ, PAWS_URL=f'http://127.0.0.1:{args.port}/')

    def run(name, command, *, env=None, working=ROOT, timeout=1200):
        with (out / (name + '.log')).open('w', encoding='utf-8') as log:
            subprocess.run([str(x) for x in command], cwd=working, env=env or environment,
                           stdout=log, stderr=subprocess.STDOUT, timeout=timeout, check=True,
                           creationflags=HIDDEN)
        report['gates'].append({'name': name, 'passed': True, 'logSha256': sha(out / (name + '.log'))})
        (out / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
        print('Release gate passed:', name, flush=True)

    run('package-identity', [sys.executable, ROOT / 'scripts/package-quest-release.py', '--site', site,
                            '--desktop', desktop, '--output', out / 'unused', '--verify-only'])
    run('frozen-fixtures', [sys.executable, ROOT / 'tools/quest-baseline/manifest.py', '--verify-all'])
    run('native-catalogue', [args.native_catalogue.resolve(), out / 'native-rules'])
    run('wasm-catalogue', [args.node, args.wasm_catalogue.resolve(), out / 'wasm-rules'])
    assert sha(out / 'native-rules/catalogue.trace') == sha(out / 'wasm-rules/catalogue.trace')
    report['portableTraceSha256'] = sha(out / 'native-rules/catalogue.trace')
    executable = desktop / 'paws_windows.exe'
    run('windows-catalogue', [executable, '--dev-beach', '--user-directory', out / 'catalogue-profile',
                              '--smoke', 'catalogue', out / 'windows-catalogue'], working=cwd)
    catalogue = json.loads((out / 'windows-catalogue/catalogue.json').read_text())
    expected = len((ROOT / 'tests/fixtures/quest-migration-m5/catalogue.tsv').read_text().strip().splitlines()) - 1
    assert len(catalogue) == expected and all(q['completed'] and q['sceneCount'] == 1 for q in catalogue)
    for version, corpus in [(12, 'm0'), (13, 'm5')]:
        original = ROOT / f'tests/fixtures/quest-migration-{corpus}/pip_missing_ball-completed.save'
        profile = out / f'migration-v{version}-profile'; (profile / 'saves').mkdir(parents=True)
        shutil.copy2(original, profile / 'saves/afternoon.save')
        for attempt in ['first', 'reload']:
            run(f'windows-v{version}-{attempt}', [executable, '--dev-beach', '--user-directory', profile,
                '--smoke', 'import', out / f'windows-v{version}-{attempt}'], working=cwd)
            assert sha(original) == sha(profile / 'saves/pre-quest-m6.save')
            assert (profile / 'saves/afternoon.save').read_text().startswith('PAWS_AFTERNOON 14\n')

    holder_out = out / 'primary'; holder_out.mkdir()
    profile = out / 'two-process-profile'
    with (out / 'primary.log').open('w') as log:
        holder = subprocess.Popen([str(executable), '--user-directory', str(profile), '--smoke', 'hold', str(holder_out)],
                                  cwd=cwd, stdout=log, stderr=subprocess.STDOUT, creationflags=HIDDEN)
        try:
            wait_for(lambda: (holder_out / 'ready').is_file())
            before = sha(profile / 'saves/afternoon.save')
            run('windows-second-writer', [executable, '--user-directory', profile, '--smoke', 'secondary', out / 'secondary'], working=cwd)
            assert sha(profile / 'saves/afternoon.save') == before
        finally:
            (holder_out / 'release').write_text('finish')
            try:
                holder.wait(timeout=90)
            except subprocess.TimeoutExpired:
                holder.terminate(); holder.wait(timeout=15)
        assert holder.returncode == 0

    server_log = (out / 'server.log').open('w')
    server = subprocess.Popen([sys.executable, str(ROOT / 'scripts/serve-site.py'), '--directory', str(site),
        '--port', str(args.port), '--port-attempts', '1'], stdout=server_log, stderr=subprocess.STDOUT, creationflags=HIDDEN)
    try:
        def serving():
            try:
                with urllib.request.urlopen(environment['PAWS_URL'] + 'build-info.json', timeout=2) as response:
                    return json.load(response) == json.loads((site / 'build-info.json').read_text())
            except OSError:
                return False
        wait_for(serving)
        with urllib.request.urlopen(environment['PAWS_URL'] + 'paws.wasm') as response:
            assert hashlib.file_digest(response, 'sha256').hexdigest() == sha(site / 'paws.wasm')
        report['web'] = json.loads((site / 'build-info.json').read_text())
        report['windows'] = json.loads((desktop / 'build-info.json').read_text())
        for backend in ['webgpu', 'webgl2']:
            env = dict(environment, QUEST_BACKEND=backend, QUEST_FROM='0', EVIDENCE_DIR=str(out / backend))
            run('catalogue-' + backend, [args.node, ROOT / 'scripts/quest-catalogue-smoke.cjs'], env=env, timeout=3600)
            result = json.loads((out / backend / 'report.json').read_text())
            assert result['passed'] and len(result['quests']) == expected and result['build'] == report['web']
            env['EVIDENCE_DIR'] = str(out / ('recovery-' + backend))
            run('save-recovery-' + backend, [args.node, ROOT / 'scripts/save-recovery-smoke.cjs'], env=env)
        env = dict(environment, EVIDENCE_DIR=str(out / 'migration-storage'))
        run('migration-storage', [args.node, ROOT / 'scripts/quest-migration-storage-smoke.cjs'], env=env)
        env.update(EVIDENCE_DIR=str(out / 'interchange'), PAWS_NATIVE=str(executable),
                   PAWS_NATIVE_SAVE=str(out / 'native-rules/complete.save'), PAWS_EXPECT_COINS='170', PAWS_RICH_SAVE='1')
        run('file-interchange', [args.node, ROOT / 'scripts/paws-host-smoke.cjs'], env=env)
        env.update(EVIDENCE_DIR=str(out / 'endings'), PAWS_QUEST_FIXTURES=str(out / 'native-rules'))
        run('ending-replay', [args.node, ROOT / 'scripts/quest-endings-smoke.cjs'], env=env)
        report.update(passed=True, questCount=expected, scope='Packaged Windows x64 SDL/Dawn and desktop Chrome forced WebGPU/WebGL2.')
        (out / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    finally:
        server.terminate(); server.wait(timeout=15); server_log.close()


if __name__ == '__main__':
    main()

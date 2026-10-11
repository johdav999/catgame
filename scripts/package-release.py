"""Package the pinned Release payload as a versioned, locally reviewable site."""
import argparse
import hashlib
import json
import re
import shutil
import subprocess
import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
FILES = ("index.html", "style.css", "favicon.svg", "host.js", "saves.js",
         "paws-audio.js", "engine-gui.js", "engine-storage.js", "engine-audio.js",
         "paws.js", "paws.wasm", "build-info.js", "build-info.json")
GATES = ["Final provider audio, rights records and actual-backend listening review",
         "Independent first-player 15–20-minute pacing review",
         "Physical mobile compatibility, sustained performance and audible output",
         "Final source/export/runtime art and animation acceptance"]


def digest(file):
    return hashlib.sha256(file.read_bytes()).hexdigest()


def validate(site):
    info = json.loads((site / "build-info.json").read_text())
    if not re.fullmatch(r"\d+\.\d+\.\d+", info["version"]):
        raise ValueError("Invalid package version")
    if info["configuration"] != "Release" or info["gallery"]:
        raise ValueError("Packaging requires Release with PAWS_GUI_GALLERY=OFF")
    pin = subprocess.check_output(["git", "ls-files", "--stage", "external/WebEngine"], cwd=ROOT, text=True).split()[1]
    if info["enginePin"] != pin:
        raise ValueError("Build engine differs from the recorded gitlink")
    if digest(site / "paws.wasm") != info["wasmSha256"]:
        raise ValueError("Wasm changed after build metadata generation")
    asset_list = sorted((p for p in (site / "assets").rglob("*") if p.is_file()), key=lambda p: p.relative_to(site).as_posix())
    content = "".join(f"{p.relative_to(site).as_posix()}:{digest(p)}\n" for p in asset_list)
    if hashlib.sha256(content.encode()).hexdigest() != info["contentSha256"]:
        raise ValueError("Assets changed after build metadata generation")
    notice_list = sorted((p for p in (site / "licenses").rglob("*") if p.is_file()), key=lambda p: p.relative_to(site).as_posix())
    notice_content = "".join(f"{p.relative_to(site).as_posix()}:{digest(p)}\n" for p in notice_list)
    if hashlib.sha256(notice_content.encode()).hexdigest() != info["noticesSha256"]:
        raise ValueError("Notices changed after build metadata generation")
    payload = f'{info["wasmSha256"]}:{info["contentSha256"]}:{info["enginePin"]}:{info["configuration"]}:{info["noticesSha256"]}'
    for name in ("index.html", "host.js", "saves.js", "paws-audio.js", "style.css", "engine-gui.js", "engine-storage.js", "engine-audio.js", "paws.js"):
        payload += ":" + digest(site / name)
    expected = f'paws-{info["version"]}-{hashlib.sha256(payload.encode()).hexdigest()[:16]}'
    if info["buildId"] != expected:
        raise ValueError("Runtime changed after build metadata generation")
    if (site / "build-info.js").read_text().strip() != "window.pawsBuild=Object.freeze(" + (site / "build-info.json").read_text().strip() + ");":
        raise ValueError("Browser build identity differs from its manifest")
    for name in FILES:
        if not (site / name).is_file():
            raise ValueError(f"Required runtime file missing: {name}")
    for reference in re.findall(r'(?:src|href)="([^"]+)"', (site / "index.html").read_text()):
        if reference.startswith(("/", "http:", "https:")) or not (site / reference).is_file():
            raise ValueError(f"Non-relative or missing shell reference: {reference}")
    notices = ["assets/fonts/nunitosans-OFL.txt", "assets/fonts/fraunces-OFL.txt",
               "assets/models/pip/NOTICE.md", "assets/models/garden/NOTICE.md"]
    notices += [f"licenses/{n}.txt" for n in ("ufbx", "cgltf", "stb", "meshoptimizer", "Jolt", "Emscripten", "Dawn")]
    for notice in notices:
        if not (site / notice).is_file():
            raise ValueError(f"Required notice missing: {notice}")
    garden = json.loads((site / "assets/models/garden/asset-manifest.json").read_text())
    for asset in garden["assets"]:
        if digest(site / "assets/models/garden" / asset["file"]) != asset["sha256"]:
            raise ValueError(f'Stale garden asset provenance: {asset["id"]}')
    pip = json.loads((site / "assets/models/pip/manifest.json").read_text())
    for asset in pip["files"]:
        if asset["path"].startswith("assets/runtime/"):
            file = site / "assets" / asset["path"][len("assets/runtime/"):]
            if digest(file) != asset["sha256"]:
                raise ValueError(f'Stale Pip asset provenance: {asset["path"]}')
    for file in asset_list:
        if file.suffix.lower() in (".blend", ".fbx", ".py", ".bak", ".psd"):
            raise ValueError(f"Authoring file in deployed assets: {file.name}")
    return info


def package(site, output):
    info = validate(site)
    target = output / info["buildId"]
    # Never overwrite a review package or any unrelated checkout directory.
    target.mkdir(parents=True, exist_ok=False)
    deployed = target / "site"
    deployed.mkdir()
    for name in FILES:
        shutil.copy2(site / name, deployed / name)
    for folder in ("assets", "licenses"):
        shutil.copytree(site / folder, deployed / folder)
    catalog = json.loads((deployed / "assets/audio/catalog.json").read_text())
    missing = [c["id"] for c in catalog["clips"] if not c["accepted"] or not (deployed / "assets/audio" / c["file"]).is_file()]
    manifest = {"schema": 1, **info, "status": "review-candidate", "productionReady": False,
                "remainingGates": GATES, "missingAudio": missing,
                "files": [{"path": p.relative_to(deployed).as_posix(), "bytes": p.stat().st_size, "sha256": digest(p)}
                          for p in sorted(deployed.rglob("*")) if p.is_file()]}
    (target / "release-manifest.json").write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    (target / "HOSTING.md").write_text(f"""# Paws {info['version']} review candidate

Build: `{info['buildId']}`. Content: `{info['contentSha256']}`.
This package is not production accepted. See release-manifest.json for open gates.

Serve the complete site directory: `python -m http.server 8080 --bind 127.0.0.1 --directory site`.
Open http://localhost:8080. Progress belongs to this browser origin; export before changing origins.
For deployment, host site/ over HTTPS, at the root or under one directory prefix.
Use application/wasm for .wasm, text/javascript for .js, and ordinary image/font MIME types.
Enable gzip or Brotli for Wasm, JS, JSON and fonts. Do not rewrite asset misses to index.html.
Publish atomically and invalidate old shell/JS/Wasm caches together. This release has no service worker.
If using CSP, allow Wasm compilation and same-origin scripts, assets, connections and media;
the pinned backend uses inline generated JavaScript. Test your CSP on the actual package.
No cross-origin isolation, accounts or external asset origins are required.
WebGPU and WebAssembly must work in the browser; incompatible devices get a Retry screen.
Only runtime assets/notices are included; source masters and developer previews are excluded.
No deployment was performed by the packaging command.
""", encoding="utf-8")
    archive = output / f"{info['buildId']}.zip"
    with zipfile.ZipFile(archive, "x", compression=zipfile.ZIP_DEFLATED, compresslevel=9) as z:
        for file in sorted(target.rglob("*")):
            if file.is_file():
                entry = zipfile.ZipInfo(file.relative_to(target).as_posix(), (2026, 1, 1, 0, 0, 0))
                entry.compress_type = zipfile.ZIP_DEFLATED
                entry.external_attr = 0o100644 << 16
                z.writestr(entry, file.read_bytes())
    print(json.dumps({"directory": str(target), "archive": str(archive), "sha256": digest(archive), "buildId": info["buildId"], "missingAudio": len(missing)}, indent=2))


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--site", type=Path, default=ROOT / "build/web-phase10-pinned-release/site")
    parser.add_argument("--output", type=Path, default=ROOT / "build/releases")
    args = parser.parse_args()
    package(args.site.resolve(), args.output.resolve())

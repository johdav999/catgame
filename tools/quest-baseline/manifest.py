"""Record/verify immutable M0 fixture identity; source hashes are capture provenance."""
import hashlib
import json
import sys
from pathlib import Path

root = Path(__file__).resolve().parents[2]
fixtures = root / "tests/fixtures/quest-migration-m0"
sources = ["src/puzzle.cpp", "src/content.cpp", "src/afternoon.cpp", "src/afternoon.hpp",
           "src/release1/rules/puzzles.cpp", "src/release1/rules/puzzles.hpp",
           "src/release1/rules/content.cpp", "src/release1/rules/beach_content.cpp",
           "src/release1/progression/policy.cpp", "src/release1/progression/policy.hpp",
           "src/release1/progression/codec.cpp"]
digest = lambda path: hashlib.sha256(path.read_bytes()).hexdigest()
artifacts = {p.name: digest(p) for p in sorted(fixtures.iterdir()) if p.suffix in (".save", ".tsv")}
path = fixtures / "manifest.json"
if sys.argv[1:] == ["--write"]:
    path.write_text(json.dumps({"captured": "2026-10-22", "questCount": 29,
        "solutionCommands": 398, "pawsSaveVersion": 12,
        "pinnedEngine": "0426d9319de715584a2e34ff9862cd6ba873ecd0",
        "developmentEngineHead": "f4cae84a2ebef51a2e2449409653f6e2901eb211",
        "developmentEngineDirty": True,
        "sourceSha256": {s: digest(root/s) for s in sources},
        "artifactsSha256": artifacts}, indent=2)+"\n")
elif sys.argv[1:] == ["--verify-all"]:
    for name in ("quest-migration-m0", "quest-migration-m5"):
        directory = root / "tests/fixtures" / name
        actual = {p.name: digest(p) for p in sorted(directory.iterdir()) if p.suffix in (".save", ".tsv")}
        assert actual == json.loads((directory / "manifest.json").read_text())["artifactsSha256"], name + " changed"
        print(f"{name}: {len(actual)} frozen artifacts verified")
elif sys.argv[1:] == ["--verify"]:
    assert artifacts == json.loads(path.read_text())["artifactsSha256"], "M0 fixture set or bytes changed"
    print(f"M0 manifest: {len(artifacts)} artifacts verified")
else:
    raise SystemExit("Usage: manifest.py --write|--verify")

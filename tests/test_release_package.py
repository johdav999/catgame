"""Tamper rejection and archive integrity checks against an actual built package."""
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import shutil
import tempfile
import unittest
import zipfile

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location("package_release", ROOT / "scripts/package-release.py")
release = importlib.util.module_from_spec(spec)
spec.loader.exec_module(release)


class ReleaseTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.source = Path(os.environ.get("PAWS_RELEASE_SITE", ROOT / "build/web-phase10-pinned-release/site"))
        if not cls.source.exists():
            raise unittest.SkipTest("Build the pinned Release site before package checks")

    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(dir=ROOT / "build")
        self.site = Path(self.temp.name) / "site"
        shutil.copytree(self.source, self.site)

    def tearDown(self):
        self.temp.cleanup()

    def test_actual_payload(self):
        self.assertFalse(release.validate(self.site)["gallery"])

    def test_changed_runtime_rejected(self):
        with (self.site / "host.js").open("a") as f:
            f.write("\n// modified after build")
        with self.assertRaisesRegex(ValueError, "Runtime changed"):
            release.validate(self.site)

    def test_changed_assets_rejected(self):
        (self.site / "assets/models/pip/pip.glb").write_bytes(b"damaged")
        with self.assertRaisesRegex(ValueError, "Assets changed"):
            release.validate(self.site)

    def test_changed_notice_rejected(self):
        (self.site / "licenses/Dawn.txt").write_text("changed notice")
        with self.assertRaisesRegex(ValueError, "Notices changed"):
            release.validate(self.site)

    def test_gallery_rejected(self):
        info = json.loads((self.site / "build-info.json").read_text())
        info["gallery"] = True
        (self.site / "build-info.json").write_text(json.dumps(info))
        with self.assertRaisesRegex(ValueError, "GALLERY=OFF"):
            release.validate(self.site)

    def test_changed_browser_identity_rejected(self):
        (self.site / "build-info.js").write_text("window.pawsBuild={};")
        with self.assertRaisesRegex(ValueError, "Browser build identity"):
            release.validate(self.site)

    def test_archive_matches_manifest_without_previews(self):
        output = Path(self.temp.name) / "release"
        release.package(self.site, output)
        archive = next(output.glob("*.zip"))
        with zipfile.ZipFile(archive) as z:
            self.assertIsNone(z.testzip())
            manifest = json.loads(z.read("release-manifest.json"))
            self.assertFalse(manifest["productionReady"])
            self.assertEqual(manifest["status"], "review-candidate")
            self.assertFalse(any("preview.html" in n or "gallery" in n for n in z.namelist()))
            for entry in manifest["files"]:
                actual = z.read("site/" + entry["path"])
                self.assertEqual(len(actual), entry["bytes"])
                self.assertEqual(hashlib.sha256(actual).hexdigest(), entry["sha256"])


if __name__ == "__main__":
    unittest.main()

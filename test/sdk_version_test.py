"""Exercise the actual CMake guard without installing or mocking ESP-IDF."""

from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

CMAKE, ROOT = sys.argv[1:]
sys.argv[1:] = []


class SdkVersionGuardTest(unittest.TestCase):
    def check_guard(self, pin, active="6.0.3", reported="v6.0.3"):
        with tempfile.TemporaryDirectory() as directory:
            work = Path(directory)
            pin_file = work / ".esp-idf-version"
            if pin is not None:
                pin_file.write_text(pin)
            script = work / "check.cmake"
            script.write_text(
                f'include("{ROOT}/cmake/CheckEspIdfVersion.cmake")\n'
                f'portal_check_esp_idf_version("{pin_file}" "{active}" "{reported}")\n'
            )
            return subprocess.run([CMAKE, "-P", str(script)], text=True,
                                  capture_output=True)

    def test_matching_release(self):
        self.assertEqual(self.check_guard("v6.0.3\n").returncode, 0)

    def test_other_patch_minor_and_major_are_rejected(self):
        for active in ("6.0.2", "6.1.3", "7.0.3"):
            with self.subTest(active=active):
                result = self.check_guard("v6.0.3\n", active, "v" + active)
                self.assertNotEqual(result.returncode, 0)
                self.assertIn("requires ESP-IDF v6.0.3", result.stderr)
                self.assertIn(active, result.stderr)

    def test_prerelease_and_modified_checkout_are_rejected(self):
        for reported in ("v6.0.3-rc1", "v6.0.3-1-gabcdef", "v6.0.3-dirty"):
            with self.subTest(reported=reported):
                result = self.check_guard("v6.0.3", reported=reported)
                self.assertNotEqual(result.returncode, 0)

    def test_missing_pin(self):
        result = self.check_guard(None)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("Missing ESP-IDF version pin", result.stderr)

    def test_malformed_pin(self):
        for pin in ("", "6.0.3", "v6.0", "v6.0.3-rc1", "v6.0.3\nv6.0.2"):
            with self.subTest(pin=pin):
                result = self.check_guard(pin)
                self.assertNotEqual(result.returncode, 0)
                self.assertIn("Invalid ESP-IDF version pin", result.stderr)

    def test_changed_pin_is_used(self):
        self.assertEqual(self.check_guard("v7.1.2", "7.1.2", "v7.1.2").returncode, 0)


if __name__ == "__main__":
    unittest.main()

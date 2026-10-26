"""Exercise the actual CMake guard without installing or mocking ESP-IDF."""

from pathlib import Path
import os
import shutil
import subprocess
import sys
import tempfile
import unittest

CMAKE, ROOT = sys.argv[1:]
sys.argv[1:] = []


class SdkVersionGuardTest(unittest.TestCase):
    def check_guard(self, pin, active="6.0.3", reported="v6.0.3"):
        with tempfile.TemporaryDirectory(prefix="portal ${literal} ") as directory:
            work = Path(directory)
            pin_file = work / ('.esp-idf-"back\\slash' if os.name != "nt" else ".esp-idf-version")
            if pin is not None:
                pin_file.write_text(pin)
            return subprocess.run([
                CMAKE, f"-DPORTAL_ROOT={Path(ROOT).resolve().as_posix()}",
                f"-DPORTAL_PIN={pin_file.as_posix()}", f"-DPORTAL_ACTIVE={active}",
                f"-DPORTAL_REPORTED={reported}", "-P",
                str(Path(ROOT) / "test/cmake/check_sdk.cmake")],
                text=True, capture_output=True, timeout=30)

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


class SdkCheckoutGuardTest(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory(prefix="portal sdk ")
        self.addCleanup(self.temporary.cleanup)
        self.work = Path(self.temporary.name)
        self.sdk = self.work / "sdk checkout"
        self.sdk.mkdir()
        self.pin = self.work / ".esp-idf-version"
        self.pin.write_text("v6.0.3\n")
        self.git = shutil.which("git")
        self.assertIsNotNone(self.git, "Git is required for SDK guard tests")
        self.version_file = self.sdk / "tools/cmake/version.cmake"
        self.version_file.parent.mkdir(parents=True)
        self.git_run("init", "-q")
        self.write_version(2)
        (self.sdk / "tracked.txt").write_text("original\n")
        self.commit("old release")
        self.git_run("tag", "v6.0.2")
        self.write_version(3)
        self.commit("pinned release")
        self.git_run("tag", "v6.0.3")

    def git_run(self, *args):
        return subprocess.run([self.git, "-C", str(self.sdk), *args],
                              check=True, text=True, capture_output=True, timeout=30)

    def write_version(self, patch):
        self.version_file.write_text(
            f"set(IDF_VERSION_MAJOR 6)\nset(IDF_VERSION_MINOR 0)\nset(IDF_VERSION_PATCH {patch})\n")

    def commit(self, message):
        self.git_run("add", ".")
        self.git_run("-c", "user.name=SDK test", "-c", "user.email=sdk-test@example.invalid",
                     "-c", "commit.gpgsign=false", "commit", "-qm", message)

    def definitions(self):
        return [f"-DPORTAL_ROOT={Path(ROOT).resolve().as_posix()}",
                f"-DPORTAL_PIN={self.pin.as_posix()}", f"-DPORTAL_SDK={self.sdk.as_posix()}"]

    def configure(self):
        self.build = self.work / "build"
        result = subprocess.run([CMAKE, "-S", str(Path(ROOT) / "test/cmake/sdk_guard_project"),
                                 "-B", str(self.build), *self.definitions()],
                                text=True, capture_output=True, timeout=30)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    def build_result(self):
        return subprocess.run([CMAKE, "--build", str(self.build), "--config", "Debug"],
                              text=True, capture_output=True, timeout=30)

    def test_clean_checkout_with_spaces_in_paths_builds(self):
        self.configure()
        result = self.build_result()
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    def test_changed_checkout_fails_without_explicit_reconfiguration(self):
        for change in ("unstaged", "staged", "tag"):
            with self.subTest(change=change):
                self.git_run("checkout", "-q", "v6.0.3")
                self.configure()
                self.assertEqual(self.build_result().returncode, 0)
                if change == "tag":
                    self.git_run("checkout", "-q", "v6.0.2")
                else:
                    (self.sdk / "tracked.txt").write_text("modified\n")
                    if change == "staged":
                        self.git_run("add", "tracked.txt")
                result = self.build_result()
                self.assertNotEqual(result.returncode, 0)
                self.assertIn("requires ESP-IDF v6.0.3", result.stdout + result.stderr)
                # Restore only this test-owned fixture before the next scenario.
                self.git_run("restore", "--source=v6.0.3", "--staged", "--worktree", ".")

    def test_git_failure_includes_original_diagnostic(self):
        non_repository = self.work / "not a repository"
        non_repository.mkdir()
        command = [CMAKE, *self.definitions(), f"-DPORTAL_SDK={non_repository.as_posix()}",
                   f"-DPORTAL_GIT={Path(self.git).as_posix()}", "-DPORTAL_CHECK_CHECKOUT=ON",
                   "-P", str(Path(ROOT) / "test/cmake/check_sdk.cmake")]
        result = subprocess.run(command, text=True, capture_output=True, timeout=30)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("Cannot identify ESP-IDF checkout", result.stderr)
        self.assertIn("not a git repository", result.stderr)


if __name__ == "__main__":
    unittest.main()

# Development setup

## Firmware SDK

The project's exact ESP-IDF tag is recorded in [`.esp-idf-version`](../.esp-idf-version).
Use that tag for local builds and future CI; do not use a moving release branch.
The initial baseline uses the newer verified bugfix release in the 6.0 series;
there is no project-specific reason to retain the previous patch version.
This file records the pin; it does not automatically install or enforce the SDK.
The firmware scaffold should check the SDK version during configuration.

The target is `esp32s3`. See the official
[ESP32-S3 installation guide](https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/get-started/index.html)
for prerequisites, selecting the documentation version that matches the pin.
For a Git-based setup, start in this repository's root. The commands below
install a separate SDK checkout under `~/esp`:

```sh
PORTAL_REPO_DIR=$(pwd)
PORTAL_IDF_VERSION=$(cat "$PORTAL_REPO_DIR/.esp-idf-version")
mkdir -p "$HOME/esp"
cd "$HOME/esp"
git clone --branch "$PORTAL_IDF_VERSION" --recursive https://github.com/espressif/esp-idf.git "esp-idf-$PORTAL_IDF_VERSION"
cd "esp-idf-$PORTAL_IDF_VERSION"
./install.sh esp32s3
. ./export.sh
idf.py --version
cd "$PORTAL_REPO_DIR"
```

Sourcing `export.sh` activates tools only in the current shell. In every new
terminal, start from the repository root and activate the pinned checkout again:

```sh
PORTAL_IDF_VERSION=$(cat .esp-idf-version)
. "$HOME/esp/esp-idf-$PORTAL_IDF_VERSION/export.sh"
idf.py --version
```

Check that the reported version matches the pin before building; an older SDK
may already be on PATH. If using a different SDK parent directory, adjust both
installation and activation paths. When the pin changes, install that checkout
and its matching tools before activating it.

See the matching tag in [Espressif's releases](https://github.com/espressif/esp-idf/releases)
for release notes. Host tests will use their own CMake build without requiring
IDF (ADR-0017).


## Git hosting

- Repository: [ciroque/portal-gun-firmware](https://github.com/ciroque/portal-gun-firmware).
- `origin` fetch/push URL: `git@github.com:ciroque/portal-gun-firmware`.
- Default branch: `main`, confirmed with `git ls-remote --symref origin HEAD`.
- SSH read access was confirmed on 2026-10-25. Write access, branch protection,
  and PR permissions have not been checked.

Use the focused branch/PR and Conventional Commit workflow in
[CONTRIBUTING.md](../CONTRIBUTING.md). GitHub CLI (`gh`) was not on PATH during
inspection; PRs can be opened through GitHub or an authenticated CLI once set up.

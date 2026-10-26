# Development setup

## Firmware SDK

The project's exact ESP-IDF tag is recorded in [`.esp-idf-version`](../.esp-idf-version).
Use that tag for local builds and future CI; do not use a moving release branch.
The initial baseline uses the newer verified bugfix release in the 6.0 series;
there is no project-specific reason to retain the previous patch version.
The firmware CMake configuration reads this file and rejects a mismatched SDK.
Both the version components and the Git description must match the pinned tag;
prerelease, post-release, and dirty checkouts are rejected. The version file
does not install the SDK automatically.

The target is `esp32s3`. Start in this repository's root and print the exact
installation-guide URL from the pin:

```sh
PORTAL_IDF_VERSION=$(cat .esp-idf-version)
printf '%s\n' "https://docs.espressif.com/projects/esp-idf/en/$PORTAL_IDF_VERSION/esp32s3/get-started/index.html"
```

Open that URL for prerequisites. This avoids a moving `stable` or `latest`
documentation link. For a Git-based setup, the commands below install a separate
SDK checkout under `~/esp`, starting from the repository root:

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

If the destination already contains a complete checkout of the pinned tag, skip
`git clone` and continue with `cd`, installation, and activation. For a partial
or uncertain checkout, stop and inspect it before continuing; an existing
directory alone does not establish that the SDK is ready to use.

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
IDF (ADR-0017). See [the project README](../README.md) for firmware and host-test
commands. If changing hardware defaults in an existing build, review the generated
`sdkconfig`: `sdkconfig.defaults` supplies initial values, not overrides for
settings already saved there. Generated configuration and build files are ignored
by Git; commit intentional baseline changes to `sdkconfig.defaults`.

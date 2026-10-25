# Development setup

## Firmware SDK

The project pins **ESP-IDF v6.0.2** in [`.esp-idf-version`](../.esp-idf-version).
Use that exact tag for local builds and CI; do not use a moving release branch.
This file records the pin; it does not automatically install or enforce the SDK.
The firmware scaffold should check the SDK version during configuration.

The target is `esp32s3`. Install the SDK and its matching tools using the
[v6.0.2 installation guide](https://docs.espressif.com/projects/esp-idf/en/v6.0.2/esp32s3/get-started/index.html).
For a Git-based setup in your chosen SDK directory:

```sh
git clone --branch v6.0.2 --recursive https://github.com/espressif/esp-idf.git esp-idf-v6.0.2
cd esp-idf-v6.0.2
./install.sh esp32s3
. ./export.sh
idf.py --version
```

The release and recursive clone instructions are documented in the
[official release](https://github.com/espressif/esp-idf/releases/tag/v6.0.2).
Use a separate SDK checkout rather than placing SDK sources in this repository.
Host tests will use their own CMake build without requiring IDF (ADR-0017).

Environment inspection on 2026-10-25 found neither `idf.py` on PATH nor an
`IDF_PATH` value. No SDK installation or firmware build was performed as part
of selecting the version. The initial firmware scaffold is still pending.

## Git hosting

- Repository: [ciroque/portal-gun-firmware](https://github.com/ciroque/portal-gun-firmware).
- `origin` fetch/push URL: `git@github.com:ciroque/portal-gun-firmware`.
- Default branch: `main`, confirmed with `git ls-remote --symref origin HEAD`.
- SSH read access was confirmed on 2026-10-25. Write access, branch protection,
  and PR permissions have not been checked.

Use the focused branch/PR and Conventional Commit workflow in
[CONTRIBUTING.md](../CONTRIBUTING.md). GitHub CLI (`gh`) was not on PATH during
inspection; PRs can be opened through GitHub or an authenticated CLI once set up.

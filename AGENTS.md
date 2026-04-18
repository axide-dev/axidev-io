# AGENTS.md

This file defines repository-specific rules for AI agents and other automated
contributors working in `axidev-io`.

## Start Here

Before changing build logic, packaging, CI, release workflows, vendored code,
or Linux platform code, read these files first:

- `docs/developers/README.md`
- `build.py`
- `.github/actions/build-linux/action.yml`
- `.github/workflows/release.yml`

Do not change build or release behavior until you understand those files.

## Contribution Rules

- Keep changes small and targeted.
- Preserve the current build entrypoint: `python build.py`.
- Preserve the current packaging entrypoint: `python build.py package`.
- Preserve the current CI checks unless a human maintainer explicitly asks for a
  different policy.
- If a requested change would conflict with the license/compliance rules below,
  stop and ask a human maintainer instead of guessing.

## Linux Dependency Policy

On Linux, `libinput`, `libudev`, and `xkbcommon` are system dependencies
discovered through `pkg-config`.

They must remain dynamically linked.

This is a hard repository policy for legal/compliance reasons. Under no
circumstances should an agent:

- switch Linux dependency discovery to `pkg-config --static`
- add `-static`, `--static`, `-Wl,-Bstatic`, or similar static-linker flags
- link Linux dependency `.a` archives directly
- vendor `libinput`, `libudev`, or `xkbcommon` into release artifacts
- remove or weaken checks that enforce dynamic linkage

If a task asks for any of the above, do not implement it without explicit human
approval.

## Required Enforcement

These safeguards must remain in place:

- `build.py` rejects static-linkage attempts for Linux dependency flags.
- Linux CI runs an explicit `ldd` check against the listener integration binary
  to confirm that `libinput`, `libudev`, and `libxkbcommon` resolve as shared
  libraries at runtime.

Agents may strengthen these checks, but must not remove or bypass them.

## Release Packaging Policy

Release artifacts must include:

- the project `LICENSE`
- the full `vendor/licenses/` directory

Agents must not ship release archives that omit those files.

If packaging changes are made, verify that the release workflow still checks for
the packaged license files before publishing artifacts.

## Verification

For build or packaging changes, prefer to verify with:

- `python -m py_compile build.py`
- `python build.py test`
- Linux CI checks, including the explicit `ldd` validation

If local verification is not possible, say so clearly in the final summary.

# Developer Guide

## Build

```sh
python build.py
python build.py compile-commands
python build.py test
python build.py example
```

Additional targets:

- `python build.py test-unit`
- `python build.py test-integration`
- `python build.py compile-commands`
- `python build.py package-integration-tests --version run-123 --arch x64`
- `python build.py clean`
- `python build.py package --version v1.2.3`

## Repo Layout

- `include/axidev-io/c_api.h`: primary public API
- `src/c_api.c`: public entrypoints
- `src/core/`: global context and logging modules
- `src/internal/`: result, thread, and UTF helpers
- `src/keyboard/common/`: key utilities and keymap logic
- `src/keyboard/sender/`: platform sender backends
- `src/keyboard/listener/`: platform listener backends
- `tests/`: C-only unit and integration tests
- `vendor/stb/stb_ds.h`: vendored container dependency

## Architecture

- The library owns one exported global context: `axidev_io_global`.
- Sender and listener state live inside fixed-size storage embedded in that
  global context.
- Public sender/listener calls are global functions. Callers do not allocate or
  pass subsystem handles.
- Internal helpers return `axidev_io_result` and place produced outputs in
  explicit out-parameters.

## Backends

- Windows:
  - sender: `src/keyboard/sender/sender_windows.c`
  - listener: `src/keyboard/listener/listener_windows.c`
  - shared mapping: `src/keyboard/common/windows_keymap.c`
- Linux:
  - sender: `src/keyboard/sender/sender_uinput.c`
  - listener: `src/keyboard/listener/listener_linux.c`
  - shared mapping: `src/keyboard/common/linux_keysym.c`
  - layout detection: `src/keyboard/common/linux_layout.c`

## Testing

- `python build.py test` runs the non-interactive C unit tests.
- `python build.py test-integration` builds and runs the interactive integration tests.
- Integration tests are intentionally manual because they depend on real focus,
  permissions, and device state.

## Dependency Policy

- No CMake, Conan, or vcpkg metadata remains.
- The only vendored third-party code is `stb_ds.h`.
- Linux dependencies are discovered through `pkg-config`.
- Linux uses system shared libraries for `libinput`, `libudev`, and
  `xkbcommon`; they must remain dynamically linked for legal/compliance
  reasons.
- `python build.py package` includes the project `LICENSE` and the full
  `vendor/licenses/` directory in the release archive.
- `python build.py package` also includes `docs/consumers/README.md` in the
  release archive for downstream integration guidance.

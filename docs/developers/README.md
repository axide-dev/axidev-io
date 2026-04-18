# Developer Guide

## Build

```sh
make
make test
make example
```

Additional targets:

- `make test-unit`
- `make test-integration`
- `make clean`

## Repo Layout

- `include/axidev-io/c_api.h`: primary public API
- `include/axidev-io/log.h`: optional logging macros
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

- `make test` runs the non-interactive C unit tests.
- `make test-integration` builds and runs the interactive integration tests.
- Integration tests are intentionally manual because they depend on real focus,
  permissions, and device state.

## Dependency Policy

- No CMake, Conan, or vcpkg metadata remains.
- The only vendored third-party code is `stb_ds.h`.
- Linux dependencies are discovered through `pkg-config`.

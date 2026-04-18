# Consumer Guide

## Include Surface

Use `include/axidev-io/c_api.h` as the normal entry header.

Optional logging macros live in `include/axidev-io/log.h`.

## Build

```sh
python build.py
python build.py example
```

On Linux the project expects system packages for:

- `libinput`
- `libudev`
- `xkbcommon`

## Basic Usage

```c
#include <axidev-io/c_api.h>

int main(void) {
  if (!axidev_io_keyboard_initialize()) {
    return 1;
  }

  axidev_io_keyboard_type_text("Hello");
  axidev_io_keyboard_tap((axidev_io_keyboard_key_with_modifier_t){
      .key = AXIDEV_IO_KEY_A,
      .mods = AXIDEV_IO_MOD_SHIFT
  });

  axidev_io_keyboard_free();
  return 0;
}
```

## Text Semantics

- `axidev_io_keyboard_type_text(const char *)` is the preferred public send
  path.
- Printable characters are resolved through the initialized keymap so the
  library sends the physical key and modifier sequence that produces the
  requested output on the active layout.
- Modifier literals such as `Ctrl+` and `Shift+` are parsed case-insensitively.
- A comma resets latched modifier literals for the next segment.

Example:

- `Ctrl+Shift+ca,E` means `Ctrl+Shift+C`, `Ctrl+Shift+A`, then uppercase `E`
  after the comma reset.

## Listener

- `axidev_io_listener_start()` starts the single global listener.
- Callbacks may run on an internal background thread.
- Keep listener callbacks thread-safe and short.

## Errors And Logging

- Failure details are available through `axidev_io_get_last_error()`.
- Strings returned by the library must be freed with `axidev_io_free_string()`.
- Logging can be controlled with `axidev_io_log_set_level()` or the macros from
  `log.h`.

## Platform Notes

- Windows uses the Win32 keyboard APIs for injection and a low-level hook for
  listening.
- Linux injection uses `uinput`.
- Linux listening uses `libinput` plus `xkbcommon`.
- macOS is not supported in this repository.

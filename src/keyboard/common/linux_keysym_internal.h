#pragma once
#ifndef AXIDEV_IO_KEYBOARD_LINUX_KEYSYM_INTERNAL_H
#define AXIDEV_IO_KEYBOARD_LINUX_KEYSYM_INTERNAL_H

#if defined(__linux__)

#include <linux/input-event-codes.h>
#include <xkbcommon/xkbcommon.h>

#include "keymap_internal.h"

typedef struct axidev_io_linux_keymap {
  axidev_io_keymap_key_to_int_entry *key_to_evdev;
  axidev_io_keymap_int_to_key_entry *evdev_to_key;
  axidev_io_keymap_char_mapping_entry *char_to_keycode;
  axidev_io_keymap_uint_to_key_entry *code_and_mods_to_key;
} axidev_io_linux_keymap;

uint32_t axidev_io_encode_evdev_mods(int evdev_code,
                                     axidev_io_keyboard_modifier_t mods);
axidev_io_keyboard_key_t axidev_io_linux_keysym_to_key(xkb_keysym_t sym);
void axidev_io_linux_keymap_fill_fallback(axidev_io_linux_keymap *keymap);
void axidev_io_linux_keymap_init(axidev_io_linux_keymap *out_keymap,
                                 struct xkb_keymap *keymap,
                                 struct xkb_state *state);
void axidev_io_linux_keymap_free(axidev_io_linux_keymap *keymap);
axidev_io_keyboard_key_t axidev_io_linux_resolve_key_from_evdev_and_mods(
    const axidev_io_linux_keymap *keymap, int evdev_code,
    axidev_io_keyboard_modifier_t mods);

#endif

#endif

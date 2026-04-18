#pragma once
#ifndef AXIDEV_IO_KEYBOARD_WINDOWS_KEYMAP_INTERNAL_H
#define AXIDEV_IO_KEYBOARD_WINDOWS_KEYMAP_INTERNAL_H

#ifdef _WIN32

#include <Windows.h>

#include "keymap_internal.h"

typedef struct axidev_io_windows_keymap {
  axidev_io_keymap_key_to_int_entry *key_to_vk;
  axidev_io_keymap_int_to_key_entry *vk_to_key;
  axidev_io_keymap_char_mapping_entry *char_to_keycode;
  axidev_io_keymap_uint_to_key_entry *vk_and_mods_to_key;
} axidev_io_windows_keymap;

uint32_t axidev_io_encode_vk_mods(WORD vk, axidev_io_keyboard_modifier_t mods);
void axidev_io_windows_keymap_init(axidev_io_windows_keymap *out_keymap,
                                   HKL layout);
void axidev_io_windows_keymap_free(axidev_io_windows_keymap *keymap);
axidev_io_keyboard_key_t axidev_io_windows_resolve_key_from_vk_and_mods(
    const axidev_io_windows_keymap *keymap, WORD vk,
    axidev_io_keyboard_modifier_t mods);
bool axidev_io_is_windows_extended_key(WORD vk);

#endif

#endif

#pragma once
#ifndef AXIDEV_IO_KEYBOARD_KEYMAP_INTERNAL_H
#define AXIDEV_IO_KEYBOARD_KEYMAP_INTERNAL_H

#include "../../internal/context.h"

#include <stb/stb_ds.h>

typedef struct axidev_io_keymap_char_entry {
  uint32_t key;
  axidev_io_keyboard_key_with_modifier_t value;
} axidev_io_keymap_char_entry;

typedef struct axidev_io_keyboard_mapping_value {
  int32_t keycode;
  axidev_io_keyboard_modifier_t required_mods;
  axidev_io_keyboard_key_t produced_key;
} axidev_io_keyboard_mapping_value;

typedef struct axidev_io_keymap_char_mapping_entry {
  uint32_t key;
  axidev_io_keyboard_mapping_value value;
} axidev_io_keymap_char_mapping_entry;

typedef struct axidev_io_keymap_int_to_key_entry {
  int32_t key;
  axidev_io_keyboard_key_t value;
} axidev_io_keymap_int_to_key_entry;

typedef struct axidev_io_keymap_uint_to_key_entry {
  uint32_t key;
  axidev_io_keyboard_key_t value;
} axidev_io_keymap_uint_to_key_entry;

typedef struct axidev_io_keymap_key_to_int_entry {
  uint32_t key;
  int32_t value;
} axidev_io_keymap_key_to_int_entry;

typedef struct axidev_io_keyboard_keymap_impl {
  axidev_io_keymap_char_mapping_entry *char_to_mapping;
  axidev_io_keymap_int_to_key_entry *code_to_key;
  axidev_io_keymap_uint_to_key_entry *code_and_mods_to_key;
  axidev_io_keymap_key_to_int_entry *key_to_code;
} axidev_io_keyboard_keymap_impl;

typedef struct axidev_io_keyboard_keymap_lookup {
  int32_t keycode;
  axidev_io_keyboard_modifier_t required_mods;
  axidev_io_keyboard_key_t produced_key;
} axidev_io_keyboard_keymap_lookup;

axidev_io_keyboard_keymap_impl *axidev_io_keymap_impl_get(void);

axidev_io_result axidev_io_keyboard_keymap_initialize(void);
void axidev_io_keyboard_keymap_free(void);

axidev_io_result axidev_io_keymap_lookup_character(
    uint32_t codepoint, axidev_io_keyboard_key_with_modifier_t *out_key);
axidev_io_result
axidev_io_keymap_lookup_mapping(uint32_t codepoint,
                                axidev_io_keyboard_keymap_lookup *out_mapping);
axidev_io_result
axidev_io_keymap_key_from_code(int32_t keycode,
                               axidev_io_keyboard_modifier_t mods,
                               axidev_io_keyboard_key_t *out_key);
axidev_io_result
axidev_io_keymap_base_key_from_code(int32_t keycode,
                                    axidev_io_keyboard_key_t *out_key);
axidev_io_result axidev_io_keymap_code_for_key(axidev_io_keyboard_key_t key,
                                               int32_t *out_keycode);
axidev_io_result axidev_io_keymap_resolve_key_request(
    axidev_io_keyboard_key_with_modifier_t request, int32_t *out_keycode,
    axidev_io_keyboard_modifier_t *out_mods,
    axidev_io_keyboard_key_t *out_resolved_key);
bool axidev_io_keymap_can_type_character(uint32_t codepoint);
uint32_t axidev_io_keymap_encode_code_mods(int32_t keycode,
                                           axidev_io_keyboard_modifier_t mods);

bool axidev_io_keyboard_key_to_codepoint(axidev_io_keyboard_key_t key,
                                         uint32_t *out_codepoint);

#endif

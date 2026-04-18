#pragma once
#ifndef AXIDEV_IO_KEYBOARD_KEY_UTILS_INTERNAL_H
#define AXIDEV_IO_KEYBOARD_KEY_UTILS_INTERNAL_H

#include "../../internal/context.h"

const char *axidev_io_key_to_string_const(axidev_io_keyboard_key_t key);
char *axidev_io_key_to_string_alloc(axidev_io_keyboard_key_t key);
axidev_io_keyboard_key_t axidev_io_string_to_key_internal(const char *input);
char *
axidev_io_key_to_string_with_modifier_alloc(axidev_io_keyboard_key_t key,
                                            axidev_io_keyboard_modifier_t mods);
bool axidev_io_string_to_key_with_modifier_internal(
    const char *input, axidev_io_keyboard_key_with_modifier_t *out_key_mod);

#endif

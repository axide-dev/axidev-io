#include "keymap_internal.h"

#include <axidev-io/c_api.h>

#include <string.h>

#include "key_utils_internal.h"

#ifdef _WIN32
#include "windows_keymap_internal.h"
#elif defined(__linux__)
#include "linux_keysym_internal.h"
#include "linux_layout_internal.h"
#include <xkbcommon/xkbcommon.h>
#endif

axidev_io_keyboard_keymap_impl *axidev_io_keymap_impl_get(void) {
  return (axidev_io_keyboard_keymap_impl *)axidev_io_keymap_storage_ptr();
}

uint32_t axidev_io_keymap_encode_code_mods(int32_t keycode,
                                           axidev_io_keyboard_modifier_t mods) {
  uint8_t mod_bits = 0;

  if (axidev_io_keyboard_has_modifier(mods, AXIDEV_IO_MOD_SHIFT)) {
    mod_bits |= 0x01u;
  }
  if (axidev_io_keyboard_has_modifier(mods, AXIDEV_IO_MOD_CTRL)) {
    mod_bits |= 0x02u;
  }
  if (axidev_io_keyboard_has_modifier(mods, AXIDEV_IO_MOD_ALT)) {
    mod_bits |= 0x04u;
  }

  return ((uint32_t)keycode << 8) | mod_bits;
}

static void axidev_io_keymap_copy_char_mappings(
    axidev_io_keymap_char_mapping_entry *source,
    axidev_io_keymap_char_mapping_entry **destination) {
  ptrdiff_t index;

  if (source == NULL) {
    return;
  }

  for (index = 0; index < hmlen(source); ++index) {
    hmput(*destination, source[index].key, source[index].value);
  }
}

static void axidev_io_keymap_copy_int_key_mappings(
    axidev_io_keymap_int_to_key_entry *source,
    axidev_io_keymap_int_to_key_entry **destination) {
  ptrdiff_t index;

  if (source == NULL) {
    return;
  }

  for (index = 0; index < hmlen(source); ++index) {
    hmput(*destination, source[index].key, source[index].value);
  }
}

static void axidev_io_keymap_copy_uint_key_mappings(
    axidev_io_keymap_uint_to_key_entry *source,
    axidev_io_keymap_uint_to_key_entry **destination) {
  ptrdiff_t index;

  if (source == NULL) {
    return;
  }

  for (index = 0; index < hmlen(source); ++index) {
    hmput(*destination, source[index].key, source[index].value);
  }
}

static void axidev_io_keymap_copy_key_int_mappings(
    axidev_io_keymap_key_to_int_entry *source,
    axidev_io_keymap_key_to_int_entry **destination) {
  ptrdiff_t index;

  if (source == NULL) {
    return;
  }

  for (index = 0; index < hmlen(source); ++index) {
    hmput(*destination, source[index].key, source[index].value);
  }
}

axidev_io_result axidev_io_keyboard_keymap_initialize(void) {
  axidev_io_keyboard_keymap_impl *impl = axidev_io_keymap_impl_get();

  axidev_io_keyboard_keymap_free();
  memset(impl, 0, sizeof(*impl));

#ifdef _WIN32
  {
    axidev_io_windows_keymap windows_keymap;
    memset(&windows_keymap, 0, sizeof(windows_keymap));
    axidev_io_windows_keymap_init(&windows_keymap, GetKeyboardLayout(0));
    axidev_io_keymap_copy_char_mappings(windows_keymap.char_to_keycode,
                                        &impl->char_to_mapping);
    axidev_io_keymap_copy_int_key_mappings(windows_keymap.vk_to_key,
                                           &impl->code_to_key);
    axidev_io_keymap_copy_uint_key_mappings(windows_keymap.vk_and_mods_to_key,
                                            &impl->code_and_mods_to_key);
    axidev_io_keymap_copy_key_int_mappings(windows_keymap.key_to_vk,
                                           &impl->key_to_code);
    axidev_io_windows_keymap_free(&windows_keymap);
    axidev_io_global->keyboard.backend_type = AXIDEV_IO_BACKEND_WINDOWS;
  }
#elif defined(__linux__)
  {
    axidev_io_linux_keymap linux_keymap;
    struct xkb_context *xkb_context = NULL;
    struct xkb_keymap *xkb_keymap = NULL;
    struct xkb_state *xkb_state = NULL;
    memset(&linux_keymap, 0, sizeof(linux_keymap));
    xkb_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    if (xkb_context != NULL) {
      axidev_io_xkb_rule_names_strings names =
          axidev_io_detect_xkb_rule_names();
      struct xkb_rule_names native_names;
      memset(&native_names, 0, sizeof(native_names));
      native_names.rules = names.rules[0] != '\0' ? names.rules : NULL;
      native_names.model = names.model[0] != '\0' ? names.model : NULL;
      native_names.layout = names.layout[0] != '\0' ? names.layout : NULL;
      native_names.variant = names.variant[0] != '\0' ? names.variant : NULL;
      native_names.options = names.options[0] != '\0' ? names.options : NULL;
      xkb_keymap = xkb_keymap_new_from_names(
          xkb_context, names.has_any ? &native_names : NULL,
          XKB_KEYMAP_COMPILE_NO_FLAGS);
      if (xkb_keymap != NULL) {
        xkb_state = xkb_state_new(xkb_keymap);
      }
    }
    axidev_io_linux_keymap_init(&linux_keymap, xkb_keymap, xkb_state);
    axidev_io_keymap_copy_char_mappings(linux_keymap.char_to_keycode,
                                        &impl->char_to_mapping);
    axidev_io_keymap_copy_int_key_mappings(linux_keymap.evdev_to_key,
                                           &impl->code_to_key);
    axidev_io_keymap_copy_uint_key_mappings(linux_keymap.code_and_mods_to_key,
                                            &impl->code_and_mods_to_key);
    axidev_io_keymap_copy_key_int_mappings(linux_keymap.key_to_evdev,
                                           &impl->key_to_code);
    axidev_io_linux_keymap_free(&linux_keymap);
    if (xkb_state != NULL) {
      xkb_state_unref(xkb_state);
    }
    if (xkb_keymap != NULL) {
      xkb_keymap_unref(xkb_keymap);
    }
    if (xkb_context != NULL) {
      xkb_context_unref(xkb_context);
    }
  }
#else
  return AXIDEV_IO_RESULT_NOT_SUPPORTED;
#endif

  axidev_io_keymap_public_context()->initialized = true;
  AXIDEV_IO_LOG_DEBUG("keymap initialized: char=%td code=%td code+mods=%td "
                      "key=%td",
                      hmlen(impl->char_to_mapping), hmlen(impl->code_to_key),
                      hmlen(impl->code_and_mods_to_key),
                      hmlen(impl->key_to_code));
  return AXIDEV_IO_RESULT_OK;
}

void axidev_io_keyboard_keymap_free(void) {
  axidev_io_keyboard_keymap_impl *impl = axidev_io_keymap_impl_get();

  hmfree(impl->char_to_mapping);
  hmfree(impl->code_to_key);
  hmfree(impl->code_and_mods_to_key);
  hmfree(impl->key_to_code);
  memset(impl, 0, sizeof(*impl));
  axidev_io_keymap_public_context()->initialized = false;
}

axidev_io_result axidev_io_keymap_lookup_character(
    uint32_t codepoint, axidev_io_keyboard_key_with_modifier_t *out_key) {
  axidev_io_keyboard_keymap_lookup mapping;
  axidev_io_result result;

  if (out_key == NULL) {
    return AXIDEV_IO_RESULT_INVALID_ARGUMENT;
  }

  result = axidev_io_keymap_lookup_mapping(codepoint, &mapping);
  if (result != AXIDEV_IO_RESULT_OK) {
    return result;
  }

  out_key->key = mapping.produced_key;
  out_key->mods = mapping.required_mods;
  return AXIDEV_IO_RESULT_OK;
}

axidev_io_result
axidev_io_keymap_lookup_mapping(uint32_t codepoint,
                                axidev_io_keyboard_keymap_lookup *out_mapping) {
  axidev_io_keyboard_keymap_impl *impl = axidev_io_keymap_impl_get();
  ptrdiff_t index;

  if (out_mapping == NULL) {
    return AXIDEV_IO_RESULT_INVALID_ARGUMENT;
  }

  if (!axidev_io_keymap_public_context()->initialized) {
    return AXIDEV_IO_RESULT_NOT_INITIALIZED;
  }

  index = hmgeti(impl->char_to_mapping, codepoint);
  if (index < 0) {
    return AXIDEV_IO_RESULT_NOT_FOUND;
  }

  out_mapping->keycode = impl->char_to_mapping[index].value.keycode;
  out_mapping->required_mods = impl->char_to_mapping[index].value.required_mods;
  out_mapping->produced_key = impl->char_to_mapping[index].value.produced_key;

  if (out_mapping->produced_key == AXIDEV_IO_KEY_UNKNOWN &&
      out_mapping->keycode >= 0) {
    axidev_io_keyboard_key_t key;
    if (axidev_io_keymap_base_key_from_code(out_mapping->keycode, &key) ==
        AXIDEV_IO_RESULT_OK) {
      out_mapping->produced_key = key;
    }
  }

  return AXIDEV_IO_RESULT_OK;
}

axidev_io_result
axidev_io_keymap_key_from_code(int32_t keycode,
                               axidev_io_keyboard_modifier_t mods,
                               axidev_io_keyboard_key_t *out_key) {
  axidev_io_keyboard_keymap_impl *impl = axidev_io_keymap_impl_get();
  uint32_t encoded_code_mods;
  ptrdiff_t index;

  if (out_key == NULL) {
    return AXIDEV_IO_RESULT_INVALID_ARGUMENT;
  }

  if (!axidev_io_keymap_public_context()->initialized) {
    return AXIDEV_IO_RESULT_NOT_INITIALIZED;
  }

  encoded_code_mods = axidev_io_keymap_encode_code_mods(keycode, mods);
  index = hmgeti(impl->code_and_mods_to_key, encoded_code_mods);
  if (index >= 0) {
    *out_key = impl->code_and_mods_to_key[index].value;
    return AXIDEV_IO_RESULT_OK;
  }

  return axidev_io_keymap_base_key_from_code(keycode, out_key);
}

axidev_io_result
axidev_io_keymap_base_key_from_code(int32_t keycode,
                                    axidev_io_keyboard_key_t *out_key) {
  axidev_io_keyboard_keymap_impl *impl = axidev_io_keymap_impl_get();
  ptrdiff_t index;

  if (out_key == NULL) {
    return AXIDEV_IO_RESULT_INVALID_ARGUMENT;
  }

  if (!axidev_io_keymap_public_context()->initialized) {
    return AXIDEV_IO_RESULT_NOT_INITIALIZED;
  }

  index = hmgeti(impl->code_to_key, keycode);
  if (index < 0) {
    return AXIDEV_IO_RESULT_NOT_FOUND;
  }

  *out_key = impl->code_to_key[index].value;
  return AXIDEV_IO_RESULT_OK;
}

axidev_io_result axidev_io_keymap_code_for_key(axidev_io_keyboard_key_t key,
                                               int32_t *out_keycode) {
  axidev_io_keyboard_keymap_impl *impl = axidev_io_keymap_impl_get();
  uint32_t key_id;
  ptrdiff_t index;

  if (out_keycode == NULL) {
    return AXIDEV_IO_RESULT_INVALID_ARGUMENT;
  }

  if (!axidev_io_keymap_public_context()->initialized) {
    return AXIDEV_IO_RESULT_NOT_INITIALIZED;
  }

  key_id = (uint32_t)key;
  index = hmgeti(impl->key_to_code, key_id);
  if (index < 0) {
    return AXIDEV_IO_RESULT_NOT_FOUND;
  }

  *out_keycode = impl->key_to_code[index].value;
  return AXIDEV_IO_RESULT_OK;
}

axidev_io_result axidev_io_keymap_resolve_key_request(
    axidev_io_keyboard_key_with_modifier_t request, int32_t *out_keycode,
    axidev_io_keyboard_modifier_t *out_mods,
    axidev_io_keyboard_key_t *out_resolved_key) {
  axidev_io_result result;
  uint32_t codepoint;

  if (out_keycode == NULL || out_mods == NULL || out_resolved_key == NULL) {
    return AXIDEV_IO_RESULT_INVALID_ARGUMENT;
  }

  if (axidev_io_keyboard_key_to_codepoint(request.key, &codepoint)) {
    axidev_io_keyboard_keymap_lookup mapping;

    result = axidev_io_keymap_lookup_mapping(codepoint, &mapping);
    if (result == AXIDEV_IO_RESULT_OK && mapping.produced_key == request.key) {
      *out_keycode = mapping.keycode;
      *out_mods = (axidev_io_keyboard_modifier_t)(mapping.required_mods |
                                                  request.mods);
      *out_resolved_key = mapping.produced_key;
      return AXIDEV_IO_RESULT_OK;
    }
  }

  result = axidev_io_keymap_code_for_key(request.key, out_keycode);
  if (result != AXIDEV_IO_RESULT_OK) {
    return result;
  }

  *out_mods = request.mods;
  *out_resolved_key = request.key;
  return AXIDEV_IO_RESULT_OK;
}

bool axidev_io_keymap_can_type_character(uint32_t codepoint) {
  axidev_io_keyboard_keymap_impl *impl = axidev_io_keymap_impl_get();
  return axidev_io_keymap_public_context()->initialized &&
         hmgeti(impl->char_to_mapping, codepoint) >= 0;
}

bool axidev_io_keyboard_key_to_codepoint(axidev_io_keyboard_key_t key,
                                         uint32_t *out_codepoint) {
  if (out_codepoint == NULL) {
    return false;
  }

  if (key >= AXIDEV_IO_KEY_A && key <= AXIDEV_IO_KEY_Z) {
    *out_codepoint = (uint32_t)('a' + (key - AXIDEV_IO_KEY_A));
    return true;
  }

  if (key >= AXIDEV_IO_KEY_NUM0 && key <= AXIDEV_IO_KEY_NUM9) {
    *out_codepoint = (uint32_t)('0' + (key - AXIDEV_IO_KEY_NUM0));
    return true;
  }

  switch (key) {
  case AXIDEV_IO_KEY_SPACE:
    *out_codepoint = ' ';
    return true;
  case AXIDEV_IO_KEY_GRAVE:
    *out_codepoint = '`';
    return true;
  case AXIDEV_IO_KEY_MINUS:
    *out_codepoint = '-';
    return true;
  case AXIDEV_IO_KEY_EQUAL:
    *out_codepoint = '=';
    return true;
  case AXIDEV_IO_KEY_LEFT_BRACKET:
    *out_codepoint = '[';
    return true;
  case AXIDEV_IO_KEY_RIGHT_BRACKET:
    *out_codepoint = ']';
    return true;
  case AXIDEV_IO_KEY_BACKSLASH:
    *out_codepoint = '\\';
    return true;
  case AXIDEV_IO_KEY_SEMICOLON:
    *out_codepoint = ';';
    return true;
  case AXIDEV_IO_KEY_APOSTROPHE:
    *out_codepoint = '\'';
    return true;
  case AXIDEV_IO_KEY_COMMA:
    *out_codepoint = ',';
    return true;
  case AXIDEV_IO_KEY_PERIOD:
    *out_codepoint = '.';
    return true;
  case AXIDEV_IO_KEY_SLASH:
    *out_codepoint = '/';
    return true;
  case AXIDEV_IO_KEY_AT:
    *out_codepoint = '@';
    return true;
  case AXIDEV_IO_KEY_EXCLAMATION:
    *out_codepoint = '!';
    return true;
  case AXIDEV_IO_KEY_DOLLAR:
    *out_codepoint = '$';
    return true;
  case AXIDEV_IO_KEY_CARET:
    *out_codepoint = '^';
    return true;
  case AXIDEV_IO_KEY_AMPERSAND:
    *out_codepoint = '&';
    return true;
  case AXIDEV_IO_KEY_ASTERISK:
    *out_codepoint = '*';
    return true;
  case AXIDEV_IO_KEY_LEFT_PAREN:
    *out_codepoint = '(';
    return true;
  case AXIDEV_IO_KEY_RIGHT_PAREN:
    *out_codepoint = ')';
    return true;
  case AXIDEV_IO_KEY_UNDERSCORE:
    *out_codepoint = '_';
    return true;
  case AXIDEV_IO_KEY_PLUS:
    *out_codepoint = '+';
    return true;
  case AXIDEV_IO_KEY_COLON:
    *out_codepoint = ':';
    return true;
  case AXIDEV_IO_KEY_QUOTE:
    *out_codepoint = '"';
    return true;
  case AXIDEV_IO_KEY_QUESTION_MARK:
    *out_codepoint = '?';
    return true;
  case AXIDEV_IO_KEY_BAR:
    *out_codepoint = '|';
    return true;
  case AXIDEV_IO_KEY_LESS_THAN:
    *out_codepoint = '<';
    return true;
  case AXIDEV_IO_KEY_GREATER_THAN:
    *out_codepoint = '>';
    return true;
  default:
    return false;
  }
}

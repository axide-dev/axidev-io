#ifdef _WIN32

#include "sender_internal.h"

#include <Windows.h>

#include <axidev-io/c_api.h>

#include <string.h>

#include "../common/key_utils_internal.h"
#include "../common/windows_keymap_internal.h"

axidev_io_keyboard_sender_impl *axidev_io_sender_impl_get(void) {
  return (axidev_io_keyboard_sender_impl *)axidev_io_sender_storage_ptr();
}

static void axidev_io_sender_delay(void) {
  uint32_t delay_us = axidev_io_sender_public_context()->key_delay_us;
  if (delay_us != 0) {
    axidev_io_sleep_us(delay_us);
  }
}

static void axidev_io_sender_update_modifier_state(axidev_io_keyboard_key_t key,
                                                   bool down) {
  axidev_io_keyboard_sender_context *sender = axidev_io_sender_public_context();
  axidev_io_keyboard_modifier_t flag = AXIDEV_IO_MOD_NONE;

  switch (key) {
  case AXIDEV_IO_KEY_SHIFT_LEFT:
  case AXIDEV_IO_KEY_SHIFT_RIGHT:
    flag = AXIDEV_IO_MOD_SHIFT;
    break;
  case AXIDEV_IO_KEY_CTRL_LEFT:
  case AXIDEV_IO_KEY_CTRL_RIGHT:
    flag = AXIDEV_IO_MOD_CTRL;
    break;
  case AXIDEV_IO_KEY_ALT_LEFT:
  case AXIDEV_IO_KEY_ALT_RIGHT:
    flag = AXIDEV_IO_MOD_ALT;
    break;
  case AXIDEV_IO_KEY_SUPER_LEFT:
  case AXIDEV_IO_KEY_SUPER_RIGHT:
    flag = AXIDEV_IO_MOD_SUPER;
    break;
  default:
    break;
  }

  if (flag == AXIDEV_IO_MOD_NONE) {
    return;
  }

  if (down) {
    sender->active_modifiers =
        axidev_io_keyboard_add_modifier(sender->active_modifiers, flag);
  } else {
    sender->active_modifiers =
        axidev_io_keyboard_remove_modifier(sender->active_modifiers, flag);
  }
}

static axidev_io_result axidev_io_windows_send_vk(WORD vk, bool down) {
  INPUT input;

  memset(&input, 0, sizeof(input));
  input.type = INPUT_KEYBOARD;
  input.ki.wVk = vk;
  input.ki.wScan = (WORD)MapVirtualKeyW(vk, MAPVK_VK_TO_VSC);
  input.ki.dwFlags = KEYEVENTF_SCANCODE;
  if (axidev_io_is_windows_extended_key(vk)) {
    input.ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;
  }
  if (!down) {
    input.ki.dwFlags |= KEYEVENTF_KEYUP;
  }

  if (SendInput(1, &input, sizeof(input)) == 0) {
    return AXIDEV_IO_RESULT_PLATFORM_ERROR;
  }
  return AXIDEV_IO_RESULT_OK;
}

static axidev_io_result axidev_io_windows_send_unicode(uint32_t codepoint) {
  INPUT inputs[4];
  size_t count = 0;

  memset(inputs, 0, sizeof(inputs));
  if (codepoint <= 0xFFFFu) {
    inputs[count].type = INPUT_KEYBOARD;
    inputs[count].ki.wScan = (WORD)codepoint;
    inputs[count].ki.dwFlags = KEYEVENTF_UNICODE;
    ++count;
    inputs[count] = inputs[count - 1];
    inputs[count].ki.dwFlags |= KEYEVENTF_KEYUP;
    ++count;
  } else if (codepoint <= 0x10FFFFu) {
    uint32_t value = codepoint - 0x10000u;
    WORD high = (WORD)(0xD800u | (value >> 10));
    WORD low = (WORD)(0xDC00u | (value & 0x3FFu));
    inputs[count].type = INPUT_KEYBOARD;
    inputs[count].ki.wScan = high;
    inputs[count].ki.dwFlags = KEYEVENTF_UNICODE;
    ++count;
    inputs[count] = inputs[count - 1];
    inputs[count].ki.dwFlags |= KEYEVENTF_KEYUP;
    ++count;
    inputs[count].type = INPUT_KEYBOARD;
    inputs[count].ki.wScan = low;
    inputs[count].ki.dwFlags = KEYEVENTF_UNICODE;
    ++count;
    inputs[count] = inputs[count - 1];
    inputs[count].ki.dwFlags |= KEYEVENTF_KEYUP;
    ++count;
  } else {
    return AXIDEV_IO_RESULT_INVALID_ARGUMENT;
  }

  if (SendInput((UINT)count, inputs, sizeof(INPUT)) == 0) {
    return AXIDEV_IO_RESULT_PLATFORM_ERROR;
  }
  return AXIDEV_IO_RESULT_OK;
}

static axidev_io_result
axidev_io_sender_resolve_mapping(axidev_io_keyboard_key_with_modifier_t request,
                                 int32_t *out_keycode,
                                 axidev_io_keyboard_modifier_t *out_mods,
                                 axidev_io_keyboard_key_t *out_resolved_key) {
  return axidev_io_keymap_resolve_key_request(request, out_keycode, out_mods,
                                              out_resolved_key);
}

static axidev_io_result
axidev_io_sender_send_raw_key(axidev_io_keyboard_key_t key, int32_t keycode,
                              bool down) {
  axidev_io_result result = axidev_io_windows_send_vk((WORD)keycode, down);
  if (result == AXIDEV_IO_RESULT_OK) {
    axidev_io_sender_update_modifier_state(key, down);
  }
  return result;
}

axidev_io_result axidev_io_keyboard_sender_initialize(void) {
  axidev_io_keyboard_sender_impl *impl = axidev_io_sender_impl_get();
  axidev_io_keyboard_sender_context *sender;

  memset(impl, 0, sizeof(*impl));
  axidev_io_keyboard_reset_public_sender_state();
  sender = axidev_io_sender_public_context();

  impl->layout = GetKeyboardLayout(0);
  sender->initialized = true;
  sender->ready = true;
  sender->capabilities.can_inject_keys = true;
  sender->capabilities.can_inject_text = true;
  sender->capabilities.can_simulate_hid = false;
  sender->capabilities.supports_key_repeat = false;
  sender->capabilities.needs_accessibility_perm = false;
  sender->capabilities.needs_input_monitoring_perm = false;
  sender->capabilities.needs_uinput_access = false;
  axidev_io_global->keyboard.backend_type = AXIDEV_IO_BACKEND_WINDOWS;
  return AXIDEV_IO_RESULT_OK;
}

void axidev_io_keyboard_sender_free(void) {
  memset(axidev_io_sender_impl_get(), 0, sizeof(*axidev_io_sender_impl_get()));
  axidev_io_keyboard_reset_public_sender_state();
}

axidev_io_result axidev_io_keyboard_sender_request_permissions(void) {
  return AXIDEV_IO_RESULT_OK;
}

axidev_io_result axidev_io_keyboard_sender_hold_modifier_internal(
    axidev_io_keyboard_modifier_t mods) {
  axidev_io_result result = AXIDEV_IO_RESULT_OK;

  if (axidev_io_keyboard_has_modifier(mods, AXIDEV_IO_MOD_SHIFT)) {
    result = axidev_io_sender_send_raw_key(AXIDEV_IO_KEY_SHIFT_LEFT, VK_LSHIFT,
                                           true);
    if (result != AXIDEV_IO_RESULT_OK) {
      return result;
    }
  }
  if (axidev_io_keyboard_has_modifier(mods, AXIDEV_IO_MOD_CTRL)) {
    result = axidev_io_sender_send_raw_key(AXIDEV_IO_KEY_CTRL_LEFT, VK_LCONTROL,
                                           true);
    if (result != AXIDEV_IO_RESULT_OK) {
      return result;
    }
  }
  if (axidev_io_keyboard_has_modifier(mods, AXIDEV_IO_MOD_ALT)) {
    result =
        axidev_io_sender_send_raw_key(AXIDEV_IO_KEY_ALT_LEFT, VK_LMENU, true);
    if (result != AXIDEV_IO_RESULT_OK) {
      return result;
    }
  }
  if (axidev_io_keyboard_has_modifier(mods, AXIDEV_IO_MOD_SUPER)) {
    result =
        axidev_io_sender_send_raw_key(AXIDEV_IO_KEY_SUPER_LEFT, VK_LWIN, true);
  }
  return result;
}

axidev_io_result axidev_io_keyboard_sender_release_modifier_internal(
    axidev_io_keyboard_modifier_t mods) {
  axidev_io_result result = AXIDEV_IO_RESULT_OK;

  if (axidev_io_keyboard_has_modifier(mods, AXIDEV_IO_MOD_SHIFT)) {
    result = axidev_io_sender_send_raw_key(AXIDEV_IO_KEY_SHIFT_LEFT, VK_LSHIFT,
                                           false);
    if (result != AXIDEV_IO_RESULT_OK) {
      return result;
    }
  }
  if (axidev_io_keyboard_has_modifier(mods, AXIDEV_IO_MOD_CTRL)) {
    result = axidev_io_sender_send_raw_key(AXIDEV_IO_KEY_CTRL_LEFT, VK_LCONTROL,
                                           false);
    if (result != AXIDEV_IO_RESULT_OK) {
      return result;
    }
  }
  if (axidev_io_keyboard_has_modifier(mods, AXIDEV_IO_MOD_ALT)) {
    result =
        axidev_io_sender_send_raw_key(AXIDEV_IO_KEY_ALT_LEFT, VK_LMENU, false);
    if (result != AXIDEV_IO_RESULT_OK) {
      return result;
    }
  }
  if (axidev_io_keyboard_has_modifier(mods, AXIDEV_IO_MOD_SUPER)) {
    result =
        axidev_io_sender_send_raw_key(AXIDEV_IO_KEY_SUPER_LEFT, VK_LWIN, false);
  }
  return result;
}

axidev_io_result
axidev_io_keyboard_sender_release_all_modifiers_internal(void) {
  return axidev_io_keyboard_sender_release_modifier_internal(
      AXIDEV_IO_MOD_SHIFT | AXIDEV_IO_MOD_CTRL | AXIDEV_IO_MOD_ALT |
      AXIDEV_IO_MOD_SUPER);
}

axidev_io_result axidev_io_keyboard_sender_key_down_internal(
    axidev_io_keyboard_key_with_modifier_t key_mod) {
  int32_t keycode;
  axidev_io_keyboard_modifier_t mods;
  axidev_io_keyboard_key_t resolved_key;
  axidev_io_result result;

  result =
      axidev_io_sender_resolve_mapping(key_mod, &keycode, &mods, &resolved_key);
  if (result != AXIDEV_IO_RESULT_OK) {
    return result;
  }

  result = axidev_io_keyboard_sender_hold_modifier_internal(mods);
  if (result != AXIDEV_IO_RESULT_OK) {
    return result;
  }

  return axidev_io_sender_send_raw_key(resolved_key, keycode, true);
}

axidev_io_result axidev_io_keyboard_sender_key_up_internal(
    axidev_io_keyboard_key_with_modifier_t key_mod) {
  int32_t keycode;
  axidev_io_keyboard_modifier_t mods;
  axidev_io_keyboard_key_t resolved_key;
  axidev_io_result result;

  result =
      axidev_io_sender_resolve_mapping(key_mod, &keycode, &mods, &resolved_key);
  if (result != AXIDEV_IO_RESULT_OK) {
    return result;
  }

  result = axidev_io_sender_send_raw_key(resolved_key, keycode, false);
  if (result != AXIDEV_IO_RESULT_OK) {
    return result;
  }
  return axidev_io_keyboard_sender_release_modifier_internal(mods);
}

axidev_io_result axidev_io_keyboard_sender_tap_internal(
    axidev_io_keyboard_key_with_modifier_t key_mod) {
  int32_t keycode;
  axidev_io_keyboard_modifier_t mods;
  axidev_io_keyboard_key_t resolved_key;
  axidev_io_result result;

  result =
      axidev_io_sender_resolve_mapping(key_mod, &keycode, &mods, &resolved_key);
  if (result != AXIDEV_IO_RESULT_OK) {
    return result;
  }

  result = axidev_io_keyboard_sender_hold_modifier_internal(mods);
  if (result != AXIDEV_IO_RESULT_OK) {
    return result;
  }
  axidev_io_sender_delay();
  result = axidev_io_sender_send_raw_key(resolved_key, keycode, true);
  if (result != AXIDEV_IO_RESULT_OK) {
    axidev_io_keyboard_sender_release_modifier_internal(mods);
    return result;
  }
  axidev_io_sender_delay();
  result = axidev_io_sender_send_raw_key(resolved_key, keycode, false);
  axidev_io_sender_delay();
  if (axidev_io_keyboard_sender_release_modifier_internal(mods) !=
      AXIDEV_IO_RESULT_OK) {
    return AXIDEV_IO_RESULT_PLATFORM_ERROR;
  }
  return result;
}

axidev_io_result
axidev_io_keyboard_sender_type_character_internal(uint32_t codepoint) {
  axidev_io_keyboard_key_with_modifier_t key_mod;

  if (axidev_io_keymap_lookup_character(codepoint, &key_mod) ==
      AXIDEV_IO_RESULT_OK) {
    return axidev_io_keyboard_sender_tap_internal(key_mod);
  }

  return axidev_io_windows_send_unicode(codepoint);
}

void axidev_io_keyboard_sender_flush_internal(void) {}

void axidev_io_keyboard_sender_set_key_delay_internal(uint32_t delay_us) {
  axidev_io_sender_public_context()->key_delay_us = delay_us;
}

#endif

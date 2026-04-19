#if defined(__linux__)

#include "sender_internal.h"

#include <axidev-io/c_api.h>

#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "../common/key_utils_internal.h"

axidev_io_keyboard_sender_impl *axidev_io_sender_impl_get(void) {
  return (axidev_io_keyboard_sender_impl *)axidev_io_sender_storage_ptr();
}

static void axidev_io_sender_delay(void) {
  uint32_t delay_us = axidev_io_sender_public_context()->key_delay_us;
  if (delay_us != 0) {
    axidev_io_sleep_us(delay_us);
  }
}

static void
axidev_io_linux_sender_update_modifier_state(axidev_io_keyboard_key_t key,
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

static void axidev_io_linux_emit(int fd, int type, int code, int value) {
  struct input_event event;
  memset(&event, 0, sizeof(event));
  event.type = (unsigned short)type;
  event.code = (unsigned short)code;
  event.value = value;
  write(fd, &event, sizeof(event));
}

static void axidev_io_linux_sync(int fd) {
  axidev_io_linux_emit(fd, EV_SYN, SYN_REPORT, 0);
}

static axidev_io_result axidev_io_linux_send_key(int fd, int keycode,
                                                 bool down) {
  if (fd < 0 || keycode < 0) {
    return AXIDEV_IO_RESULT_PLATFORM_ERROR;
  }
  axidev_io_linux_emit(fd, EV_KEY, keycode, down ? 1 : 0);
  axidev_io_linux_sync(fd);
  return AXIDEV_IO_RESULT_OK;
}

static axidev_io_result
axidev_io_linux_resolve_mapping(axidev_io_keyboard_key_with_modifier_t request,
                                int32_t *out_keycode,
                                axidev_io_keyboard_modifier_t *out_mods,
                                axidev_io_keyboard_key_t *out_key) {
  return axidev_io_keymap_resolve_key_request(request, out_keycode, out_mods,
                                              out_key);
}

static axidev_io_result
axidev_io_linux_send_raw_key(axidev_io_keyboard_key_t key, int32_t keycode,
                             bool down) {
  axidev_io_result result =
      axidev_io_linux_send_key(axidev_io_sender_impl_get()->fd, keycode, down);
  if (result == AXIDEV_IO_RESULT_OK) {
    axidev_io_linux_sender_update_modifier_state(key, down);
  }
  return result;
}

axidev_io_result axidev_io_keyboard_sender_initialize(void) {
  axidev_io_keyboard_sender_impl *impl = axidev_io_sender_impl_get();
  axidev_io_keyboard_sender_context *sender;
  struct uinput_setup setup;
  int keycode;

  memset(impl, 0, sizeof(*impl));
  impl->fd = -1;
  axidev_io_keyboard_reset_public_sender_state();
  sender = axidev_io_sender_public_context();

  impl->fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
  if (impl->fd < 0) {
    return AXIDEV_IO_RESULT_PERMISSION_DENIED;
  }

  ioctl(impl->fd, UI_SET_EVBIT, EV_KEY);
  for (keycode = 0; keycode < KEY_MAX; ++keycode) {
    ioctl(impl->fd, UI_SET_KEYBIT, keycode);
  }

  memset(&setup, 0, sizeof(setup));
  setup.id.bustype = BUS_USB;
  setup.id.vendor = 0x1234;
  setup.id.product = 0x5678;
  snprintf(setup.name, sizeof(setup.name), "axidev-io virtual keyboard");
  ioctl(impl->fd, UI_DEV_SETUP, &setup);
  ioctl(impl->fd, UI_DEV_CREATE);
  axidev_io_sleep_ms(100);

  sender->initialized = true;
  sender->ready = true;
  sender->capabilities.can_inject_keys = true;
  sender->capabilities.can_inject_text = true;
  sender->capabilities.can_simulate_hid = true;
  sender->capabilities.supports_key_repeat = true;
  sender->capabilities.needs_accessibility_perm = false;
  sender->capabilities.needs_input_monitoring_perm = false;
  sender->capabilities.needs_uinput_access = true;
  axidev_io_global->keyboard.backend_type = AXIDEV_IO_BACKEND_LINUX_UINPUT;
  return AXIDEV_IO_RESULT_OK;
}

void axidev_io_keyboard_sender_free(void) {
  axidev_io_keyboard_sender_impl *impl = axidev_io_sender_impl_get();

  if (impl->fd >= 0) {
    ioctl(impl->fd, UI_DEV_DESTROY);
    close(impl->fd);
  }
  memset(impl, 0, sizeof(*impl));
  impl->fd = -1;
  axidev_io_keyboard_reset_public_sender_state();
}

axidev_io_result axidev_io_keyboard_sender_request_permissions(void) {
  return axidev_io_sender_impl_get()->fd >= 0
             ? AXIDEV_IO_RESULT_OK
             : AXIDEV_IO_RESULT_PERMISSION_DENIED;
}

axidev_io_result axidev_io_keyboard_sender_hold_modifier_internal(
    axidev_io_keyboard_modifier_t mods) {
  axidev_io_result result = AXIDEV_IO_RESULT_OK;

  if (axidev_io_keyboard_has_modifier(mods, AXIDEV_IO_MOD_SHIFT)) {
    result = axidev_io_linux_send_raw_key(AXIDEV_IO_KEY_SHIFT_LEFT,
                                          KEY_LEFTSHIFT, true);
    if (result != AXIDEV_IO_RESULT_OK) {
      return result;
    }
  }
  if (axidev_io_keyboard_has_modifier(mods, AXIDEV_IO_MOD_CTRL)) {
    result = axidev_io_linux_send_raw_key(AXIDEV_IO_KEY_CTRL_LEFT, KEY_LEFTCTRL,
                                          true);
    if (result != AXIDEV_IO_RESULT_OK) {
      return result;
    }
  }
  if (axidev_io_keyboard_has_modifier(mods, AXIDEV_IO_MOD_ALT)) {
    result =
        axidev_io_linux_send_raw_key(AXIDEV_IO_KEY_ALT_LEFT, KEY_LEFTALT, true);
    if (result != AXIDEV_IO_RESULT_OK) {
      return result;
    }
  }
  if (axidev_io_keyboard_has_modifier(mods, AXIDEV_IO_MOD_SUPER)) {
    result = axidev_io_linux_send_raw_key(AXIDEV_IO_KEY_SUPER_LEFT,
                                          KEY_LEFTMETA, true);
  }
  return result;
}

axidev_io_result axidev_io_keyboard_sender_release_modifier_internal(
    axidev_io_keyboard_modifier_t mods) {
  axidev_io_result result = AXIDEV_IO_RESULT_OK;

  if (axidev_io_keyboard_has_modifier(mods, AXIDEV_IO_MOD_SHIFT)) {
    result = axidev_io_linux_send_raw_key(AXIDEV_IO_KEY_SHIFT_LEFT,
                                          KEY_LEFTSHIFT, false);
    if (result != AXIDEV_IO_RESULT_OK) {
      return result;
    }
  }
  if (axidev_io_keyboard_has_modifier(mods, AXIDEV_IO_MOD_CTRL)) {
    result = axidev_io_linux_send_raw_key(AXIDEV_IO_KEY_CTRL_LEFT, KEY_LEFTCTRL,
                                          false);
    if (result != AXIDEV_IO_RESULT_OK) {
      return result;
    }
  }
  if (axidev_io_keyboard_has_modifier(mods, AXIDEV_IO_MOD_ALT)) {
    result = axidev_io_linux_send_raw_key(AXIDEV_IO_KEY_ALT_LEFT, KEY_LEFTALT,
                                          false);
    if (result != AXIDEV_IO_RESULT_OK) {
      return result;
    }
  }
  if (axidev_io_keyboard_has_modifier(mods, AXIDEV_IO_MOD_SUPER)) {
    result = axidev_io_linux_send_raw_key(AXIDEV_IO_KEY_SUPER_LEFT,
                                          KEY_LEFTMETA, false);
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
  axidev_io_result result =
      axidev_io_linux_resolve_mapping(key_mod, &keycode, &mods, &resolved_key);
  if (result != AXIDEV_IO_RESULT_OK) {
    return result;
  }
  result = axidev_io_keyboard_sender_hold_modifier_internal(mods);
  if (result != AXIDEV_IO_RESULT_OK) {
    return result;
  }
  return axidev_io_linux_send_raw_key(resolved_key, keycode, true);
}

axidev_io_result axidev_io_keyboard_sender_key_up_internal(
    axidev_io_keyboard_key_with_modifier_t key_mod) {
  int32_t keycode;
  axidev_io_keyboard_modifier_t mods;
  axidev_io_keyboard_key_t resolved_key;
  axidev_io_result result =
      axidev_io_linux_resolve_mapping(key_mod, &keycode, &mods, &resolved_key);
  if (result != AXIDEV_IO_RESULT_OK) {
    return result;
  }
  result = axidev_io_linux_send_raw_key(resolved_key, keycode, false);
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
  axidev_io_result result =
      axidev_io_linux_resolve_mapping(key_mod, &keycode, &mods, &resolved_key);
  if (result != AXIDEV_IO_RESULT_OK) {
    return result;
  }
  result = axidev_io_keyboard_sender_hold_modifier_internal(mods);
  if (result != AXIDEV_IO_RESULT_OK) {
    return result;
  }
  axidev_io_sender_delay();
  result = axidev_io_linux_send_raw_key(resolved_key, keycode, true);
  if (result != AXIDEV_IO_RESULT_OK) {
    axidev_io_keyboard_sender_release_modifier_internal(mods);
    return result;
  }
  axidev_io_sender_delay();
  result = axidev_io_linux_send_raw_key(resolved_key, keycode, false);
  axidev_io_sender_delay();
  axidev_io_keyboard_sender_release_modifier_internal(mods);
  return result;
}

axidev_io_result
axidev_io_keyboard_sender_type_character_internal(uint32_t codepoint) {
  axidev_io_keyboard_key_with_modifier_t key_mod;

  if (axidev_io_keymap_lookup_character(codepoint, &key_mod) !=
      AXIDEV_IO_RESULT_OK) {
    return AXIDEV_IO_RESULT_NOT_SUPPORTED;
  }
  return axidev_io_keyboard_sender_tap_internal(key_mod);
}

void axidev_io_keyboard_sender_flush_internal(void) {
  if (axidev_io_sender_impl_get()->fd >= 0) {
    axidev_io_linux_sync(axidev_io_sender_impl_get()->fd);
  }
}

void axidev_io_keyboard_sender_set_key_delay_internal(uint32_t delay_us) {
  axidev_io_sender_public_context()->key_delay_us = delay_us;
}

#endif

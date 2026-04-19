#if defined(__linux__)

#include "listener_internal.h"

#include <axidev-io/c_api.h>

#include <errno.h>
#include <fcntl.h>
#include <libinput.h>
#include <libudev.h>
#include <poll.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <xkbcommon/xkbcommon-keysyms.h>
#include <xkbcommon/xkbcommon.h>

#include <stb/stb_ds.h>

#include "../common/key_utils_internal.h"
#include "../common/linux_keysym_internal.h"
#include "../common/linux_layout_internal.h"

typedef struct axidev_io_pending_codepoint_entry {
  uint32_t key;
  uint32_t value;
} axidev_io_pending_codepoint_entry;

struct axidev_io_linux_listener_platform {
  struct libinput *libinput;
  struct xkb_context *xkb_context;
  struct xkb_keymap *xkb_keymap;
  struct xkb_state *xkb_state;
  axidev_io_linux_keymap keymap;
  axidev_io_pending_codepoint_entry *pending_codepoints;
};

static void axidev_io_linux_listener_reset_session_state(
    struct axidev_io_linux_listener_platform *platform) {
  if (platform == NULL) {
    return;
  }
  hmfree(platform->pending_codepoints);
}

static int axidev_io_open_restricted(const char *path, int flags,
                                     void *user_data) {
  (void)user_data;
  {
    int fd = open(path, flags);
    return fd < 0 ? -errno : fd;
  }
}

static void axidev_io_close_restricted(int fd, void *user_data) {
  (void)user_data;
  close(fd);
}

static const struct libinput_interface g_libinput_interface = {
    axidev_io_open_restricted, axidev_io_close_restricted};

static uint32_t axidev_io_codepoint_from_key(axidev_io_keyboard_key_t key) {
  if (key >= AXIDEV_IO_KEY_A && key <= AXIDEV_IO_KEY_Z) {
    return (uint32_t)('a' + (key - AXIDEV_IO_KEY_A));
  }
  if (key >= AXIDEV_IO_KEY_NUM0 && key <= AXIDEV_IO_KEY_NUM9) {
    return (uint32_t)('0' + (key - AXIDEV_IO_KEY_NUM0));
  }
  if (key >= AXIDEV_IO_KEY_NUMPAD0 && key <= AXIDEV_IO_KEY_NUMPAD9) {
    return (uint32_t)('0' + (key - AXIDEV_IO_KEY_NUMPAD0));
  }
  return 0;
}

static void axidev_io_listener_invoke_callback(
    axidev_io_keyboard_listener_impl *impl, uint32_t codepoint,
    axidev_io_keyboard_key_with_modifier_t key_mod, bool pressed) {
  axidev_io_keyboard_listener_cb callback;
  void *user_data;

  axidev_io_mutex_lock(&impl->callback_lock);
  callback = impl->callback;
  user_data = impl->user_data;
  axidev_io_mutex_unlock(&impl->callback_lock);

  if (callback != NULL) {
    callback(codepoint, key_mod, pressed, user_data);
  }
}

static axidev_io_keyboard_modifier_t
axidev_io_linux_current_mods(struct xkb_state *state) {
  axidev_io_keyboard_modifier_t mods = AXIDEV_IO_MOD_NONE;

  if (xkb_state_mod_name_is_active(state, XKB_MOD_NAME_SHIFT,
                                   XKB_STATE_MODS_EFFECTIVE)) {
    mods |= AXIDEV_IO_MOD_SHIFT;
  }
  if (xkb_state_mod_name_is_active(state, XKB_MOD_NAME_CTRL,
                                   XKB_STATE_MODS_EFFECTIVE)) {
    mods |= AXIDEV_IO_MOD_CTRL;
  }
  if (xkb_state_mod_name_is_active(state, XKB_MOD_NAME_ALT,
                                   XKB_STATE_MODS_EFFECTIVE)) {
    mods |= AXIDEV_IO_MOD_ALT;
  }
  if (xkb_state_mod_name_is_active(state, XKB_MOD_NAME_LOGO,
                                   XKB_STATE_MODS_EFFECTIVE)) {
    mods |= AXIDEV_IO_MOD_SUPER;
  }
  if (xkb_state_mod_name_is_active(state, "Lock", XKB_STATE_MODS_EFFECTIVE)) {
    mods |= AXIDEV_IO_MOD_CAPSLOCK;
  }

  return mods;
}

static void axidev_io_listener_handle_key_event(
    axidev_io_keyboard_listener_impl *impl,
    struct libinput_event_keyboard *keyboard_event) {
  struct axidev_io_linux_listener_platform *platform = impl->platform;
  uint32_t keycode;
  bool pressed;
  xkb_keycode_t xkb_key;
  axidev_io_keyboard_modifier_t mods;
  xkb_keysym_t keysym;
  uint32_t codepoint = 0;
  axidev_io_keyboard_key_t mapped_key;
  ptrdiff_t pending_index;

  if (platform == NULL || keyboard_event == NULL) {
    return;
  }

  keycode = libinput_event_keyboard_get_key(keyboard_event);
  pressed = libinput_event_keyboard_get_key_state(keyboard_event) ==
            LIBINPUT_KEY_STATE_PRESSED;
  xkb_key = (xkb_keycode_t)(keycode + 8u);
  xkb_state_update_key(platform->xkb_state, xkb_key,
                       pressed ? XKB_KEY_DOWN : XKB_KEY_UP);
  mods = axidev_io_linux_current_mods(platform->xkb_state);
  keysym = xkb_state_key_get_one_sym(platform->xkb_state, xkb_key);

  if (pressed) {
    uint32_t cp = (uint32_t)xkb_keysym_to_utf32(keysym);
    if (cp >= 0x20u && cp != 0x7Fu) {
      hmput(platform->pending_codepoints, keycode, cp);
    } else {
      (void)hmdel(platform->pending_codepoints, keycode);
    }
  } else {
    pending_index = hmgeti(platform->pending_codepoints, keycode);
    if (pending_index >= 0) {
      codepoint = platform->pending_codepoints[pending_index].value;
      (void)hmdel(platform->pending_codepoints, keycode);
    }
  }

  mapped_key = axidev_io_linux_resolve_key_from_evdev_and_mods(
      &platform->keymap, (int)keycode, mods);
  if (mapped_key == AXIDEV_IO_KEY_UNKNOWN) {
    mapped_key = axidev_io_linux_keysym_to_key(keysym);
    if (mapped_key == AXIDEV_IO_KEY_UNKNOWN) {
      char name[64];
      if (xkb_keysym_get_name(keysym, name, sizeof(name)) > 0) {
        mapped_key = axidev_io_string_to_key_internal(name);
      }
    }
  }

  if (mapped_key != AXIDEV_IO_KEY_UNKNOWN &&
      !axidev_io_keyboard_has_modifier(mods, AXIDEV_IO_MOD_SHIFT)) {
    uint32_t derived = axidev_io_codepoint_from_key(mapped_key);
    if (derived != 0) {
      codepoint = derived;
      if (pressed) {
        hmput(platform->pending_codepoints, keycode, derived);
      }
    }
  }

  {
    axidev_io_keyboard_key_with_modifier_t key_mod = {mapped_key, mods};
    axidev_io_listener_invoke_callback(impl, codepoint, key_mod, pressed);
  }
}

static int axidev_io_listener_thread_main(void *user_data) {
  axidev_io_keyboard_listener_impl *impl =
      (axidev_io_keyboard_listener_impl *)user_data;
  struct axidev_io_linux_listener_platform *platform = impl->platform;
  struct udev *udev;
  int fd;
  struct pollfd poll_fd;

  if (platform == NULL) {
    atomic_store(&impl->running, false);
    return 1;
  }

  axidev_io_linux_listener_reset_session_state(platform);

  udev = udev_new();
  if (udev == NULL) {
    atomic_store(&impl->running, false);
    return 1;
  }

  platform->libinput =
      libinput_udev_create_context(&g_libinput_interface, NULL, udev);
  if (platform->libinput == NULL ||
      libinput_udev_assign_seat(platform->libinput, "seat0") < 0) {
    if (platform->libinput != NULL) {
      libinput_unref(platform->libinput);
      platform->libinput = NULL;
    }
    udev_unref(udev);
    atomic_store(&impl->running, false);
    return 1;
  }

  platform->xkb_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
  if (platform->xkb_context == NULL) {
    libinput_unref(platform->libinput);
    platform->libinput = NULL;
    udev_unref(udev);
    atomic_store(&impl->running, false);
    return 1;
  }

  {
    axidev_io_xkb_rule_names_strings names = axidev_io_detect_xkb_rule_names();
    struct xkb_rule_names native_names;
    memset(&native_names, 0, sizeof(native_names));
    native_names.rules = names.rules[0] != '\0' ? names.rules : NULL;
    native_names.model = names.model[0] != '\0' ? names.model : NULL;
    native_names.layout = names.layout[0] != '\0' ? names.layout : NULL;
    native_names.variant = names.variant[0] != '\0' ? names.variant : NULL;
    native_names.options = names.options[0] != '\0' ? names.options : NULL;
    platform->xkb_keymap = xkb_keymap_new_from_names(
        platform->xkb_context, names.has_any ? &native_names : NULL,
        XKB_KEYMAP_COMPILE_NO_FLAGS);
  }
  if (platform->xkb_keymap == NULL) {
    xkb_context_unref(platform->xkb_context);
    platform->xkb_context = NULL;
    libinput_unref(platform->libinput);
    platform->libinput = NULL;
    udev_unref(udev);
    atomic_store(&impl->running, false);
    return 1;
  }

  platform->xkb_state = xkb_state_new(platform->xkb_keymap);
  if (platform->xkb_state == NULL) {
    xkb_keymap_unref(platform->xkb_keymap);
    platform->xkb_keymap = NULL;
    xkb_context_unref(platform->xkb_context);
    platform->xkb_context = NULL;
    libinput_unref(platform->libinput);
    platform->libinput = NULL;
    udev_unref(udev);
    atomic_store(&impl->running, false);
    return 1;
  }

  axidev_io_linux_keymap_init(&platform->keymap, platform->xkb_keymap,
                              platform->xkb_state);
  atomic_store(&impl->ready, true);

  fd = libinput_get_fd(platform->libinput);
  poll_fd.fd = fd;
  poll_fd.events = POLLIN;
  poll_fd.revents = 0;

  while (atomic_load(&impl->running)) {
    int poll_result = poll(&poll_fd, 1, 100);
    if (poll_result > 0 && (poll_fd.revents & POLLIN)) {
      struct libinput_event *event;
      libinput_dispatch(platform->libinput);
      while ((event = libinput_get_event(platform->libinput)) != NULL) {
        if (libinput_event_get_type(event) == LIBINPUT_EVENT_KEYBOARD_KEY) {
          axidev_io_listener_handle_key_event(
              impl, libinput_event_get_keyboard_event(event));
        }
        libinput_event_destroy(event);
      }
    }
    axidev_io_sleep_ms(1);
  }

  axidev_io_linux_keymap_free(&platform->keymap);
  axidev_io_linux_listener_reset_session_state(platform);
  if (platform->xkb_state != NULL) {
    xkb_state_unref(platform->xkb_state);
    platform->xkb_state = NULL;
  }
  if (platform->xkb_keymap != NULL) {
    xkb_keymap_unref(platform->xkb_keymap);
    platform->xkb_keymap = NULL;
  }
  if (platform->xkb_context != NULL) {
    xkb_context_unref(platform->xkb_context);
    platform->xkb_context = NULL;
  }
  if (platform->libinput != NULL) {
    libinput_unref(platform->libinput);
    platform->libinput = NULL;
  }
  udev_unref(udev);
  atomic_store(&impl->ready, false);
  return 0;
}

axidev_io_keyboard_listener_impl *axidev_io_listener_impl_get(void) {
  return (axidev_io_keyboard_listener_impl *)axidev_io_listener_storage_ptr();
}

axidev_io_result axidev_io_keyboard_listener_start_internal(
    axidev_io_keyboard_listener_cb callback, void *user_data) {
  axidev_io_keyboard_listener_impl *impl = axidev_io_listener_impl_get();

  if (callback == NULL) {
    return AXIDEV_IO_RESULT_INVALID_ARGUMENT;
  }

  if (!impl->callback_lock_ready) {
    axidev_io_mutex_init(&impl->callback_lock);
    impl->callback_lock_ready = true;
  }

  if (atomic_load(&impl->running)) {
    return AXIDEV_IO_RESULT_ALREADY_INITIALIZED;
  }

  if (impl->platform == NULL) {
    impl->platform = (struct axidev_io_linux_listener_platform *)calloc(
        1, sizeof(*impl->platform));
    if (impl->platform == NULL) {
      return AXIDEV_IO_RESULT_INTERNAL_ERROR;
    }
  } else {
    axidev_io_linux_listener_reset_session_state(impl->platform);
  }

  axidev_io_mutex_lock(&impl->callback_lock);
  impl->callback = callback;
  impl->user_data = user_data;
  axidev_io_mutex_unlock(&impl->callback_lock);

  atomic_store(&impl->running, true);
  atomic_store(&impl->ready, false);
  if (!axidev_io_thread_create(&impl->worker, axidev_io_listener_thread_main,
                               impl)) {
    atomic_store(&impl->running, false);
    return AXIDEV_IO_RESULT_PLATFORM_ERROR;
  }

  for (int i = 0; i < 40; ++i) {
    if (!atomic_load(&impl->running)) {
      axidev_io_thread_join(&impl->worker);
      return AXIDEV_IO_RESULT_PLATFORM_ERROR;
    }
    if (atomic_load(&impl->ready)) {
      axidev_io_listener_public_context()->initialized = true;
      axidev_io_listener_public_context()->is_listening = true;
      return AXIDEV_IO_RESULT_OK;
    }
    axidev_io_sleep_ms(5);
  }

  if (atomic_load(&impl->ready)) {
    axidev_io_listener_public_context()->initialized = true;
    axidev_io_listener_public_context()->is_listening = true;
    return AXIDEV_IO_RESULT_OK;
  }

  atomic_store(&impl->running, false);
  axidev_io_thread_join(&impl->worker);
  return AXIDEV_IO_RESULT_PLATFORM_ERROR;
}

void axidev_io_keyboard_listener_stop_internal(void) {
  axidev_io_keyboard_listener_impl *impl = axidev_io_listener_impl_get();

  if (!atomic_load(&impl->running)) {
    axidev_io_listener_public_context()->is_listening = false;
    return;
  }

  atomic_store(&impl->running, false);
  axidev_io_thread_join(&impl->worker);
  axidev_io_listener_public_context()->is_listening = false;
  axidev_io_listener_public_context()->initialized = impl->platform != NULL;
}

#endif

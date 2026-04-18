#ifdef _WIN32

#include "listener_internal.h"

#include <Windows.h>

#include <axidev-io/log.h>

#include <stdatomic.h>
#include <stdlib.h>
#include <string.h>

#include <stb/stb_ds.h>

#include "../common/key_utils_internal.h"
#include "../common/windows_keymap_internal.h"

typedef struct axidev_io_vk_codepoint_entry {
  uint32_t key;
  uint32_t value;
} axidev_io_vk_codepoint_entry;

typedef struct axidev_io_vk_time_entry {
  uint32_t key;
  uint64_t value;
} axidev_io_vk_time_entry;

typedef struct axidev_io_vk_signature {
  uint32_t codepoint;
  axidev_io_keyboard_modifier_t mods;
} axidev_io_vk_signature;

typedef struct axidev_io_vk_signature_entry {
  uint32_t key;
  axidev_io_vk_signature value;
} axidev_io_vk_signature_entry;

struct axidev_io_windows_keymap_private {
  axidev_io_windows_keymap keymap;
  axidev_io_vk_codepoint_entry *last_press_cp;
  axidev_io_vk_time_entry *last_release_time;
  axidev_io_vk_signature_entry *last_release_sig;
};

static _Atomic(axidev_io_keyboard_listener_impl *) g_active_listener;

static void axidev_io_windows_listener_reset_session_state(
    struct axidev_io_windows_keymap_private *platform) {
  if (platform == NULL) {
    return;
  }
  hmfree(platform->last_press_cp);
  hmfree(platform->last_release_time);
  hmfree(platform->last_release_sig);
}

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

static axidev_io_keyboard_modifier_t axidev_io_listener_derive_modifiers(void) {
  axidev_io_keyboard_modifier_t mods = AXIDEV_IO_MOD_NONE;

  if (GetKeyState(VK_SHIFT) & 0x8000) {
    mods |= AXIDEV_IO_MOD_SHIFT;
  }
  if (GetKeyState(VK_CONTROL) & 0x8000) {
    mods |= AXIDEV_IO_MOD_CTRL;
  }
  if (GetKeyState(VK_MENU) & 0x8000) {
    mods |= AXIDEV_IO_MOD_ALT;
  }
  if ((GetKeyState(VK_LWIN) & 0x8000) || (GetKeyState(VK_RWIN) & 0x8000)) {
    mods |= AXIDEV_IO_MOD_SUPER;
  }
  if (GetKeyState(VK_CAPITAL) & 0x0001) {
    mods |= AXIDEV_IO_MOD_CAPSLOCK;
  }

  return mods;
}

static void axidev_io_listener_invoke_callback(
    axidev_io_keyboard_listener_impl *impl,
    uint32_t codepoint,
    axidev_io_keyboard_key_with_modifier_t key_mod,
    bool pressed) {
  axidev_io_keyboard_listener_cb callback = NULL;
  void *user_data = NULL;

  axidev_io_mutex_lock(&impl->callback_lock);
  callback = impl->callback;
  user_data = impl->user_data;
  axidev_io_mutex_unlock(&impl->callback_lock);

  if (callback != NULL) {
    callback(codepoint, key_mod, pressed, user_data);
  }
}

static void axidev_io_listener_handle_event(
    axidev_io_keyboard_listener_impl *impl,
    const KBDLLHOOKSTRUCT *kbd,
    bool pressed) {
  struct axidev_io_windows_keymap_private *platform =
      impl->platform;
  WORD vk;
  axidev_io_keyboard_modifier_t mods;
  axidev_io_keyboard_key_t mapped_key;
  BYTE keyboard_state[256];
  wchar_t wbuf[4] = {0};
  uint32_t codepoint = 0;
  int ret;

  if (impl == NULL || kbd == NULL || platform == NULL) {
    return;
  }

  vk = (WORD)kbd->vkCode;
  mods = axidev_io_listener_derive_modifiers();
  mapped_key = axidev_io_windows_resolve_key_from_vk_and_mods(
      &platform->keymap, vk, mods);

  if (!GetKeyboardState(keyboard_state)) {
    axidev_io_keyboard_key_with_modifier_t key_mod = {mapped_key, mods};
    axidev_io_listener_invoke_callback(impl, 0, key_mod, pressed);
    return;
  }

  ret = ToUnicodeEx(vk, kbd->scanCode, keyboard_state, wbuf,
                    (int)(sizeof(wbuf) / sizeof(wbuf[0])), 0,
                    GetKeyboardLayout(0));
  if (ret == 1) {
    codepoint = (uint32_t)wbuf[0];
  } else if (ret >= 2 && wbuf[0] >= 0xD800 && wbuf[0] <= 0xDBFF &&
             wbuf[1] >= 0xDC00 && wbuf[1] <= 0xDFFF) {
    codepoint = 0x10000u +
                ((((uint32_t)wbuf[0] - 0xD800u) << 10) |
                 ((uint32_t)wbuf[1] - 0xDC00u));
  }

  if (mapped_key != AXIDEV_IO_KEY_UNKNOWN &&
      !axidev_io_keyboard_has_modifier(mods, AXIDEV_IO_MOD_SHIFT)) {
    uint32_t derived = axidev_io_codepoint_from_key(mapped_key);
    if (derived != 0) {
      codepoint = derived;
    }
  }

  if (mapped_key == AXIDEV_IO_KEY_ENTER ||
      mapped_key == AXIDEV_IO_KEY_BACKSPACE) {
    codepoint = 0;
  }

  if (pressed) {
    if (codepoint != 0) {
      hmput(platform->last_press_cp, (uint32_t)vk, codepoint);
    } else {
      (void)hmdel(platform->last_press_cp, (uint32_t)vk);
    }
  } else {
    ptrdiff_t press_index;
    press_index = hmgeti(platform->last_press_cp, (uint32_t)vk);
    if (codepoint == 0 && press_index >= 0 &&
        mapped_key != AXIDEV_IO_KEY_ENTER &&
        mapped_key != AXIDEV_IO_KEY_BACKSPACE) {
      codepoint = platform->last_press_cp[press_index].value;
    }

    {
      ptrdiff_t time_index =
          hmgeti(platform->last_release_time, (uint32_t)vk);
      ptrdiff_t sig_index =
          hmgeti(platform->last_release_sig, (uint32_t)vk);
      uint64_t now = axidev_io_monotonic_time_ms();
      if (time_index >= 0 && sig_index >= 0 &&
          (now - platform->last_release_time[time_index].value) < 50u &&
          platform->last_release_sig[sig_index].value.codepoint == codepoint &&
          platform->last_release_sig[sig_index].value.mods == mods) {
        hmput(platform->last_release_time, (uint32_t)vk, now);
        hmput(platform->last_release_sig, (uint32_t)vk,
              ((axidev_io_vk_signature){codepoint, mods}));
        (void)hmdel(platform->last_press_cp, (uint32_t)vk);
        return;
      }
      hmput(platform->last_release_time, (uint32_t)vk, now);
      hmput(platform->last_release_sig, (uint32_t)vk,
            ((axidev_io_vk_signature){codepoint, mods}));
      (void)hmdel(platform->last_press_cp, (uint32_t)vk);
    }
  }

  {
    axidev_io_keyboard_key_with_modifier_t key_mod = {mapped_key, mods};
    axidev_io_listener_invoke_callback(impl, codepoint, key_mod, pressed);
  }
}

static LRESULT CALLBACK axidev_io_low_level_keyboard_proc(int nCode,
                                                          WPARAM wParam,
                                                          LPARAM lParam) {
  axidev_io_keyboard_listener_impl *impl;

  if (nCode < 0) {
    return CallNextHookEx(NULL, nCode, wParam, lParam);
  }

  impl = atomic_load(&g_active_listener);
  if (impl == NULL) {
    return CallNextHookEx(NULL, nCode, wParam, lParam);
  }

  axidev_io_listener_handle_event(
      impl, (const KBDLLHOOKSTRUCT *)lParam,
      wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN);
  return CallNextHookEx(NULL, nCode, wParam, lParam);
}

static int axidev_io_listener_thread_main(void *user_data) {
  axidev_io_keyboard_listener_impl *impl =
      (axidev_io_keyboard_listener_impl *)user_data;
  MSG message;

  impl->thread_id = GetCurrentThreadId();
  atomic_store(&g_active_listener, impl);
  axidev_io_windows_listener_reset_session_state(impl->platform);
  impl->hook = SetWindowsHookEx(WH_KEYBOARD_LL,
                                axidev_io_low_level_keyboard_proc,
                                GetModuleHandle(NULL), 0);
  if (impl->hook == NULL) {
    atomic_store(&g_active_listener, NULL);
    impl->thread_id = 0;
    atomic_store(&impl->running, false);
    atomic_store(&impl->ready, false);
    return 1;
  }

  atomic_store(&impl->ready, true);
  while (GetMessage(&message, NULL, 0, 0) > 0) {
    TranslateMessage(&message);
    DispatchMessage(&message);
  }

  UnhookWindowsHookEx((HHOOK)impl->hook);
  impl->hook = NULL;
  impl->thread_id = 0;
  axidev_io_windows_listener_reset_session_state(impl->platform);
  atomic_store(&impl->ready, false);
  atomic_store(&g_active_listener, NULL);
  return 0;
}

axidev_io_keyboard_listener_impl *axidev_io_listener_impl_get(void) {
  return (axidev_io_keyboard_listener_impl *)axidev_io_listener_storage_ptr();
}

axidev_io_result axidev_io_keyboard_listener_start_internal(
    axidev_io_keyboard_listener_cb callback,
    void *user_data) {
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
    impl->platform = (struct axidev_io_windows_keymap_private *)calloc(
        1, sizeof(*impl->platform));
    if (impl->platform == NULL) {
      return AXIDEV_IO_RESULT_INTERNAL_ERROR;
    }
    axidev_io_windows_keymap_init(&impl->platform->keymap, GetKeyboardLayout(0));
  } else {
    axidev_io_windows_listener_reset_session_state(impl->platform);
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
  if (impl->thread_id != 0) {
    PostThreadMessage(impl->thread_id, WM_QUIT, 0, 0);
  }
  axidev_io_thread_join(&impl->worker);

  axidev_io_listener_public_context()->is_listening = false;
  axidev_io_listener_public_context()->initialized = impl->platform != NULL;
}

#endif

#ifdef _WIN32

#include "windows_keymap_internal.h"

#include <axidev-io/log.h>

#include <string.h>

#include <stb/stb_ds.h>

#include "key_utils_internal.h"

uint32_t axidev_io_encode_vk_mods(WORD vk, axidev_io_keyboard_modifier_t mods) {
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

  return ((uint32_t)vk << 8) | mod_bits;
}

bool axidev_io_is_windows_extended_key(WORD vk) {
  switch (vk) {
  case VK_INSERT:
  case VK_DELETE:
  case VK_HOME:
  case VK_END:
  case VK_PRIOR:
  case VK_NEXT:
  case VK_LEFT:
  case VK_RIGHT:
  case VK_UP:
  case VK_DOWN:
  case VK_SNAPSHOT:
  case VK_DIVIDE:
  case VK_NUMLOCK:
  case VK_RCONTROL:
  case VK_RMENU:
  case VK_LWIN:
  case VK_RWIN:
  case VK_APPS:
    return true;
  default:
    return false;
  }
}

static void axidev_io_windows_set_if_missing(axidev_io_windows_keymap *keymap,
                                             axidev_io_keyboard_key_t key,
                                             WORD vk) {
  if (hmgeti(keymap->key_to_vk, (uint32_t)key) < 0) {
    hmput(keymap->key_to_vk, (uint32_t)key, (int32_t)vk);
  }
  if (hmgeti(keymap->vk_to_key, (int32_t)vk) < 0) {
    hmput(keymap->vk_to_key, (int32_t)vk, key);
  }
}

static void axidev_io_windows_fill_fallback(axidev_io_windows_keymap *keymap) {
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_SPACE, VK_SPACE);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_ENTER, VK_RETURN);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_TAB, VK_TAB);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_BACKSPACE, VK_BACK);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_DELETE, VK_DELETE);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_ESCAPE, VK_ESCAPE);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_LEFT, VK_LEFT);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_RIGHT, VK_RIGHT);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_UP, VK_UP);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_DOWN, VK_DOWN);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_HOME, VK_HOME);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_END, VK_END);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_PAGE_UP, VK_PRIOR);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_PAGE_DOWN, VK_NEXT);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_INSERT, VK_INSERT);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_PRINT_SCREEN,
                                   VK_SNAPSHOT);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_SCROLL_LOCK,
                                   VK_SCROLL);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_PAUSE, VK_PAUSE);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_SHIFT_LEFT, VK_LSHIFT);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_SHIFT_RIGHT,
                                   VK_RSHIFT);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_CTRL_LEFT,
                                   VK_LCONTROL);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_CTRL_RIGHT,
                                   VK_RCONTROL);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_ALT_LEFT, VK_LMENU);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_ALT_RIGHT, VK_RMENU);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_SUPER_LEFT, VK_LWIN);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_SUPER_RIGHT, VK_RWIN);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_CAPS_LOCK, VK_CAPITAL);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_NUM_LOCK, VK_NUMLOCK);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_F1, VK_F1);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_F2, VK_F2);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_F3, VK_F3);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_F4, VK_F4);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_F5, VK_F5);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_F6, VK_F6);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_F7, VK_F7);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_F8, VK_F8);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_F9, VK_F9);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_F10, VK_F10);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_F11, VK_F11);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_F12, VK_F12);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_F13, VK_F13);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_F14, VK_F14);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_F15, VK_F15);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_F16, VK_F16);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_F17, VK_F17);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_F18, VK_F18);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_F19, VK_F19);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_F20, VK_F20);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_NUMPAD0, VK_NUMPAD0);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_NUMPAD1, VK_NUMPAD1);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_NUMPAD2, VK_NUMPAD2);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_NUMPAD3, VK_NUMPAD3);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_NUMPAD4, VK_NUMPAD4);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_NUMPAD5, VK_NUMPAD5);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_NUMPAD6, VK_NUMPAD6);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_NUMPAD7, VK_NUMPAD7);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_NUMPAD8, VK_NUMPAD8);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_NUMPAD9, VK_NUMPAD9);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_NUMPAD_DIVIDE,
                                   VK_DIVIDE);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_NUMPAD_MULTIPLY,
                                   VK_MULTIPLY);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_NUMPAD_MINUS,
                                   VK_SUBTRACT);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_NUMPAD_PLUS, VK_ADD);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_NUMPAD_DECIMAL,
                                   VK_DECIMAL);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_MENU, VK_APPS);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_MUTE, VK_VOLUME_MUTE);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_VOLUME_DOWN,
                                   VK_VOLUME_DOWN);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_VOLUME_UP,
                                   VK_VOLUME_UP);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_MEDIA_PLAY_PAUSE,
                                   VK_MEDIA_PLAY_PAUSE);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_MEDIA_STOP,
                                   VK_MEDIA_STOP);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_MEDIA_NEXT,
                                   VK_MEDIA_NEXT_TRACK);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_MEDIA_PREVIOUS,
                                   VK_MEDIA_PREV_TRACK);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_GRAVE, VK_OEM_3);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_MINUS, VK_OEM_MINUS);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_EQUAL, VK_OEM_PLUS);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_LEFT_BRACKET,
                                   VK_OEM_4);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_RIGHT_BRACKET,
                                   VK_OEM_6);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_BACKSLASH, VK_OEM_5);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_SEMICOLON, VK_OEM_1);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_APOSTROPHE, VK_OEM_7);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_COMMA, VK_OEM_COMMA);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_PERIOD, VK_OEM_PERIOD);
  axidev_io_windows_set_if_missing(keymap, AXIDEV_IO_KEY_SLASH, VK_OEM_2);
}

void axidev_io_windows_keymap_init(axidev_io_windows_keymap *out_keymap,
                                   HKL layout) {
  struct modifier_scan {
    bool shift;
    bool ctrl;
    bool alt;
    axidev_io_keyboard_modifier_t mods;
  };
  const struct modifier_scan scans[] = {
      {false, false, false, AXIDEV_IO_MOD_NONE},
      {true, false, false, AXIDEV_IO_MOD_SHIFT},
      {false, true, false, AXIDEV_IO_MOD_CTRL},
      {false, false, true, AXIDEV_IO_MOD_ALT},
      {false, true, true, AXIDEV_IO_MOD_CTRL | AXIDEV_IO_MOD_ALT},
      {true, true, true,
       AXIDEV_IO_MOD_SHIFT | AXIDEV_IO_MOD_CTRL | AXIDEV_IO_MOD_ALT}};
  wchar_t buffer[4];
  UINT scan_code;

  if (out_keymap == NULL) {
    return;
  }

  memset(out_keymap, 0, sizeof(*out_keymap));
  if (layout == NULL) {
    layout = GetKeyboardLayout(0);
  }

  for (scan_code = 0; scan_code < 128u; ++scan_code) {
    BYTE key_state[256];
    UINT vk;
    int translated;

    vk = MapVirtualKeyEx(scan_code, MAPVK_VSC_TO_VK, layout);
    if (vk == 0) {
      continue;
    }

    memset(key_state, 0, sizeof(key_state));
    translated =
        ToUnicodeEx(vk, scan_code, key_state, buffer,
                    (int)(sizeof(buffer) / sizeof(buffer[0])), 0, layout);
    if (translated > 0) {
      char char_buffer[2];
      char_buffer[0] = '\0';
      if (buffer[0] == L' ') {
        strcpy(char_buffer, " ");
      } else if (buffer[0] < 0x80) {
        char_buffer[0] = (char)buffer[0];
        char_buffer[1] = '\0';
      }
      if (char_buffer[0] != '\0') {
        axidev_io_keyboard_key_t mapped_key =
            axidev_io_string_to_key_internal(char_buffer);
        if (mapped_key != AXIDEV_IO_KEY_UNKNOWN) {
          if (hmgeti(out_keymap->key_to_vk, (uint32_t)mapped_key) < 0) {
            hmput(out_keymap->key_to_vk, (uint32_t)mapped_key, (int32_t)vk);
          }
          if (hmgeti(out_keymap->vk_to_key, (int32_t)vk) < 0) {
            hmput(out_keymap->vk_to_key, (int32_t)vk, mapped_key);
          }
        }
      }
    }

    for (size_t i = 0; i < sizeof(scans) / sizeof(scans[0]); ++i) {
      BYTE mod_state[256] = {0};
      int ret;
      uint32_t codepoint = 0;

      if (scans[i].shift) {
        mod_state[VK_SHIFT] = 0x80;
      }
      if (scans[i].ctrl) {
        mod_state[VK_CONTROL] = 0x80;
      }
      if (scans[i].alt) {
        mod_state[VK_MENU] = 0x80;
      }

      ret = ToUnicodeEx(vk, scan_code, mod_state, buffer,
                        (int)(sizeof(buffer) / sizeof(buffer[0])), 0, layout);
      if (ret <= 0) {
        continue;
      }

      if (ret == 1) {
        codepoint = (uint32_t)buffer[0];
      } else if (ret >= 2 && buffer[0] >= 0xD800 && buffer[0] <= 0xDBFF &&
                 buffer[1] >= 0xDC00 && buffer[1] <= 0xDFFF) {
        codepoint = 0x10000u + ((((uint32_t)buffer[0] - 0xD800u) << 10) |
                                ((uint32_t)buffer[1] - 0xDC00u));
      } else {
        codepoint = (uint32_t)buffer[0];
      }

      if (codepoint != 0) {
        axidev_io_keyboard_key_t mapped_key = AXIDEV_IO_KEY_UNKNOWN;
        if (codepoint < 0x80u) {
          char char_buffer[2];
          char_buffer[0] = (char)codepoint;
          char_buffer[1] = '\0';
          mapped_key = axidev_io_string_to_key_internal(char_buffer);
        }

        if (hmgeti(out_keymap->char_to_keycode, codepoint) < 0) {
          axidev_io_keyboard_mapping_value mapping_value;

          mapping_value.keycode = (int32_t)vk;
          mapping_value.required_mods = scans[i].mods;
          mapping_value.produced_key = mapped_key;
          hmput(out_keymap->char_to_keycode, codepoint, mapping_value);
        }
        if (mapped_key != AXIDEV_IO_KEY_UNKNOWN &&
            hmgeti(out_keymap->vk_and_mods_to_key,
                   axidev_io_encode_vk_mods((WORD)vk, scans[i].mods)) < 0) {
          hmput(out_keymap->vk_and_mods_to_key,
                axidev_io_encode_vk_mods((WORD)vk, scans[i].mods), mapped_key);
        }
      }
    }
  }

  axidev_io_windows_fill_fallback(out_keymap);
}

void axidev_io_windows_keymap_free(axidev_io_windows_keymap *keymap) {
  if (keymap == NULL) {
    return;
  }
  hmfree(keymap->key_to_vk);
  hmfree(keymap->vk_to_key);
  hmfree(keymap->char_to_keycode);
  hmfree(keymap->vk_and_mods_to_key);
  memset(keymap, 0, sizeof(*keymap));
}

axidev_io_keyboard_key_t axidev_io_windows_resolve_key_from_vk_and_mods(
    const axidev_io_windows_keymap *keymap, WORD vk,
    axidev_io_keyboard_modifier_t mods) {
  axidev_io_keymap_uint_to_key_entry *vk_and_mods_to_key;
  axidev_io_keymap_int_to_key_entry *vk_to_key;
  ptrdiff_t index;

  if (keymap == NULL) {
    return AXIDEV_IO_KEY_UNKNOWN;
  }

  vk_and_mods_to_key = keymap->vk_and_mods_to_key;
  index = hmgeti(vk_and_mods_to_key, axidev_io_encode_vk_mods(vk, mods));
  if (index >= 0) {
    return vk_and_mods_to_key[index].value;
  }

  vk_to_key = keymap->vk_to_key;
  index = hmgeti(vk_to_key, (int32_t)vk);
  if (index >= 0) {
    return vk_to_key[index].value;
  }

  return AXIDEV_IO_KEY_UNKNOWN;
}

#endif

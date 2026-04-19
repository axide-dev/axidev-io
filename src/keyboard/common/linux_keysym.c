#if defined(__linux__)

#include "linux_keysym_internal.h"

#include <axidev-io/c_api.h>

#include <string.h>

#include <stb/stb_ds.h>

#include "key_utils_internal.h"

uint32_t axidev_io_encode_evdev_mods(int evdev_code,
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

  return ((uint32_t)evdev_code << 8) | mod_bits;
}

axidev_io_keyboard_key_t axidev_io_linux_keysym_to_key(xkb_keysym_t sym) {
  if (sym >= XKB_KEY_a && sym <= XKB_KEY_z) {
    return (axidev_io_keyboard_key_t)(AXIDEV_IO_KEY_A + (sym - XKB_KEY_a));
  }
  if (sym >= XKB_KEY_A && sym <= XKB_KEY_Z) {
    return (axidev_io_keyboard_key_t)(AXIDEV_IO_KEY_A + (sym - XKB_KEY_A));
  }
  if (sym >= XKB_KEY_0 && sym <= XKB_KEY_9) {
    return (axidev_io_keyboard_key_t)(AXIDEV_IO_KEY_NUM0 + (sym - XKB_KEY_0));
  }
  if (sym >= XKB_KEY_F1 && sym <= XKB_KEY_F20) {
    return (axidev_io_keyboard_key_t)(AXIDEV_IO_KEY_F1 + (sym - XKB_KEY_F1));
  }

  switch (sym) {
  case XKB_KEY_Return:
    return AXIDEV_IO_KEY_ENTER;
  case XKB_KEY_BackSpace:
    return AXIDEV_IO_KEY_BACKSPACE;
  case XKB_KEY_space:
    return AXIDEV_IO_KEY_SPACE;
  case XKB_KEY_Tab:
    return AXIDEV_IO_KEY_TAB;
  case XKB_KEY_Escape:
    return AXIDEV_IO_KEY_ESCAPE;
  case XKB_KEY_Left:
    return AXIDEV_IO_KEY_LEFT;
  case XKB_KEY_Right:
    return AXIDEV_IO_KEY_RIGHT;
  case XKB_KEY_Up:
    return AXIDEV_IO_KEY_UP;
  case XKB_KEY_Down:
    return AXIDEV_IO_KEY_DOWN;
  case XKB_KEY_Home:
    return AXIDEV_IO_KEY_HOME;
  case XKB_KEY_End:
    return AXIDEV_IO_KEY_END;
  case XKB_KEY_Page_Up:
    return AXIDEV_IO_KEY_PAGE_UP;
  case XKB_KEY_Page_Down:
    return AXIDEV_IO_KEY_PAGE_DOWN;
  case XKB_KEY_Delete:
    return AXIDEV_IO_KEY_DELETE;
  case XKB_KEY_Insert:
    return AXIDEV_IO_KEY_INSERT;
  case XKB_KEY_KP_Divide:
    return AXIDEV_IO_KEY_NUMPAD_DIVIDE;
  case XKB_KEY_KP_Multiply:
    return AXIDEV_IO_KEY_NUMPAD_MULTIPLY;
  case XKB_KEY_KP_Subtract:
    return AXIDEV_IO_KEY_NUMPAD_MINUS;
  case XKB_KEY_KP_Add:
    return AXIDEV_IO_KEY_NUMPAD_PLUS;
  case XKB_KEY_KP_Enter:
    return AXIDEV_IO_KEY_NUMPAD_ENTER;
  case XKB_KEY_KP_Decimal:
    return AXIDEV_IO_KEY_NUMPAD_DECIMAL;
  case XKB_KEY_KP_0:
    return AXIDEV_IO_KEY_NUMPAD0;
  case XKB_KEY_KP_1:
    return AXIDEV_IO_KEY_NUMPAD1;
  case XKB_KEY_KP_2:
    return AXIDEV_IO_KEY_NUMPAD2;
  case XKB_KEY_KP_3:
    return AXIDEV_IO_KEY_NUMPAD3;
  case XKB_KEY_KP_4:
    return AXIDEV_IO_KEY_NUMPAD4;
  case XKB_KEY_KP_5:
    return AXIDEV_IO_KEY_NUMPAD5;
  case XKB_KEY_KP_6:
    return AXIDEV_IO_KEY_NUMPAD6;
  case XKB_KEY_KP_7:
    return AXIDEV_IO_KEY_NUMPAD7;
  case XKB_KEY_KP_8:
    return AXIDEV_IO_KEY_NUMPAD8;
  case XKB_KEY_KP_9:
    return AXIDEV_IO_KEY_NUMPAD9;
  case XKB_KEY_Shift_L:
    return AXIDEV_IO_KEY_SHIFT_LEFT;
  case XKB_KEY_Shift_R:
    return AXIDEV_IO_KEY_SHIFT_RIGHT;
  case XKB_KEY_Control_L:
    return AXIDEV_IO_KEY_CTRL_LEFT;
  case XKB_KEY_Control_R:
    return AXIDEV_IO_KEY_CTRL_RIGHT;
  case XKB_KEY_Alt_L:
    return AXIDEV_IO_KEY_ALT_LEFT;
  case XKB_KEY_Alt_R:
    return AXIDEV_IO_KEY_ALT_RIGHT;
  case XKB_KEY_Super_L:
    return AXIDEV_IO_KEY_SUPER_LEFT;
  case XKB_KEY_Super_R:
    return AXIDEV_IO_KEY_SUPER_RIGHT;
  case XKB_KEY_Caps_Lock:
    return AXIDEV_IO_KEY_CAPS_LOCK;
  case XKB_KEY_Num_Lock:
    return AXIDEV_IO_KEY_NUM_LOCK;
  case XKB_KEY_comma:
    return AXIDEV_IO_KEY_COMMA;
  case XKB_KEY_period:
    return AXIDEV_IO_KEY_PERIOD;
  case XKB_KEY_slash:
    return AXIDEV_IO_KEY_SLASH;
  case XKB_KEY_backslash:
    return AXIDEV_IO_KEY_BACKSLASH;
  case XKB_KEY_semicolon:
    return AXIDEV_IO_KEY_SEMICOLON;
  case XKB_KEY_apostrophe:
    return AXIDEV_IO_KEY_APOSTROPHE;
  case XKB_KEY_minus:
    return AXIDEV_IO_KEY_MINUS;
  case XKB_KEY_equal:
    return AXIDEV_IO_KEY_EQUAL;
  case XKB_KEY_grave:
    return AXIDEV_IO_KEY_GRAVE;
  case XKB_KEY_bracketleft:
    return AXIDEV_IO_KEY_LEFT_BRACKET;
  case XKB_KEY_bracketright:
    return AXIDEV_IO_KEY_RIGHT_BRACKET;
  default:
    return AXIDEV_IO_KEY_UNKNOWN;
  }
}

static void axidev_io_linux_set_if_missing(axidev_io_linux_keymap *keymap,
                                           axidev_io_keyboard_key_t key,
                                           int evdev_code) {
  if (hmgeti(keymap->key_to_evdev, (uint32_t)key) < 0) {
    hmput(keymap->key_to_evdev, (uint32_t)key, evdev_code);
  }
  if (hmgeti(keymap->evdev_to_key, evdev_code) < 0) {
    hmput(keymap->evdev_to_key, evdev_code, key);
  }
}

void axidev_io_linux_keymap_fill_fallback(axidev_io_linux_keymap *keymap) {
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_SHIFT_LEFT,
                                 KEY_LEFTSHIFT);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_SHIFT_RIGHT,
                                 KEY_RIGHTSHIFT);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_CTRL_LEFT, KEY_LEFTCTRL);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_CTRL_RIGHT,
                                 KEY_RIGHTCTRL);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_ALT_LEFT, KEY_LEFTALT);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_ALT_RIGHT, KEY_RIGHTALT);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_SUPER_LEFT,
                                 KEY_LEFTMETA);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_SUPER_RIGHT,
                                 KEY_RIGHTMETA);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_CAPS_LOCK, KEY_CAPSLOCK);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_NUM_LOCK, KEY_NUMLOCK);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_SPACE, KEY_SPACE);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_ENTER, KEY_ENTER);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_TAB, KEY_TAB);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_BACKSPACE,
                                 KEY_BACKSPACE);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_DELETE, KEY_DELETE);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_ESCAPE, KEY_ESC);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_LEFT, KEY_LEFT);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_RIGHT, KEY_RIGHT);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_UP, KEY_UP);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_DOWN, KEY_DOWN);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_HOME, KEY_HOME);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_END, KEY_END);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_PAGE_UP, KEY_PAGEUP);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_PAGE_DOWN, KEY_PAGEDOWN);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_INSERT, KEY_INSERT);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_A, KEY_A);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_B, KEY_B);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_C, KEY_C);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_D, KEY_D);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_E, KEY_E);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_F, KEY_F);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_G, KEY_G);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_H, KEY_H);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_I, KEY_I);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_J, KEY_J);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_K, KEY_K);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_L, KEY_L);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_M, KEY_M);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_N, KEY_N);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_O, KEY_O);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_P, KEY_P);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_Q, KEY_Q);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_R, KEY_R);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_S, KEY_S);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_T, KEY_T);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_U, KEY_U);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_V, KEY_V);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_W, KEY_W);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_X, KEY_X);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_Y, KEY_Y);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_Z, KEY_Z);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_NUM0, KEY_0);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_NUM1, KEY_1);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_NUM2, KEY_2);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_NUM3, KEY_3);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_NUM4, KEY_4);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_NUM5, KEY_5);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_NUM6, KEY_6);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_NUM7, KEY_7);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_NUM8, KEY_8);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_NUM9, KEY_9);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_F1, KEY_F1);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_F2, KEY_F2);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_F3, KEY_F3);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_F4, KEY_F4);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_F5, KEY_F5);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_F6, KEY_F6);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_F7, KEY_F7);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_F8, KEY_F8);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_F9, KEY_F9);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_F10, KEY_F10);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_F11, KEY_F11);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_F12, KEY_F12);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_NUMPAD0, KEY_KP0);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_NUMPAD1, KEY_KP1);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_NUMPAD2, KEY_KP2);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_NUMPAD3, KEY_KP3);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_NUMPAD4, KEY_KP4);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_NUMPAD5, KEY_KP5);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_NUMPAD6, KEY_KP6);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_NUMPAD7, KEY_KP7);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_NUMPAD8, KEY_KP8);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_NUMPAD9, KEY_KP9);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_NUMPAD_DIVIDE,
                                 KEY_KPSLASH);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_NUMPAD_MULTIPLY,
                                 KEY_KPASTERISK);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_NUMPAD_MINUS,
                                 KEY_KPMINUS);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_NUMPAD_PLUS, KEY_KPPLUS);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_NUMPAD_ENTER,
                                 KEY_KPENTER);
  axidev_io_linux_set_if_missing(keymap, AXIDEV_IO_KEY_NUMPAD_DECIMAL,
                                 KEY_KPDOT);
}

void axidev_io_linux_keymap_init(axidev_io_linux_keymap *out_keymap,
                                 struct xkb_keymap *keymap,
                                 struct xkb_state *state) {
  xkb_keycode_t min_key;
  xkb_keycode_t max_key;

  if (out_keymap == NULL) {
    return;
  }

  memset(out_keymap, 0, sizeof(*out_keymap));
  if (keymap == NULL || state == NULL) {
    axidev_io_linux_keymap_fill_fallback(out_keymap);
    return;
  }

  min_key = xkb_keymap_min_keycode(keymap);
  max_key = xkb_keymap_max_keycode(keymap);

  {
    xkb_mod_index_t shift_mod =
        xkb_keymap_mod_get_index(keymap, XKB_MOD_NAME_SHIFT);
    xkb_mod_index_t ctrl_mod =
        xkb_keymap_mod_get_index(keymap, XKB_MOD_NAME_CTRL);
    xkb_mod_index_t alt_mod =
        xkb_keymap_mod_get_index(keymap, XKB_MOD_NAME_ALT);
    struct modifier_scan {
      xkb_mod_mask_t mask;
      axidev_io_keyboard_modifier_t mods;
    } scans[6];
    size_t scan_count = 0;
    xkb_keycode_t xkb_key;

    scans[scan_count++] = (struct modifier_scan){0, AXIDEV_IO_MOD_NONE};
    if (shift_mod != XKB_MOD_INVALID) {
      scans[scan_count++] =
          (struct modifier_scan){1u << shift_mod, AXIDEV_IO_MOD_SHIFT};
    }
    if (ctrl_mod != XKB_MOD_INVALID) {
      scans[scan_count++] =
          (struct modifier_scan){1u << ctrl_mod, AXIDEV_IO_MOD_CTRL};
    }
    if (alt_mod != XKB_MOD_INVALID) {
      scans[scan_count++] =
          (struct modifier_scan){1u << alt_mod, AXIDEV_IO_MOD_ALT};
    }
    if (ctrl_mod != XKB_MOD_INVALID && alt_mod != XKB_MOD_INVALID) {
      scans[scan_count++] =
          (struct modifier_scan){(1u << ctrl_mod) | (1u << alt_mod),
                                 AXIDEV_IO_MOD_CTRL | AXIDEV_IO_MOD_ALT};
    }
    if (shift_mod != XKB_MOD_INVALID && ctrl_mod != XKB_MOD_INVALID &&
        alt_mod != XKB_MOD_INVALID) {
      scans[scan_count++] = (struct modifier_scan){
          (1u << shift_mod) | (1u << ctrl_mod) | (1u << alt_mod),
          AXIDEV_IO_MOD_SHIFT | AXIDEV_IO_MOD_CTRL | AXIDEV_IO_MOD_ALT};
    }

    for (xkb_key = min_key; xkb_key <= max_key; ++xkb_key) {
      int evdev_code = (int)xkb_key - 8;
      xkb_keysym_t keysym;
      axidev_io_keyboard_key_t mapped_key;
      size_t i;

      if (evdev_code <= 0) {
        continue;
      }

      keysym = xkb_state_key_get_one_sym(state, xkb_key);
      mapped_key = axidev_io_linux_keysym_to_key(keysym);
      if (mapped_key != AXIDEV_IO_KEY_UNKNOWN &&
          hmgeti(out_keymap->key_to_evdev, (uint32_t)mapped_key) < 0) {
        hmput(out_keymap->key_to_evdev, (uint32_t)mapped_key, evdev_code);
      }
      if (mapped_key != AXIDEV_IO_KEY_UNKNOWN &&
          hmgeti(out_keymap->evdev_to_key, evdev_code) < 0) {
        hmput(out_keymap->evdev_to_key, evdev_code, mapped_key);
      }

      for (i = 0; i < scan_count; ++i) {
        uint32_t character;
        axidev_io_keyboard_key_t char_key = AXIDEV_IO_KEY_UNKNOWN;
        axidev_io_keyboard_mapping_value mapping_value;

        xkb_state_update_mask(state, scans[i].mask, 0, 0, 0, 0, 0);
        character = xkb_state_key_get_utf32(state, xkb_key);
        xkb_state_update_mask(state, 0, 0, 0, 0, 0, 0);
        if (character == 0) {
          continue;
        }

        if (character < 0x80u) {
          char buffer[2];
          buffer[0] = (char)character;
          buffer[1] = '\0';
          char_key = axidev_io_string_to_key_internal(buffer);
        }

        mapping_value.keycode = evdev_code;
        mapping_value.required_mods = scans[i].mods;
        mapping_value.produced_key = char_key;

        if (hmgeti(out_keymap->char_to_keycode, character) < 0) {
          hmput(out_keymap->char_to_keycode, character, mapping_value);
        }
        if (char_key != AXIDEV_IO_KEY_UNKNOWN &&
            hmgeti(out_keymap->code_and_mods_to_key,
                   axidev_io_encode_evdev_mods(evdev_code, scans[i].mods)) <
                0) {
          hmput(out_keymap->code_and_mods_to_key,
                axidev_io_encode_evdev_mods(evdev_code, scans[i].mods),
                char_key);
        }
      }
    }
  }

  axidev_io_linux_keymap_fill_fallback(out_keymap);
}

void axidev_io_linux_keymap_free(axidev_io_linux_keymap *keymap) {
  if (keymap == NULL) {
    return;
  }
  hmfree(keymap->key_to_evdev);
  hmfree(keymap->evdev_to_key);
  hmfree(keymap->char_to_keycode);
  hmfree(keymap->code_and_mods_to_key);
  memset(keymap, 0, sizeof(*keymap));
}

axidev_io_keyboard_key_t axidev_io_linux_resolve_key_from_evdev_and_mods(
    const axidev_io_linux_keymap *keymap, int evdev_code,
    axidev_io_keyboard_modifier_t mods) {
  axidev_io_keymap_uint_to_key_entry *code_and_mods_to_key;
  axidev_io_keymap_int_to_key_entry *evdev_to_key;
  ptrdiff_t index;

  if (keymap == NULL) {
    return AXIDEV_IO_KEY_UNKNOWN;
  }

  code_and_mods_to_key = keymap->code_and_mods_to_key;
  index = hmgeti(code_and_mods_to_key,
                 axidev_io_encode_evdev_mods(evdev_code, mods));
  if (index >= 0) {
    return code_and_mods_to_key[index].value;
  }

  evdev_to_key = keymap->evdev_to_key;
  index = hmgeti(evdev_to_key, evdev_code);
  if (index >= 0) {
    return evdev_to_key[index].value;
  }

  return AXIDEV_IO_KEY_UNKNOWN;
}

#endif

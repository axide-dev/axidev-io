/**
 * @file keyboard/common/linux_keysym.cpp
 * @brief Internal helpers for Linux XKB keysym to Key mapping.
 */

#if defined(__linux__)

#include "keyboard/common/linux_keysym.hpp"

#include <axidev-io/log.hpp>
#include <xkbcommon/xkbcommon-keysyms.h>

namespace axidev::io::keyboard::detail {

Key keysymToKey(xkb_keysym_t sym) {
  if (sym >= XKB_KEY_a && sym <= XKB_KEY_z)
    return static_cast<Key>(static_cast<int>(Key::A) + (sym - XKB_KEY_a));
  if (sym >= XKB_KEY_A && sym <= XKB_KEY_Z)
    return static_cast<Key>(static_cast<int>(Key::A) + (sym - XKB_KEY_A));
  if (sym >= XKB_KEY_0 && sym <= XKB_KEY_9)
    return static_cast<Key>(static_cast<int>(Key::Num0) + (sym - XKB_KEY_0));
  if (sym >= XKB_KEY_F1 && sym <= XKB_KEY_F20)
    return static_cast<Key>(static_cast<int>(Key::F1) + (sym - XKB_KEY_F1));

  switch (sym) {
  case XKB_KEY_Return:
    return Key::Enter;
  case XKB_KEY_BackSpace:
    return Key::Backspace;
  case XKB_KEY_space:
    return Key::Space;
  case XKB_KEY_Tab:
    return Key::Tab;
  case XKB_KEY_Escape:
    return Key::Escape;

  case XKB_KEY_Left:
    return Key::Left;
  case XKB_KEY_Right:
    return Key::Right;
  case XKB_KEY_Up:
    return Key::Up;
  case XKB_KEY_Down:
    return Key::Down;
  case XKB_KEY_Home:
    return Key::Home;
  case XKB_KEY_End:
    return Key::End;
  case XKB_KEY_Page_Up:
    return Key::PageUp;
  case XKB_KEY_Page_Down:
    return Key::PageDown;
  case XKB_KEY_Delete:
    return Key::Delete;
  case XKB_KEY_Insert:
    return Key::Insert;

  // Numpad
  case XKB_KEY_KP_Divide:
    return Key::NumpadDivide;
  case XKB_KEY_KP_Multiply:
    return Key::NumpadMultiply;
  case XKB_KEY_KP_Subtract:
    return Key::NumpadMinus;
  case XKB_KEY_KP_Add:
    return Key::NumpadPlus;
  case XKB_KEY_KP_Enter:
    return Key::NumpadEnter;
  case XKB_KEY_KP_Decimal:
    return Key::NumpadDecimal;
  case XKB_KEY_KP_0:
    return Key::Numpad0;
  case XKB_KEY_KP_1:
    return Key::Numpad1;
  case XKB_KEY_KP_2:
    return Key::Numpad2;
  case XKB_KEY_KP_3:
    return Key::Numpad3;
  case XKB_KEY_KP_4:
    return Key::Numpad4;
  case XKB_KEY_KP_5:
    return Key::Numpad5;
  case XKB_KEY_KP_6:
    return Key::Numpad6;
  case XKB_KEY_KP_7:
    return Key::Numpad7;
  case XKB_KEY_KP_8:
    return Key::Numpad8;
  case XKB_KEY_KP_9:
    return Key::Numpad9;

  // Modifiers
  case XKB_KEY_Shift_L:
    return Key::ShiftLeft;
  case XKB_KEY_Shift_R:
    return Key::ShiftRight;
  case XKB_KEY_Control_L:
    return Key::CtrlLeft;
  case XKB_KEY_Control_R:
    return Key::CtrlRight;
  case XKB_KEY_Alt_L:
    return Key::AltLeft;
  case XKB_KEY_Alt_R:
    return Key::AltRight;
  case XKB_KEY_Super_L:
    return Key::SuperLeft;
  case XKB_KEY_Super_R:
    return Key::SuperRight;
  case XKB_KEY_Caps_Lock:
    return Key::CapsLock;
  case XKB_KEY_Num_Lock:
    return Key::NumLock;

  // Common punctuation
  case XKB_KEY_comma:
    return Key::Comma;
  case XKB_KEY_period:
    return Key::Period;
  case XKB_KEY_slash:
    return Key::Slash;
  case XKB_KEY_backslash:
    return Key::Backslash;
  case XKB_KEY_semicolon:
    return Key::Semicolon;
  case XKB_KEY_apostrophe:
    return Key::Apostrophe;
  case XKB_KEY_minus:
    return Key::Minus;
  case XKB_KEY_equal:
    return Key::Equal;
  case XKB_KEY_grave:
    return Key::Grave;
  case XKB_KEY_bracketleft:
    return Key::LeftBracket;
  case XKB_KEY_bracketright:
    return Key::RightBracket;

  default:
    return Key::Unknown;
  }
}

void fillLinuxFallbackMappings(LinuxKeyMap &keyMap) {
  auto set = [&keyMap](Key k, int v) {
    if (keyMap.keyToEvdev.find(k) == keyMap.keyToEvdev.end())
      keyMap.keyToEvdev[k] = v;
  };

  // Modifiers
  set(Key::ShiftLeft, KEY_LEFTSHIFT);
  set(Key::ShiftRight, KEY_RIGHTSHIFT);
  set(Key::CtrlLeft, KEY_LEFTCTRL);
  set(Key::CtrlRight, KEY_RIGHTCTRL);
  set(Key::AltLeft, KEY_LEFTALT);
  set(Key::AltRight, KEY_RIGHTALT);
  set(Key::SuperLeft, KEY_LEFTMETA);
  set(Key::SuperRight, KEY_RIGHTMETA);
  set(Key::CapsLock, KEY_CAPSLOCK);
  set(Key::NumLock, KEY_NUMLOCK);

  // Common
  set(Key::Space, KEY_SPACE);
  set(Key::Enter, KEY_ENTER);
  set(Key::Tab, KEY_TAB);
  set(Key::Backspace, KEY_BACKSPACE);
  set(Key::Delete, KEY_DELETE);
  set(Key::Escape, KEY_ESC);
  set(Key::Left, KEY_LEFT);
  set(Key::Right, KEY_RIGHT);
  set(Key::Up, KEY_UP);
  set(Key::Down, KEY_DOWN);
  set(Key::Home, KEY_HOME);
  set(Key::End, KEY_END);
  set(Key::PageUp, KEY_PAGEUP);
  set(Key::PageDown, KEY_PAGEDOWN);
  set(Key::Insert, KEY_INSERT);

  // Function keys
  set(Key::F1, KEY_F1);
  set(Key::F2, KEY_F2);
  set(Key::F3, KEY_F3);
  set(Key::F4, KEY_F4);
  set(Key::F5, KEY_F5);
  set(Key::F6, KEY_F6);
  set(Key::F7, KEY_F7);
  set(Key::F8, KEY_F8);
  set(Key::F9, KEY_F9);
  set(Key::F10, KEY_F10);
  set(Key::F11, KEY_F11);
  set(Key::F12, KEY_F12);

  // Numpad
  set(Key::Numpad0, KEY_KP0);
  set(Key::Numpad1, KEY_KP1);
  set(Key::Numpad2, KEY_KP2);
  set(Key::Numpad3, KEY_KP3);
  set(Key::Numpad4, KEY_KP4);
  set(Key::Numpad5, KEY_KP5);
  set(Key::Numpad6, KEY_KP6);
  set(Key::Numpad7, KEY_KP7);
  set(Key::Numpad8, KEY_KP8);
  set(Key::Numpad9, KEY_KP9);
  set(Key::NumpadDivide, KEY_KPSLASH);
  set(Key::NumpadMultiply, KEY_KPASTERISK);
  set(Key::NumpadMinus, KEY_KPMINUS);
  set(Key::NumpadPlus, KEY_KPPLUS);
  set(Key::NumpadEnter, KEY_KPENTER);
  set(Key::NumpadDecimal, KEY_KPDOT);
}

LinuxKeyMap initLinuxKeyMap(struct xkb_keymap *keymap,
                            struct xkb_state *state) {
  LinuxKeyMap out;

  if (!keymap || !state) {
    fillLinuxFallbackMappings(out);
    return out;
  }

  xkb_keycode_t minKey = xkb_keymap_min_keycode(keymap);
  xkb_keycode_t maxKey = xkb_keymap_max_keycode(keymap);

  xkb_mod_index_t shiftMod =
      xkb_keymap_mod_get_index(keymap, XKB_MOD_NAME_SHIFT);
  const bool hasShift = (shiftMod != XKB_MOD_INVALID);

  for (xkb_keycode_t xkbKey = minKey; xkbKey <= maxKey; ++xkbKey) {
    int evdevCode = static_cast<int>(xkbKey) - 8; // XKB offset
    if (evdevCode <= 0)
      continue;

    // Unshifted character
    uint32_t unshifted = xkb_state_key_get_utf32(state, xkbKey);

    // Key enum mapping from keysym (prefer unshifted, try shifted fallback)
    xkb_keysym_t sym = xkb_state_key_get_one_sym(state, xkbKey);
    Key mappedKey = keysymToKey(sym);

    if (mappedKey == Key::Unknown && hasShift) {
      xkb_state_update_mask(state, (1u << shiftMod), 0, 0, 0, 0, 0);
      xkb_keysym_t shiftedSym = xkb_state_key_get_one_sym(state, xkbKey);
      mappedKey = keysymToKey(shiftedSym);
      xkb_state_update_mask(state, 0, 0, 0, 0, 0, 0);
    }

    if (mappedKey != Key::Unknown &&
        out.keyToEvdev.find(mappedKey) == out.keyToEvdev.end()) {
      out.keyToEvdev[mappedKey] = evdevCode;
    }

    if (unshifted != 0 && out.charToKeycode.find(static_cast<char32_t>(
                              unshifted)) == out.charToKeycode.end()) {
      out.charToKeycode[static_cast<char32_t>(unshifted)] = {evdevCode, false};
    }

    if (hasShift) {
      xkb_state_update_mask(state, (1u << shiftMod), 0, 0, 0, 0, 0);
      uint32_t shifted = xkb_state_key_get_utf32(state, xkbKey);
      xkb_state_update_mask(state, 0, 0, 0, 0, 0, 0);

      if (shifted != 0 && shifted != unshifted &&
          out.charToKeycode.find(static_cast<char32_t>(shifted)) ==
              out.charToKeycode.end()) {
        out.charToKeycode[static_cast<char32_t>(shifted)] = {evdevCode, true};
      }
    }
  }

  fillLinuxFallbackMappings(out);

  AXIDEV_IO_LOG_DEBUG("Linux keymap: initialized with %zu keyToEvdev and %zu "
                      "charToKeycode entries",
                      out.keyToEvdev.size(), out.charToKeycode.size());

  return out;
}

} // namespace axidev::io::keyboard::detail

#endif // __linux__

/**
 * @file keyboard/common/windows_keymap.cpp
 * @brief Internal helpers for Windows keyboard layout detection and key
 * mapping.
 *
 * This file implements shared functionality for mapping between Windows VK
 * codes and the logical `axidev::io::keyboard::Key` enum. It is used by both
 * the Sender and Listener implementations to ensure consistent key translation.
 */

#ifdef _WIN32

#include "keyboard/common/windows_keymap.hpp"

#include <axidev-io/log.hpp>

namespace axidev::io::keyboard::detail {

bool isWindowsExtendedKey(WORD vk) {
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

void fillWindowsFallbackMappings(WindowsKeyMap &keyMap) {
  // Helper to set mapping in both directions if not already present
  auto setIfMissing = [&keyMap](Key key, WORD vk) {
    if (keyMap.keyToVk.find(key) == keyMap.keyToVk.end()) {
      keyMap.keyToVk[key] = vk;
    }
    if (keyMap.vkToKey.find(vk) == keyMap.vkToKey.end()) {
      keyMap.vkToKey[vk] = key;
    }
  };

  // Common keys
  setIfMissing(Key::Space, VK_SPACE);
  setIfMissing(Key::Enter, VK_RETURN);
  setIfMissing(Key::Tab, VK_TAB);
  setIfMissing(Key::Backspace, VK_BACK);
  setIfMissing(Key::Delete, VK_DELETE);
  setIfMissing(Key::Escape, VK_ESCAPE);
  setIfMissing(Key::Left, VK_LEFT);
  setIfMissing(Key::Right, VK_RIGHT);
  setIfMissing(Key::Up, VK_UP);
  setIfMissing(Key::Down, VK_DOWN);
  setIfMissing(Key::Home, VK_HOME);
  setIfMissing(Key::End, VK_END);
  setIfMissing(Key::PageUp, VK_PRIOR);
  setIfMissing(Key::PageDown, VK_NEXT);
  setIfMissing(Key::Insert, VK_INSERT);
  setIfMissing(Key::PrintScreen, VK_SNAPSHOT);
  setIfMissing(Key::ScrollLock, VK_SCROLL);
  setIfMissing(Key::Pause, VK_PAUSE);

  // Modifiers
  setIfMissing(Key::ShiftLeft, VK_LSHIFT);
  setIfMissing(Key::ShiftRight, VK_RSHIFT);
  setIfMissing(Key::CtrlLeft, VK_LCONTROL);
  setIfMissing(Key::CtrlRight, VK_RCONTROL);
  setIfMissing(Key::AltLeft, VK_LMENU);
  setIfMissing(Key::AltRight, VK_RMENU);
  setIfMissing(Key::SuperLeft, VK_LWIN);
  setIfMissing(Key::SuperRight, VK_RWIN);
  setIfMissing(Key::CapsLock, VK_CAPITAL);
  setIfMissing(Key::NumLock, VK_NUMLOCK);

  // Function keys
  setIfMissing(Key::F1, VK_F1);
  setIfMissing(Key::F2, VK_F2);
  setIfMissing(Key::F3, VK_F3);
  setIfMissing(Key::F4, VK_F4);
  setIfMissing(Key::F5, VK_F5);
  setIfMissing(Key::F6, VK_F6);
  setIfMissing(Key::F7, VK_F7);
  setIfMissing(Key::F8, VK_F8);
  setIfMissing(Key::F9, VK_F9);
  setIfMissing(Key::F10, VK_F10);
  setIfMissing(Key::F11, VK_F11);
  setIfMissing(Key::F12, VK_F12);
  setIfMissing(Key::F13, VK_F13);
  setIfMissing(Key::F14, VK_F14);
  setIfMissing(Key::F15, VK_F15);
  setIfMissing(Key::F16, VK_F16);
  setIfMissing(Key::F17, VK_F17);
  setIfMissing(Key::F18, VK_F18);
  setIfMissing(Key::F19, VK_F19);
  setIfMissing(Key::F20, VK_F20);

  // Numpad
  setIfMissing(Key::Numpad0, VK_NUMPAD0);
  setIfMissing(Key::Numpad1, VK_NUMPAD1);
  setIfMissing(Key::Numpad2, VK_NUMPAD2);
  setIfMissing(Key::Numpad3, VK_NUMPAD3);
  setIfMissing(Key::Numpad4, VK_NUMPAD4);
  setIfMissing(Key::Numpad5, VK_NUMPAD5);
  setIfMissing(Key::Numpad6, VK_NUMPAD6);
  setIfMissing(Key::Numpad7, VK_NUMPAD7);
  setIfMissing(Key::Numpad8, VK_NUMPAD8);
  setIfMissing(Key::Numpad9, VK_NUMPAD9);
  setIfMissing(Key::NumpadDivide, VK_DIVIDE);
  setIfMissing(Key::NumpadMultiply, VK_MULTIPLY);
  setIfMissing(Key::NumpadMinus, VK_SUBTRACT);
  setIfMissing(Key::NumpadPlus, VK_ADD);
  setIfMissing(Key::NumpadDecimal, VK_DECIMAL);
  // NumpadEnter shares VK_RETURN with main Enter; we rely on extended flag

  // Media / misc keys
  setIfMissing(Key::Menu, VK_APPS);
  setIfMissing(Key::Mute, VK_VOLUME_MUTE);
  setIfMissing(Key::VolumeDown, VK_VOLUME_DOWN);
  setIfMissing(Key::VolumeUp, VK_VOLUME_UP);
  setIfMissing(Key::MediaPlayPause, VK_MEDIA_PLAY_PAUSE);
  setIfMissing(Key::MediaStop, VK_MEDIA_STOP);
  setIfMissing(Key::MediaNext, VK_MEDIA_NEXT_TRACK);
  setIfMissing(Key::MediaPrevious, VK_MEDIA_PREV_TRACK);

  // Letter keys (physical VK codes - layout independent)
  // VK_A through VK_Z are 0x41-0x5A (same as ASCII 'A'-'Z')
  setIfMissing(Key::A, 0x41);
  setIfMissing(Key::B, 0x42);
  setIfMissing(Key::C, 0x43);
  setIfMissing(Key::D, 0x44);
  setIfMissing(Key::E, 0x45);
  setIfMissing(Key::F, 0x46);
  setIfMissing(Key::G, 0x47);
  setIfMissing(Key::H, 0x48);
  setIfMissing(Key::I, 0x49);
  setIfMissing(Key::J, 0x4A);
  setIfMissing(Key::K, 0x4B);
  setIfMissing(Key::L, 0x4C);
  setIfMissing(Key::M, 0x4D);
  setIfMissing(Key::N, 0x4E);
  setIfMissing(Key::O, 0x4F);
  setIfMissing(Key::P, 0x50);
  setIfMissing(Key::Q, 0x51);
  setIfMissing(Key::R, 0x52);
  setIfMissing(Key::S, 0x53);
  setIfMissing(Key::T, 0x54);
  setIfMissing(Key::U, 0x55);
  setIfMissing(Key::V, 0x56);
  setIfMissing(Key::W, 0x57);
  setIfMissing(Key::X, 0x58);
  setIfMissing(Key::Y, 0x59);
  setIfMissing(Key::Z, 0x5A);

  // Number keys (physical VK codes - layout independent)
  // VK_0 through VK_9 are 0x30-0x39 (same as ASCII '0'-'9')
  setIfMissing(Key::Num0, 0x30);
  setIfMissing(Key::Num1, 0x31);
  setIfMissing(Key::Num2, 0x32);
  setIfMissing(Key::Num3, 0x33);
  setIfMissing(Key::Num4, 0x34);
  setIfMissing(Key::Num5, 0x35);
  setIfMissing(Key::Num6, 0x36);
  setIfMissing(Key::Num7, 0x37);
  setIfMissing(Key::Num8, 0x38);
  setIfMissing(Key::Num9, 0x39);

  // Punctuation (OEM keys - layout dependent but commonly consistent)
  setIfMissing(Key::Grave, VK_OEM_3);
  setIfMissing(Key::Minus, VK_OEM_MINUS);
  setIfMissing(Key::Equal, VK_OEM_PLUS);
  setIfMissing(Key::LeftBracket, VK_OEM_4);
  setIfMissing(Key::RightBracket, VK_OEM_6);
  setIfMissing(Key::Backslash, VK_OEM_5);
  setIfMissing(Key::Semicolon, VK_OEM_1);
  setIfMissing(Key::Apostrophe, VK_OEM_7);
  setIfMissing(Key::Comma, VK_OEM_COMMA);
  setIfMissing(Key::Period, VK_OEM_PERIOD);
  setIfMissing(Key::Slash, VK_OEM_2);
}

WindowsKeyMap initWindowsKeyMap(HKL layout) {
  WindowsKeyMap keyMap;

  // Use current thread's layout if not specified
  if (layout == nullptr) {
    layout = GetKeyboardLayout(0);
  }

  // Define modifier combinations to scan.
  // Windows uses VK_SHIFT (index 0x10), VK_CONTROL (0x11), VK_MENU/ALT (0x12)
  struct ModifierScan {
    bool shift;
    bool ctrl;
    bool alt;
    Modifier axidevMods;
  };

  const ModifierScan modScans[] = {
      {false, false, false, Modifier::None}, // No modifiers
      {true, false, false, Modifier::Shift}, // Shift only
      {false, true, false, Modifier::Ctrl},  // Ctrl only
      {false, false, true, Modifier::Alt},   // Alt only
      {false, true, true,
       Modifier::Ctrl | Modifier::Alt}, // Ctrl+Alt (AltGr on some layouts)
      {true, true, true,
       Modifier::Shift | Modifier::Ctrl | Modifier::Alt}, // Shift+Ctrl+Alt
  };

  wchar_t buf[4];
  static constexpr int kMaxScanCode = 128;

  for (UINT sc = 0; sc < kMaxScanCode; ++sc) {
    UINT vk = MapVirtualKeyEx(sc, MAPVK_VSC_TO_VK, layout);
    if (vk == 0)
      continue;

    // First pass: unmodified for Key enum mapping
    BYTE keyState[256]{};
    int ret =
        ToUnicodeEx(vk, sc, keyState, buf,
                    static_cast<int>(sizeof(buf) / sizeof(buf[0])), 0, layout);
    if (ret > 0) {
      wchar_t first = buf[0];
      std::string mappedKeyString;

      if (first == L' ') {
        mappedKeyString = "space";
      } else if (first == L'\t') {
        mappedKeyString = "tab";
      } else if (first == L'\r' || first == L'\n') {
        mappedKeyString = "enter";
      } else if (first < 0x80) {
        mappedKeyString = std::string(1, static_cast<char>(first));
      }

      if (!mappedKeyString.empty()) {
        Key mapped = stringToKey(mappedKeyString);
        if (mapped != Key::Unknown) {
          WORD vkWord = static_cast<WORD>(vk);
          // Only add if not already present (first mapping wins)
          if (keyMap.keyToVk.find(mapped) == keyMap.keyToVk.end()) {
            keyMap.keyToVk[mapped] = vkWord;
          }
          if (keyMap.vkToKey.find(vkWord) == keyMap.vkToKey.end()) {
            keyMap.vkToKey[vkWord] = mapped;
          }
        }
      }
    }

    // Second pass: scan all modifier combinations for charToKeycode
    for (const auto &scan : modScans) {
      BYTE modKeyState[256]{};
      if (scan.shift) {
        modKeyState[VK_SHIFT] = 0x80;
      }
      if (scan.ctrl) {
        modKeyState[VK_CONTROL] = 0x80;
      }
      if (scan.alt) {
        modKeyState[VK_MENU] = 0x80;
      }

      ret = ToUnicodeEx(vk, sc, modKeyState, buf,
                        static_cast<int>(sizeof(buf) / sizeof(buf[0])), 0,
                        layout);

      if (ret > 0) {
        // Handle both BMP characters and surrogate pairs
        char32_t codepoint = 0;
        if (ret == 1) {
          codepoint = static_cast<char32_t>(buf[0]);
        } else if (ret >= 2) {
          // Check for surrogate pair
          wchar_t high = buf[0];
          wchar_t low = buf[1];
          if (high >= 0xD800 && high <= 0xDBFF && low >= 0xDC00 &&
              low <= 0xDFFF) {
            codepoint =
                0x10000 + ((static_cast<char32_t>(high - 0xD800) << 10) |
                           static_cast<char32_t>(low - 0xDC00));
          } else {
            // Just use the first character if not a valid surrogate pair
            codepoint = static_cast<char32_t>(buf[0]);
          }
        }

        if (codepoint != 0) {
          // Only add if not already present (prefer simpler modifier combos)
          if (keyMap.charToKeycode.find(codepoint) ==
              keyMap.charToKeycode.end()) {
            keyMap.charToKeycode[codepoint] =
                KeyMapping(static_cast<int32_t>(vk), scan.axidevMods);
          }
        }
      }
    }
  }

  // Add fallback mappings for keys not covered by layout scan
  fillWindowsFallbackMappings(keyMap);

  AXIDEV_IO_LOG_DEBUG("Windows keymap: initialized with %zu keyToVk, %zu "
                      "vkToKey, and %zu charToKeycode entries",
                      keyMap.keyToVk.size(), keyMap.vkToKey.size(),
                      keyMap.charToKeycode.size());

  return keyMap;
}

} // namespace axidev::io::keyboard::detail

#endif // _WIN32

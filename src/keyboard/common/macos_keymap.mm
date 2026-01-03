/**
 * @file keyboard/common/macos_keymap.mm
 * @brief Internal helpers for macOS keyboard layout detection and key mapping.
 *
 * This file implements shared functionality for mapping between macOS CGKeyCode
 * values and the logical `axidev::io::keyboard::Key` enum. It is used by both
 * the Sender and Listener implementations to ensure consistent key translation.
 */

#ifdef __APPLE__

#include "keyboard/common/macos_keymap.hpp"

#include <ApplicationServices/ApplicationServices.h>
#include <Carbon/Carbon.h>
#import <Foundation/Foundation.h>
#include <array>
#include <axidev-io/log.hpp>

namespace axidev::io::keyboard::detail {

void fillMacOSFallbackMappings(MacOSKeyMap &keyMap) {
  // Helper to set mapping in both directions if not already present
  auto setIfMissing = [&keyMap](Key key, CGKeyCode code) {
    if (keyMap.keyToCode.find(key) == keyMap.keyToCode.end()) {
      keyMap.keyToCode[key] = code;
    }
    if (keyMap.codeToKey.find(code) == keyMap.codeToKey.end()) {
      keyMap.codeToKey[code] = key;
    }
  };

  // Common keys
  setIfMissing(Key::Space, kVK_Space);
  setIfMissing(Key::Enter, kVK_Return);
  setIfMissing(Key::Tab, kVK_Tab);
  setIfMissing(Key::Backspace, kVK_Delete); // Backspace key on macOS
  setIfMissing(Key::Delete, kVK_ForwardDelete);
  setIfMissing(Key::Escape, kVK_Escape);
  setIfMissing(Key::Left, kVK_LeftArrow);
  setIfMissing(Key::Right, kVK_RightArrow);
  setIfMissing(Key::Up, kVK_UpArrow);
  setIfMissing(Key::Down, kVK_DownArrow);
  setIfMissing(Key::Home, kVK_Home);
  setIfMissing(Key::End, kVK_End);
  setIfMissing(Key::PageUp, kVK_PageUp);
  setIfMissing(Key::PageDown, kVK_PageDown);
  setIfMissing(Key::Insert,
               kVK_Help); // macOS doesn't have Insert, Help is closest

  // Modifiers
  setIfMissing(Key::ShiftLeft, kVK_Shift);
  setIfMissing(Key::ShiftRight, kVK_RightShift);
  setIfMissing(Key::CtrlLeft, kVK_Control);
  setIfMissing(Key::CtrlRight, kVK_RightControl);
  setIfMissing(Key::AltLeft, kVK_Option);
  setIfMissing(Key::AltRight, kVK_RightOption);
  setIfMissing(Key::SuperLeft, kVK_Command);
  setIfMissing(Key::SuperRight, kVK_RightCommand);
  setIfMissing(Key::CapsLock, kVK_CapsLock);

  // Function keys
  setIfMissing(Key::F1, kVK_F1);
  setIfMissing(Key::F2, kVK_F2);
  setIfMissing(Key::F3, kVK_F3);
  setIfMissing(Key::F4, kVK_F4);
  setIfMissing(Key::F5, kVK_F5);
  setIfMissing(Key::F6, kVK_F6);
  setIfMissing(Key::F7, kVK_F7);
  setIfMissing(Key::F8, kVK_F8);
  setIfMissing(Key::F9, kVK_F9);
  setIfMissing(Key::F10, kVK_F10);
  setIfMissing(Key::F11, kVK_F11);
  setIfMissing(Key::F12, kVK_F12);
  setIfMissing(Key::F13, kVK_F13);
  setIfMissing(Key::F14, kVK_F14);
  setIfMissing(Key::F15, kVK_F15);
  setIfMissing(Key::F16, kVK_F16);
  setIfMissing(Key::F17, kVK_F17);
  setIfMissing(Key::F18, kVK_F18);
  setIfMissing(Key::F19, kVK_F19);
  setIfMissing(Key::F20, kVK_F20);

  // Numpad
  setIfMissing(Key::Numpad0, kVK_ANSI_Keypad0);
  setIfMissing(Key::Numpad1, kVK_ANSI_Keypad1);
  setIfMissing(Key::Numpad2, kVK_ANSI_Keypad2);
  setIfMissing(Key::Numpad3, kVK_ANSI_Keypad3);
  setIfMissing(Key::Numpad4, kVK_ANSI_Keypad4);
  setIfMissing(Key::Numpad5, kVK_ANSI_Keypad5);
  setIfMissing(Key::Numpad6, kVK_ANSI_Keypad6);
  setIfMissing(Key::Numpad7, kVK_ANSI_Keypad7);
  setIfMissing(Key::Numpad8, kVK_ANSI_Keypad8);
  setIfMissing(Key::Numpad9, kVK_ANSI_Keypad9);
  setIfMissing(Key::NumpadDivide, kVK_ANSI_KeypadDivide);
  setIfMissing(Key::NumpadMultiply, kVK_ANSI_KeypadMultiply);
  setIfMissing(Key::NumpadMinus, kVK_ANSI_KeypadMinus);
  setIfMissing(Key::NumpadPlus, kVK_ANSI_KeypadPlus);
  setIfMissing(Key::NumpadEnter, kVK_ANSI_KeypadEnter);
  setIfMissing(Key::NumpadDecimal, kVK_ANSI_KeypadDecimal);

  // ANSI letter keys (physical key positions - layout independent)
  setIfMissing(Key::A, kVK_ANSI_A);
  setIfMissing(Key::B, kVK_ANSI_B);
  setIfMissing(Key::C, kVK_ANSI_C);
  setIfMissing(Key::D, kVK_ANSI_D);
  setIfMissing(Key::E, kVK_ANSI_E);
  setIfMissing(Key::F, kVK_ANSI_F);
  setIfMissing(Key::G, kVK_ANSI_G);
  setIfMissing(Key::H, kVK_ANSI_H);
  setIfMissing(Key::I, kVK_ANSI_I);
  setIfMissing(Key::J, kVK_ANSI_J);
  setIfMissing(Key::K, kVK_ANSI_K);
  setIfMissing(Key::L, kVK_ANSI_L);
  setIfMissing(Key::M, kVK_ANSI_M);
  setIfMissing(Key::N, kVK_ANSI_N);
  setIfMissing(Key::O, kVK_ANSI_O);
  setIfMissing(Key::P, kVK_ANSI_P);
  setIfMissing(Key::Q, kVK_ANSI_Q);
  setIfMissing(Key::R, kVK_ANSI_R);
  setIfMissing(Key::S, kVK_ANSI_S);
  setIfMissing(Key::T, kVK_ANSI_T);
  setIfMissing(Key::U, kVK_ANSI_U);
  setIfMissing(Key::V, kVK_ANSI_V);
  setIfMissing(Key::W, kVK_ANSI_W);
  setIfMissing(Key::X, kVK_ANSI_X);
  setIfMissing(Key::Y, kVK_ANSI_Y);
  setIfMissing(Key::Z, kVK_ANSI_Z);

  // ANSI number keys (physical key positions - layout independent)
  setIfMissing(Key::Num0, kVK_ANSI_0);
  setIfMissing(Key::Num1, kVK_ANSI_1);
  setIfMissing(Key::Num2, kVK_ANSI_2);
  setIfMissing(Key::Num3, kVK_ANSI_3);
  setIfMissing(Key::Num4, kVK_ANSI_4);
  setIfMissing(Key::Num5, kVK_ANSI_5);
  setIfMissing(Key::Num6, kVK_ANSI_6);
  setIfMissing(Key::Num7, kVK_ANSI_7);
  setIfMissing(Key::Num8, kVK_ANSI_8);
  setIfMissing(Key::Num9, kVK_ANSI_9);

  // ANSI punctuation (physical key positions)
  setIfMissing(Key::Grave, kVK_ANSI_Grave);
  setIfMissing(Key::Minus, kVK_ANSI_Minus);
  setIfMissing(Key::Equal, kVK_ANSI_Equal);
  setIfMissing(Key::LeftBracket, kVK_ANSI_LeftBracket);
  setIfMissing(Key::RightBracket, kVK_ANSI_RightBracket);
  setIfMissing(Key::Backslash, kVK_ANSI_Backslash);
  setIfMissing(Key::Semicolon, kVK_ANSI_Semicolon);
  setIfMissing(Key::Apostrophe, kVK_ANSI_Quote);
  setIfMissing(Key::Comma, kVK_ANSI_Comma);
  setIfMissing(Key::Period, kVK_ANSI_Period);
  setIfMissing(Key::Slash, kVK_ANSI_Slash);
}

MacOSKeyMap initMacOSKeyMap() {
  MacOSKeyMap keyMap;

  TISInputSourceRef currentKeyboard = TISCopyCurrentKeyboardInputSource();
  if (currentKeyboard == nullptr) {
    AXIDEV_IO_LOG_WARN("macOS keymap: TISCopyCurrentKeyboardInputSource "
                       "failed, using fallbacks only");
    fillMacOSFallbackMappings(keyMap);
    return keyMap;
  }

  const auto *layoutData = static_cast<CFDataRef>(TISGetInputSourceProperty(
      currentKeyboard, kTISPropertyUnicodeKeyLayoutData));
  if (layoutData == nullptr) {
    CFRelease(currentKeyboard);
    AXIDEV_IO_LOG_WARN("macOS keymap: keyboard layout data not available, "
                       "using fallbacks only");
    fillMacOSFallbackMappings(keyMap);
    return keyMap;
  }

  const auto *keyboardLayout = static_cast<const UCKeyboardLayout *>(
      static_cast<const void *>(CFDataGetBytePtr(layoutData)));

  UInt32 keysDown = 0;
  static constexpr int kMaxKeyCode = 128;
  static constexpr size_t kUnicodeStringSize = 4;

  for (int keyCode = 0; keyCode < kMaxKeyCode; keyCode++) {
    std::array<UniChar, kUnicodeStringSize> unicodeString{};
    UniCharCount actualStringLength = 0;

    OSStatus status = UCKeyTranslate(
        keyboardLayout, keyCode, kUCKeyActionDisplay,
        0, // no modifier
        LMGetKbdType(), kUCKeyTranslateNoDeadKeysBit, &keysDown,
        static_cast<UInt32>(unicodeString.size()), &actualStringLength,
        static_cast<UniChar *>(unicodeString.data()));

    if (status == noErr && actualStringLength > 0) {
      UniChar firstUnicodeChar = unicodeString[0];
      std::string mappedKeyString;
      static constexpr UniChar kAsciiSpace = ' ';
      static constexpr UniChar kAsciiTab = '\t';
      static constexpr UniChar kAsciiCR = '\r';
      static constexpr UniChar kAsciiLF = '\n';
      static constexpr UniChar kAsciiMax = 0x80;

      if (firstUnicodeChar == kAsciiSpace) {
        mappedKeyString = "space";
      } else if (firstUnicodeChar == kAsciiTab) {
        mappedKeyString = "tab";
      } else if (firstUnicodeChar == kAsciiCR || firstUnicodeChar == kAsciiLF) {
        mappedKeyString = "enter";
      } else if (firstUnicodeChar < kAsciiMax) {
        mappedKeyString = std::string(1, static_cast<char>(firstUnicodeChar));
      } else {
        // Non-ASCII mapping isn't covered by `Key` enum; skip
        continue;
      }

      Key mappedKeyEnum = stringToKey(mappedKeyString);
      if (mappedKeyEnum != Key::Unknown) {
        CGKeyCode cgCode = static_cast<CGKeyCode>(keyCode);
        // Only add if not already present (first mapping wins)
        if (keyMap.keyToCode.find(mappedKeyEnum) == keyMap.keyToCode.end()) {
          keyMap.keyToCode[mappedKeyEnum] = cgCode;
        }
        if (keyMap.codeToKey.find(cgCode) == keyMap.codeToKey.end()) {
          keyMap.codeToKey[cgCode] = mappedKeyEnum;
        }
      }
    }
  }

  CFRelease(currentKeyboard);

  // Add fallback mappings for keys not covered by layout scan
  fillMacOSFallbackMappings(keyMap);

  AXIDEV_IO_LOG_DEBUG("macOS keymap: initialized with %zu keyToCode and %zu "
                      "codeToKey entries",
                      keyMap.keyToCode.size(), keyMap.codeToKey.size());

  return keyMap;
}

} // namespace axidev::io::keyboard::detail

#endif // __APPLE__

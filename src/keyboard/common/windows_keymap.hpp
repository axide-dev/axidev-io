#pragma once
/**
 * @file keyboard/common/windows_keymap.hpp
 * @brief Internal helpers for Windows keyboard layout detection and key
 * mapping.
 *
 * This header provides shared functionality for mapping between Windows virtual
 * key codes (VK) and the logical `axidev::io::keyboard::Key` enum. It is used
 * by both the Sender (input injection) and Listener (event monitoring)
 * implementations to ensure consistent key translation across the library.
 *
 * This header is intentionally placed under `src/` (not installed) because it
 * is an internal implementation detail shared by Windows backends.
 */

#ifdef _WIN32

#include <Windows.h>
#include <axidev-io/keyboard/common.hpp>
#include <unordered_map>

namespace axidev::io::keyboard::detail {

/**
 * @brief Container for Windows keycode mappings.
 *
 * Provides bidirectional mappings between Windows virtual key codes (VK) and
 * logical Key enum values. The mappings are layout-aware when possible,
 * falling back to standard VK codes for non-printable keys.
 */
struct WindowsKeyMap {
  /// Map from logical Key enum to Windows VK code (for Sender)
  std::unordered_map<Key, WORD> keyToVk;

  /// Map from Windows VK code to logical Key enum (for Listener - base keys)
  std::unordered_map<WORD, Key> vkToKey;

  /// Map for character to VK code + modifier requirements (for text typing)
  /// Uses KeyMapping to track which modifiers are needed to produce each char.
  std::unordered_map<char32_t, KeyMapping> charToKeycode;

  /// Map from (VK code, modifiers) to the Key produced.
  /// This enables the Listener to resolve the correct Key based on what
  /// modifiers were active when the key was pressed.
  /// The key is encoded as: (vk << 8) | modifier_bits
  std::unordered_map<uint32_t, Key> vkAndModsToKey;
};

/**
 * @brief Encode a VK code and modifier combination into a single lookup key.
 * @param vk The Windows virtual key code.
 * @param mods The modifier flags (only Shift, Ctrl, Alt are typically
 * relevant).
 * @return uint32_t Encoded lookup key for vkAndModsToKey map.
 */
inline uint32_t encodeVkMods(WORD vk, Modifier mods) {
  // We care about Shift, Ctrl, and Alt for character production
  uint8_t modBits = 0;
  if (hasModifier(mods, Modifier::Shift))
    modBits |= 0x01;
  if (hasModifier(mods, Modifier::Ctrl))
    modBits |= 0x02;
  if (hasModifier(mods, Modifier::Alt))
    modBits |= 0x04;
  return (static_cast<uint32_t>(vk) << 8) | modBits;
}

/**
 * @brief Initialize Windows key mappings using the specified keyboard layout.
 *
 * This function queries the specified keyboard layout handle and builds
 * bidirectional mappings between Windows VK codes and logical Key enum values.
 * It first attempts layout-aware discovery for printable characters via
 * scan code enumeration, then fills in fallback mappings for non-printable
 * keys (modifiers, function keys, navigation, numpad, etc.).
 *
 * @param layout Keyboard layout handle (HKL) to use. Pass nullptr or
 *               GetKeyboardLayout(0) to use the current thread's layout.
 * @return WindowsKeyMap Populated key mappings for the specified layout.
 */
WindowsKeyMap initWindowsKeyMap(HKL layout = nullptr);

/**
 * @brief Fill fallback mappings for non-printable keys.
 *
 * This function populates the provided key map with conservative, layout-
 * independent mappings for modifiers, function keys, navigation keys, numpad
 * keys, media keys, and common punctuation. These mappings correspond to
 * standard Windows VK codes and are used as fallbacks when layout-based
 * discovery does not produce a mapping.
 *
 * @param keyMap Reference to the key map to populate with fallback entries.
 */
void fillWindowsFallbackMappings(WindowsKeyMap &keyMap);

/**
 * @brief Resolve a Key from a VK code and active modifiers.
 *
 * This function looks up the Key that would be produced by pressing the given
 * VK code with the specified modifiers active. This is useful for the Listener
 * to determine which character/key the user intended based on the modifier
 * state.
 *
 * @param keyMap The initialized key map.
 * @param vk The Windows VK code that was pressed.
 * @param mods The modifiers that were active when the key was pressed.
 * @return Key The logical Key produced, or Key::Unknown if not found.
 */
Key resolveKeyFromVkAndMods(const WindowsKeyMap &keyMap, WORD vk,
                            Modifier mods);

/**
 * @brief Check if a virtual-key code represents an extended key.
 *
 * Certain VK codes require the KEYEVENTF_EXTENDEDKEY flag during input
 * synthesis or special handling during hook processing. This helper
 * centralizes that decision for consistency across the Windows backend.
 *
 * @param vk Virtual-key code to test.
 * @return true if the key is considered an extended key; false otherwise.
 */
bool isWindowsExtendedKey(WORD vk);

} // namespace axidev::io::keyboard::detail

#endif // _WIN32

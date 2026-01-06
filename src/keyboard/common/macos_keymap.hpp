#pragma once
/**
 * @file keyboard/common/macos_keymap.hpp
 * @brief Internal helpers for macOS keyboard layout detection and key mapping.
 *
 * This header provides shared functionality for mapping between macOS CGKeyCode
 * values and the logical `axidev::io::keyboard::Key` enum. It is used by both
 * the Sender (input injection) and Listener (event monitoring) implementations
 * to ensure consistent key translation across the library.
 *
 * This header is intentionally placed under `src/` (not installed) because it
 * is an internal implementation detail shared by macOS backends.
 */

#ifdef __APPLE__

#include <Carbon/Carbon.h>
#include <axidev-io/keyboard/common.hpp>
#include <unordered_map>

namespace axidev::io::keyboard::detail {

/**
 * @brief Container for macOS keycode mappings.
 *
 * Provides bidirectional mappings between CGKeyCode values and logical Key
 * enum values. The mappings are layout-aware when possible, falling back to
 * ANSI-standard physical positions for non-printable keys.
 */
struct MacOSKeyMap {
  /// Map from logical Key enum to macOS CGKeyCode (for Sender)
  std::unordered_map<Key, CGKeyCode> keyToCode;

  /// Map from macOS CGKeyCode to logical Key enum (for Listener - base keys)
  std::unordered_map<CGKeyCode, Key> codeToKey;

  /// Map for character to keycode + modifier requirements (for text typing)
  /// Uses KeyMapping to track which modifiers are needed to produce each char.
  std::unordered_map<char32_t, KeyMapping> charToKeycode;

  /// Map from (keycode, modifiers) to the Key produced.
  /// This enables the Listener to resolve the correct Key based on what
  /// modifiers were active when the key was pressed.
  /// The key is encoded as: (keycode << 8) | modifier_bits
  std::unordered_map<uint32_t, Key> codeAndModsToKey;
};

/**
 * @brief Encode a keycode and modifier combination into a single lookup key.
 * @param keycode The platform-specific keycode.
 * @param mods The modifier flags (only Shift and Alt are typically relevant).
 * @return uint32_t Encoded lookup key for codeAndModsToKey map.
 */
inline uint32_t encodeKeycodeMods(CGKeyCode keycode, Modifier mods) {
  // We only care about Shift and Alt for character production
  uint8_t modBits = 0;
  if (hasModifier(mods, Modifier::Shift))
    modBits |= 0x01;
  if (hasModifier(mods, Modifier::Alt))
    modBits |= 0x02;
  return (static_cast<uint32_t>(keycode) << 8) | modBits;
}

/**
 * @brief Initialize macOS key mappings using the current keyboard layout.
 *
 * This function queries the active keyboard input source via TIS APIs and
 * builds bidirectional mappings between CGKeyCode values and logical Key
 * enum values. It first attempts layout-aware discovery for printable
 * characters, then fills in fallback mappings for non-printable keys
 * (modifiers, function keys, navigation, numpad, etc.).
 *
 * @return MacOSKeyMap Populated key mappings for the current keyboard layout.
 */
MacOSKeyMap initMacOSKeyMap();

/**
 * @brief Fill fallback mappings for non-printable keys.
 *
 * This function populates the provided key map with conservative, layout-
 * independent mappings for modifiers, function keys, navigation keys, numpad
 * keys, and common punctuation. These mappings correspond to ANSI-standard
 * physical key positions and are used as fallbacks when layout-based
 * discovery does not produce a mapping.
 *
 * @param keyMap Reference to the key map to populate with fallback entries.
 */
void fillMacOSFallbackMappings(MacOSKeyMap &keyMap);

/**
 * @brief Resolve a Key from a keycode and active modifiers.
 *
 * This function looks up the Key that would be produced by pressing the given
 * keycode with the specified modifiers active. This is useful for the Listener
 * to determine which character/key the user intended based on the modifier
 * state.
 *
 * @param keyMap The initialized key map.
 * @param keycode The platform keycode that was pressed.
 * @param mods The modifiers that were active when the key was pressed.
 * @return Key The logical Key produced, or Key::Unknown if not found.
 */
Key resolveKeyFromCodeAndMods(const MacOSKeyMap &keyMap, CGKeyCode keycode,
                              Modifier mods);

/**
 * @brief Invalid keycode constant for macOS.
 */
inline constexpr CGKeyCode kMacOSInvalidKeyCode = UINT16_MAX;

} // namespace axidev::io::keyboard::detail

#endif // __APPLE__

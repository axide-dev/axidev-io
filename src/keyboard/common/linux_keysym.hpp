#pragma once
/**
 * @file keyboard/common/linux_keysym.hpp
 * @brief Internal helpers for Linux XKB keysym to Key mapping.
 *
 * This header provides shared functionality for mapping between XKB keysyms
 * and the logical `axidev::io::keyboard::Key` enum. It is used by both the
 * Sender (input injection) and Listener (event monitoring) implementations
 * on Linux to ensure consistent key translation across the library.
 *
 * This header is intentionally placed under `src/` (not installed) because it
 * is an internal implementation detail shared by Linux backends.
 */

#if defined(__linux__)

#include <axidev-io/keyboard/common.hpp>
#include <linux/input-event-codes.h>
#include <unordered_map>
#include <xkbcommon/xkbcommon.h>

namespace axidev::io::keyboard::detail {

/**
 * @brief Container for Linux keycode mappings.
 *
 * Provides mappings between evdev keycodes, XKB keysyms, and logical Key
 * enum values. The mappings are layout-aware when possible, falling back to
 * standard evdev keycodes for non-printable keys.
 */
struct LinuxKeyMap {
  /// Map from logical Key enum to evdev keycode (for Sender)
  std::unordered_map<Key, int> keyToEvdev;

  /// Map from evdev keycode to logical Key enum (for Listener - base keys)
  std::unordered_map<int, Key> evdevToKey;

  /// Map for character to keycode + modifier requirements (for text typing)
  /// Uses KeyMapping to track which modifiers are needed to produce each char.
  std::unordered_map<char32_t, KeyMapping> charToKeycode;

  /// Map from (evdev keycode, modifiers) to the Key produced.
  /// This enables the Listener to resolve the correct Key based on what
  /// modifiers were active when the key was pressed.
  /// The key is encoded as: (keycode << 8) | modifier_bits
  std::unordered_map<uint32_t, Key> codeAndModsToKey;
};

/**
 * @brief Encode an evdev keycode and modifier combination into a single lookup
 * key.
 * @param evdevCode The evdev keycode.
 * @param mods The modifier flags (only Shift, Ctrl, Alt are typically
 * relevant).
 * @return uint32_t Encoded lookup key for codeAndModsToKey map.
 */
inline uint32_t encodeEvdevMods(int evdevCode, Modifier mods) {
  // We care about Shift, Ctrl, and Alt for character production
  uint8_t modBits = 0;
  if (hasModifier(mods, Modifier::Shift))
    modBits |= 0x01;
  if (hasModifier(mods, Modifier::Ctrl))
    modBits |= 0x02;
  if (hasModifier(mods, Modifier::Alt))
    modBits |= 0x04;
  return (static_cast<uint32_t>(evdevCode) << 8) | modBits;
}

/**
 * @brief Map an XKB keysym to a logical Key enum value.
 *
 * This function attempts efficient mappings for common contiguous ranges:
 * - ASCII letters (a-z, A-Z) map to Key::A..Key::Z
 * - Digits (0-9) map to Key::Num0..Key::Num9
 * - Function keys (F1..F20) map to Key::F1..Key::F20
 *
 * When a keysym does not fall into those ranges, the function falls back to
 * an explicit switch that covers common control keys, punctuation and other
 * symbol keysyms. If no mapping exists the function returns Key::Unknown.
 *
 * @param sym XKB keysym to translate.
 * @return Key Mapped logical key, or Key::Unknown if unmapped.
 */
Key keysymToKey(xkb_keysym_t sym);

/**
 * @brief Fill fallback mappings for common keys using evdev keycodes.
 *
 * This function populates the provided key map with conservative, layout-
 * independent mappings for modifiers, function keys, navigation keys, and
 * numpad keys. These mappings correspond to standard evdev keycodes and are
 * used as fallbacks when XKB-based discovery does not produce a mapping.
 *
 * @param keyMap Reference to the key map to populate with fallback entries.
 */
void fillLinuxFallbackMappings(LinuxKeyMap &keyMap);

/**
 * @brief Resolve a Key from an evdev keycode and active modifiers.
 *
 * This function looks up the Key that would be produced by pressing the given
 * evdev keycode with the specified modifiers active. This is useful for the
 * Listener to determine which character/key the user intended based on the
 * modifier state.
 *
 * @param keyMap The initialized key map.
 * @param evdevCode The evdev keycode that was pressed.
 * @param mods The modifiers that were active when the key was pressed.
 * @return Key The logical Key produced, or Key::Unknown if not found.
 */
Key resolveKeyFromEvdevAndMods(const LinuxKeyMap &keyMap, int evdevCode,
                               Modifier mods);

/**
 * @brief Initialize Linux key mappings using XKB keymap and state.
 *
 * This function queries the provided XKB keymap and state to build layout-
 * aware mappings between evdev keycodes and logical Key enum values, as well
 * as character-to-keycode mappings for text injection.
 *
 * @param keymap XKB keymap to query. May be nullptr for fallback-only mode.
 * @param state XKB state to query. May be nullptr for fallback-only mode.
 * @return LinuxKeyMap Populated key mappings for the specified layout.
 */
LinuxKeyMap initLinuxKeyMap(struct xkb_keymap *keymap, struct xkb_state *state);

} // namespace axidev::io::keyboard::detail

#endif // __linux__

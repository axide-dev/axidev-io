#pragma once
/**
 * @file keyboard/common/keymap.hpp
 * @brief Platform-agnostic keymap interface for layout-aware key mappings.
 *
 * This header provides a unified interface for querying keyboard layout
 * mappings across all platforms. The keymap is initialized lazily on first
 * access and provides mappings between:
 * - Characters and their required key + modifier combinations
 * - Keycodes (with modifiers) to logical Key enum values
 * - Logical Key enum values to platform keycodes
 *
 * This enables layout-independent keyboard handling where the caller doesn't
 * need to know the specific keyboard layout in use.
 */

#include <axidev-io/keyboard/common.hpp>
#include <optional>
#include <unordered_map>

namespace axidev::io::keyboard {

/**
 * @class KeyMap
 * @brief Platform-agnostic keymap for layout-aware key mappings.
 *
 * This class provides a unified interface for querying keyboard layout
 * mappings. It wraps the platform-specific keymap implementations and
 * provides convenient accessors for common operations.
 *
 * The keymap is initialized lazily when first accessed via `instance()`.
 */
class KeyMap {
public:
  /**
   * @brief Get the singleton KeyMap instance.
   *
   * The keymap is initialized lazily on first call using the current
   * keyboard layout. Subsequent calls return the same instance.
   *
   * @return const KeyMap& Reference to the singleton instance.
   */
  static const KeyMap &instance();

  /**
   * @brief Reinitialize the keymap with the current keyboard layout.
   *
   * Call this if the keyboard layout has changed and you want to
   * update the mappings.
   */
  static void reinitialize();

  /**
   * @brief Look up the Key and required modifiers to produce a character.
   *
   * Given a Unicode codepoint, returns the Key and Modifier combination
   * needed to produce that character on the current keyboard layout.
   *
   * @param codepoint Unicode codepoint to look up.
   * @return std::optional<KeyWithModifier> The key and modifiers, or nullopt
   *         if the character cannot be produced with this layout.
   */
  [[nodiscard]] std::optional<KeyWithModifier>
  keyForCharacter(char32_t codepoint) const;

  /**
   * @brief Look up the Key produced by a keycode with given modifiers.
   *
   * Given a platform-specific keycode and the active modifiers, returns
   * the logical Key that would be produced.
   *
   * @param keycode Platform-specific keycode.
   * @param mods Active modifiers.
   * @return Key The logical Key, or Key::Unknown if not mapped.
   */
  [[nodiscard]] Key keyFromCode(int32_t keycode, Modifier mods) const;

  /**
   * @brief Look up the base Key for a keycode (without modifier consideration).
   *
   * @param keycode Platform-specific keycode.
   * @return Key The base logical Key, or Key::Unknown if not mapped.
   */
  [[nodiscard]] Key baseKeyFromCode(int32_t keycode) const;

  /**
   * @brief Look up the keycode for a logical Key.
   *
   * @param key Logical key to look up.
   * @return std::optional<int32_t> Platform keycode, or nullopt if not mapped.
   */
  [[nodiscard]] std::optional<int32_t> codeForKey(Key key) const;

  /**
   * @brief Get the full KeyMapping for a character.
   *
   * Returns the complete mapping including keycode, modifiers, and produced
   * key for a given character.
   *
   * @param codepoint Unicode codepoint to look up.
   * @return std::optional<KeyMapping> The full mapping, or nullopt if not
   * found.
   */
  [[nodiscard]] std::optional<KeyMapping>
  mappingForCharacter(char32_t codepoint) const;

  /**
   * @brief Check if a character can be typed with this keyboard layout.
   *
   * @param codepoint Unicode codepoint to check.
   * @return true if the character can be typed, false otherwise.
   */
  [[nodiscard]] bool canTypeCharacter(char32_t codepoint) const;

private:
  KeyMap();

  // Internal storage - platform-specific
  std::unordered_map<char32_t, KeyMapping> charToMapping_;
  std::unordered_map<int32_t, Key> codeToKey_;
  std::unordered_map<uint32_t, Key> codeAndModsToKey_;
  std::unordered_map<Key, int32_t> keyToCode_;
};

} // namespace axidev::io::keyboard

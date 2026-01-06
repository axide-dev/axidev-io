/**
 * @file keyboard/common/keymap.cpp
 * @brief Platform-agnostic keymap implementation.
 *
 * This file implements the KeyMap class which provides a unified interface
 * for querying keyboard layout mappings across all platforms.
 */

#include "keyboard/common/keymap.hpp"
#include <axidev-io/log.hpp>
#include <mutex>

#ifdef __APPLE__
#include "keyboard/common/macos_keymap.hpp"
#elif defined(_WIN32)
#include "keyboard/common/windows_keymap.hpp"
#elif defined(__linux__)
#include "keyboard/common/linux_keysym.hpp"
#endif

namespace axidev::io::keyboard {

namespace {

// Encode keycode + modifiers for lookup (same encoding as platform keymaps)
uint32_t encodeCodeMods(int32_t keycode, Modifier mods) {
  uint8_t modBits = 0;
  if (hasModifier(mods, Modifier::Shift))
    modBits |= 0x01;
  if (hasModifier(mods, Modifier::Ctrl))
    modBits |= 0x02;
  if (hasModifier(mods, Modifier::Alt))
    modBits |= 0x04;
  return (static_cast<uint32_t>(keycode) << 8) | modBits;
}

} // namespace

// Singleton instance and mutex
static std::unique_ptr<KeyMap> g_instance;
static std::once_flag g_initFlag;
static std::mutex g_reinitMutex;

const KeyMap &KeyMap::instance() {
  std::call_once(g_initFlag,
                 []() { g_instance = std::unique_ptr<KeyMap>(new KeyMap()); });
  return *g_instance;
}

void KeyMap::reinitialize() {
  std::lock_guard<std::mutex> lock(g_reinitMutex);
  g_instance = std::unique_ptr<KeyMap>(new KeyMap());
}

KeyMap::KeyMap() {
  AXIDEV_IO_LOG_DEBUG("KeyMap: initializing platform keymap");

#ifdef __APPLE__
  auto km = detail::initMacOSKeyMap();

  // Copy charToKeycode
  for (const auto &[cp, mapping] : km.charToKeycode) {
    m_charToMapping[cp] = mapping;
  }

  // Copy codeToKey
  for (const auto &[code, key] : km.codeToKey) {
    m_codeToKey[static_cast<int32_t>(code)] = key;
  }

  // Copy codeAndModsToKey
  for (const auto &[encoded, key] : km.codeAndModsToKey) {
    m_codeAndModsToKey[encoded] = key;
  }

  // Copy keyToCode
  for (const auto &[key, code] : km.keyToCode) {
    m_keyToCode[key] = static_cast<int32_t>(code);
  }

#elif defined(_WIN32)
  auto km = detail::initWindowsKeyMap();

  // Copy charToKeycode
  for (const auto &[cp, mapping] : km.charToKeycode) {
    m_charToMapping[cp] = mapping;
  }

  // Copy vkToKey
  for (const auto &[vk, key] : km.vkToKey) {
    m_codeToKey[static_cast<int32_t>(vk)] = key;
  }

  // Copy vkAndModsToKey
  for (const auto &[encoded, key] : km.vkAndModsToKey) {
    m_codeAndModsToKey[encoded] = key;
  }

  // Copy keyToVk
  for (const auto &[key, vk] : km.keyToVk) {
    m_keyToCode[key] = static_cast<int32_t>(vk);
  }

#elif defined(__linux__)
  // Linux needs XKB context - for now use fallback-only mode
  auto km = detail::initLinuxKeyMap(nullptr, nullptr);

  // Copy charToKeycode
  for (const auto &[cp, mapping] : km.charToKeycode) {
    m_charToMapping[cp] = mapping;
  }

  // Copy evdevToKey
  for (const auto &[evdev, key] : km.evdevToKey) {
    m_codeToKey[evdev] = key;
  }

  // Copy codeAndModsToKey
  for (const auto &[encoded, key] : km.codeAndModsToKey) {
    m_codeAndModsToKey[encoded] = key;
  }

  // Copy keyToEvdev
  for (const auto &[key, evdev] : km.keyToEvdev) {
    m_keyToCode[key] = evdev;
  }

#else
  AXIDEV_IO_LOG_WARN("KeyMap: no platform keymap available");
#endif

  AXIDEV_IO_LOG_DEBUG(
      "KeyMap: initialized with %zu char mappings, %zu code->key, "
      "%zu code+mods->key, %zu key->code",
      m_charToMapping.size(), m_codeToKey.size(), m_codeAndModsToKey.size(),
      m_keyToCode.size());
}

std::optional<KeyWithModifier>
KeyMap::keyForCharacter(char32_t codepoint) const {
  auto it = m_charToMapping.find(codepoint);
  if (it != m_charToMapping.end()) {
    const KeyMapping &mapping = it->second;
    if (mapping.producedKey != Key::Unknown) {
      return KeyWithModifier(mapping.producedKey, mapping.requiredMods);
    }
    // If we have a keycode but no produced key, try to look up the base key
    if (mapping.keycode >= 0) {
      auto baseIt = m_codeToKey.find(mapping.keycode);
      if (baseIt != m_codeToKey.end()) {
        return KeyWithModifier(baseIt->second, mapping.requiredMods);
      }
    }
  }
  return std::nullopt;
}

Key KeyMap::keyFromCode(int32_t keycode, Modifier mods) const {
  // First try exact match with modifiers
  uint32_t encoded = encodeCodeMods(keycode, mods);
  auto it = m_codeAndModsToKey.find(encoded);
  if (it != m_codeAndModsToKey.end()) {
    return it->second;
  }

  // Fall back to base key
  return baseKeyFromCode(keycode);
}

Key KeyMap::baseKeyFromCode(int32_t keycode) const {
  auto it = m_codeToKey.find(keycode);
  if (it != m_codeToKey.end()) {
    return it->second;
  }
  return Key::Unknown;
}

std::optional<int32_t> KeyMap::codeForKey(Key key) const {
  auto it = m_keyToCode.find(key);
  if (it != m_keyToCode.end()) {
    return it->second;
  }
  return std::nullopt;
}

std::optional<KeyMapping>
KeyMap::mappingForCharacter(char32_t codepoint) const {
  auto it = m_charToMapping.find(codepoint);
  if (it != m_charToMapping.end()) {
    return it->second;
  }
  return std::nullopt;
}

bool KeyMap::canTypeCharacter(char32_t codepoint) const {
  return m_charToMapping.find(codepoint) != m_charToMapping.end();
}

} // namespace axidev::io::keyboard

// test_key_utils.cpp
// Comprehensive unit tests for key/string utilities, modifiers, and
// capabilities.
//
// These tests use Google Test and exercise:
//  - keyToString / stringToKey round-trip behavior and uniqueness of canonical
//  names
//  - alias/synonym lookups (e.g., \"esc\" -> Escape, \"kp1\" -> Numpad1)
//  - edge cases (invalid strings, whitespace handling)
//  - Modifier bit-ops and hasModifier helper
//  - Capabilities default values
//
// To run these tests enable AXIDEV_IO_BUILD_TESTS=ON when configuring the
// project.

#include <gtest/gtest.h>

#include <axidev-io/keyboard/common.hpp>
#include <axidev-io/log.hpp>

#include <algorithm>
#include <cctype>
#include <string>
#include <unordered_map>
#include <unordered_set>

using namespace axidev::io::keyboard;

static std::string toLowerCopy(const std::string &s) {
  std::string out = s;
  std::ranges::transform(out, out.begin(), [](char c) {
    return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  });
  return out;
}

TEST(KeyUtilsTest, RoundtripAndUniqueness) {
  AXIDEV_IO_LOG_INFO("test_key_utils: roundtrip/uniqueness start");
  std::unordered_set<std::string> seen;
  int canonicalCount = 0;

  // Precompute lowercase/uppercase canonical counts to detect case collisions
  // (e.g., X11 / XF86 aliases like "OE" vs "oe") so the test can avoid
  // impossible assertions when two canonical names differ only by case.
  std::unordered_map<std::string, int> lowerCounts;
  std::unordered_map<std::string, int> upperCounts;
  for (unsigned j = 0; j <= 255u; ++j) {
    Key kk = static_cast<Key>(j);
    std::string nm = keyToString(kk);
    if (nm == "Unknown")
      continue;
    ++lowerCounts[toLowerCopy(nm)];
    std::string up = nm;
    std::ranges::transform(up, up.begin(), [](char c) {
      return static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    });
    ++upperCounts[up];
  }

  for (unsigned i = 0; i <= 255u; ++i) {
    Key k = static_cast<Key>(i);
    std::string name = keyToString(k);

    // Canonical round-trip: when the canonical name is "Unknown" the
    // mapping should resolve to Key::Unknown. For all other canonical
    // names, the round-trip should return the same Key value.
    if (name == "Unknown") {
      EXPECT_EQ(stringToKey(name), Key::Unknown);
    } else {
      EXPECT_EQ(stringToKey(name), k);
    }

    // Count & uniqueness of non-Unknown canonical names
    if (name != "Unknown") {
      ++canonicalCount;
      auto [it, inserted] = seen.emplace(name);
      EXPECT_TRUE(inserted)
          << "Canonical name '" << name
          << "' is duplicated"; // canonical names must be unique

      // Lowercased canonical should map back when unambiguous.
      // If multiple canonical names collapse to the same lowercased string
      // (for example, both \"OE\" and \"oe\" present), skip the assertion to
      // avoid impossible-to-satisfy checks (both can't round-trip from the
      // identical lowercased string).
      std::string lower = toLowerCopy(name);
      if (lowerCounts[lower] == 1) {
        EXPECT_EQ(stringToKey(lower), k);
      } else {
        AXIDEV_IO_LOG_DEBUG(
            "Skipping lowercased roundtrip for ambiguous canonical name '%s'",
            name.c_str());
      }

      // Uppercased canonical should also map back when unambiguous.
      std::string upper = name;
      std::ranges::transform(upper, upper.begin(), [](char c) {
        return static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
      });
      if (upperCounts[upper] == 1) {
        EXPECT_EQ(stringToKey(upper), k);
      } else {
        AXIDEV_IO_LOG_DEBUG(
            "Skipping uppercased roundtrip for ambiguous canonical name '%s'",
            name.c_str());
      }
    }
  }

  // Sanity: ensure we found a healthy number of canonical keys
  EXPECT_GT(canonicalCount, 40);
}

TEST(KeyUtilsTest, AliasesSynonyms) {
  AXIDEV_IO_LOG_INFO("test_key_utils: aliases/synonyms start");
  EXPECT_EQ(stringToKey("esc"), Key::Escape);
  EXPECT_EQ(stringToKey("ESC"), Key::Escape);
  EXPECT_EQ(stringToKey("return"), Key::Enter);
  EXPECT_EQ(stringToKey("spacebar"), Key::Space);
  EXPECT_EQ(stringToKey("space"), Key::Space);
  EXPECT_EQ(stringToKey("ctrl"), Key::CtrlLeft);
  EXPECT_EQ(stringToKey("control"), Key::CtrlLeft);
  EXPECT_EQ(stringToKey("shift"), Key::ShiftLeft);
  EXPECT_EQ(stringToKey("alt"), Key::AltLeft);
  EXPECT_EQ(stringToKey("super"), Key::SuperLeft);
  EXPECT_EQ(stringToKey("meta"), Key::SuperLeft);
  EXPECT_EQ(stringToKey("win"), Key::SuperLeft);

  EXPECT_EQ(stringToKey("num0"), Key::Num0);
  EXPECT_EQ(stringToKey("num1"), Key::Num1);
  EXPECT_EQ(stringToKey("num2"), Key::Num2);

  EXPECT_EQ(stringToKey("dash"), Key::Minus);
  EXPECT_EQ(stringToKey("hyphen"), Key::Minus);
  EXPECT_EQ(stringToKey("minus"), Key::Minus);
  EXPECT_EQ(stringToKey("-"), Key::Minus);

  EXPECT_EQ(stringToKey("grave"), Key::Grave);
  EXPECT_EQ(stringToKey("`"), Key::Grave);

  EXPECT_EQ(stringToKey("backslash"), Key::Backslash);
  EXPECT_EQ(stringToKey("\\"), Key::Backslash);

  EXPECT_EQ(stringToKey("bracketleft"), Key::LeftBracket);
  EXPECT_EQ(stringToKey("bracketright"), Key::RightBracket);

  EXPECT_EQ(stringToKey("kp0"), Key::Numpad0);
  EXPECT_EQ(stringToKey("kp1"), Key::Numpad1);
  EXPECT_EQ(stringToKey("kp9"), Key::Numpad9);
  EXPECT_EQ(stringToKey("numpad1"), Key::Numpad1);

  EXPECT_EQ(stringToKey("dot"), Key::Period);
  EXPECT_EQ(stringToKey("period"), Key::Period);

  // Additional symbol aliases commonly encountered on US-style keyboard
  // layouts. The implementation maps these to named symbol keys (e.g.,
  // "@" -> Key::At) rather than their shifted numeric equivalents.
  EXPECT_EQ(stringToKey("@"), Key::At);
  EXPECT_EQ(stringToKey("hash"), Key::Hashtag);
  EXPECT_EQ(stringToKey("hashtag"), Key::Hashtag);
  EXPECT_EQ(stringToKey("pound"), Key::Hashtag);
  EXPECT_EQ(stringToKey("!"), Key::Exclamation);
  EXPECT_EQ(stringToKey("$"), Key::Dollar);
  EXPECT_EQ(stringToKey("percent"), Key::Percent);
  EXPECT_EQ(stringToKey("^"), Key::Caret);
  EXPECT_EQ(stringToKey("&"), Key::Ampersand);
  EXPECT_EQ(stringToKey("*"), Key::Asterisk);
  EXPECT_EQ(stringToKey("("), Key::LeftParen);
  EXPECT_EQ(stringToKey(")"), Key::RightParen);

  EXPECT_EQ(stringToKey("_"), Key::Minus);
  EXPECT_EQ(stringToKey("+"), Key::Equal);
  EXPECT_EQ(stringToKey("|"), Key::Backslash);
  EXPECT_EQ(stringToKey("~"), Key::Grave);
  EXPECT_EQ(stringToKey(":"), Key::Semicolon);
  EXPECT_EQ(stringToKey("\""), Key::Apostrophe);
  EXPECT_EQ(stringToKey("<"), Key::Comma);
  EXPECT_EQ(stringToKey(">"), Key::Period);
  EXPECT_EQ(stringToKey("?"), Key::Slash);

  // Whitespace aliases
  EXPECT_EQ(stringToKey(" "), Key::Space);
  EXPECT_EQ(stringToKey("\t"), Key::Tab);
}
TEST(KeyUtilsTest, X11XF86Aliases) {
  AXIDEV_IO_LOG_INFO("test_key_utils: x11 aliases start");

  // Modifier / control variants
  EXPECT_EQ(stringToKey("Control_L"), Key::CtrlLeft);
  EXPECT_EQ(stringToKey("Control_R"), Key::CtrlRight);
  EXPECT_EQ(stringToKey("Shift_L"), Key::ShiftLeft);
  EXPECT_EQ(stringToKey("Shift_R"), Key::ShiftRight);
  EXPECT_EQ(stringToKey("Alt_L"), Key::AltLeft);
  EXPECT_EQ(stringToKey("Meta_L"), Key::SuperLeft);
  EXPECT_EQ(stringToKey("ISO_Left_Tab"), Key::Tab);
  EXPECT_EQ(stringToKey("ISO_Level3_Shift"), Key::AltRight);

  // X11 punctuation / named symbols
  EXPECT_EQ(stringToKey("quotedbl"), Key::Quote);
  EXPECT_EQ(stringToKey("parenleft"), Key::LeftParen);
  EXPECT_EQ(stringToKey("parenright"), Key::RightParen);
  EXPECT_EQ(stringToKey("equal"), Key::Equal);
  EXPECT_EQ(stringToKey("question"), Key::QuestionMark);
  EXPECT_EQ(stringToKey("exclam"), Key::Exclamation);
  EXPECT_EQ(stringToKey("section"), Key::Section);
  EXPECT_EQ(stringToKey("degree"), Key::Degree);
  EXPECT_EQ(stringToKey("sterling"), Key::Sterling);
  EXPECT_EQ(stringToKey("plusminus"), Key::PlusMinus);

  // Accented/locale keys
  EXPECT_EQ(stringToKey("eacute"), Key::E);
  EXPECT_EQ(stringToKey("egrave"), Key::E);
  EXPECT_EQ(stringToKey("agrave"), Key::A);
  EXPECT_EQ(stringToKey("ugrave"), Key::U);
  EXPECT_EQ(stringToKey("ccedilla"), Key::C);
  EXPECT_EQ(stringToKey("oe"), Key::oe);
  EXPECT_EQ(stringToKey("OE"), Key::OE);
  EXPECT_EQ(stringToKey("mu"), Key::Mu);

  // Linefeed / control synonyms
  EXPECT_EQ(stringToKey("linefeed"), Key::Enter);
  EXPECT_EQ(stringToKey("prior"), Key::PageUp);
  EXPECT_EQ(stringToKey("next"), Key::PageDown);

  // Numeric keypad / KP_* variants (underscore and non-underscore forms)
  EXPECT_EQ(stringToKey("KP_Multiply"), Key::NumpadMultiply);
  EXPECT_EQ(stringToKey("kp_multiply"), Key::NumpadMultiply);
  EXPECT_EQ(stringToKey("KP_Divide"), Key::NumpadDivide);
  EXPECT_EQ(stringToKey("KP_Enter"), Key::NumpadEnter);
  EXPECT_EQ(stringToKey("KP_Equal"), Key::NumpadEqual);
  EXPECT_EQ(stringToKey("KP_7"), Key::Numpad7);
  EXPECT_EQ(stringToKey("KP_Up"), Key::Numpad8);
  EXPECT_EQ(stringToKey("KP_Decimal"), Key::NumpadDecimal);

  // XF86 / multimedia / hardware keys
  EXPECT_EQ(stringToKey("XF86AudioMute"), Key::Mute);
  EXPECT_EQ(stringToKey("XF86AudioLowerVolume"), Key::VolumeDown);
  EXPECT_EQ(stringToKey("XF86AudioRaiseVolume"), Key::VolumeUp);
  EXPECT_EQ(stringToKey("XF86AudioPlay"), Key::MediaPlayPause);
  EXPECT_EQ(stringToKey("XF86AudioNext"), Key::MediaNext);
  EXPECT_EQ(stringToKey("XF86Eject"), Key::Eject);
  EXPECT_EQ(stringToKey("XF86MonBrightnessDown"), Key::BrightnessDown);
  EXPECT_EQ(stringToKey("XF86MonBrightnessUp"), Key::BrightnessUp);
  EXPECT_EQ(stringToKey("XF86Launch1"), Key::Launch1);
  EXPECT_EQ(stringToKey("XF86LaunchA"), Key::LaunchA);
  EXPECT_EQ(stringToKey("XF86KbdBrightnessDown"), Key::KbdBrightnessDown);
  EXPECT_EQ(stringToKey("XF86KbdBrightnessUp"), Key::KbdBrightnessUp);

  // Ensure case-insensitivity / underscore handling is robust
  EXPECT_EQ(stringToKey("kp_multiply"), Key::NumpadMultiply);
  EXPECT_EQ(stringToKey("KP_Multiply"), stringToKey("kp_multiply"));
}
TEST(KeyUtilsTest, ASCIIControls) {
  AXIDEV_IO_LOG_INFO("test_key_utils: ascii controls start");
  // Control characters commonly observed in terminal / listener input.
  EXPECT_EQ(stringToKey("\x08"), Key::Backspace);
  EXPECT_EQ(stringToKey("\x03"), Key::AsciiETX);
  EXPECT_EQ(stringToKey("\x1B"), Key::Escape);
  EXPECT_EQ(stringToKey("\x1D"), Key::AsciiGS);
  EXPECT_EQ(stringToKey("\x1C"), Key::AsciiFS);
  EXPECT_EQ(stringToKey("\x1F"), Key::AsciiUS);
  EXPECT_EQ(stringToKey("\x1E"), Key::AsciiRS);
  EXPECT_EQ(stringToKey("\x10"), Key::AsciiDLE);
  EXPECT_EQ(stringToKey("\x05"), Key::AsciiENQ);
  EXPECT_EQ(stringToKey("\x01"), Key::AsciiSOH);
  EXPECT_EQ(stringToKey("\x0B"), Key::AsciiVT);
  EXPECT_EQ(stringToKey("\x0C"), Key::AsciiFF);
  EXPECT_EQ(stringToKey("\x04"), Key::AsciiEOT);
  EXPECT_EQ(stringToKey("\x7F"), Key::Delete);
  // Common newline/whitespace control mappings
  EXPECT_EQ(stringToKey("\n"), Key::Enter);
  EXPECT_EQ(stringToKey("\r"), Key::Enter);
  EXPECT_EQ(stringToKey("\t"), Key::Tab);
}
TEST(KeyUtilsTest, InvalidEdgeCaseInputs) {
  AXIDEV_IO_LOG_INFO("test_key_utils: edge-case inputs start");
  EXPECT_EQ(stringToKey("NotAKey"), Key::Unknown);
  EXPECT_EQ(stringToKey(""), Key::Unknown);
  // whitespace is NOT trimmed by design
  EXPECT_EQ(stringToKey(" Enter"), Key::Unknown);
  EXPECT_EQ(stringToKey("Enter "), Key::Unknown);
}

TEST(KeyUtilsTest, CanonicalValues) {
  AXIDEV_IO_LOG_INFO("test_key_utils: canonical values start");
  EXPECT_EQ(keyToString(Key::A), "A");
  EXPECT_EQ(keyToString(Key::Num1), "1");
  EXPECT_EQ(keyToString(Key::F5), "F5");
  EXPECT_EQ(keyToString(Key::Tab), "Tab");
  EXPECT_EQ(keyToString(Key::Period), ".");
  EXPECT_EQ(keyToString(Key::Backslash), "\\");
  EXPECT_EQ(keyToString(Key::Minus), "-");
}

TEST(ModifierTest, BitOpsAndHelpers) {
  AXIDEV_IO_LOG_INFO("test_key_utils: modifier bit-ops start");
  Modifier m = Modifier::None;
  EXPECT_FALSE(hasModifier(m, Modifier::Shift));

  m |= Modifier::Shift;
  EXPECT_TRUE(hasModifier(m, Modifier::Shift));

  m |= Modifier::Ctrl;
  EXPECT_TRUE(hasModifier(m, Modifier::Shift));
  EXPECT_TRUE(hasModifier(m, Modifier::Ctrl));
  EXPECT_FALSE(hasModifier(m, Modifier::Alt));

  Modifier n = m & Modifier::Ctrl;
  EXPECT_TRUE(hasModifier(n, Modifier::Ctrl));
  EXPECT_TRUE((!hasModifier(n, Modifier::Shift) || (n == Modifier::Ctrl)));

  m &= Modifier::Shift; // keep only Shift
  EXPECT_TRUE(hasModifier(m, Modifier::Shift));
  EXPECT_FALSE(hasModifier(m, Modifier::Ctrl));
}

TEST(CapabilitiesTest, DefaultsToFalse) {
  AXIDEV_IO_LOG_INFO("test_key_utils: capabilities defaults start");
  Capabilities c;
  EXPECT_FALSE(c.canInjectKeys);
  EXPECT_FALSE(c.canInjectText);
  EXPECT_FALSE(c.canSimulateHID);
  EXPECT_FALSE(c.supportsKeyRepeat);
  EXPECT_FALSE(c.needsAccessibilityPerm);
  EXPECT_FALSE(c.needsInputMonitoringPerm);
  EXPECT_FALSE(c.needsUinputAccess);
}

#include <axidev-io/c_api.h>

#include "test_assert.h"

#include <ctype.h>

static void test_roundtrip_and_uniqueness(void) {
  unsigned int i;
  int canonical_count = 0;

  for (i = 0; i <= 267u; ++i) {
    char *name = axidev_io_keyboard_key_to_string((axidev_io_keyboard_key_t)i);
    TEST_CHECK(name != NULL);
    if (name == NULL) {
      continue;
    }
    if (strcmp(name, "Unknown") == 0) {
      TEST_CHECK_EQ_INT(axidev_io_keyboard_string_to_key(name),
                        AXIDEV_IO_KEY_UNKNOWN);
    } else {
      TEST_CHECK_EQ_INT(axidev_io_keyboard_string_to_key(name), i);
      ++canonical_count;
    }
    axidev_io_free_string(name);
  }

  TEST_CHECK(canonical_count > 40);
}

static void test_aliases_and_synonyms(void) {
  TEST_CHECK_EQ_INT(axidev_io_keyboard_string_to_key("esc"),
                    AXIDEV_IO_KEY_ESCAPE);
  TEST_CHECK_EQ_INT(axidev_io_keyboard_string_to_key("spacebar"),
                    AXIDEV_IO_KEY_SPACE);
  TEST_CHECK_EQ_INT(axidev_io_keyboard_string_to_key("ctrl"),
                    AXIDEV_IO_KEY_CTRL_LEFT);
  TEST_CHECK_EQ_INT(axidev_io_keyboard_string_to_key("num1"),
                    AXIDEV_IO_KEY_NUM1);
  TEST_CHECK_EQ_INT(axidev_io_keyboard_string_to_key("-"), AXIDEV_IO_KEY_MINUS);
  TEST_CHECK_EQ_INT(axidev_io_keyboard_string_to_key("grave"),
                    AXIDEV_IO_KEY_GRAVE);
  TEST_CHECK_EQ_INT(axidev_io_keyboard_string_to_key("@"), AXIDEV_IO_KEY_AT);
  TEST_CHECK_EQ_INT(axidev_io_keyboard_string_to_key("hash"),
                    AXIDEV_IO_KEY_HASHTAG);
  TEST_CHECK_EQ_INT(axidev_io_keyboard_string_to_key("?"), AXIDEV_IO_KEY_SLASH);
  TEST_CHECK_EQ_INT(axidev_io_keyboard_string_to_key("Control_L"),
                    AXIDEV_IO_KEY_CTRL_LEFT);
  TEST_CHECK_EQ_INT(axidev_io_keyboard_string_to_key("KP_Decimal"),
                    AXIDEV_IO_KEY_NUMPAD_DECIMAL);
  TEST_CHECK_EQ_INT(axidev_io_keyboard_string_to_key("XF86AudioMute"),
                    AXIDEV_IO_KEY_MUTE);
  TEST_CHECK_EQ_INT(axidev_io_keyboard_string_to_key("\n"),
                    AXIDEV_IO_KEY_ENTER);
  TEST_CHECK_EQ_INT(axidev_io_keyboard_string_to_key("\x03"),
                    AXIDEV_IO_KEY_ASCII_ETX);
}

static void test_invalid_inputs(void) {
  TEST_CHECK_EQ_INT(axidev_io_keyboard_string_to_key("NotAKey"),
                    AXIDEV_IO_KEY_UNKNOWN);
  TEST_CHECK_EQ_INT(axidev_io_keyboard_string_to_key(""),
                    AXIDEV_IO_KEY_UNKNOWN);
  TEST_CHECK_EQ_INT(axidev_io_keyboard_string_to_key(" Enter"),
                    AXIDEV_IO_KEY_UNKNOWN);
}

static void test_key_with_modifier_helpers(void) {
  axidev_io_keyboard_key_with_modifier_t parsed;
  char *text;

  text = axidev_io_keyboard_key_to_string_with_modifier(
      (axidev_io_keyboard_key_with_modifier_t){AXIDEV_IO_KEY_A,
                                               AXIDEV_IO_MOD_SHIFT});
  TEST_CHECK_STR(text, "Shift+A");
  axidev_io_free_string(text);

  TEST_CHECK(
      axidev_io_keyboard_string_to_key_with_modifier("Ctrl+Shift+C", &parsed));
  TEST_CHECK_EQ_INT(parsed.key, AXIDEV_IO_KEY_C);
  TEST_CHECK((parsed.mods & AXIDEV_IO_MOD_CTRL) != 0);
  TEST_CHECK((parsed.mods & AXIDEV_IO_MOD_SHIFT) != 0);

  TEST_CHECK(
      axidev_io_keyboard_string_to_key_with_modifier("shift+a", &parsed));
  TEST_CHECK_EQ_INT(parsed.key, AXIDEV_IO_KEY_A);
  TEST_CHECK((parsed.mods & AXIDEV_IO_MOD_SHIFT) != 0);

  TEST_CHECK(
      axidev_io_keyboard_string_to_key_with_modifier("Shift+NotAKey", &parsed));
  TEST_CHECK_EQ_INT(parsed.key, AXIDEV_IO_KEY_UNKNOWN);
}

int main(void) {
  TEST_RUN(test_roundtrip_and_uniqueness);
  TEST_RUN(test_aliases_and_synonyms);
  TEST_RUN(test_invalid_inputs);
  TEST_RUN(test_key_with_modifier_helpers);
  return g_axidev_test_failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

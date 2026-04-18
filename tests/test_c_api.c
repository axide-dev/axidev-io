#include <axidev-io/c_api.h>

#include "test_assert.h"

static void noop_listener_cb(uint32_t codepoint,
                             axidev_io_keyboard_key_with_modifier_t key_mod,
                             bool pressed, void *user_data) {
  (void)codepoint;
  (void)key_mod;
  (void)pressed;
  (void)user_data;
}

static void test_conversion_helpers(void) {
  char *text;
  axidev_io_keyboard_key_with_modifier_t parsed;

  TEST_CHECK(axidev_io_library_version() != NULL);
  TEST_CHECK_EQ_INT(axidev_io_keyboard_string_to_key("A"), AXIDEV_IO_KEY_A);

  text = axidev_io_keyboard_key_to_string(AXIDEV_IO_KEY_A);
  TEST_CHECK_STR(text, "A");
  axidev_io_free_string(text);

  TEST_CHECK(
      axidev_io_keyboard_string_to_key_with_modifier("Shift+A", &parsed));
  TEST_CHECK_EQ_INT(parsed.key, AXIDEV_IO_KEY_A);
  TEST_CHECK((parsed.mods & AXIDEV_IO_MOD_SHIFT) != 0);
}

static void test_sender_lifecycle_and_errors(void) {
  char *error_text;
  axidev_io_keyboard_capabilities_t capabilities;
  bool initialized;

  TEST_CHECK(!axidev_io_keyboard_tap((axidev_io_keyboard_key_with_modifier_t){
      AXIDEV_IO_KEY_A, AXIDEV_IO_MOD_NONE}));
  error_text = axidev_io_get_last_error();
  TEST_CHECK(error_text != NULL);
  axidev_io_free_string(error_text);

  initialized = axidev_io_keyboard_initialize();
#if defined(__linux__)
  if (!initialized) {
    error_text = axidev_io_get_last_error();
    TEST_CHECK(error_text != NULL);
    TEST_CHECK(error_text != NULL &&
               strstr(error_text, "permission_denied") != NULL);
    if (error_text != NULL) {
      axidev_io_free_string(error_text);
    }
    TEST_CHECK(!axidev_io_keyboard_is_ready());
    axidev_io_keyboard_get_capabilities(&capabilities);
    TEST_CHECK(!capabilities.can_inject_keys);
    axidev_io_keyboard_free();
    return;
  }
#endif
  TEST_CHECK(initialized);
  TEST_CHECK(axidev_io_keyboard_is_ready());
  axidev_io_keyboard_get_capabilities(&capabilities);
  TEST_CHECK(capabilities.can_inject_keys);

  axidev_io_keyboard_set_key_delay(1000);
  TEST_CHECK(axidev_io_keyboard_release_all_modifiers());
  axidev_io_keyboard_flush();
  axidev_io_keyboard_free();
  TEST_CHECK(!axidev_io_keyboard_is_ready());
}

static void test_listener_lifecycle(void) {
  char *error_text;
  bool started;

  TEST_CHECK(!axidev_io_listener_start(NULL, NULL));
  error_text = axidev_io_get_last_error();
  TEST_CHECK(error_text != NULL);
  axidev_io_free_string(error_text);

  started = axidev_io_listener_start(noop_listener_cb, NULL);
  if (started) {
    TEST_CHECK(axidev_io_listener_is_listening());
    axidev_io_listener_stop();
    TEST_CHECK(!axidev_io_listener_is_listening());
  } else {
    error_text = axidev_io_get_last_error();
    if (error_text != NULL) {
      axidev_io_free_string(error_text);
    }
  }
}

int main(void) {
  TEST_RUN(test_conversion_helpers);
  TEST_RUN(test_sender_lifecycle_and_errors);
  TEST_RUN(test_listener_lifecycle);
  return g_axidev_test_failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

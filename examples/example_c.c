/*
 * examples/example_c.c
 *
 * Minimal C example demonstrating the axidev-io C API (c_api.h).
 *
 * - Demonstrates logging control and message emission
 * - Creates a Sender and prints its capabilities
 * - Attempts to tap a logical key (A) and type a short UTF-8 string
 * - Creates a Listener and prints observed key events for a short period
 *
 * Build: enable examples in CMake: -DAXIDEV_IO_BUILD_EXAMPLES=ON
 * Then build the project and run the produced `axidev-io-example-c` executable.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
static void sleep_ms(unsigned int ms) { Sleep(ms); }
#else
#include <unistd.h>
static void sleep_ms(unsigned int ms) { usleep(ms * 1000); }
#endif

#include <axidev-io/c_api.h>

static void print_last_error_if_any(const char *ctx) {
  char *err = axidev_io_get_last_error();
  if (err) {
    fprintf(stderr, "%s: %s\n", ctx, err);
    axidev_io_free_string(err);
  }
}

static void my_listener_cb(uint32_t codepoint, axidev_io_keyboard_key_t key,
                           axidev_io_keyboard_modifier_t mods, bool pressed,
                           void *user_data) {
  (void)user_data;
  char *kname = axidev_io_keyboard_key_to_string(key);
  if (kname) {
    printf("Listener event: codepoint=%u key=%s mods=0x%02x %s\n",
           (unsigned)codepoint, kname, (unsigned)mods,
           pressed ? "PRESSED" : "RELEASED");
    axidev_io_free_string(kname);
  } else {
    /* Fallback if key->string failed for some reason */
    printf("Listener event: codepoint=%u key=%u mods=0x%02x %s\n",
           (unsigned)codepoint, (unsigned)key, (unsigned)mods,
           pressed ? "PRESSED" : "RELEASED");
    print_last_error_if_any("axidev_io_keyboard_key_to_string");
  }
}

int main(void) {
  const char *ver = axidev_io_library_version();
  printf("axidev-io C API example (library version: %s)\n",
         ver ? ver : "(unknown)");

  /* Logging demonstration */
  printf("\n--- Logging API Demo ---\n");
  printf("Current log level: %u\n", (unsigned)axidev_io_log_get_level());
  printf("Debug enabled: %s\n",
         axidev_io_log_is_enabled(AXIDEV_IO_LOG_LEVEL_DEBUG) ? "yes" : "no");
  printf("Info enabled: %s\n",
         axidev_io_log_is_enabled(AXIDEV_IO_LOG_LEVEL_INFO) ? "yes" : "no");

  printf("Emitting sample log messages:\n");
  axidev_io_log_message(AXIDEV_IO_LOG_LEVEL_DEBUG, __FILE__, __LINE__,
                      "Debug message with value: %d", 42);
  axidev_io_log_message(AXIDEV_IO_LOG_LEVEL_INFO, __FILE__, __LINE__,
                      "Info message: %s", "example");
  axidev_io_log_message(AXIDEV_IO_LOG_LEVEL_WARN, __FILE__, __LINE__,
                      "Warning message");

  /* Change log level to suppress debug/info messages */
  printf("\nSetting log level to WARN...\n");
  axidev_io_log_set_level(AXIDEV_IO_LOG_LEVEL_WARN);
  axidev_io_log_message(AXIDEV_IO_LOG_LEVEL_DEBUG, __FILE__, __LINE__,
                      "This debug message should NOT appear");
  axidev_io_log_message(AXIDEV_IO_LOG_LEVEL_WARN, __FILE__, __LINE__,
                      "This warning SHOULD appear");

  /* Reset to debug for testing */
  axidev_io_log_set_level(AXIDEV_IO_LOG_LEVEL_DEBUG);

  printf("\n--- Keyboard Sender/Listener Demo ---\n");

  axidev_io_keyboard_sender_t sender = axidev_io_keyboard_sender_create();
  if (!sender) {
    fprintf(stderr, "Failed to create Sender\n");
    print_last_error_if_any("axidev_io_keyboard_sender_create");
    return EXIT_FAILURE;
  }

  axidev_io_keyboard_capabilities_t caps;
  axidev_io_keyboard_sender_get_capabilities(sender, &caps);
  printf("Sender capabilities: can_inject_keys=%d can_inject_text=%d "
         "can_simulate_hid=%d\n",
         caps.can_inject_keys ? 1 : 0, caps.can_inject_text ? 1 : 0,
         caps.can_simulate_hid ? 1 : 0);

  if (caps.can_inject_keys) {
    axidev_io_keyboard_key_t keyA = axidev_io_keyboard_string_to_key("A");
    if (keyA != (axidev_io_keyboard_key_t)0) {
      printf("Tapping key 'A'\n");
      if (!axidev_io_keyboard_sender_tap(sender, keyA)) {
        fprintf(stderr, "axidev_io_keyboard_sender_tap failed\n");
        print_last_error_if_any("axidev_io_keyboard_sender_tap");
      }
    } else {
      fprintf(stderr, "Could not resolve key 'A'\n");
      print_last_error_if_any("axidev_io_keyboard_string_to_key");
    }
  } else {
    printf("Key injection not supported by this backend.\n");
  }

  if (caps.can_inject_text) {
    printf("Typing text via sender: \"Hello from axidev-io C API\\n\"\n");
    if (!axidev_io_keyboard_sender_type_text_utf8(sender,
                                                "Hello from axidev-io C API\n")) {
      fprintf(stderr, "axidev_io_keyboard_sender_type_text_utf8 failed\n");
      print_last_error_if_any("axidev_io_keyboard_sender_type_text_utf8");
    }
  } else {
    printf("Text injection not supported by this backend.\n");
  }

  /* Listener demo (may require platform permissions). */
  axidev_io_keyboard_listener_t listener = axidev_io_keyboard_listener_create();
  if (!listener) {
    fprintf(stderr, "Failed to create Listener\n");
    print_last_error_if_any("axidev_io_keyboard_listener_create");
    axidev_io_keyboard_sender_destroy(sender);
    return EXIT_FAILURE;
  }

  printf("Starting listener for 5 seconds. Press some keys to see events.\n");
  if (!axidev_io_keyboard_listener_start(listener, my_listener_cb, NULL)) {
    fprintf(stderr, "axidev_io_keyboard_listener_start failed\n");
    print_last_error_if_any("axidev_io_keyboard_listener_start");
  } else {
    sleep_ms(5000);
    axidev_io_keyboard_listener_stop(listener);
  }

  axidev_io_keyboard_listener_destroy(listener);
  axidev_io_keyboard_sender_destroy(sender);

  printf("Example complete.\n");
  return EXIT_SUCCESS;
}

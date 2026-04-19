#include <axidev-io/c_api.h>

#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <Windows.h>
static void sleep_ms(unsigned int milliseconds) { Sleep(milliseconds); }
#else
#include <unistd.h>
static void sleep_ms(unsigned int milliseconds) {
  usleep(milliseconds * 1000u);
}
#endif

static void print_last_error_if_any(const char *context) {
  char *error_text = axidev_io_get_last_error();
  if (error_text != NULL) {
    fprintf(stderr, "%s: %s\n", context, error_text);
    axidev_io_free_string(error_text);
  }
}

static void example_listener_cb(uint32_t codepoint,
                                axidev_io_keyboard_key_with_modifier_t key_mod,
                                bool pressed, void *user_data) {
  char *label;
  (void)user_data;
  (void)codepoint;

  label = axidev_io_keyboard_key_to_string_with_modifier(key_mod);
  if (label != NULL) {
    printf("[listener] %s %s\n", label, pressed ? "down" : "up");
    axidev_io_free_string(label);
  }
}

int main(void) {
  axidev_io_keyboard_capabilities_t capabilities;

  printf("axidev-io %s\n", axidev_io_library_version());
  if (!axidev_io_keyboard_initialize()) {
    print_last_error_if_any("axidev_io_keyboard_initialize");
    return EXIT_FAILURE;
  }

  axidev_io_keyboard_get_capabilities(&capabilities);
  printf("backend=%u inject_keys=%d inject_text=%d hid=%d\n",
         (unsigned)axidev_io_keyboard_type(),
         capabilities.can_inject_keys ? 1 : 0,
         capabilities.can_inject_text ? 1 : 0,
         capabilities.can_simulate_hid ? 1 : 0);

  axidev_io_log_set_level(AXIDEV_IO_LOG_LEVEL_DEBUG);
  AXIDEV_IO_LOG_INFO("example initialized");

  if (!axidev_io_keyboard_type_text("Hello from axidev-io")) {
    print_last_error_if_any("axidev_io_keyboard_type_text");
  }

  if (!axidev_io_keyboard_tap((axidev_io_keyboard_key_with_modifier_t){
          AXIDEV_IO_KEY_A, AXIDEV_IO_MOD_SHIFT})) {
    print_last_error_if_any("axidev_io_keyboard_tap");
  }

  printf("starting listener for 3 seconds\n");
  if (!axidev_io_listener_start(example_listener_cb, NULL)) {
    print_last_error_if_any("axidev_io_listener_start");
  } else {
    sleep_ms(3000);
    axidev_io_listener_stop();
  }

  axidev_io_keyboard_free();
  return EXIT_SUCCESS;
}

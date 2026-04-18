#include <axidev-io/c_api.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void append_utf8(char *buffer, size_t buffer_size, uint32_t codepoint) {
  size_t length = strlen(buffer);
  if (codepoint <= 0x7Fu && length + 1 < buffer_size) {
    buffer[length] = (char)codepoint;
    buffer[length + 1] = '\0';
  }
}

static void listener_cb(uint32_t codepoint,
                        axidev_io_keyboard_key_with_modifier_t key_mod,
                        bool pressed, void *user_data) {
  char *observed = (char *)user_data;
  if (pressed) {
    return;
  }
  if (key_mod.key == AXIDEV_IO_KEY_BACKSPACE) {
    size_t length = strlen(observed);
    if (length > 0) {
      observed[length - 1] = '\0';
    }
  } else if (key_mod.key != AXIDEV_IO_KEY_ENTER && codepoint != 0) {
    append_utf8(observed, 256, codepoint);
  }
}

int main(void) {
  char observed[256];
  char typed[256];

  memset(observed, 0, sizeof(observed));
  memset(typed, 0, sizeof(typed));

  printf("Listener integration test\n");
  printf("Type a short string, then press ENTER.\n");

  if (!axidev_io_listener_start(listener_cb, observed)) {
    char *error_text = axidev_io_get_last_error();
    fprintf(stderr, "listener start failed: %s\n",
            error_text != NULL ? error_text : "(no error)");
    axidev_io_free_string(error_text);
    return EXIT_FAILURE;
  }

  fgets(typed, sizeof(typed), stdin);
  axidev_io_listener_stop();
  printf("typed=%sobserved=%s\n", typed, observed);
  return EXIT_SUCCESS;
}

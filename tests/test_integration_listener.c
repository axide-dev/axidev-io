#include <axidev-io/c_api.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct listener_observation_t {
  char text[512];
  unsigned int pressed_count;
  unsigned int released_count;
  unsigned int repeat_press_count;
  unsigned int shifted_count;
  unsigned int ctrl_count;
  unsigned int alt_count;
  unsigned int super_count;
  unsigned int capslock_count;
  unsigned int numlock_count;
  axidev_io_keyboard_key_t last_pressed_key;
  bool pressed_keys[AXIDEV_IO_KEY_RF_KILL + 1];
  bool released_keys[AXIDEV_IO_KEY_RF_KILL + 1];
} listener_observation_t;

static void append_utf8(char *buffer, size_t buffer_size, uint32_t codepoint) {
  size_t length = strlen(buffer);
  if (codepoint <= 0x7Fu && length + 1 < buffer_size) {
    buffer[length] = (char)codepoint;
    buffer[length + 1] = '\0';
  }
}

static void mark_key(bool *keys, axidev_io_keyboard_key_t key) {
  if (key > AXIDEV_IO_KEY_UNKNOWN && key <= AXIDEV_IO_KEY_RF_KILL) {
    keys[key] = true;
  }
}

static void listener_cb(uint32_t codepoint,
                        axidev_io_keyboard_key_with_modifier_t key_mod,
                        bool pressed, void *user_data) {
  listener_observation_t *observed = (listener_observation_t *)user_data;

  if (pressed) {
    ++observed->pressed_count;
    mark_key(observed->pressed_keys, key_mod.key);
    if (observed->last_pressed_key == key_mod.key &&
        key_mod.key != AXIDEV_IO_KEY_UNKNOWN) {
      ++observed->repeat_press_count;
    }
    observed->last_pressed_key = key_mod.key;
  } else {
    ++observed->released_count;
    mark_key(observed->released_keys, key_mod.key);
    observed->last_pressed_key = AXIDEV_IO_KEY_UNKNOWN;
  }

  if ((key_mod.mods & AXIDEV_IO_MOD_SHIFT) != 0) {
    ++observed->shifted_count;
  }
  if ((key_mod.mods & AXIDEV_IO_MOD_CTRL) != 0) {
    ++observed->ctrl_count;
  }
  if ((key_mod.mods & AXIDEV_IO_MOD_ALT) != 0) {
    ++observed->alt_count;
  }
  if ((key_mod.mods & AXIDEV_IO_MOD_SUPER) != 0) {
    ++observed->super_count;
  }
  if ((key_mod.mods & AXIDEV_IO_MOD_CAPSLOCK) != 0) {
    ++observed->capslock_count;
  }
  if ((key_mod.mods & AXIDEV_IO_MOD_NUMLOCK) != 0) {
    ++observed->numlock_count;
  }

  if (!pressed) {
    if (key_mod.key == AXIDEV_IO_KEY_BACKSPACE) {
      size_t length = strlen(observed->text);
      if (length > 0) {
        observed->text[length - 1] = '\0';
      }
    } else if (key_mod.key != AXIDEV_IO_KEY_ENTER && codepoint != 0) {
      append_utf8(observed->text, sizeof(observed->text), codepoint);
    }
  }
}

static void print_key_seen(const char *label, listener_observation_t *observed,
                           axidev_io_keyboard_key_t key) {
  char *name = axidev_io_keyboard_key_to_string(key);
  printf("%-16s %-18s down=%s up=%s\n", label,
         name != NULL ? name : "(unknown)",
         observed->pressed_keys[key] ? "yes" : "no",
         observed->released_keys[key] ? "yes" : "no");
  axidev_io_free_string(name);
}

int main(void) {
  listener_observation_t observed;
  char typed[256];

  memset(&observed, 0, sizeof(observed));
  memset(typed, 0, sizeof(typed));

  printf("Listener integration test\n");
  printf("Exercise the keyboard, then press ENTER to stop.\n");
  printf("Suggested pass: letters, digits, shifted symbols, hold a key for "
         "repeat, Backspace, Tab, arrows, Home/End, Insert/Delete, "
         "CapsLock twice, NumLock twice, and a harmless modifier combo.\n");

  if (!axidev_io_listener_start(listener_cb, &observed)) {
    char *error_text = axidev_io_get_last_error();
    fprintf(stderr, "listener start failed: %s\n",
            error_text != NULL ? error_text : "(no error)");
    axidev_io_free_string(error_text);
    return EXIT_FAILURE;
  }

  fgets(typed, sizeof(typed), stdin);
  axidev_io_listener_stop();
  printf("typed=%sobserved=%s\n", typed, observed.text);
  printf("events: pressed=%u released=%u repeat-presses=%u\n",
         observed.pressed_count, observed.released_count,
         observed.repeat_press_count);
  printf("modifier observations: shift=%u ctrl=%u alt=%u super=%u caps=%u "
         "num=%u\n",
         observed.shifted_count, observed.ctrl_count, observed.alt_count,
         observed.super_count, observed.capslock_count,
         observed.numlock_count);
  print_key_seen("editing", &observed, AXIDEV_IO_KEY_BACKSPACE);
  print_key_seen("editing", &observed, AXIDEV_IO_KEY_DELETE);
  print_key_seen("navigation", &observed, AXIDEV_IO_KEY_LEFT);
  print_key_seen("navigation", &observed, AXIDEV_IO_KEY_RIGHT);
  print_key_seen("navigation", &observed, AXIDEV_IO_KEY_HOME);
  print_key_seen("navigation", &observed, AXIDEV_IO_KEY_END);
  print_key_seen("layout", &observed, AXIDEV_IO_KEY_CAPS_LOCK);
  print_key_seen("layout", &observed, AXIDEV_IO_KEY_NUM_LOCK);
  print_key_seen("modifier", &observed, AXIDEV_IO_KEY_SHIFT_LEFT);
  print_key_seen("modifier", &observed, AXIDEV_IO_KEY_SHIFT_RIGHT);
  print_key_seen("modifier", &observed, AXIDEV_IO_KEY_CTRL_LEFT);
  print_key_seen("modifier", &observed, AXIDEV_IO_KEY_ALT_LEFT);
  return EXIT_SUCCESS;
}

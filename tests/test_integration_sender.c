#include <axidev-io/c_api.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#endif

static int send_key(axidev_io_keyboard_key_t key,
                    axidev_io_keyboard_modifier_t mods) {
  if (!axidev_io_keyboard_tap((axidev_io_keyboard_key_with_modifier_t){key,
                                                                       mods})) {
    char *error_text = axidev_io_get_last_error();
    fprintf(stderr, "tap failed for key %d mods 0x%x: %s\n", (int)key,
            (unsigned int)mods, error_text != NULL ? error_text : "(no error)");
    axidev_io_free_string(error_text);
    return 0;
  }
  return 1;
}

static int send_text(const char *text) {
  if (!axidev_io_keyboard_type_text(text)) {
    char *error_text = axidev_io_get_last_error();
    fprintf(stderr, "type_text failed for \"%s\": %s\n", text,
            error_text != NULL ? error_text : "(no error)");
    axidev_io_free_string(error_text);
    return 0;
  }
  return 1;
}

static int send_down(axidev_io_keyboard_key_t key,
                     axidev_io_keyboard_modifier_t mods, bool repeat) {
  if (!axidev_io_keyboard_key_down(
          (axidev_io_keyboard_key_with_modifier_t){key, mods}, repeat)) {
    char *error_text = axidev_io_get_last_error();
    fprintf(stderr, "key_down failed for key %d mods 0x%x repeat=%d: %s\n",
            (int)key, (unsigned int)mods, repeat ? 1 : 0,
            error_text != NULL ? error_text : "(no error)");
    axidev_io_free_string(error_text);
    return 0;
  }
  return 1;
}

static int send_up(axidev_io_keyboard_key_t key,
                   axidev_io_keyboard_modifier_t mods) {
  if (!axidev_io_keyboard_key_up(
          (axidev_io_keyboard_key_with_modifier_t){key, mods})) {
    char *error_text = axidev_io_get_last_error();
    fprintf(stderr, "key_up failed for key %d mods 0x%x: %s\n", (int)key,
            (unsigned int)mods, error_text != NULL ? error_text : "(no error)");
    axidev_io_free_string(error_text);
    return 0;
  }
  return 1;
}

static void send_repeat_pause(void) {
#ifdef _WIN32
  Sleep(900);
#else
  struct timespec delay;
  delay.tv_sec = 0;
  delay.tv_nsec = 900000000L;
  nanosleep(&delay, NULL);
#endif
}

static int send_test_keys(void) {
  axidev_io_keyboard_set_key_delay(2500);

  if (!send_text("az09")) {
    return 0;
  }
  if (!send_key(AXIDEV_IO_KEY_A, AXIDEV_IO_MOD_SHIFT) ||
      !send_key(AXIDEV_IO_KEY_NUM1, AXIDEV_IO_MOD_NONE) ||
      !send_key(AXIDEV_IO_KEY_EXCLAMATION, AXIDEV_IO_MOD_NONE) ||
      !send_key(AXIDEV_IO_KEY_AT, AXIDEV_IO_MOD_NONE) ||
      !send_key(AXIDEV_IO_KEY_HASHTAG, AXIDEV_IO_MOD_NONE) ||
      !send_key(AXIDEV_IO_KEY_DOLLAR, AXIDEV_IO_MOD_NONE) ||
      !send_key(AXIDEV_IO_KEY_PERCENT, AXIDEV_IO_MOD_NONE) ||
      !send_key(AXIDEV_IO_KEY_CARET, AXIDEV_IO_MOD_NONE) ||
      !send_key(AXIDEV_IO_KEY_AMPERSAND, AXIDEV_IO_MOD_NONE) ||
      !send_key(AXIDEV_IO_KEY_ASTERISK, AXIDEV_IO_MOD_NONE) ||
      !send_key(AXIDEV_IO_KEY_LEFT_PAREN, AXIDEV_IO_MOD_NONE) ||
      !send_key(AXIDEV_IO_KEY_RIGHT_PAREN, AXIDEV_IO_MOD_NONE) ||
      !send_key(AXIDEV_IO_KEY_MINUS, AXIDEV_IO_MOD_NONE) ||
      !send_key(AXIDEV_IO_KEY_UNDERSCORE, AXIDEV_IO_MOD_NONE) ||
      !send_key(AXIDEV_IO_KEY_EQUAL, AXIDEV_IO_MOD_NONE) ||
      !send_key(AXIDEV_IO_KEY_PLUS, AXIDEV_IO_MOD_NONE) ||
      !send_key(AXIDEV_IO_KEY_LEFT_BRACKET, AXIDEV_IO_MOD_NONE) ||
      !send_key(AXIDEV_IO_KEY_RIGHT_BRACKET, AXIDEV_IO_MOD_NONE) ||
      !send_key(AXIDEV_IO_KEY_BACKSLASH, AXIDEV_IO_MOD_NONE) ||
      !send_key(AXIDEV_IO_KEY_BAR, AXIDEV_IO_MOD_NONE) ||
      !send_key(AXIDEV_IO_KEY_SEMICOLON, AXIDEV_IO_MOD_NONE) ||
      !send_key(AXIDEV_IO_KEY_COLON, AXIDEV_IO_MOD_NONE) ||
      !send_key(AXIDEV_IO_KEY_APOSTROPHE, AXIDEV_IO_MOD_NONE) ||
      !send_key(AXIDEV_IO_KEY_QUOTE, AXIDEV_IO_MOD_NONE) ||
      !send_key(AXIDEV_IO_KEY_COMMA, AXIDEV_IO_MOD_NONE) ||
      !send_key(AXIDEV_IO_KEY_LESS_THAN, AXIDEV_IO_MOD_NONE) ||
      !send_key(AXIDEV_IO_KEY_PERIOD, AXIDEV_IO_MOD_NONE) ||
      !send_key(AXIDEV_IO_KEY_GREATER_THAN, AXIDEV_IO_MOD_NONE) ||
      !send_key(AXIDEV_IO_KEY_SLASH, AXIDEV_IO_MOD_NONE) ||
      !send_key(AXIDEV_IO_KEY_QUESTION_MARK, AXIDEV_IO_MOD_NONE)) {
    return 0;
  }

  if (!axidev_io_keyboard_hold_modifier(AXIDEV_IO_MOD_SHIFT) ||
      !send_key(AXIDEV_IO_KEY_B, AXIDEV_IO_MOD_NONE) ||
      !axidev_io_keyboard_release_modifier(AXIDEV_IO_MOD_SHIFT)) {
    return 0;
  }

  if (!send_key(AXIDEV_IO_KEY_Z, AXIDEV_IO_MOD_ALT) ||
      !send_key(AXIDEV_IO_KEY_LEFT, AXIDEV_IO_MOD_NONE) ||
      !send_key(AXIDEV_IO_KEY_RIGHT, AXIDEV_IO_MOD_NONE) ||
      !send_key(AXIDEV_IO_KEY_HOME, AXIDEV_IO_MOD_NONE) ||
      !send_key(AXIDEV_IO_KEY_END, AXIDEV_IO_MOD_NONE) ||
      !send_key(AXIDEV_IO_KEY_INSERT, AXIDEV_IO_MOD_NONE) ||
      !send_key(AXIDEV_IO_KEY_INSERT, AXIDEV_IO_MOD_NONE) ||
      !send_key(AXIDEV_IO_KEY_CAPS_LOCK, AXIDEV_IO_MOD_NONE) ||
      !send_key(AXIDEV_IO_KEY_CAPS_LOCK, AXIDEV_IO_MOD_NONE) ||
      !send_key(AXIDEV_IO_KEY_NUM_LOCK, AXIDEV_IO_MOD_NONE) ||
      !send_key(AXIDEV_IO_KEY_NUM_LOCK, AXIDEV_IO_MOD_NONE) ||
      !send_key(AXIDEV_IO_KEY_F1, AXIDEV_IO_MOD_NONE) ||
      !send_key(AXIDEV_IO_KEY_F12, AXIDEV_IO_MOD_NONE)) {
    axidev_io_keyboard_release_all_modifiers();
    return 0;
  }

  if (!send_text("repeat:") || !send_down(AXIDEV_IO_KEY_R, AXIDEV_IO_MOD_NONE, true)) {
    return 0;
  }
  send_repeat_pause();
  if (!send_up(AXIDEV_IO_KEY_R, AXIDEV_IO_MOD_NONE)) {
    return 0;
  }

  if (!send_text(":backspaceX") ||
      !send_key(AXIDEV_IO_KEY_BACKSPACE, AXIDEV_IO_MOD_NONE) ||
      !send_text("Y") || !send_key(AXIDEV_IO_KEY_ENTER, AXIDEV_IO_MOD_NONE)) {
    return 0;
  }

  return axidev_io_keyboard_release_all_modifiers();
}

#ifdef _WIN32
static DWORD WINAPI send_thread_main(LPVOID user_data) {
  int *ok = (int *)user_data;
  Sleep(500);
  *ok = send_test_keys();
  return 0;
}
#else
static void sleep_half_second(void) {
  struct timespec delay;

  delay.tv_sec = 0;
  delay.tv_nsec = 500000000L;
  nanosleep(&delay, NULL);
}

static int g_tty_fd = -1;
static struct termios g_old_termios;
static int g_termios_saved = 0;

static int open_tty(void) {
  if (g_tty_fd >= 0) {
    return 1;
  }

  g_tty_fd = open("/dev/tty", O_RDWR);
  if (g_tty_fd < 0) {
    perror("open(/dev/tty)");
    return 0;
  }

  return 1;
}

static int read_line_from_tty(char *buffer, size_t buffer_size) {
  size_t len = 0;

  while (len + 1 < buffer_size) {
    ssize_t read_count = read(g_tty_fd, &buffer[len], 1);
    if (read_count < 0) {
      if (errno == EINTR) {
        continue;
      }
      perror("read");
      return 0;
    }
    if (read_count == 0) {
      break;
    }
    if (buffer[len] == '\n' || buffer[len] == '\r') {
      ++len;
      break;
    }
    ++len;
  }

  buffer[len] = '\0';
  return 1;
}

static int enable_raw_mode(void) {
  struct termios t;

  if (tcgetattr(g_tty_fd, &g_old_termios) != 0) {
    perror("tcgetattr");
    return 0;
  }

  g_termios_saved = 1;
  t = g_old_termios;

  t.c_lflag &= ~(ICANON | ECHO);
  t.c_cc[VMIN] = 1;
  t.c_cc[VTIME] = 0;

  if (tcsetattr(g_tty_fd, TCSANOW, &t) != 0) {
    perror("tcsetattr");
    return 0;
  }

  return 1;
}

static void restore_terminal(void) {
  if (g_termios_saved && g_tty_fd >= 0) {
    tcsetattr(g_tty_fd, TCSANOW, &g_old_termios);
  }
}

static void close_tty(void) {
  if (g_tty_fd >= 0) {
    close(g_tty_fd);
    g_tty_fd = -1;
  }
}

static void *send_thread_main(void *user_data) {
  int *ok = (int *)user_data;
  sleep_half_second();
  *ok = send_test_keys();
  return NULL;
}
#endif

static int observed_has_expected_parts(const char *observed) {
  const char *required_prefix =
      "az09A1!@#$%^&*()-_=+[]\\|;:'\",<.>/?B";
  const char *repeat_marker;

  if (strncmp(observed, required_prefix, strlen(required_prefix)) != 0) {
    fprintf(stderr, "expected prefix: %s\n", required_prefix);
    return 0;
  }

  repeat_marker = strstr(observed, "repeat:r");
  if (repeat_marker == NULL) {
    fprintf(stderr, "expected repeat marker with at least one r\n");
    return 0;
  }
  if (strstr(observed, ":backspaceY") == NULL) {
    fprintf(stderr, "expected corrected backspace suffix\n");
    return 0;
  }
  return 1;
}

int main(void) {
  char buffer[256];
  int send_ok = 0;

  if (!axidev_io_keyboard_initialize()) {
    fprintf(stderr, "sender init failed\n");
    return EXIT_FAILURE;
  }

  printf("Sender integration test\n");
  printf("Keep this terminal focused, then press ENTER.\n");
  printf("The test will inject printable layout-sensitive keys, modifier combos, "
         "lock-key double taps, navigation keys, repeat, backspace, and ENTER.\n");
  fflush(stdout);

#ifdef _WIN32
  {
    HANDLE thread = CreateThread(NULL, 0, send_thread_main, &send_ok, 0, NULL);
    fgets(buffer, sizeof(buffer), stdin);
    fgets(buffer, sizeof(buffer), stdin);
    WaitForSingleObject(thread, INFINITE);
    CloseHandle(thread);
  }
#else
  {
    size_t len = 0;
    pthread_t thread;
    char ch;

    if (!open_tty()) {
      axidev_io_keyboard_free();
      return EXIT_FAILURE;
    }

    if (!read_line_from_tty(buffer, sizeof(buffer))) {
      close_tty();
      axidev_io_keyboard_free();
      return EXIT_FAILURE;
    }

    if (!enable_raw_mode()) {
      close_tty();
      axidev_io_keyboard_free();
      return EXIT_FAILURE;
    }

    pthread_create(&thread, NULL, send_thread_main, &send_ok);

    // on attend réellement les caractères reçus par le process
    while (len + 1 < sizeof(buffer)) {
      ssize_t read_count = read(g_tty_fd, &ch, 1);
      if (read_count < 0) {
        if (errno == EINTR) {
          continue;
        }
        perror("read");
        break;
      }
      if (read_count == 0) {
        break;
      }

      buffer[len++] = ch;

      if (ch == '\n' || ch == '\r') {
        break;
      }
    }

    buffer[len] = '\0';

    pthread_join(thread, NULL);
    restore_terminal();
    close_tty();
  }
#endif

  printf("Observed line: %s\n", buffer);
  fflush(stdout);

  axidev_io_keyboard_free();
  if (!send_ok || !observed_has_expected_parts(buffer)) {
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

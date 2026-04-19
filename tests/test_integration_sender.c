#include <axidev-io/c_api.h>

#include <stdio.h>
#include <stdlib.h>

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

static void send_test_keys(void) {
  axidev_io_keyboard_tap(
      (axidev_io_keyboard_key_with_modifier_t){AXIDEV_IO_KEY_Z, 0});
  axidev_io_keyboard_tap(
      (axidev_io_keyboard_key_with_modifier_t){AXIDEV_IO_KEY_W, 0});
  axidev_io_keyboard_tap(
      (axidev_io_keyboard_key_with_modifier_t){AXIDEV_IO_KEY_NUM1, 0});
  axidev_io_keyboard_tap(
      (axidev_io_keyboard_key_with_modifier_t){AXIDEV_IO_KEY_ENTER, 0});
}

#ifdef _WIN32
static DWORD WINAPI send_thread_main(LPVOID user_data) {
  (void)user_data;
  Sleep(500);
  send_test_keys();
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
  (void)user_data;
  sleep_half_second();
  send_test_keys();
  return NULL;
}
#endif

int main(void) {
  char buffer[256];

  if (!axidev_io_keyboard_initialize()) {
    fprintf(stderr, "sender init failed\n");
    return EXIT_FAILURE;
  }

  printf("Sender integration test\n");
  printf("Keep this terminal focused, then press ENTER.\n");
  fflush(stdout);

#ifdef _WIN32
  {
    HANDLE thread = CreateThread(NULL, 0, send_thread_main, NULL, 0, NULL);
    fgets(buffer, sizeof(buffer), stdin); // validation manuelle du focus
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

    pthread_create(&thread, NULL, send_thread_main, NULL);

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
  return EXIT_SUCCESS;
}

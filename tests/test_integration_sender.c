#ifndef _WIN32
#include <errno.h>
#include <pthread.h>
#include <termios.h>
#include <unistd.h>

static struct termios g_old_termios;
static int g_termios_saved = 0;

static int enable_raw_mode(void) {
  struct termios t;

  if (tcgetattr(STDIN_FILENO, &g_old_termios) != 0) {
    perror("tcgetattr");
    return 0;
  }

  g_termios_saved = 1;
  t = g_old_termios;

  t.c_lflag &= ~(ICANON | ECHO);
  t.c_cc[VMIN] = 1;
  t.c_cc[VTIME] = 0;

  if (tcsetattr(STDIN_FILENO, TCSANOW, &t) != 0) {
    perror("tcsetattr");
    return 0;
  }

  return 1;
}

static void restore_terminal(void) {
  if (g_termios_saved) {
    tcsetattr(STDIN_FILENO, TCSANOW, &g_old_termios);
  }
}

static void *send_thread_main(void *user_data) {
  (void)user_data;
  usleep(500000);

  axidev_io_keyboard_tap(
      (axidev_io_keyboard_key_with_modifier_t){AXIDEV_IO_KEY_Z, 0});
  axidev_io_keyboard_tap(
      (axidev_io_keyboard_key_with_modifier_t){AXIDEV_IO_KEY_W, 0});
  axidev_io_keyboard_tap(
      (axidev_io_keyboard_key_with_modifier_t){AXIDEV_IO_KEY_NUM1, 0});
  axidev_io_keyboard_tap(
      (axidev_io_keyboard_key_with_modifier_t){AXIDEV_IO_KEY_ENTER, 0});

  return NULL;
}
#endif

int main(void) {
  char buffer[256];
  size_t len = 0;

  if (!axidev_io_keyboard_initialize()) {
    fprintf(stderr, "sender init failed\n");
    return EXIT_FAILURE;
  }

  printf("Sender integration test\n");
  printf("Keep this terminal focused, then press ENTER.\n");
  fflush(stdout);

  fgets(buffer, sizeof(buffer), stdin); // validation manuelle du focus

#ifdef _WIN32
  {
    HANDLE thread = CreateThread(NULL, 0, send_thread_main, NULL, 0, NULL);
    fgets(buffer, sizeof(buffer), stdin);
    WaitForSingleObject(thread, INFINITE);
    CloseHandle(thread);
  }
#else
  {
    pthread_t thread;
    int ch;

    if (!enable_raw_mode()) {
      axidev_io_keyboard_free();
      return EXIT_FAILURE;
    }

    pthread_create(&thread, NULL, send_thread_main, NULL);

    // on attend réellement les caractères reçus par le process
    while (len + 1 < sizeof(buffer)) {
      ch = getchar();
      if (ch == EOF) {
        break;
      }

      buffer[len++] = (char)ch;

      if (ch == '\n' || ch == '\r') {
        break;
      }
    }

    buffer[len] = '\0';

    pthread_join(thread, NULL);
    restore_terminal();
  }
#endif

  printf("Observed line: %s\n", buffer);
  fflush(stdout);

  axidev_io_keyboard_free();
  return EXIT_SUCCESS;
}

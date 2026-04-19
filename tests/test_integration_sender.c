#include <axidev-io/c_api.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <Windows.h>
static DWORD WINAPI send_thread_main(LPVOID user_data) {
  (void)user_data;
  Sleep(500);
  axidev_io_keyboard_tap(
      (axidev_io_keyboard_key_with_modifier_t){AXIDEV_IO_KEY_Z, 0});
  axidev_io_keyboard_tap(
      (axidev_io_keyboard_key_with_modifier_t){AXIDEV_IO_KEY_W, 0});
  axidev_io_keyboard_tap(
      (axidev_io_keyboard_key_with_modifier_t){AXIDEV_IO_KEY_NUM1, 0});
  axidev_io_keyboard_tap(
      (axidev_io_keyboard_key_with_modifier_t){AXIDEV_IO_KEY_ENTER, 0});
  return 0;
}
#else
#include <pthread.h>
#include <unistd.h>
static void *send_thread_main(void *user_data) {
  (void)user_data;
  usleep(500000);
  axidev_io_keyboard_tap(
      (axidev_io_keyboard_key_with_modifier_t){AXIDEV_IO_KEY_Z, 0});
  axidev_io_keyboard_tap(
      (axidev_io_keyboard_key_with_modifier_t){AXIDEV_IO_KEY_W, 0});
  axidev_io_keyboard_tap(
      (axidev_io_keyboard_key_with_modifier_t){AXIDEV_IO_KEY_NUM1, 0});
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
  fgets(buffer, sizeof(buffer), stdin);

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
    pthread_create(&thread, NULL, send_thread_main, NULL);
    printf("The test will type zw1. Press ENTER after you see it.\n");
    fflush(stdout);
    fgets(buffer, sizeof(buffer), stdin);
    pthread_join(thread, NULL);
  }
#endif

  printf("Observed line: %s\n", buffer);
  axidev_io_keyboard_free();
  return EXIT_SUCCESS;
}

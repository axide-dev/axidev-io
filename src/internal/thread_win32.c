#ifdef _WIN32

#include "thread.h"

#include <process.h>

typedef struct axidev_io_thread_start_data {
  axidev_io_thread_fn fn;
  void *user_data;
} axidev_io_thread_start_data;

static BOOL CALLBACK axidev_io_once_trampoline(PINIT_ONCE once, PVOID param,
                                               PVOID *context) {
  void (*fn)(void) = (void (*)(void))param;
  (void)once;
  (void)context;
  fn();
  return TRUE;
}

static unsigned __stdcall axidev_io_thread_entry(void *param) {
  axidev_io_thread_start_data *start_data;
  int result;

  start_data = (axidev_io_thread_start_data *)param;
  result = start_data->fn(start_data->user_data);
  free(start_data);
  return (unsigned)result;
}

bool axidev_io_mutex_init(axidev_io_mutex *mutex) {
  if (mutex == NULL || mutex->initialized) {
    return mutex != NULL;
  }
  InitializeCriticalSection(&mutex->native);
  mutex->initialized = true;
  return true;
}

void axidev_io_mutex_destroy(axidev_io_mutex *mutex) {
  if (mutex == NULL || !mutex->initialized) {
    return;
  }
  DeleteCriticalSection(&mutex->native);
  mutex->initialized = false;
}

void axidev_io_mutex_lock(axidev_io_mutex *mutex) {
  if (mutex != NULL && mutex->initialized) {
    EnterCriticalSection(&mutex->native);
  }
}

void axidev_io_mutex_unlock(axidev_io_mutex *mutex) {
  if (mutex != NULL && mutex->initialized) {
    LeaveCriticalSection(&mutex->native);
  }
}

bool axidev_io_thread_create(axidev_io_thread *thread, axidev_io_thread_fn fn,
                             void *user_data) {
  axidev_io_thread_start_data *start_data;
  uintptr_t handle;

  if (thread == NULL || fn == NULL) {
    return false;
  }

  start_data = (axidev_io_thread_start_data *)malloc(sizeof(*start_data));
  if (start_data == NULL) {
    return false;
  }

  start_data->fn = fn;
  start_data->user_data = user_data;

  handle = _beginthreadex(NULL, 0, axidev_io_thread_entry, start_data, 0,
                          (unsigned *)&thread->id);
  if (handle == 0) {
    free(start_data);
    return false;
  }

  thread->handle = (HANDLE)handle;
  thread->joinable = true;
  return true;
}

void axidev_io_thread_join(axidev_io_thread *thread) {
  if (thread == NULL || !thread->joinable) {
    return;
  }
  WaitForSingleObject(thread->handle, INFINITE);
  CloseHandle(thread->handle);
  thread->handle = NULL;
  thread->id = 0;
  thread->joinable = false;
}

void axidev_io_call_once(axidev_io_once *once, void (*fn)(void)) {
  if (once == NULL || fn == NULL) {
    return;
  }
  InitOnceExecuteOnce(&once->native, axidev_io_once_trampoline, fn, NULL);
}

void axidev_io_sleep_ms(uint32_t milliseconds) { Sleep(milliseconds); }

void axidev_io_sleep_us(uint32_t microseconds) {
  DWORD milliseconds = (DWORD)((microseconds + 999u) / 1000u);
  if (milliseconds == 0) {
    milliseconds = 1;
  }
  Sleep(milliseconds);
}

uint64_t axidev_io_monotonic_time_ms(void) { return GetTickCount64(); }

#endif

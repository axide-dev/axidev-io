#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#ifndef _WIN32

#include "thread.h"

#include <errno.h>
#include <stdlib.h>
#include <time.h>

typedef struct axidev_io_thread_start_data {
  axidev_io_thread_fn fn;
  void *user_data;
} axidev_io_thread_start_data;

static void *axidev_io_thread_entry(void *param) {
  axidev_io_thread_start_data *start_data;
  int result;

  start_data = (axidev_io_thread_start_data *)param;
  result = start_data->fn(start_data->user_data);
  free(start_data);
  return (void *)(intptr_t)result;
}

bool axidev_io_mutex_init(axidev_io_mutex *mutex) {
  if (mutex == NULL || mutex->initialized) {
    return mutex != NULL;
  }
  if (pthread_mutex_init(&mutex->native, NULL) != 0) {
    return false;
  }
  mutex->initialized = true;
  return true;
}

void axidev_io_mutex_destroy(axidev_io_mutex *mutex) {
  if (mutex == NULL || !mutex->initialized) {
    return;
  }
  pthread_mutex_destroy(&mutex->native);
  mutex->initialized = false;
}

void axidev_io_mutex_lock(axidev_io_mutex *mutex) {
  if (mutex != NULL && mutex->initialized) {
    pthread_mutex_lock(&mutex->native);
  }
}

void axidev_io_mutex_unlock(axidev_io_mutex *mutex) {
  if (mutex != NULL && mutex->initialized) {
    pthread_mutex_unlock(&mutex->native);
  }
}

bool axidev_io_thread_create(axidev_io_thread *thread, axidev_io_thread_fn fn,
                             void *user_data) {
  axidev_io_thread_start_data *start_data;

  if (thread == NULL || fn == NULL) {
    return false;
  }

  start_data = (axidev_io_thread_start_data *)malloc(sizeof(*start_data));
  if (start_data == NULL) {
    return false;
  }

  start_data->fn = fn;
  start_data->user_data = user_data;
  if (pthread_create(&thread->handle, NULL, axidev_io_thread_entry,
                     start_data) != 0) {
    free(start_data);
    return false;
  }

  thread->joinable = true;
  return true;
}

void axidev_io_thread_join(axidev_io_thread *thread) {
  if (thread == NULL || !thread->joinable) {
    return;
  }
  pthread_join(thread->handle, NULL);
  thread->joinable = false;
}

void axidev_io_call_once(axidev_io_once *once, void (*fn)(void)) {
  if (once == NULL || fn == NULL) {
    return;
  }
  pthread_mutex_lock(&once->mutex);
  if (once->called) {
    pthread_mutex_unlock(&once->mutex);
    return;
  }
  once->called = true;
  pthread_mutex_unlock(&once->mutex);
  fn();
}

static void axidev_io_sleep_ns(uint64_t nanoseconds) {
  struct timespec request;
  struct timespec remaining;

  request.tv_sec = (time_t)(nanoseconds / 1000000000u);
  request.tv_nsec = (long)(nanoseconds % 1000000000u);
  while (nanosleep(&request, &remaining) != 0 && errno == EINTR) {
    request = remaining;
  }
}

void axidev_io_sleep_ms(uint32_t milliseconds) {
  axidev_io_sleep_ns((uint64_t)milliseconds * 1000000u);
}

void axidev_io_sleep_us(uint32_t microseconds) {
  axidev_io_sleep_ns((uint64_t)microseconds * 1000u);
}

uint64_t axidev_io_monotonic_time_ms(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ((uint64_t)ts.tv_sec * 1000u) + ((uint64_t)ts.tv_nsec / 1000000u);
}

#endif

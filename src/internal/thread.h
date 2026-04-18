#pragma once
#ifndef AXIDEV_IO_INTERNAL_THREAD_H
#define AXIDEV_IO_INTERNAL_THREAD_H

#include <stdbool.h>
#include <stdint.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <pthread.h>
#endif

typedef int (*axidev_io_thread_fn)(void *user_data);

typedef struct axidev_io_thread {
#ifdef _WIN32
  HANDLE handle;
  DWORD id;
#else
  pthread_t handle;
#endif
  bool joinable;
} axidev_io_thread;

typedef struct axidev_io_mutex {
#ifdef _WIN32
  CRITICAL_SECTION native;
#else
  pthread_mutex_t native;
#endif
  bool initialized;
} axidev_io_mutex;

typedef struct axidev_io_once {
#ifdef _WIN32
  INIT_ONCE native;
#else
  pthread_mutex_t mutex;
  bool called;
#endif
} axidev_io_once;

#ifdef _WIN32
#define AXIDEV_IO_ONCE_INIT {INIT_ONCE_STATIC_INIT}
#else
#define AXIDEV_IO_ONCE_INIT {PTHREAD_MUTEX_INITIALIZER, false}
#endif

bool axidev_io_mutex_init(axidev_io_mutex *mutex);
void axidev_io_mutex_destroy(axidev_io_mutex *mutex);
void axidev_io_mutex_lock(axidev_io_mutex *mutex);
void axidev_io_mutex_unlock(axidev_io_mutex *mutex);

bool axidev_io_thread_create(axidev_io_thread *thread, axidev_io_thread_fn fn,
                             void *user_data);
void axidev_io_thread_join(axidev_io_thread *thread);

void axidev_io_call_once(axidev_io_once *once, void (*fn)(void));
void axidev_io_sleep_ms(uint32_t milliseconds);
void axidev_io_sleep_us(uint32_t microseconds);
uint64_t axidev_io_monotonic_time_ms(void);

#endif

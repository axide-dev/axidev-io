#pragma once
#ifndef AXIDEV_IO_KEYBOARD_LISTENER_INTERNAL_H
#define AXIDEV_IO_KEYBOARD_LISTENER_INTERNAL_H

#include "../../internal/context.h"

#include <stdatomic.h>

#include "../common/keymap_internal.h"

typedef struct axidev_io_keyboard_listener_impl {
  axidev_io_keyboard_listener_cb callback;
  void *user_data;
  axidev_io_mutex callback_lock;
  bool callback_lock_ready;
#ifdef _WIN32
  axidev_io_thread worker;
  void *hook;
  atomic_bool running;
  atomic_bool ready;
  uint32_t thread_id;
  struct axidev_io_windows_keymap_private *platform;
#elif defined(__linux__)
  axidev_io_thread worker;
  atomic_bool running;
  atomic_bool ready;
  struct axidev_io_linux_listener_platform *platform;
#else
  int unused;
#endif
} axidev_io_keyboard_listener_impl;

_Static_assert(sizeof(axidev_io_keyboard_listener_impl) <=
                   AXIDEV_IO_KEYBOARD_LISTENER_STORAGE_SIZE,
               "listener storage is too small");

axidev_io_keyboard_listener_impl *axidev_io_listener_impl_get(void);

axidev_io_result axidev_io_keyboard_listener_start_internal(
    axidev_io_keyboard_listener_cb callback, void *user_data);
void axidev_io_keyboard_listener_stop_internal(void);

#endif

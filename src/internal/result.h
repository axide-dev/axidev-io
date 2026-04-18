#pragma once
#ifndef AXIDEV_IO_INTERNAL_RESULT_H
#define AXIDEV_IO_INTERNAL_RESULT_H

typedef enum axidev_io_result {
  AXIDEV_IO_RESULT_OK = 0,
  AXIDEV_IO_RESULT_INVALID_ARGUMENT,
  AXIDEV_IO_RESULT_NOT_INITIALIZED,
  AXIDEV_IO_RESULT_ALREADY_INITIALIZED,
  AXIDEV_IO_RESULT_NOT_SUPPORTED,
  AXIDEV_IO_RESULT_PERMISSION_DENIED,
  AXIDEV_IO_RESULT_NOT_FOUND,
  AXIDEV_IO_RESULT_BUFFER_TOO_SMALL,
  AXIDEV_IO_RESULT_PLATFORM_ERROR,
  AXIDEV_IO_RESULT_INTERNAL_ERROR
} axidev_io_result;

static inline const char *axidev_io_result_to_string(axidev_io_result result) {
  switch (result) {
  case AXIDEV_IO_RESULT_OK:
    return "ok";
  case AXIDEV_IO_RESULT_INVALID_ARGUMENT:
    return "invalid_argument";
  case AXIDEV_IO_RESULT_NOT_INITIALIZED:
    return "not_initialized";
  case AXIDEV_IO_RESULT_ALREADY_INITIALIZED:
    return "already_initialized";
  case AXIDEV_IO_RESULT_NOT_SUPPORTED:
    return "not_supported";
  case AXIDEV_IO_RESULT_PERMISSION_DENIED:
    return "permission_denied";
  case AXIDEV_IO_RESULT_NOT_FOUND:
    return "not_found";
  case AXIDEV_IO_RESULT_BUFFER_TOO_SMALL:
    return "buffer_too_small";
  case AXIDEV_IO_RESULT_PLATFORM_ERROR:
    return "platform_error";
  case AXIDEV_IO_RESULT_INTERNAL_ERROR:
  default:
    return "internal_error";
  }
}

#endif

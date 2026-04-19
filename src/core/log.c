#if !defined(_WIN32)
#define _POSIX_C_SOURCE 200809L
#endif

#include "../internal/context.h"

#include <axidev-io/c_api.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static axidev_io_once g_log_once = AXIDEV_IO_ONCE_INIT;
static axidev_io_mutex g_log_output_lock;

static void axidev_io_log_init_once(void) {
  axidev_io_mutex_init(&g_log_output_lock);
}

static const char *axidev_io_log_level_string(axidev_io_log_level_t level) {
  switch (level) {
  case AXIDEV_IO_LOG_LEVEL_DEBUG:
    return "DEBUG";
  case AXIDEV_IO_LOG_LEVEL_INFO:
    return "INFO";
  case AXIDEV_IO_LOG_LEVEL_WARN:
    return "WARN";
  case AXIDEV_IO_LOG_LEVEL_ERROR:
  default:
    return "ERROR";
  }
}

static const char *axidev_io_log_level_color(axidev_io_log_level_t level) {
  switch (level) {
  case AXIDEV_IO_LOG_LEVEL_DEBUG:
    return "\x1b[33m";
  case AXIDEV_IO_LOG_LEVEL_INFO:
    return "\x1b[34m";
  case AXIDEV_IO_LOG_LEVEL_WARN:
    return "\x1b[38;5;208m";
  case AXIDEV_IO_LOG_LEVEL_ERROR:
  default:
    return "\x1b[31m";
  }
}

static bool axidev_io_log_colors_enabled(void) {
  const char *force = getenv("AXIDEV_IO_FORCE_COLORS");
  const char *disable = getenv("AXIDEV_IO_NO_COLOR");

  if (force != NULL && force[0] != '\0') {
    return true;
  }
  if (disable != NULL && disable[0] != '\0') {
    return false;
  }
  return false;
}

static const char *axidev_io_trim_path(const char *path) {
  const char *slash;
  const char *backslash;

  if (path == NULL) {
    return "";
  }

  slash = strrchr(path, '/');
  backslash = strrchr(path, '\\');
  if (slash == NULL && backslash == NULL) {
    return path;
  }
  if (slash == NULL) {
    return backslash + 1;
  }
  if (backslash == NULL) {
    return slash + 1;
  }
  return (slash > backslash ? slash : backslash) + 1;
}

AXIDEV_IO_API void axidev_io_log_set_level(axidev_io_log_level_t level) {
  axidev_io_context_ensure_runtime();
  axidev_io_global->log_level = level;
}

AXIDEV_IO_API axidev_io_log_level_t axidev_io_log_get_level(void) {
  axidev_io_context_ensure_runtime();
  return axidev_io_global->log_level;
}

AXIDEV_IO_API bool axidev_io_log_is_enabled(axidev_io_log_level_t level) {
  return (int)level >= (int)axidev_io_log_get_level();
}

AXIDEV_IO_API void axidev_io_log_message(axidev_io_log_level_t level,
                                         const char *file, int line,
                                         const char *fmt, ...) {
  va_list args;
  char time_buffer[64];
  time_t now;
  struct tm local_time;
  const bool colors = axidev_io_log_colors_enabled();

  if (!axidev_io_log_is_enabled(level) || fmt == NULL) {
    return;
  }

  axidev_io_call_once(&g_log_once, axidev_io_log_init_once);
  now = time(NULL);
#ifdef _WIN32
  localtime_s(&local_time, &now);
#else
  localtime_r(&now, &local_time);
#endif
  strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", &local_time);

  axidev_io_mutex_lock(&g_log_output_lock);
  fprintf(stderr, "[axidev-io] %s [", time_buffer);
  if (colors) {
    fputs(axidev_io_log_level_color(level), stderr);
  }
  fputs(axidev_io_log_level_string(level), stderr);
  if (colors) {
    fputs("\x1b[0m", stderr);
  }
  fprintf(stderr, "] %s:%d: ", axidev_io_trim_path(file), line);

  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
  fputc('\n', stderr);
  fflush(stderr);
  axidev_io_mutex_unlock(&g_log_output_lock);
}

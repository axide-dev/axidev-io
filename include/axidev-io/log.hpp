#pragma once

/**
 * @file log.hpp
 * @brief Lightweight, header-only logging utility used by axidev-io backends.
 *
 * Usage:
 *   @code{.cpp}
 *   #include <axidev-io/log.hpp>
 *   AXIDEV_IO_LOG_DEBUG("something happened: %d", value);
 *   AXIDEV_IO_LOG_INFO("ready");
 *   @endcode
 *
 * Runtime configuration is controlled by environment variables:
 *  - AXIDEV_IO_LOG_LEVEL: one of "debug", "info", "warn", "error".
 *    If unset, the legacy AXIDEV_OSK_DEBUG_BACKEND is consulted (unset ->
 *    debug enabled for testing; "0" disables debug).
 *  - AXIDEV_IO_FORCE_COLORS: non-empty -> force ANSI colors on.
 *  - AXIDEV_IO_NO_COLOR: non-empty -> disable ANSI colors.
 *
 * The header is intentionally small and portable and works in plain C++ and
 * Objective-C++ translation units.
 */

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <mutex>
#include <string>
#if defined(_WIN32) || defined(_WIN64)
#include <io.h>
#else
#include <unistd.h>
#endif

namespace axidev {
namespace io {
namespace log {

/**
 * @enum Level
 * @brief Logging severity levels used by the internal logging facility.
 *
 * Lower enum values are more verbose (Debug is the most verbose).
 */
enum class Level : int {
  Debug = 0,
  Info = 1,
  Warn = 2,
  Error = 3,
};

inline const char *levelToString(Level l) {
  switch (l) {
  case Level::Debug:
    return "DEBUG";
  case Level::Info:
    return "INFO";
  case Level::Warn:
    return "WARN";
  case Level::Error:
    return "ERROR";
  default:
    return "UNKNOWN";
  }
}

/**
 * @brief Parse runtime configuration to determine the default log level.
 *
 * Prefers the AXIDEV_IO_LOG_LEVEL environment variable; falls back to the
 * legacy AXIDEV_OSK_DEBUG_BACKEND behavior when AXIDEV_IO_LOG_LEVEL is not set.
 * @return Level The determined default log level.
 */
inline Level parseLevelFromEnv() {
  const char *lvlEnv = std::getenv("AXIDEV_IO_LOG_LEVEL");
  if (lvlEnv && lvlEnv[0] != '\0') {
    std::string s(lvlEnv);
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
      return static_cast<char>(std::tolower(c));
    });
    if (s == "debug" || s == "d" || s == "0")
      return Level::Debug;
    if (s == "info" || s == "i" || s == "1")
      return Level::Info;
    if (s == "warn" || s == "warning" || s == "w" || s == "2")
      return Level::Warn;
    if (s == "error" || s == "e" || s == "3")
      return Level::Error;
    // Unrecognized -> fallback to Info
    return Level::Info;
  }

  const char *legacy = std::getenv("AXIDEV_OSK_DEBUG_BACKEND");
  if (!legacy) {
    // Default to enabled for the time being while testing (preserve legacy
    // behaviour).
    return Level::Debug;
  }
  if (legacy[0] == '0')
    return Level::Info;
  return Level::Debug;
}

/**
 * @brief Accessor for the global log level used by the library.
 *
 * The log level is stored in an atomic so it can be changed safely at runtime.
 * @return std::atomic<Level>& Reference to the global atomic log level.
 */
inline std::atomic<Level> &globalLevel() {
  static std::atomic<Level> lvl(parseLevelFromEnv());
  return lvl;
}

inline void setLevel(Level l) { globalLevel().store(l); }
inline Level getLevel() { return globalLevel().load(); }

/**
 * @brief Determine whether a message at `level` should be emitted under the
 * current global level.
 * @param level Candidate message level to test.
 * @return true if the message should be emitted.
 */
inline bool isEnabled(Level level) {
  return static_cast<int>(level) >= static_cast<int>(getLevel());
}

/**
 * @internal
 * @brief Internal mutex used to serialize access to stderr.
 * @return std::mutex& Reference to the mutex used for output serialization.
 */
inline std::mutex &outputMutex() {
  static std::mutex m;
  return m;
}

/**
 * @internal
 * @brief Return an ANSI color escape sequence for the given log level.
 * @param l Log level.
 * @return const char* Null-terminated ANSI escape sequence (or reset code).
 */
inline const char *levelColor(Level l) {
  switch (l) {
  case Level::Debug:
    return "\x1b[33m"; // Yellow
  case Level::Info:
    return "\x1b[34m"; // Blue
  case Level::Warn:
    return "\x1b[38;5;208m"; // Orange (256-color)
  case Level::Error:
    return "\x1b[31m"; // Red
  default:
    return "\x1b[0m";
  }
}

/**
 * @internal
 * @brief Determine whether ANSI colors should be emitted.
 *
 * Colors can be forced via AXIDEV_IO_FORCE_COLORS or disabled with
 * AXIDEV_IO_NO_COLOR. Otherwise colors are enabled when stderr is a TTY.
 * @return true if colors are enabled, false otherwise.
 */
inline bool colorsEnabled() {
  const char *force = std::getenv("AXIDEV_IO_FORCE_COLORS");
  if (force && force[0] != '\0')
    return true;
  const char *no = std::getenv("AXIDEV_IO_NO_COLOR");
  if (no && no[0] != '\0')
    return false;
#if defined(_WIN32) || defined(_WIN64)
  return _isatty(_fileno(stderr));
#else
  return isatty(fileno(stderr));
#endif
}

/**
 * @internal
 * @brief Trim a file path so it starts at the last "axidev-io" component.
 *
 * If the substring "axidev-io" is not present this function returns the
 * basename (the portion after the last slash/backslash).
 * @param path Null-terminated file path.
 * @return const char* Pointer into `path` that points to the trimmed start.
 */
inline const char *trimPathToAxidevIo(const char *path) {
  if (!path)
    return path;
  const char *needle = "axidev-io";
  const size_t needle_len = std::strlen(needle);
  const char *last = nullptr;
  const char *p = path;
  while (true) {
    const char *found = std::strstr(p, needle);
    if (!found)
      break;
    const char *after = found + needle_len;
    if (*after == '/' || *after == '\\' || *after == '\0')
      last = found;
    p = found + 1;
  }
  if (last)
    return last;
  // Fallback to basename (after last slash/backslash)
  const char *last_slash = std::strrchr(path, '/');
  const char *last_backslash = std::strrchr(path, '\\');
  const char *base = path;
  if (last_slash && last_backslash)
    base = (last_slash > last_backslash) ? last_slash + 1 : last_backslash + 1;
  else if (last_slash)
    base = last_slash + 1;
  else if (last_backslash)
    base = last_backslash + 1;
  return base;
}

/**
 * @internal
 * @brief Emit a formatted log message using a va_list (thread-safe).
 *
 * Produces a timestamp (local time, millisecond precision), level name,
 * and source file/line prefix before the formatted message body.
 *
 * @param level Log level for the message.
 * @param file Source file name (typically `__FILE__`).
 * @param line Source line number (typically `__LINE__`).
 * @param fmt printf-style format string.
 * @param ap Preinitialized va_list of arguments for `fmt`.
 */
inline void vlog(Level level, const char *file, int line, const char *fmt,
                 va_list ap) {
  if (!isEnabled(level))
    return;

  // Grab time
  using namespace std::chrono;
  auto now = system_clock::now();
  auto ms =
      duration_cast<milliseconds>(now.time_since_epoch()) % milliseconds(1000);
  std::time_t t = system_clock::to_time_t(now);

  // Convert to local time in a portable way
  std::tm tmbuf;
#if defined(_MSC_VER) || defined(_WIN32)
  localtime_s(&tmbuf, &t);
#else
  localtime_r(&t, &tmbuf);
#endif

  char timebuf[64];
  if (std::strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", &tmbuf) ==
      0) {
    // Fallback if strftime fails
    std::snprintf(timebuf, sizeof(timebuf), "%lld", static_cast<long long>(t));
  }

  // Serialise output to avoid interleaving
  std::lock_guard<std::mutex> lk(outputMutex());

  const bool use_colors = colorsEnabled();
  const char *reset = use_colors ? "\x1b[0m" : "";
  const char *file_color = use_colors ? "\x1b[90m" : "";
  const char *lvl_color = use_colors ? levelColor(level) : "";

  const char *trimmed = trimPathToAxidevIo(file);

  // Header: timestamp.millis [LEVEL] file:line: (with coloring)
  std::fprintf(stderr, "[axidev-io] %s.%03d [", timebuf,
               static_cast<int>(ms.count()));
  if (use_colors)
    std::fputs(lvl_color, stderr);
  std::fprintf(stderr, "%s", levelToString(level));
  if (use_colors)
    std::fputs(reset, stderr);
  std::fprintf(stderr, "] ");
  if (use_colors)
    std::fputs(file_color, stderr);
  std::fprintf(stderr, "%s:%d: ", trimmed, line);
  if (use_colors)
    std::fputs(reset, stderr);

  // Body
  std::vfprintf(stderr, fmt, ap);

  // Newline and flush for immediacy (useful when used from tests / CI)
  std::fprintf(stderr, "\n");
  std::fflush(stderr);
}

/**
 * @brief Log a message with varargs.
 *
 * Convenience wrapper around `vlog` that accepts printf-style variadic
 * arguments. Automatically checks the log level before formatting.
 *
 * @param level Log level for the message.
 * @param file Source file name (typically `__FILE__`).
 * @param line Source line number (typically `__LINE__`).
 * @param fmt printf-style format string.
 */
inline void log(Level level, const char *file, int line, const char *fmt, ...) {
  if (!isEnabled(level))
    return;
  va_list ap;
  va_start(ap, fmt);
  vlog(level, file, line, fmt, ap);
  va_end(ap);
}

/**
 * @brief Convenience helper that returns whether debug logging is enabled.
 * @return true if Debug messages will be emitted.
 */
inline bool debugEnabled() { return isEnabled(Level::Debug); }

} // namespace log
} // namespace io
} // namespace axidev

/**
 * @defgroup LoggingMacros Helper logging macros
 * @brief Convenience macros that include file and line automatically.
 *
 * These macros wrap `::axidev::io::log::log` and automatically supply
 * `__FILE__` and `__LINE__`.
 * @{
 */
#define AXIDEV_IO_LOG_DEBUG(fmt, ...)                                          \
  ::axidev::io::log::log(::axidev::io::log::Level::Debug, __FILE__, __LINE__,  \
                         fmt, ##__VA_ARGS__)
#define AXIDEV_IO_LOG_INFO(fmt, ...)                                           \
  ::axidev::io::log::log(::axidev::io::log::Level::Info, __FILE__, __LINE__,   \
                         fmt, ##__VA_ARGS__)
#define AXIDEV_IO_LOG_WARN(fmt, ...)                                           \
  ::axidev::io::log::log(::axidev::io::log::Level::Warn, __FILE__, __LINE__,   \
                         fmt, ##__VA_ARGS__)
#define AXIDEV_IO_LOG_ERROR(fmt, ...)                                          \
  ::axidev::io::log::log(::axidev::io::log::Level::Error, __FILE__, __LINE__,  \
                         fmt, ##__VA_ARGS__)
/** @} */ /* end of LoggingMacros */

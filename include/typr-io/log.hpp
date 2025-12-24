#pragma once

// typr-io - log.hpp
// Lightweight, header-only logging utility used by typr-io backends.
//
// Usage:
//   #include <typr-io/log.hpp>
//   TYPR_IO_LOG_DEBUG("something happened: %d", value);
//   TYPR_IO_LOG_INFO("ready");
//
// Configuration via environment:
//   - TYPR_IO_LOG_LEVEL (preferred): one of "debug", "info", "warn", "error"
//   - If TYPR_IO_LOG_LEVEL is not set, legacy env var TYPR_OSK_DEBUG_BACKEND is
//     respected: unset -> debug enabled (default during testing), "0" -> debug
//     off, otherwise debug enabled.
//   - TYPR_IO_FORCE_COLORS: if set and non-empty, force ANSI colors on (useful
//     for CI or when stderr is not a TTY).
//   - TYPR_IO_NO_COLOR: if set and non-empty, disable ANSI colors (overrides
//     automatic detection).
//
// The header is intentionally small and portable (works in C++ and
// Objective-C++).

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

namespace typr {
namespace io {
namespace log {

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

// Parse runtime configuration to determine default log level. This prefers
// TYPR_IO_LOG_LEVEL if present; otherwise falls back to legacy
// TYPR_OSK_DEBUG_BACKEND behaviour (defaulting to Debug when unset for
// testing).
inline Level parseLevelFromEnv() {
  const char *lvlEnv = std::getenv("TYPR_IO_LOG_LEVEL");
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

  const char *legacy = std::getenv("TYPR_OSK_DEBUG_BACKEND");
  if (!legacy) {
    // Default to enabled for the time being while testing (preserve legacy
    // behaviour).
    return Level::Debug;
  }
  if (legacy[0] == '0')
    return Level::Info;
  return Level::Debug;
}

// Global log level (atomic to allow runtime changes).
inline std::atomic<Level> &globalLevel() {
  static std::atomic<Level> lvl(parseLevelFromEnv());
  return lvl;
}

inline void setLevel(Level l) { globalLevel().store(l); }
inline Level getLevel() { return globalLevel().load(); }

// Whether a message at `level` should be emitted under the current level.
// Lower enum value is more verbose (Debug = 0). Emit if message level is
// >= current minimum level.
inline bool isEnabled(Level level) {
  return static_cast<int>(level) >= static_cast<int>(getLevel());
}

// Internal: get a mutex for serialising output to stderr.
inline std::mutex &outputMutex() {
  static std::mutex m;
  return m;
}

// Internal: return ANSI color code for a level.
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

// Internal: whether to emit ANSI color codes. Can be forced by
// TYPR_IO_FORCE_COLORS or disabled by TYPR_IO_NO_COLOR. Otherwise colors are
// enabled when stderr is a TTY.
inline bool colorsEnabled() {
  const char *force = std::getenv("TYPR_IO_FORCE_COLORS");
  if (force && force[0] != '\0')
    return true;
  const char *no = std::getenv("TYPR_IO_NO_COLOR");
  if (no && no[0] != '\0')
    return false;
#if defined(_WIN32) || defined(_WIN64)
  return _isatty(_fileno(stderr));
#else
  return isatty(fileno(stderr));
#endif
}

// Internal: trim a file path so it starts at the last "typr-io" path
// component (e.g. "typr-io/..."). If not found, fall back to basename.
inline const char *trimPathToTyprIo(const char *path) {
  if (!path)
    return path;
  const char *needle = "typr-io";
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

// Internal: emit a formatted message using a va_list. Thread-safe and prints
// an ISO timestamp (local time) with millisecond precision.
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

  const char *trimmed = trimPathToTyprIo(file);

  // Header: timestamp.millis [LEVEL] file:line: (with coloring)
  std::fprintf(stderr, "[typr-io] %s.%03d [", timebuf,
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

inline void log(Level level, const char *file, int line, const char *fmt, ...) {
  if (!isEnabled(level))
    return;
  va_list ap;
  va_start(ap, fmt);
  vlog(level, file, line, fmt, ap);
  va_end(ap);
}

// Convenience helpers
inline bool debugEnabled() { return isEnabled(Level::Debug); }

} // namespace log
} // namespace io
} // namespace typr

// Helper macros (include file/line automatically)
#define TYPR_IO_LOG_DEBUG(fmt, ...)                                            \
  ::typr::io::log::log(::typr::io::log::Level::Debug, __FILE__, __LINE__, fmt, \
                       ##__VA_ARGS__)
#define TYPR_IO_LOG_INFO(fmt, ...)                                             \
  ::typr::io::log::log(::typr::io::log::Level::Info, __FILE__, __LINE__, fmt,  \
                       ##__VA_ARGS__)
#define TYPR_IO_LOG_WARN(fmt, ...)                                             \
  ::typr::io::log::log(::typr::io::log::Level::Warn, __FILE__, __LINE__, fmt,  \
                       ##__VA_ARGS__)
#define TYPR_IO_LOG_ERROR(fmt, ...)                                            \
  ::typr::io::log::log(::typr::io::log::Level::Error, __FILE__, __LINE__, fmt, \
                       ##__VA_ARGS__)

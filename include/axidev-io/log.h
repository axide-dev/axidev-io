#pragma once
#ifndef AXIDEV_IO_LOG_H
#define AXIDEV_IO_LOG_H

#include <axidev-io/c_api.h>

#define AXIDEV_IO_LOG_DEBUG(fmt, ...)                                          \
  axidev_io_log_message(AXIDEV_IO_LOG_LEVEL_DEBUG, __FILE__, __LINE__, fmt,    \
                        ##__VA_ARGS__)
#define AXIDEV_IO_LOG_INFO(fmt, ...)                                           \
  axidev_io_log_message(AXIDEV_IO_LOG_LEVEL_INFO, __FILE__, __LINE__, fmt,     \
                        ##__VA_ARGS__)
#define AXIDEV_IO_LOG_WARN(fmt, ...)                                           \
  axidev_io_log_message(AXIDEV_IO_LOG_LEVEL_WARN, __FILE__, __LINE__, fmt,     \
                        ##__VA_ARGS__)
#define AXIDEV_IO_LOG_ERROR(fmt, ...)                                          \
  axidev_io_log_message(AXIDEV_IO_LOG_LEVEL_ERROR, __FILE__, __LINE__, fmt,    \
                        ##__VA_ARGS__)

#endif

#pragma once
#ifndef AXIDEV_IO_KEYBOARD_LINUX_LAYOUT_INTERNAL_H
#define AXIDEV_IO_KEYBOARD_LINUX_LAYOUT_INTERNAL_H

#if defined(__linux__)

#include <stdbool.h>

typedef struct axidev_io_xkb_rule_names_strings {
  char rules[64];
  char model[64];
  char layout[64];
  char variant[64];
  char options[128];
  bool has_any;
} axidev_io_xkb_rule_names_strings;

axidev_io_xkb_rule_names_strings axidev_io_detect_xkb_rule_names(void);

#endif

#endif

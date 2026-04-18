#pragma once
#ifndef AXIDEV_IO_INTERNAL_UTF_H
#define AXIDEV_IO_INTERNAL_UTF_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

bool axidev_io_utf8_decode_one(const char **cursor, uint32_t *out_codepoint);
bool axidev_io_utf8_append(uint32_t codepoint, char **buffer);
bool axidev_io_utf8_is_ascii_alpha(uint32_t codepoint);
uint32_t axidev_io_utf8_to_lower_ascii(uint32_t codepoint);

#endif

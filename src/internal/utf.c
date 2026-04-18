#include "utf.h"

#include <string.h>

#include <stb/stb_ds.h>

bool axidev_io_utf8_decode_one(const char **cursor, uint32_t *out_codepoint) {
  const unsigned char *ptr;
  uint32_t codepoint;

  if (cursor == NULL || *cursor == NULL || out_codepoint == NULL) {
    return false;
  }

  ptr = (const unsigned char *)*cursor;
  if (*ptr == '\0') {
    return false;
  }

  if ((ptr[0] & 0x80u) == 0u) {
    codepoint = ptr[0];
    *cursor += 1;
    *out_codepoint = codepoint;
    return true;
  }

  if ((ptr[0] & 0xE0u) == 0xC0u && ptr[1] != 0) {
    codepoint = ((uint32_t)(ptr[0] & 0x1Fu) << 6) | (uint32_t)(ptr[1] & 0x3Fu);
    *cursor += 2;
    *out_codepoint = codepoint;
    return true;
  }

  if ((ptr[0] & 0xF0u) == 0xE0u && ptr[1] != 0 && ptr[2] != 0) {
    codepoint = ((uint32_t)(ptr[0] & 0x0Fu) << 12) |
                ((uint32_t)(ptr[1] & 0x3Fu) << 6) | (uint32_t)(ptr[2] & 0x3Fu);
    *cursor += 3;
    *out_codepoint = codepoint;
    return true;
  }

  if ((ptr[0] & 0xF8u) == 0xF0u && ptr[1] != 0 && ptr[2] != 0 && ptr[3] != 0) {
    codepoint = ((uint32_t)(ptr[0] & 0x07u) << 18) |
                ((uint32_t)(ptr[1] & 0x3Fu) << 12) |
                ((uint32_t)(ptr[2] & 0x3Fu) << 6) | (uint32_t)(ptr[3] & 0x3Fu);
    *cursor += 4;
    *out_codepoint = codepoint;
    return true;
  }

  *out_codepoint = ptr[0];
  *cursor += 1;
  return false;
}

bool axidev_io_utf8_append(uint32_t codepoint, char **buffer) {
  char bytes[4];
  char *output;
  size_t count = 0;
  ptrdiff_t old_length;

  if (buffer == NULL) {
    return false;
  }

  if (codepoint <= 0x7Fu) {
    bytes[0] = (char)codepoint;
    count = 1;
  } else if (codepoint <= 0x7FFu) {
    bytes[0] = (char)(0xC0u | ((codepoint >> 6) & 0x1Fu));
    bytes[1] = (char)(0x80u | (codepoint & 0x3Fu));
    count = 2;
  } else if (codepoint <= 0xFFFFu) {
    bytes[0] = (char)(0xE0u | ((codepoint >> 12) & 0x0Fu));
    bytes[1] = (char)(0x80u | ((codepoint >> 6) & 0x3Fu));
    bytes[2] = (char)(0x80u | (codepoint & 0x3Fu));
    count = 3;
  } else if (codepoint <= 0x10FFFFu) {
    bytes[0] = (char)(0xF0u | ((codepoint >> 18) & 0x07u));
    bytes[1] = (char)(0x80u | ((codepoint >> 12) & 0x3Fu));
    bytes[2] = (char)(0x80u | ((codepoint >> 6) & 0x3Fu));
    bytes[3] = (char)(0x80u | (codepoint & 0x3Fu));
    count = 4;
  } else {
    return false;
  }

  old_length = arrlen(*buffer);
  arrsetlen(*buffer, old_length + (ptrdiff_t)count);
  output = *buffer;
  if (output == NULL) {
    return false;
  }
  memcpy(output + old_length, bytes, count);
  return true;
}

bool axidev_io_utf8_is_ascii_alpha(uint32_t codepoint) {
  return (codepoint >= 'A' && codepoint <= 'Z') ||
         (codepoint >= 'a' && codepoint <= 'z');
}

uint32_t axidev_io_utf8_to_lower_ascii(uint32_t codepoint) {
  if (codepoint >= 'A' && codepoint <= 'Z') {
    return codepoint + ('a' - 'A');
  }
  return codepoint;
}

#if defined(__linux__)

#include "linux_layout_internal.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void axidev_io_trim_in_place(char *text) {
  size_t start = 0;
  size_t end;
  size_t length;

  if (text == NULL) {
    return;
  }

  length = strlen(text);
  while (start < length && (text[start] == ' ' || text[start] == '\t' ||
                            text[start] == '\r' || text[start] == '\n')) {
    ++start;
  }

  end = length;
  while (end > start && (text[end - 1] == ' ' || text[end - 1] == '\t' ||
                         text[end - 1] == '\r' || text[end - 1] == '\n')) {
    --end;
  }

  if (start > 0) {
    memmove(text, text + start, end - start);
  }
  text[end - start] = '\0';
}

static void axidev_io_strip_quotes(char *text) {
  size_t length;
  if (text == NULL) {
    return;
  }
  length = strlen(text);
  if (length >= 2 && ((text[0] == '"' && text[length - 1] == '"') ||
                      (text[0] == '\'' && text[length - 1] == '\''))) {
    memmove(text, text + 1, length - 2);
    text[length - 2] = '\0';
  }
}

static void axidev_io_uppercase_in_place(char *text) {
  size_t i;
  if (text == NULL) {
    return;
  }
  for (i = 0; text[i] != '\0'; ++i) {
    text[i] = (char)toupper((unsigned char)text[i]);
  }
}

static void axidev_io_copy_if_empty(char *destination, size_t destination_size,
                                    const char *value, bool *has_any) {
  if (destination[0] == '\0' && value != NULL && value[0] != '\0') {
    snprintf(destination, destination_size, "%s", value);
    if (has_any != NULL) {
      *has_any = true;
    }
  }
}

axidev_io_xkb_rule_names_strings axidev_io_detect_xkb_rule_names(void) {
  axidev_io_xkb_rule_names_strings result;
  FILE *keyboard_file;
  char line[256];

  memset(&result, 0, sizeof(result));

  if (getenv("XKB_DEFAULT_RULES") != NULL) {
    snprintf(result.rules, sizeof(result.rules), "%s",
             getenv("XKB_DEFAULT_RULES"));
  }
  if (getenv("XKB_DEFAULT_MODEL") != NULL) {
    snprintf(result.model, sizeof(result.model), "%s",
             getenv("XKB_DEFAULT_MODEL"));
  }
  if (getenv("XKB_DEFAULT_LAYOUT") != NULL) {
    snprintf(result.layout, sizeof(result.layout), "%s",
             getenv("XKB_DEFAULT_LAYOUT"));
  }
  if (getenv("XKB_DEFAULT_VARIANT") != NULL) {
    snprintf(result.variant, sizeof(result.variant), "%s",
             getenv("XKB_DEFAULT_VARIANT"));
  }
  if (getenv("XKB_DEFAULT_OPTIONS") != NULL) {
    snprintf(result.options, sizeof(result.options), "%s",
             getenv("XKB_DEFAULT_OPTIONS"));
  }

  axidev_io_trim_in_place(result.rules);
  axidev_io_trim_in_place(result.model);
  axidev_io_trim_in_place(result.layout);
  axidev_io_trim_in_place(result.variant);
  axidev_io_trim_in_place(result.options);
  result.has_any = result.rules[0] != '\0' || result.model[0] != '\0' ||
                   result.layout[0] != '\0' || result.variant[0] != '\0' ||
                   result.options[0] != '\0';

  keyboard_file = fopen("/etc/default/keyboard", "r");
  if (keyboard_file != NULL) {
    while (fgets(line, sizeof(line), keyboard_file) != NULL) {
      char key[64];
      char value[192];
      char *equals;
      char *comment;

      comment = strchr(line, '#');
      if (comment != NULL) {
        *comment = '\0';
      }
      axidev_io_trim_in_place(line);
      if (line[0] == '\0') {
        continue;
      }

      equals = strchr(line, '=');
      if (equals == NULL) {
        continue;
      }

      snprintf(key, sizeof(key), "%.*s", (int)(equals - line), line);
      snprintf(value, sizeof(value), "%s", equals + 1);
      axidev_io_trim_in_place(key);
      axidev_io_trim_in_place(value);
      axidev_io_strip_quotes(value);
      axidev_io_trim_in_place(value);
      axidev_io_uppercase_in_place(key);
      if (value[0] == '\0') {
        continue;
      }

      if (strcmp(key, "XKBRULES") == 0 ||
          strcmp(key, "XKB_DEFAULT_RULES") == 0) {
        axidev_io_copy_if_empty(result.rules, sizeof(result.rules), value,
                                &result.has_any);
      } else if (strcmp(key, "XKBMODEL") == 0 ||
                 strcmp(key, "XKB_DEFAULT_MODEL") == 0) {
        axidev_io_copy_if_empty(result.model, sizeof(result.model), value,
                                &result.has_any);
      } else if (strcmp(key, "XKBLAYOUT") == 0 ||
                 strcmp(key, "XKB_DEFAULT_LAYOUT") == 0) {
        axidev_io_copy_if_empty(result.layout, sizeof(result.layout), value,
                                &result.has_any);
      } else if (strcmp(key, "XKBVARIANT") == 0 ||
                 strcmp(key, "XKB_DEFAULT_VARIANT") == 0) {
        axidev_io_copy_if_empty(result.variant, sizeof(result.variant), value,
                                &result.has_any);
      } else if (strcmp(key, "XKBOPTIONS") == 0 ||
                 strcmp(key, "XKB_DEFAULT_OPTIONS") == 0) {
        axidev_io_copy_if_empty(result.options, sizeof(result.options), value,
                                &result.has_any);
      }
    }
    fclose(keyboard_file);
  }

  if (result.layout[0] == '\0') {
    const char *locale = getenv("LC_ALL");
    char buffer[64];
    char *underscore;
    char *dot;
    char *at;

    if (locale == NULL || locale[0] == '\0') {
      locale = getenv("LC_MESSAGES");
    }
    if (locale == NULL || locale[0] == '\0') {
      locale = getenv("LANG");
    }

    if (locale != NULL && locale[0] != '\0') {
      snprintf(buffer, sizeof(buffer), "%s", locale);
      dot = strchr(buffer, '.');
      if (dot != NULL) {
        *dot = '\0';
      }
      at = strchr(buffer, '@');
      if (at != NULL) {
        *at = '\0';
      }

      underscore = strchr(buffer, '_');
      if (underscore != NULL) {
        *underscore = '\0';
        ++underscore;
        axidev_io_uppercase_in_place(underscore);
      }

      if (strcmp(buffer, "en") == 0) {
        if (underscore != NULL &&
            (strcmp(underscore, "GB") == 0 || strcmp(underscore, "UK") == 0)) {
          snprintf(result.layout, sizeof(result.layout), "gb");
        } else {
          snprintf(result.layout, sizeof(result.layout), "us");
        }
      } else if (strcmp(buffer, "pt") == 0 && underscore != NULL &&
                 strcmp(underscore, "BR") == 0) {
        snprintf(result.layout, sizeof(result.layout), "br");
      } else if (strcmp(buffer, "da") == 0) {
        snprintf(result.layout, sizeof(result.layout), "dk");
      } else if (strcmp(buffer, "sv") == 0) {
        snprintf(result.layout, sizeof(result.layout), "se");
      } else if (buffer[0] != '\0') {
        snprintf(result.layout, sizeof(result.layout), "%s", buffer);
      }

      if (result.layout[0] != '\0') {
        result.has_any = true;
      }
    }
  }

  return result;
}

#endif

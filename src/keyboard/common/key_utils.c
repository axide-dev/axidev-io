#include "key_utils_internal.h"

#include <axidev-io/c_api.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stb/stb_ds.h>

#include "../../internal/thread.h"

typedef struct axidev_io_key_name_pair {
  axidev_io_keyboard_key_t key;
  const char *name;
} axidev_io_key_name_pair;

typedef struct axidev_io_string_key_entry {
  char *key;
  axidev_io_keyboard_key_t value;
} axidev_io_string_key_entry;

static axidev_io_once g_reverse_map_once = AXIDEV_IO_ONCE_INIT;
static axidev_io_string_key_entry *g_reverse_map;

static char *axidev_io_to_lower_copy(const char *input) {
  size_t i;
  size_t length;
  char *output;

  if (input == NULL) {
    return NULL;
  }

  length = strlen(input);
  output = (char *)malloc(length + 1u);
  if (output == NULL) {
    return NULL;
  }

  for (i = 0; i < length; ++i) {
    output[i] = (char)tolower((unsigned char)input[i]);
  }
  output[length] = '\0';
  return output;
}

static char *axidev_io_escape_for_log(const char *input) {
  char *output = NULL;
  size_t i;

  if (input == NULL) {
    return axidev_io_duplicate_string("");
  }

  for (i = 0; input[i] != '\0'; ++i) {
    unsigned char c = (unsigned char)input[i];
    if (c == '\n') {
      arrput(output, '\\');
      arrput(output, 'n');
    } else if (c == '\r') {
      arrput(output, '\\');
      arrput(output, 'r');
    } else if (c == '\t') {
      arrput(output, '\\');
      arrput(output, 't');
    } else if (isprint(c)) {
      arrput(output, (char)c);
    } else {
      char buffer[8];
      snprintf(buffer, sizeof(buffer), "\\x%02X", c);
      for (size_t j = 0; buffer[j] != '\0'; ++j) {
        arrput(output, buffer[j]);
      }
    }
  }

  arrput(output, '\0');
  return output;
}

static const axidev_io_key_name_pair *
axidev_io_key_string_pairs(size_t *count) {
  static const axidev_io_key_name_pair pairs[] = {
      {AXIDEV_IO_KEY_UNKNOWN, "Unknown"},
      {AXIDEV_IO_KEY_A, "A"},
      {AXIDEV_IO_KEY_B, "B"},
      {AXIDEV_IO_KEY_C, "C"},
      {AXIDEV_IO_KEY_D, "D"},
      {AXIDEV_IO_KEY_E, "E"},
      {AXIDEV_IO_KEY_F, "F"},
      {AXIDEV_IO_KEY_G, "G"},
      {AXIDEV_IO_KEY_H, "H"},
      {AXIDEV_IO_KEY_I, "I"},
      {AXIDEV_IO_KEY_J, "J"},
      {AXIDEV_IO_KEY_K, "K"},
      {AXIDEV_IO_KEY_L, "L"},
      {AXIDEV_IO_KEY_M, "M"},
      {AXIDEV_IO_KEY_N, "N"},
      {AXIDEV_IO_KEY_O, "O"},
      {AXIDEV_IO_KEY_P, "P"},
      {AXIDEV_IO_KEY_Q, "Q"},
      {AXIDEV_IO_KEY_R, "R"},
      {AXIDEV_IO_KEY_S, "S"},
      {AXIDEV_IO_KEY_T, "T"},
      {AXIDEV_IO_KEY_U, "U"},
      {AXIDEV_IO_KEY_V, "V"},
      {AXIDEV_IO_KEY_W, "W"},
      {AXIDEV_IO_KEY_X, "X"},
      {AXIDEV_IO_KEY_Y, "Y"},
      {AXIDEV_IO_KEY_Z, "Z"},
      {AXIDEV_IO_KEY_NUM0, "0"},
      {AXIDEV_IO_KEY_NUM1, "1"},
      {AXIDEV_IO_KEY_NUM2, "2"},
      {AXIDEV_IO_KEY_NUM3, "3"},
      {AXIDEV_IO_KEY_NUM4, "4"},
      {AXIDEV_IO_KEY_NUM5, "5"},
      {AXIDEV_IO_KEY_NUM6, "6"},
      {AXIDEV_IO_KEY_NUM7, "7"},
      {AXIDEV_IO_KEY_NUM8, "8"},
      {AXIDEV_IO_KEY_NUM9, "9"},
      {AXIDEV_IO_KEY_F1, "F1"},
      {AXIDEV_IO_KEY_F2, "F2"},
      {AXIDEV_IO_KEY_F3, "F3"},
      {AXIDEV_IO_KEY_F4, "F4"},
      {AXIDEV_IO_KEY_F5, "F5"},
      {AXIDEV_IO_KEY_F6, "F6"},
      {AXIDEV_IO_KEY_F7, "F7"},
      {AXIDEV_IO_KEY_F8, "F8"},
      {AXIDEV_IO_KEY_F9, "F9"},
      {AXIDEV_IO_KEY_F10, "F10"},
      {AXIDEV_IO_KEY_F11, "F11"},
      {AXIDEV_IO_KEY_F12, "F12"},
      {AXIDEV_IO_KEY_F13, "F13"},
      {AXIDEV_IO_KEY_F14, "F14"},
      {AXIDEV_IO_KEY_F15, "F15"},
      {AXIDEV_IO_KEY_F16, "F16"},
      {AXIDEV_IO_KEY_F17, "F17"},
      {AXIDEV_IO_KEY_F18, "F18"},
      {AXIDEV_IO_KEY_F19, "F19"},
      {AXIDEV_IO_KEY_F20, "F20"},
      {AXIDEV_IO_KEY_ENTER, "Enter"},
      {AXIDEV_IO_KEY_ESCAPE, "Escape"},
      {AXIDEV_IO_KEY_BACKSPACE, "Backspace"},
      {AXIDEV_IO_KEY_TAB, "Tab"},
      {AXIDEV_IO_KEY_SPACE, "Space"},
      {AXIDEV_IO_KEY_LEFT, "Left"},
      {AXIDEV_IO_KEY_RIGHT, "Right"},
      {AXIDEV_IO_KEY_UP, "Up"},
      {AXIDEV_IO_KEY_DOWN, "Down"},
      {AXIDEV_IO_KEY_HOME, "Home"},
      {AXIDEV_IO_KEY_END, "End"},
      {AXIDEV_IO_KEY_PAGE_UP, "PageUp"},
      {AXIDEV_IO_KEY_PAGE_DOWN, "PageDown"},
      {AXIDEV_IO_KEY_DELETE, "Delete"},
      {AXIDEV_IO_KEY_INSERT, "Insert"},
      {AXIDEV_IO_KEY_PRINT_SCREEN, "PrintScreen"},
      {AXIDEV_IO_KEY_SCROLL_LOCK, "ScrollLock"},
      {AXIDEV_IO_KEY_PAUSE, "Pause"},
      {AXIDEV_IO_KEY_NUMPAD_DIVIDE, "NumpadDivide"},
      {AXIDEV_IO_KEY_NUMPAD_MULTIPLY, "NumpadMultiply"},
      {AXIDEV_IO_KEY_NUMPAD_MINUS, "NumpadMinus"},
      {AXIDEV_IO_KEY_NUMPAD_PLUS, "NumpadPlus"},
      {AXIDEV_IO_KEY_NUMPAD_ENTER, "NumpadEnter"},
      {AXIDEV_IO_KEY_NUMPAD_DECIMAL, "NumpadDecimal"},
      {AXIDEV_IO_KEY_NUMPAD0, "Numpad0"},
      {AXIDEV_IO_KEY_NUMPAD1, "Numpad1"},
      {AXIDEV_IO_KEY_NUMPAD2, "Numpad2"},
      {AXIDEV_IO_KEY_NUMPAD3, "Numpad3"},
      {AXIDEV_IO_KEY_NUMPAD4, "Numpad4"},
      {AXIDEV_IO_KEY_NUMPAD5, "Numpad5"},
      {AXIDEV_IO_KEY_NUMPAD6, "Numpad6"},
      {AXIDEV_IO_KEY_NUMPAD7, "Numpad7"},
      {AXIDEV_IO_KEY_NUMPAD8, "Numpad8"},
      {AXIDEV_IO_KEY_NUMPAD9, "Numpad9"},
      {AXIDEV_IO_KEY_SHIFT_LEFT, "ShiftLeft"},
      {AXIDEV_IO_KEY_SHIFT_RIGHT, "ShiftRight"},
      {AXIDEV_IO_KEY_CTRL_LEFT, "CtrlLeft"},
      {AXIDEV_IO_KEY_CTRL_RIGHT, "CtrlRight"},
      {AXIDEV_IO_KEY_ALT_LEFT, "AltLeft"},
      {AXIDEV_IO_KEY_ALT_RIGHT, "AltRight"},
      {AXIDEV_IO_KEY_SUPER_LEFT, "SuperLeft"},
      {AXIDEV_IO_KEY_SUPER_RIGHT, "SuperRight"},
      {AXIDEV_IO_KEY_CAPS_LOCK, "CapsLock"},
      {AXIDEV_IO_KEY_NUM_LOCK, "NumLock"},
      {AXIDEV_IO_KEY_HELP, "Help"},
      {AXIDEV_IO_KEY_MENU, "Menu"},
      {AXIDEV_IO_KEY_POWER, "Power"},
      {AXIDEV_IO_KEY_SLEEP, "Sleep"},
      {AXIDEV_IO_KEY_WAKE, "Wake"},
      {AXIDEV_IO_KEY_MUTE, "Mute"},
      {AXIDEV_IO_KEY_VOLUME_DOWN, "VolumeDown"},
      {AXIDEV_IO_KEY_VOLUME_UP, "VolumeUp"},
      {AXIDEV_IO_KEY_MEDIA_PLAY_PAUSE, "MediaPlayPause"},
      {AXIDEV_IO_KEY_MEDIA_STOP, "MediaStop"},
      {AXIDEV_IO_KEY_MEDIA_NEXT, "MediaNext"},
      {AXIDEV_IO_KEY_MEDIA_PREVIOUS, "MediaPrevious"},
      {AXIDEV_IO_KEY_BRIGHTNESS_DOWN, "BrightnessDown"},
      {AXIDEV_IO_KEY_BRIGHTNESS_UP, "BrightnessUp"},
      {AXIDEV_IO_KEY_EJECT, "Eject"},
      {AXIDEV_IO_KEY_GRAVE, "`"},
      {AXIDEV_IO_KEY_MINUS, "-"},
      {AXIDEV_IO_KEY_EQUAL, "="},
      {AXIDEV_IO_KEY_LEFT_BRACKET, "["},
      {AXIDEV_IO_KEY_RIGHT_BRACKET, "]"},
      {AXIDEV_IO_KEY_BACKSLASH, "\\"},
      {AXIDEV_IO_KEY_SEMICOLON, ";"},
      {AXIDEV_IO_KEY_APOSTROPHE, "'"},
      {AXIDEV_IO_KEY_COMMA, ","},
      {AXIDEV_IO_KEY_PERIOD, "."},
      {AXIDEV_IO_KEY_SLASH, "/"},
      {AXIDEV_IO_KEY_AT, "At"},
      {AXIDEV_IO_KEY_HASHTAG, "Hashtag"},
      {AXIDEV_IO_KEY_EXCLAMATION, "Exclamation"},
      {AXIDEV_IO_KEY_DOLLAR, "Dollar"},
      {AXIDEV_IO_KEY_PERCENT, "Percent"},
      {AXIDEV_IO_KEY_CARET, "Caret"},
      {AXIDEV_IO_KEY_AMPERSAND, "Ampersand"},
      {AXIDEV_IO_KEY_ASTERISK, "Asterisk"},
      {AXIDEV_IO_KEY_LEFT_PAREN, "LeftParen"},
      {AXIDEV_IO_KEY_RIGHT_PAREN, "RightParen"},
      {AXIDEV_IO_KEY_UNDERSCORE, "Underscore"},
      {AXIDEV_IO_KEY_PLUS, "Plus"},
      {AXIDEV_IO_KEY_COLON, "Colon"},
      {AXIDEV_IO_KEY_QUOTE, "Quote"},
      {AXIDEV_IO_KEY_QUESTION_MARK, "QuestionMark"},
      {AXIDEV_IO_KEY_BAR, "Bar"},
      {AXIDEV_IO_KEY_LESS_THAN, "LessThan"},
      {AXIDEV_IO_KEY_GREATER_THAN, "GreaterThan"},
      {AXIDEV_IO_KEY_ASCII_NUL, "NUL"},
      {AXIDEV_IO_KEY_ASCII_SOH, "SOH"},
      {AXIDEV_IO_KEY_ASCII_STX, "STX"},
      {AXIDEV_IO_KEY_ASCII_ETX, "ETX"},
      {AXIDEV_IO_KEY_ASCII_EOT, "EOT"},
      {AXIDEV_IO_KEY_ASCII_ENQ, "ENQ"},
      {AXIDEV_IO_KEY_ASCII_ACK, "ACK"},
      {AXIDEV_IO_KEY_ASCII_BELL, "Bell"},
      {AXIDEV_IO_KEY_ASCII_VT, "VT"},
      {AXIDEV_IO_KEY_ASCII_FF, "FF"},
      {AXIDEV_IO_KEY_ASCII_SO, "SO"},
      {AXIDEV_IO_KEY_ASCII_SI, "SI"},
      {AXIDEV_IO_KEY_ASCII_DLE, "DLE"},
      {AXIDEV_IO_KEY_ASCII_DC1, "DC1"},
      {AXIDEV_IO_KEY_ASCII_DC2, "DC2"},
      {AXIDEV_IO_KEY_ASCII_DC3, "DC3"},
      {AXIDEV_IO_KEY_ASCII_DC4, "DC4"},
      {AXIDEV_IO_KEY_ASCII_NAK, "NAK"},
      {AXIDEV_IO_KEY_ASCII_SYN, "SYN"},
      {AXIDEV_IO_KEY_ASCII_ETB, "ETB"},
      {AXIDEV_IO_KEY_ASCII_CAN, "CAN"},
      {AXIDEV_IO_KEY_ASCII_EM, "EM"},
      {AXIDEV_IO_KEY_ASCII_SUB, "SUB"},
      {AXIDEV_IO_KEY_ASCII_FS, "FS"},
      {AXIDEV_IO_KEY_ASCII_GS, "GS"},
      {AXIDEV_IO_KEY_ASCII_RS, "RS"},
      {AXIDEV_IO_KEY_ASCII_US, "US"},
      {AXIDEV_IO_KEY_ASCII_DEL, "DEL"},
      {AXIDEV_IO_KEY_NUMPAD_EQUAL, "NumpadEqual"},
      {AXIDEV_IO_KEY_DEGREE, "Degree"},
      {AXIDEV_IO_KEY_STERLING, "Sterling"},
      {AXIDEV_IO_KEY_MU, "Mu"},
      {AXIDEV_IO_KEY_PLUS_MINUS, "PlusMinus"},
      {AXIDEV_IO_KEY_DEAD_CIRCUMFLEX, "DeadCircumflex"},
      {AXIDEV_IO_KEY_DEAD_DIAERESIS, "DeadDiaeresis"},
      {AXIDEV_IO_KEY_SECTION, "Section"},
      {AXIDEV_IO_KEY_CANCEL, "Cancel"},
      {AXIDEV_IO_KEY_REDO, "Redo"},
      {AXIDEV_IO_KEY_UNDO, "Undo"},
      {AXIDEV_IO_KEY_FIND, "Find"},
      {AXIDEV_IO_KEY_HANGUL, "Hangul"},
      {AXIDEV_IO_KEY_HANGUL_HANJA, "HangulHanja"},
      {AXIDEV_IO_KEY_KATAKANA, "Katakana"},
      {AXIDEV_IO_KEY_HIRAGANA, "Hiragana"},
      {AXIDEV_IO_KEY_HENKAN, "Henkan"},
      {AXIDEV_IO_KEY_MUHENKAN, "Muhenkan"},
      {AXIDEV_IO_KEY_OE, "OE"},
      {AXIDEV_IO_KEY_oe, "oe"},
      {AXIDEV_IO_KEY_SUN_PROPS, "SunProps"},
      {AXIDEV_IO_KEY_SUN_FRONT, "SunFront"},
      {AXIDEV_IO_KEY_COPY, "Copy"},
      {AXIDEV_IO_KEY_OPEN, "Open"},
      {AXIDEV_IO_KEY_PASTE, "Paste"},
      {AXIDEV_IO_KEY_CUT, "Cut"},
      {AXIDEV_IO_KEY_CALCULATOR, "Calculator"},
      {AXIDEV_IO_KEY_EXPLORER, "Explorer"},
      {AXIDEV_IO_KEY_PHONE, "Phone"},
      {AXIDEV_IO_KEY_WEB_CAM, "WebCam"},
      {AXIDEV_IO_KEY_AUDIO_RECORD, "AudioRecord"},
      {AXIDEV_IO_KEY_AUDIO_REWIND, "AudioRewind"},
      {AXIDEV_IO_KEY_AUDIO_PRESET, "AudioPreset"},
      {AXIDEV_IO_KEY_MESSENGER, "Messenger"},
      {AXIDEV_IO_KEY_SEARCH, "Search"},
      {AXIDEV_IO_KEY_GO, "Go"},
      {AXIDEV_IO_KEY_FINANCE, "Finance"},
      {AXIDEV_IO_KEY_GAME, "Game"},
      {AXIDEV_IO_KEY_SHOP, "Shop"},
      {AXIDEV_IO_KEY_HOME_PAGE, "HomePage"},
      {AXIDEV_IO_KEY_RELOAD, "Reload"},
      {AXIDEV_IO_KEY_CLOSE, "Close"},
      {AXIDEV_IO_KEY_SEND, "Send"},
      {AXIDEV_IO_KEY_XFER, "Xfer"},
      {AXIDEV_IO_KEY_LAUNCH_A, "LaunchA"},
      {AXIDEV_IO_KEY_LAUNCH_B, "LaunchB"},
      {AXIDEV_IO_KEY_LAUNCH1, "Launch1"},
      {AXIDEV_IO_KEY_LAUNCH2, "Launch2"},
      {AXIDEV_IO_KEY_LAUNCH3, "Launch3"},
      {AXIDEV_IO_KEY_LAUNCH4, "Launch4"},
      {AXIDEV_IO_KEY_LAUNCH5, "Launch5"},
      {AXIDEV_IO_KEY_LAUNCH6, "Launch6"},
      {AXIDEV_IO_KEY_LAUNCH7, "Launch7"},
      {AXIDEV_IO_KEY_LAUNCH8, "Launch8"},
      {AXIDEV_IO_KEY_LAUNCH9, "Launch9"},
      {AXIDEV_IO_KEY_TOUCHPAD_TOGGLE, "TouchpadToggle"},
      {AXIDEV_IO_KEY_TOUCHPAD_ON, "TouchpadOn"},
      {AXIDEV_IO_KEY_TOUCHPAD_OFF, "TouchpadOff"},
      {AXIDEV_IO_KEY_KBD_LIGHT_ON_OFF, "KbdLightOnOff"},
      {AXIDEV_IO_KEY_KBD_BRIGHTNESS_DOWN, "KbdBrightnessDown"},
      {AXIDEV_IO_KEY_KBD_BRIGHTNESS_UP, "KbdBrightnessUp"},
      {AXIDEV_IO_KEY_MAIL, "Mail"},
      {AXIDEV_IO_KEY_MAIL_FORWARD, "MailForward"},
      {AXIDEV_IO_KEY_SAVE, "Save"},
      {AXIDEV_IO_KEY_DOCUMENTS, "Documents"},
      {AXIDEV_IO_KEY_BATTERY, "Battery"},
      {AXIDEV_IO_KEY_BLUETOOTH, "Bluetooth"},
      {AXIDEV_IO_KEY_WLAN, "WLAN"},
      {AXIDEV_IO_KEY_UWB, "UWB"},
      {AXIDEV_IO_KEY_NEXT_VMODE, "Next_VMode"},
      {AXIDEV_IO_KEY_PREV_VMODE, "Prev_VMode"},
      {AXIDEV_IO_KEY_MON_BRIGHTNESS_CYCLE, "MonBrightnessCycle"},
      {AXIDEV_IO_KEY_BRIGHTNESS_AUTO, "BrightnessAuto"},
      {AXIDEV_IO_KEY_DISPLAY_OFF, "DisplayOff"},
      {AXIDEV_IO_KEY_WWAN, "WWAN"},
      {AXIDEV_IO_KEY_RF_KILL, "RFKill"}};

  if (count != NULL) {
    *count = sizeof(pairs) / sizeof(pairs[0]);
  }
  return pairs;
}

static void axidev_io_seed_alias(const char *key,
                                 axidev_io_keyboard_key_t value) {
  shput(g_reverse_map, key, value);
}

static void axidev_io_init_reverse_map(void) {
  size_t count = 0;
  const axidev_io_key_name_pair *pairs = axidev_io_key_string_pairs(&count);
  size_t i;

  sh_new_strdup(g_reverse_map);

  axidev_io_seed_alias("esc", AXIDEV_IO_KEY_ESCAPE);
  axidev_io_seed_alias("return", AXIDEV_IO_KEY_ENTER);
  axidev_io_seed_alias("spacebar", AXIDEV_IO_KEY_SPACE);
  axidev_io_seed_alias("space", AXIDEV_IO_KEY_SPACE);
  axidev_io_seed_alias("ctrl", AXIDEV_IO_KEY_CTRL_LEFT);
  axidev_io_seed_alias("control", AXIDEV_IO_KEY_CTRL_LEFT);
  axidev_io_seed_alias("shift", AXIDEV_IO_KEY_SHIFT_LEFT);
  axidev_io_seed_alias("alt", AXIDEV_IO_KEY_ALT_LEFT);
  axidev_io_seed_alias("super", AXIDEV_IO_KEY_SUPER_LEFT);
  axidev_io_seed_alias("meta", AXIDEV_IO_KEY_SUPER_LEFT);
  axidev_io_seed_alias("win", AXIDEV_IO_KEY_SUPER_LEFT);
  axidev_io_seed_alias("num0", AXIDEV_IO_KEY_NUM0);
  axidev_io_seed_alias("num1", AXIDEV_IO_KEY_NUM1);
  axidev_io_seed_alias("num2", AXIDEV_IO_KEY_NUM2);
  axidev_io_seed_alias("num3", AXIDEV_IO_KEY_NUM3);
  axidev_io_seed_alias("num4", AXIDEV_IO_KEY_NUM4);
  axidev_io_seed_alias("num5", AXIDEV_IO_KEY_NUM5);
  axidev_io_seed_alias("num6", AXIDEV_IO_KEY_NUM6);
  axidev_io_seed_alias("num7", AXIDEV_IO_KEY_NUM7);
  axidev_io_seed_alias("num8", AXIDEV_IO_KEY_NUM8);
  axidev_io_seed_alias("num9", AXIDEV_IO_KEY_NUM9);
  axidev_io_seed_alias("dash", AXIDEV_IO_KEY_MINUS);
  axidev_io_seed_alias("hyphen", AXIDEV_IO_KEY_MINUS);
  axidev_io_seed_alias("minus", AXIDEV_IO_KEY_MINUS);
  axidev_io_seed_alias("grave", AXIDEV_IO_KEY_GRAVE);
  axidev_io_seed_alias("backslash", AXIDEV_IO_KEY_BACKSLASH);
  axidev_io_seed_alias("semicolon", AXIDEV_IO_KEY_SEMICOLON);
  axidev_io_seed_alias("apostrophe", AXIDEV_IO_KEY_APOSTROPHE);
  axidev_io_seed_alias("comma", AXIDEV_IO_KEY_COMMA);
  axidev_io_seed_alias("period", AXIDEV_IO_KEY_PERIOD);
  axidev_io_seed_alias("dot", AXIDEV_IO_KEY_PERIOD);
  axidev_io_seed_alias("slash", AXIDEV_IO_KEY_SLASH);
  axidev_io_seed_alias("bracketleft", AXIDEV_IO_KEY_LEFT_BRACKET);
  axidev_io_seed_alias("bracketright", AXIDEV_IO_KEY_RIGHT_BRACKET);
  axidev_io_seed_alias("@", AXIDEV_IO_KEY_AT);
  axidev_io_seed_alias("&", AXIDEV_IO_KEY_AMPERSAND);
  axidev_io_seed_alias("(", AXIDEV_IO_KEY_LEFT_PAREN);
  axidev_io_seed_alias(")", AXIDEV_IO_KEY_RIGHT_PAREN);
  axidev_io_seed_alias("!", AXIDEV_IO_KEY_EXCLAMATION);
  axidev_io_seed_alias("$", AXIDEV_IO_KEY_DOLLAR);
  axidev_io_seed_alias("^", AXIDEV_IO_KEY_CARET);
  axidev_io_seed_alias("*", AXIDEV_IO_KEY_ASTERISK);
  axidev_io_seed_alias(" ", AXIDEV_IO_KEY_SPACE);
  axidev_io_seed_alias("\t", AXIDEV_IO_KEY_TAB);
  axidev_io_seed_alias("\x00", AXIDEV_IO_KEY_ASCII_NUL);
  axidev_io_seed_alias("\x01", AXIDEV_IO_KEY_ASCII_SOH);
  axidev_io_seed_alias("\x02", AXIDEV_IO_KEY_ASCII_STX);
  axidev_io_seed_alias("\x03", AXIDEV_IO_KEY_ASCII_ETX);
  axidev_io_seed_alias("\x04", AXIDEV_IO_KEY_ASCII_EOT);
  axidev_io_seed_alias("\x05", AXIDEV_IO_KEY_ASCII_ENQ);
  axidev_io_seed_alias("\x06", AXIDEV_IO_KEY_ASCII_ACK);
  axidev_io_seed_alias("\x07", AXIDEV_IO_KEY_ASCII_BELL);
  axidev_io_seed_alias("\x08", AXIDEV_IO_KEY_BACKSPACE);
  axidev_io_seed_alias("\x09", AXIDEV_IO_KEY_TAB);
  axidev_io_seed_alias("\x0A", AXIDEV_IO_KEY_ENTER);
  axidev_io_seed_alias("\x0B", AXIDEV_IO_KEY_ASCII_VT);
  axidev_io_seed_alias("\x0C", AXIDEV_IO_KEY_ASCII_FF);
  axidev_io_seed_alias("\x0D", AXIDEV_IO_KEY_ENTER);
  axidev_io_seed_alias("\x0E", AXIDEV_IO_KEY_ASCII_SO);
  axidev_io_seed_alias("\x0F", AXIDEV_IO_KEY_ASCII_SI);
  axidev_io_seed_alias("\x10", AXIDEV_IO_KEY_ASCII_DLE);
  axidev_io_seed_alias("\x11", AXIDEV_IO_KEY_ASCII_DC1);
  axidev_io_seed_alias("\x12", AXIDEV_IO_KEY_ASCII_DC2);
  axidev_io_seed_alias("\x13", AXIDEV_IO_KEY_ASCII_DC3);
  axidev_io_seed_alias("\x14", AXIDEV_IO_KEY_ASCII_DC4);
  axidev_io_seed_alias("\x15", AXIDEV_IO_KEY_ASCII_NAK);
  axidev_io_seed_alias("\x16", AXIDEV_IO_KEY_ASCII_SYN);
  axidev_io_seed_alias("\x17", AXIDEV_IO_KEY_ASCII_ETB);
  axidev_io_seed_alias("\x18", AXIDEV_IO_KEY_ASCII_CAN);
  axidev_io_seed_alias("\x19", AXIDEV_IO_KEY_ASCII_EM);
  axidev_io_seed_alias("\x1A", AXIDEV_IO_KEY_ASCII_SUB);
  axidev_io_seed_alias("\x1B", AXIDEV_IO_KEY_ESCAPE);
  axidev_io_seed_alias("\x1C", AXIDEV_IO_KEY_ASCII_FS);
  axidev_io_seed_alias("\x1D", AXIDEV_IO_KEY_ASCII_GS);
  axidev_io_seed_alias("\x1E", AXIDEV_IO_KEY_ASCII_RS);
  axidev_io_seed_alias("\x1F", AXIDEV_IO_KEY_ASCII_US);
  axidev_io_seed_alias("\x7F", AXIDEV_IO_KEY_DELETE);
  axidev_io_seed_alias("_", AXIDEV_IO_KEY_MINUS);
  axidev_io_seed_alias("+", AXIDEV_IO_KEY_EQUAL);
  axidev_io_seed_alias(":", AXIDEV_IO_KEY_SEMICOLON);
  axidev_io_seed_alias("\"", AXIDEV_IO_KEY_APOSTROPHE);
  axidev_io_seed_alias("?", AXIDEV_IO_KEY_SLASH);
  axidev_io_seed_alias("|", AXIDEV_IO_KEY_BACKSLASH);
  axidev_io_seed_alias("<", AXIDEV_IO_KEY_COMMA);
  axidev_io_seed_alias(">", AXIDEV_IO_KEY_PERIOD);
  axidev_io_seed_alias("{", AXIDEV_IO_KEY_LEFT_BRACKET);
  axidev_io_seed_alias("}", AXIDEV_IO_KEY_RIGHT_BRACKET);
  axidev_io_seed_alias("~", AXIDEV_IO_KEY_GRAVE);
  axidev_io_seed_alias("at", AXIDEV_IO_KEY_AT);
  axidev_io_seed_alias("hash", AXIDEV_IO_KEY_HASHTAG);
  axidev_io_seed_alias("hashtag", AXIDEV_IO_KEY_HASHTAG);
  axidev_io_seed_alias("pound", AXIDEV_IO_KEY_HASHTAG);
  axidev_io_seed_alias("bang", AXIDEV_IO_KEY_EXCLAMATION);
  axidev_io_seed_alias("exclamation", AXIDEV_IO_KEY_EXCLAMATION);
  axidev_io_seed_alias("dollar", AXIDEV_IO_KEY_DOLLAR);
  axidev_io_seed_alias("percent", AXIDEV_IO_KEY_PERCENT);
  axidev_io_seed_alias("caret", AXIDEV_IO_KEY_CARET);
  axidev_io_seed_alias("ampersand", AXIDEV_IO_KEY_AMPERSAND);
  axidev_io_seed_alias("star", AXIDEV_IO_KEY_ASTERISK);
  axidev_io_seed_alias("asterisk", AXIDEV_IO_KEY_ASTERISK);
  axidev_io_seed_alias("lparen", AXIDEV_IO_KEY_LEFT_PAREN);
  axidev_io_seed_alias("rparen", AXIDEV_IO_KEY_RIGHT_PAREN);
  axidev_io_seed_alias("underscore", AXIDEV_IO_KEY_UNDERSCORE);
  axidev_io_seed_alias("plus", AXIDEV_IO_KEY_PLUS);
  axidev_io_seed_alias("colon", AXIDEV_IO_KEY_COLON);
  axidev_io_seed_alias("quote", AXIDEV_IO_KEY_QUOTE);
  axidev_io_seed_alias("pipe", AXIDEV_IO_KEY_BAR);
  axidev_io_seed_alias("bar", AXIDEV_IO_KEY_BAR);
  axidev_io_seed_alias("lt", AXIDEV_IO_KEY_LESS_THAN);
  axidev_io_seed_alias("gt", AXIDEV_IO_KEY_GREATER_THAN);
  axidev_io_seed_alias("less", AXIDEV_IO_KEY_LESS_THAN);
  axidev_io_seed_alias("greater", AXIDEV_IO_KEY_GREATER_THAN);
  axidev_io_seed_alias("nul", AXIDEV_IO_KEY_ASCII_NUL);
  axidev_io_seed_alias("bell", AXIDEV_IO_KEY_ASCII_BELL);
  axidev_io_seed_alias("vt", AXIDEV_IO_KEY_ASCII_VT);
  axidev_io_seed_alias("ff", AXIDEV_IO_KEY_ASCII_FF);
  axidev_io_seed_alias("dle", AXIDEV_IO_KEY_ASCII_DLE);
  axidev_io_seed_alias("sub", AXIDEV_IO_KEY_ASCII_SUB);
  axidev_io_seed_alias("can", AXIDEV_IO_KEY_ASCII_CAN);
  axidev_io_seed_alias("fs", AXIDEV_IO_KEY_ASCII_FS);
  axidev_io_seed_alias("gs", AXIDEV_IO_KEY_ASCII_GS);
  axidev_io_seed_alias("rs", AXIDEV_IO_KEY_ASCII_RS);
  axidev_io_seed_alias("us", AXIDEV_IO_KEY_ASCII_US);
  axidev_io_seed_alias("del", AXIDEV_IO_KEY_ASCII_DEL);
  axidev_io_seed_alias("kp0", AXIDEV_IO_KEY_NUMPAD0);
  axidev_io_seed_alias("kp1", AXIDEV_IO_KEY_NUMPAD1);
  axidev_io_seed_alias("kp2", AXIDEV_IO_KEY_NUMPAD2);
  axidev_io_seed_alias("kp3", AXIDEV_IO_KEY_NUMPAD3);
  axidev_io_seed_alias("kp4", AXIDEV_IO_KEY_NUMPAD4);
  axidev_io_seed_alias("kp5", AXIDEV_IO_KEY_NUMPAD5);
  axidev_io_seed_alias("kp6", AXIDEV_IO_KEY_NUMPAD6);
  axidev_io_seed_alias("kp7", AXIDEV_IO_KEY_NUMPAD7);
  axidev_io_seed_alias("kp8", AXIDEV_IO_KEY_NUMPAD8);
  axidev_io_seed_alias("kp9", AXIDEV_IO_KEY_NUMPAD9);
  axidev_io_seed_alias("control_l", AXIDEV_IO_KEY_CTRL_LEFT);
  axidev_io_seed_alias("control_r", AXIDEV_IO_KEY_CTRL_RIGHT);
  axidev_io_seed_alias("shift_l", AXIDEV_IO_KEY_SHIFT_LEFT);
  axidev_io_seed_alias("shift_r", AXIDEV_IO_KEY_SHIFT_RIGHT);
  axidev_io_seed_alias("alt_l", AXIDEV_IO_KEY_ALT_LEFT);
  axidev_io_seed_alias("alt_r", AXIDEV_IO_KEY_ALT_RIGHT);
  axidev_io_seed_alias("meta_l", AXIDEV_IO_KEY_SUPER_LEFT);
  axidev_io_seed_alias("super_l", AXIDEV_IO_KEY_SUPER_LEFT);
  axidev_io_seed_alias("super_r", AXIDEV_IO_KEY_SUPER_RIGHT);
  axidev_io_seed_alias("hyper_l", AXIDEV_IO_KEY_SUPER_LEFT);
  axidev_io_seed_alias("caps_lock", AXIDEV_IO_KEY_CAPS_LOCK);
  axidev_io_seed_alias("num_lock", AXIDEV_IO_KEY_NUM_LOCK);
  axidev_io_seed_alias("scroll_lock", AXIDEV_IO_KEY_SCROLL_LOCK);
  axidev_io_seed_alias("iso_left_tab", AXIDEV_IO_KEY_TAB);
  axidev_io_seed_alias("iso_level3_shift", AXIDEV_IO_KEY_ALT_RIGHT);
  axidev_io_seed_alias("iso_level5_shift", AXIDEV_IO_KEY_ALT_RIGHT);
  axidev_io_seed_alias("quotedbl", AXIDEV_IO_KEY_QUOTE);
  axidev_io_seed_alias("parenleft", AXIDEV_IO_KEY_LEFT_PAREN);
  axidev_io_seed_alias("parenright", AXIDEV_IO_KEY_RIGHT_PAREN);
  axidev_io_seed_alias("equal", AXIDEV_IO_KEY_EQUAL);
  axidev_io_seed_alias("question", AXIDEV_IO_KEY_QUESTION_MARK);
  axidev_io_seed_alias("exclam", AXIDEV_IO_KEY_EXCLAMATION);
  axidev_io_seed_alias("section", AXIDEV_IO_KEY_SECTION);
  axidev_io_seed_alias("degree", AXIDEV_IO_KEY_DEGREE);
  axidev_io_seed_alias("sterling", AXIDEV_IO_KEY_STERLING);
  axidev_io_seed_alias("plusminus", AXIDEV_IO_KEY_PLUS_MINUS);
  axidev_io_seed_alias("dead_circumflex", AXIDEV_IO_KEY_DEAD_CIRCUMFLEX);
  axidev_io_seed_alias("dead_diaeresis", AXIDEV_IO_KEY_DEAD_DIAERESIS);
  axidev_io_seed_alias("eacute", AXIDEV_IO_KEY_E);
  axidev_io_seed_alias("egrave", AXIDEV_IO_KEY_E);
  axidev_io_seed_alias("agrave", AXIDEV_IO_KEY_A);
  axidev_io_seed_alias("ugrave", AXIDEV_IO_KEY_U);
  axidev_io_seed_alias("ccedilla", AXIDEV_IO_KEY_C);
  axidev_io_seed_alias("oe", AXIDEV_IO_KEY_oe);
  axidev_io_seed_alias("OE", AXIDEV_IO_KEY_OE);
  axidev_io_seed_alias("mu", AXIDEV_IO_KEY_MU);
  axidev_io_seed_alias("linefeed", AXIDEV_IO_KEY_ENTER);
  axidev_io_seed_alias("prior", AXIDEV_IO_KEY_PAGE_UP);
  axidev_io_seed_alias("next", AXIDEV_IO_KEY_PAGE_DOWN);
  axidev_io_seed_alias("print", AXIDEV_IO_KEY_PRINT_SCREEN);
  axidev_io_seed_alias("sys_req", AXIDEV_IO_KEY_PRINT_SCREEN);
  axidev_io_seed_alias("break", AXIDEV_IO_KEY_PAUSE);
  axidev_io_seed_alias("cancel", AXIDEV_IO_KEY_CANCEL);
  axidev_io_seed_alias("redo", AXIDEV_IO_KEY_REDO);
  axidev_io_seed_alias("undo", AXIDEV_IO_KEY_UNDO);
  axidev_io_seed_alias("find", AXIDEV_IO_KEY_FIND);
  axidev_io_seed_alias("sunprops", AXIDEV_IO_KEY_SUN_PROPS);
  axidev_io_seed_alias("sunfront", AXIDEV_IO_KEY_SUN_FRONT);
  axidev_io_seed_alias("menu", AXIDEV_IO_KEY_MENU);
  axidev_io_seed_alias("copy", AXIDEV_IO_KEY_COPY);
  axidev_io_seed_alias("open", AXIDEV_IO_KEY_OPEN);
  axidev_io_seed_alias("paste", AXIDEV_IO_KEY_PASTE);
  axidev_io_seed_alias("cut", AXIDEV_IO_KEY_CUT);
  axidev_io_seed_alias("calculator", AXIDEV_IO_KEY_CALCULATOR);
  axidev_io_seed_alias("explorer", AXIDEV_IO_KEY_EXPLORER);
  axidev_io_seed_alias("phone", AXIDEV_IO_KEY_PHONE);
  axidev_io_seed_alias("webcam", AXIDEV_IO_KEY_WEB_CAM);
  axidev_io_seed_alias("mail", AXIDEV_IO_KEY_MAIL);
  axidev_io_seed_alias("mailforward", AXIDEV_IO_KEY_MAIL_FORWARD);
  axidev_io_seed_alias("save", AXIDEV_IO_KEY_SAVE);
  axidev_io_seed_alias("documents", AXIDEV_IO_KEY_DOCUMENTS);

  for (i = 0; i < count; ++i) {
    char *lower = axidev_io_to_lower_copy(pairs[i].name);
    if (lower != NULL) {
      if (shgeti(g_reverse_map, lower) < 0) {
        shput(g_reverse_map, lower, pairs[i].key);
      }
      free(lower);
    }
  }
}

const char *axidev_io_key_to_string_const(axidev_io_keyboard_key_t key) {
  size_t count = 0;
  const axidev_io_key_name_pair *pairs = axidev_io_key_string_pairs(&count);

  for (size_t i = 0; i < count; ++i) {
    if (pairs[i].key == key) {
      return pairs[i].name;
    }
  }
  return "Unknown";
}

char *axidev_io_key_to_string_alloc(axidev_io_keyboard_key_t key) {
  return axidev_io_duplicate_string(axidev_io_key_to_string_const(key));
}

axidev_io_keyboard_key_t axidev_io_string_to_key_internal(const char *input) {
  char *lower;
  char *escaped;
  ptrdiff_t index;

  if (input == NULL || input[0] == '\0') {
    return AXIDEV_IO_KEY_UNKNOWN;
  }

  axidev_io_call_once(&g_reverse_map_once, axidev_io_init_reverse_map);

  index = shgeti(g_reverse_map, input);
  if (index >= 0) {
    return g_reverse_map[index].value;
  }

  lower = axidev_io_to_lower_copy(input);
  if (lower == NULL) {
    return AXIDEV_IO_KEY_UNKNOWN;
  }

  index = shgeti(g_reverse_map, lower);
  if (index >= 0) {
    axidev_io_keyboard_key_t key = g_reverse_map[index].value;
    free(lower);
    return key;
  }

  if (strncmp(lower, "kp", 2) == 0) {
    char *suffix = lower + 2;
    if (*suffix == '_') {
      ++suffix;
    }
    if (strcmp(suffix, "multiply") == 0 || strcmp(suffix, "mul") == 0) {
      free(lower);
      return AXIDEV_IO_KEY_NUMPAD_MULTIPLY;
    }
    if (strcmp(suffix, "divide") == 0 || strcmp(suffix, "div") == 0) {
      free(lower);
      return AXIDEV_IO_KEY_NUMPAD_DIVIDE;
    }
    if (strcmp(suffix, "add") == 0 || strcmp(suffix, "plus") == 0) {
      free(lower);
      return AXIDEV_IO_KEY_NUMPAD_PLUS;
    }
    if (strcmp(suffix, "subtract") == 0 || strcmp(suffix, "minus") == 0) {
      free(lower);
      return AXIDEV_IO_KEY_NUMPAD_MINUS;
    }
    if (strcmp(suffix, "enter") == 0) {
      free(lower);
      return AXIDEV_IO_KEY_NUMPAD_ENTER;
    }
    if (strcmp(suffix, "decimal") == 0 || strcmp(suffix, "delete") == 0 ||
        strcmp(suffix, "del") == 0) {
      free(lower);
      return AXIDEV_IO_KEY_NUMPAD_DECIMAL;
    }
    if (strcmp(suffix, "equal") == 0) {
      free(lower);
      return AXIDEV_IO_KEY_NUMPAD_EQUAL;
    }
    if (strcmp(suffix, "home") == 0 || strcmp(suffix, "7") == 0) {
      free(lower);
      return AXIDEV_IO_KEY_NUMPAD7;
    }
    if (strcmp(suffix, "up") == 0 || strcmp(suffix, "8") == 0) {
      free(lower);
      return AXIDEV_IO_KEY_NUMPAD8;
    }
    if (strcmp(suffix, "prior") == 0 || strcmp(suffix, "9") == 0) {
      free(lower);
      return AXIDEV_IO_KEY_NUMPAD9;
    }
    if (strcmp(suffix, "left") == 0 || strcmp(suffix, "4") == 0) {
      free(lower);
      return AXIDEV_IO_KEY_NUMPAD4;
    }
    if (strcmp(suffix, "begin") == 0 || strcmp(suffix, "5") == 0) {
      free(lower);
      return AXIDEV_IO_KEY_NUMPAD5;
    }
    if (strcmp(suffix, "right") == 0 || strcmp(suffix, "6") == 0) {
      free(lower);
      return AXIDEV_IO_KEY_NUMPAD6;
    }
    if (strcmp(suffix, "end") == 0 || strcmp(suffix, "1") == 0) {
      free(lower);
      return AXIDEV_IO_KEY_NUMPAD1;
    }
    if (strcmp(suffix, "down") == 0 || strcmp(suffix, "2") == 0) {
      free(lower);
      return AXIDEV_IO_KEY_NUMPAD2;
    }
    if (strcmp(suffix, "next") == 0 || strcmp(suffix, "3") == 0) {
      free(lower);
      return AXIDEV_IO_KEY_NUMPAD3;
    }
    if (strcmp(suffix, "insert") == 0 || strcmp(suffix, "0") == 0) {
      free(lower);
      return AXIDEV_IO_KEY_NUMPAD0;
    }
  }

  if (strncmp(lower, "xf86", 4) == 0) {
    if (strstr(lower, "audiomute") != NULL) {
      free(lower);
      return AXIDEV_IO_KEY_MUTE;
    }
    if (strstr(lower, "audiolowervolume") != NULL) {
      free(lower);
      return AXIDEV_IO_KEY_VOLUME_DOWN;
    }
    if (strstr(lower, "audioraisevolume") != NULL) {
      free(lower);
      return AXIDEV_IO_KEY_VOLUME_UP;
    }
    if (strstr(lower, "audionext") != NULL) {
      free(lower);
      return AXIDEV_IO_KEY_MEDIA_NEXT;
    }
    if (strstr(lower, "audioplay") != NULL ||
        strstr(lower, "audiopause") != NULL) {
      free(lower);
      return AXIDEV_IO_KEY_MEDIA_PLAY_PAUSE;
    }
    if (strstr(lower, "audioprev") != NULL) {
      free(lower);
      return AXIDEV_IO_KEY_MEDIA_PREVIOUS;
    }
    if (strstr(lower, "audiostop") != NULL) {
      free(lower);
      return AXIDEV_IO_KEY_MEDIA_STOP;
    }
    if (strstr(lower, "audiorecord") != NULL) {
      free(lower);
      return AXIDEV_IO_KEY_AUDIO_RECORD;
    }
    if (strstr(lower, "audiorewind") != NULL) {
      free(lower);
      return AXIDEV_IO_KEY_AUDIO_REWIND;
    }
    if (strstr(lower, "power") != NULL) {
      free(lower);
      return AXIDEV_IO_KEY_POWER;
    }
    if (strstr(lower, "sleep") != NULL) {
      free(lower);
      return AXIDEV_IO_KEY_SLEEP;
    }
    if (strstr(lower, "wakeup") != NULL) {
      free(lower);
      return AXIDEV_IO_KEY_WAKE;
    }
    if (strstr(lower, "eject") != NULL) {
      free(lower);
      return AXIDEV_IO_KEY_EJECT;
    }
    if (strstr(lower, "monbrightnessdown") != NULL) {
      free(lower);
      return AXIDEV_IO_KEY_BRIGHTNESS_DOWN;
    }
    if (strstr(lower, "monbrightnessup") != NULL) {
      free(lower);
      return AXIDEV_IO_KEY_BRIGHTNESS_UP;
    }
    if (strstr(lower, "calculator") != NULL) {
      free(lower);
      return AXIDEV_IO_KEY_CALCULATOR;
    }
    if (strstr(lower, "mail") != NULL) {
      free(lower);
      return AXIDEV_IO_KEY_MAIL;
    }
    if (strstr(lower, "webcam") != NULL) {
      free(lower);
      return AXIDEV_IO_KEY_WEB_CAM;
    }
    if (strstr(lower, "search") != NULL) {
      free(lower);
      return AXIDEV_IO_KEY_SEARCH;
    }
    if (strstr(lower, "launcha") != NULL) {
      free(lower);
      return AXIDEV_IO_KEY_LAUNCH_A;
    }
    if (strstr(lower, "launchb") != NULL) {
      free(lower);
      return AXIDEV_IO_KEY_LAUNCH_B;
    }
    if (strstr(lower, "launch1") != NULL) {
      free(lower);
      return AXIDEV_IO_KEY_LAUNCH1;
    }
    if (strstr(lower, "launch2") != NULL) {
      free(lower);
      return AXIDEV_IO_KEY_LAUNCH2;
    }
    if (strstr(lower, "launch3") != NULL) {
      free(lower);
      return AXIDEV_IO_KEY_LAUNCH3;
    }
    if (strstr(lower, "launch4") != NULL) {
      free(lower);
      return AXIDEV_IO_KEY_LAUNCH4;
    }
    if (strstr(lower, "launch5") != NULL) {
      free(lower);
      return AXIDEV_IO_KEY_LAUNCH5;
    }
    if (strstr(lower, "launch6") != NULL) {
      free(lower);
      return AXIDEV_IO_KEY_LAUNCH6;
    }
    if (strstr(lower, "launch7") != NULL) {
      free(lower);
      return AXIDEV_IO_KEY_LAUNCH7;
    }
    if (strstr(lower, "launch8") != NULL) {
      free(lower);
      return AXIDEV_IO_KEY_LAUNCH8;
    }
    if (strstr(lower, "launch9") != NULL) {
      free(lower);
      return AXIDEV_IO_KEY_LAUNCH9;
    }
    if (strstr(lower, "touchpad") != NULL) {
      free(lower);
      return AXIDEV_IO_KEY_TOUCHPAD_TOGGLE;
    }
    if (strstr(lower, "kbdbrightnessdown") != NULL) {
      free(lower);
      return AXIDEV_IO_KEY_KBD_BRIGHTNESS_DOWN;
    }
    if (strstr(lower, "kbdbrightnessup") != NULL) {
      free(lower);
      return AXIDEV_IO_KEY_KBD_BRIGHTNESS_UP;
    }
    if (strstr(lower, "kbd") != NULL) {
      free(lower);
      return AXIDEV_IO_KEY_KBD_LIGHT_ON_OFF;
    }
    if (strstr(lower, "battery") != NULL) {
      free(lower);
      return AXIDEV_IO_KEY_BATTERY;
    }
    if (strstr(lower, "bluetooth") != NULL) {
      free(lower);
      return AXIDEV_IO_KEY_BLUETOOTH;
    }
    if (strstr(lower, "wlan") != NULL) {
      free(lower);
      return AXIDEV_IO_KEY_WLAN;
    }
    if (strstr(lower, "wwan") != NULL) {
      free(lower);
      return AXIDEV_IO_KEY_WWAN;
    }
    if (strstr(lower, "rfkill") != NULL) {
      free(lower);
      return AXIDEV_IO_KEY_RF_KILL;
    }
  }

  escaped = axidev_io_escape_for_log(input);
  AXIDEV_IO_LOG_DEBUG("stringToKey: unknown input='%s'", escaped);
  arrfree(escaped);
  free(lower);
  return AXIDEV_IO_KEY_UNKNOWN;
}

char *axidev_io_key_to_string_with_modifier_alloc(
    axidev_io_keyboard_key_t key, axidev_io_keyboard_modifier_t mods) {
  char buffer[256];
  buffer[0] = '\0';

  if (axidev_io_keyboard_has_modifier(mods, AXIDEV_IO_MOD_SUPER)) {
    strcat(buffer, "Super+");
  }
  if (axidev_io_keyboard_has_modifier(mods, AXIDEV_IO_MOD_CTRL)) {
    strcat(buffer, "Ctrl+");
  }
  if (axidev_io_keyboard_has_modifier(mods, AXIDEV_IO_MOD_ALT)) {
    strcat(buffer, "Alt+");
  }
  if (axidev_io_keyboard_has_modifier(mods, AXIDEV_IO_MOD_SHIFT)) {
    strcat(buffer, "Shift+");
  }
  strcat(buffer, axidev_io_key_to_string_const(key));
  return axidev_io_duplicate_string(buffer);
}

bool axidev_io_string_to_key_with_modifier_internal(
    const char *input, axidev_io_keyboard_key_with_modifier_t *out_key_mod) {
  const char *remaining;
  axidev_io_keyboard_modifier_t mods = AXIDEV_IO_MOD_NONE;
  bool found_modifier = true;

  if (out_key_mod == NULL) {
    return false;
  }

  out_key_mod->key = AXIDEV_IO_KEY_UNKNOWN;
  out_key_mod->mods = AXIDEV_IO_MOD_NONE;

  if (input == NULL || input[0] == '\0') {
    return true;
  }

  remaining = input;
  while (found_modifier && remaining[0] != '\0') {
    static const struct {
      const char *prefix;
      axidev_io_keyboard_modifier_t modifier;
    } prefixes[] = {
        {"super+", AXIDEV_IO_MOD_SUPER},  {"super-", AXIDEV_IO_MOD_SUPER},
        {"cmd+", AXIDEV_IO_MOD_SUPER},    {"cmd-", AXIDEV_IO_MOD_SUPER},
        {"win+", AXIDEV_IO_MOD_SUPER},    {"win-", AXIDEV_IO_MOD_SUPER},
        {"meta+", AXIDEV_IO_MOD_SUPER},   {"meta-", AXIDEV_IO_MOD_SUPER},
        {"ctrl+", AXIDEV_IO_MOD_CTRL},    {"ctrl-", AXIDEV_IO_MOD_CTRL},
        {"control+", AXIDEV_IO_MOD_CTRL}, {"control-", AXIDEV_IO_MOD_CTRL},
        {"alt+", AXIDEV_IO_MOD_ALT},      {"alt-", AXIDEV_IO_MOD_ALT},
        {"opt+", AXIDEV_IO_MOD_ALT},      {"opt-", AXIDEV_IO_MOD_ALT},
        {"option+", AXIDEV_IO_MOD_ALT},   {"option-", AXIDEV_IO_MOD_ALT},
        {"shift+", AXIDEV_IO_MOD_SHIFT},  {"shift-", AXIDEV_IO_MOD_SHIFT}};
    char *lower = axidev_io_to_lower_copy(remaining);

    found_modifier = false;
    if (lower == NULL) {
      return false;
    }

    for (size_t i = 0; i < sizeof(prefixes) / sizeof(prefixes[0]); ++i) {
      size_t prefix_length = strlen(prefixes[i].prefix);
      if (strncmp(lower, prefixes[i].prefix, prefix_length) == 0) {
        mods = axidev_io_keyboard_add_modifier(mods, prefixes[i].modifier);
        remaining += prefix_length;
        found_modifier = true;
        break;
      }
    }
    free(lower);
  }

  out_key_mod->key = axidev_io_string_to_key_internal(remaining);
  out_key_mod->mods = mods;
  return true;
}

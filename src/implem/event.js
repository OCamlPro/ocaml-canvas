/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

//Provides: SLANT
var SLANT = {
  ROMAN   : "normal",
  ITALIC  : "italic",
  OBLIQUE : "oblique",
};

//Provides: FOCUS
var FOCUS = {
  OUT : 0,
  IN  : 1
};

//Provides: KEY
var KEY = {
  UNDEFINED : 0x03, /* to handle translation errors */

  /* Function */
  ESCAPE : 0x29,
  F1 : 0x3A,
  F2 : 0x3B,
  F3 : 0x3C,
  F4 : 0x3D,
  F5 : 0x3E,
  F6 : 0x3F,
  F7 : 0x40,
  F8 : 0x41,
  F9 : 0x42,
  F10 : 0x43,
  F11 : 0x44,
  F12 : 0x45,
  PRINTSCREEN : 0x46,
  SCROLLLOCK : 0x47,
  PAUSE : 0x48,

  /* Alphanumeric, first row */
  GRAVE_TILDE : 0x35, /* possibly Zenkaku/Hankaku key on Japanese KB */
  _1_EXCLAMATION : 0x1E,
  _2_AT : 0x1F,
  _3_NUMBER : 0x20,
  _4_DOLLAR : 0x21,
  _5_PERCENT : 0x22,
  _6_CARET : 0x23,
  _7_AMPERSAND : 0x24,
  _8_ASTERISK : 0x25,
  _9_LPARENTHESIS : 0x26,
  _0_RPARENTHESIS : 0x27,
  MINUS_UNDERSCORE : 0x2D,
  EQUAL_PLUS : 0x2E,
  BACKSPACE : 0x2A, /* also called Delete */

  /* Alphanumeric, second row */
  TAB : 0x2B,
  Q : 0x14,
  W : 0x1A,
  E : 0x08,
  R : 0x15,
  T : 0x17,
  Y : 0x1C,
  U : 0x18,
  I : 0x0C,
  O : 0x12,
  P : 0x13,
  LBRACKET_CURLY : 0x2F,
  RBRACKET_CURLY : 0x30,
  BACKSLASH_PIPE : 0x31, /* replaced by NONUS_NUMBER_TILDE on ISO KB */

  /* Alphanumeric, third row */
  CAPSLOCK : 0x39, /* also Eisu (英数) on JIS keyboards */
  A : 0x04,
  S : 0x16,
  D : 0x07,
  F : 0x09,
  G : 0x0A,
  H : 0x0B,
  J : 0x0D,
  K : 0x0E,
  L : 0x0F,
  SEMICOLON_COLON : 0x33,
  QUOTE_DOUBLEQUOTE : 0x34,
  NONUS_NUMBER_TILDE : 0x32, /* extra key left of Return on ISO KB */
  RETURN : 0x28, /* also called Enter */

  /* Alphanumeric, fourth row */
  LSHIFT : 0xE1,
  NONUS_BACKSLASH_PIPE : 0x64, /* extra key right of LShift on ISO KB */
  Z : 0x1D,
  X : 0x1B,
  C : 0x06,
  V : 0x19,
  B : 0x05,
  N : 0x11,
  M : 0x10,
  COMMA_LESS : 0x36,
  PERIOD_GREATER : 0x37,
  SLASH_QUESTION : 0x38,
  RSHIFT : 0xE5,

  /* Alphanumeric, fifth row */
  LCONTROL : 0xE0,
  LMETA : 0xE3, /* Left Windows, Left Command */
  LALT : 0xE2,
  SPACEBAR : 0x2C,
  RALT : 0xE6,
  RMETA : 0xE7, /* Right Windows, Right Command */
  MENU : 0x76,
  RCONTROL : 0xE4,

  /* Control pad */
  INSERT : 0x49, /* replaced by a Fn key on Mac (different code) */
  HOME : 0x4A,
  PAGEUP : 0x4B,
  DELETEFORWARD : 0x4C,
  END : 0x4D,
  PAGEDOWN : 0x4E,

  /* Arrow pad */
  UPARROW : 0x52,
  LEFTARROW : 0x50,
  DOWNARROW : 0x51,
  RIGHTARROW : 0x4F,

  /* Numeric pad */
  PAD_NUMLOCK_CLEAR : 0x53, /* no NumLock on Mac keyboards */
  PAD_EQUALS : 0x67, /* on Mac keyboards */
  PAD_DIVIDE : 0x54,
  PAD_MULTIPLY : 0x55,
  PAD_MINUS : 0x56,
  PAD_7_HOME : 0x5F,
  PAD_8_UPARROW : 0x60,
  PAD_9_PAGEUP : 0x61,
  PAD_PLUS : 0x57,
  PAD_4_LEFTARROW : 0x5C,
  PAD_5 : 0x5D,
  PAD_6_RIGHTARROW : 0x5E,
  PAD_COMMA : 0x85, /* specific to Brazilian keyboards */
  PAD_1_END : 0x59,
  PAD_2_DOWNARROW : 0x5A,
  PAD_3_PAGEDOWN : 0x5B,
  PAD_0_INSERT : 0x62,
  PAD_DECIMAL_DELETE : 0x63,
  PAD_ENTER : 0x58,
  // AS/400 keyboards also have an equals sign at 0x86

  /* Extra function keys */
  F13 : 0x68,
  F14 : 0x69,
  F15 : 0x6A,
  F16 : 0x6B,
  F17 : 0x6C,
  F18 : 0x6D,
  F19 : 0x6E,
  F20 : 0x6F,
  F21 : 0x70,
  F22 : 0x71,
  F23 : 0x72,
  F24 : 0x73,

  /* International & LANG keys */
  INTERNATIONAL1 : 0x87, /* _ and \ key left of RShift on JIS KB */
                             /* also Brazilian / and ? key */
  INTERNATIONAL2 : 0x88, /* Katakana/Hiragana key right of Space on JIS KB */
  INTERNATIONAL3 : 0x89, /* | and Yen key left of Backspace on JIS KB */
  INTERNATIONAL4 : 0x8A, /* Henkan key right of Space on JIS KB + 前候補 */
  INTERNATIONAL5 : 0x8B, /* Muhenkan key left of Space on JIS KB */
  INTERNATIONAL6 : 0x8C, /* Kanma (comma), right of KP0 on JIS KB */
  INTERNATIONAL7 : 0x8D, /* Double-Byte/Single-Byte toggle key */
  INTERNATIONAL8 : 0x8E, /* Undefined */
  INTERNATIONAL9 : 0x8F, /* Undefined */
  LANG1 : 0x90, /* Hangul/English toggle key (Korean) */
  LANG2 : 0x91, /* Hanja conversion key (Korean) */
  LANG3 : 0x92, /* Katakana key (Japanese) */
  LANG4 : 0x93, /* Hiragana key (Japanese) */
  LANG5 : 0x94, /* Zenkaku/Hankaku key (Japanese) */

  /* Extensions */
  HELP : 0x75,
  MUTE : 0x7F,
  VOLUMEUP : 0x80,
  VOLUMEDOWN : 0x81
};

//Provides: KEY_STATE
var KEY_STATE = {
  UP : 0,
  DOWN  : 1
};

//Provides: BUTTON
var BUTTON = {
  NONE       : 0,
  LEFT       : 1,
  MIDDLE     : 2,
  RIGHT      : 3,
  WHEEL_UP   : 4,
  WHEEL_DOWN : 5
};

//Provides: BUTTON_STATE
var BUTTON_STATE = {
  UP : 0,
  DOWN  : 1
};

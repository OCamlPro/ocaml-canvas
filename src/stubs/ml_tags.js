/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

//Provides: FOCUS_TAG
var FOCUS_TAG = {
  OUT : 0,
  IN  : 1
};

//Provides: KEY_TAG
var KEY_TAG = {
  ESCAPE               : 0,
  F1                   : 1,
  F2                   : 2,
  F3                   : 3,
  F4                   : 4,
  F5                   : 5,
  F6                   : 6,
  F7                   : 7,
  F8                   : 8,
  F9                   : 9,
  F10                  : 10,
  F11                  : 11,
  F12                  : 12,
  PRINTSCREEN          : 13,
  SCROLLLOCK           : 14,
  PAUSE                : 15,
  GRAVE_TILDE          : 16,
  _1_EXCLAMATION       : 17,
  _2_AT                : 18,
  _3_NUMBER            : 19,
  _4_DOLLAR            : 20,
  _5_PERCENT           : 21,
  _6_CARET             : 22,
  _7_AMPERSAND         : 23,
  _8_ASTERISK          : 24,
  _9_LPARENTHESIS      : 25,
  _0_RPARENTHESIS      : 26,
  MINUS_UNDERSCORE     : 27,
  EQUAL_PLUS           : 28,
  BACKSPACE            : 29,
  TAB                  : 30,
  Q                    : 31,
  W                    : 32,
  E                    : 33,
  R                    : 34,
  T                    : 35,
  Y                    : 36,
  U                    : 37,
  I                    : 38,
  O                    : 39,
  P                    : 40,
  LBRACKET_CURLY       : 41,
  RBRACKET_CURLY       : 42,
  BACKSLASH_PIPE       : 43,
  CAPSLOCK             : 44,
  A                    : 45,
  S                    : 46,
  D                    : 47,
  F                    : 48,
  G                    : 49,
  H                    : 50,
  J                    : 51,
  K                    : 52,
  L                    : 53,
  SEMICOLON_COLON      : 54,
  QUOTE_DOUBLEQUOTE    : 55,
  NONUS_NUMBER_TILDE   : 56,
  RETURN               : 57,
  LSHIFT               : 58,
  NONUS_BACKSLASH_PIPE : 59,
  Z                    : 60,
  X                    : 61,
  C                    : 62,
  V                    : 63,
  B                    : 64,
  N                    : 65,
  M                    : 66,
  COMMA_LESS           : 67,
  PERIOD_GREATER       : 68,
  SLASH_QUESTION       : 69,
  RSHIFT               : 70,
  LCONTROL             : 71,
  LMETA                : 72,
  LALT                 : 73,
  SPACEBAR             : 74,
  RALT                 : 75,
  RMETA                : 76,
  MENU                 : 77,
  RCONTROL             : 78,
  INSERT               : 79,
  HOME                 : 80,
  PAGEUP               : 81,
  DELETEFORWARD        : 82,
  END                  : 83,
  PAGEDOWN             : 84,
  UPARROW              : 85,
  LEFTARROW            : 86,
  DOWNARROW            : 87,
  RIGHTARROW           : 88,
  PAD_NUMLOCK_CLEAR    : 89,
  PAD_EQUALS           : 90,
  PAD_DIVIDE           : 91,
  PAD_MULTIPLY         : 92,
  PAD_MINUS            : 93,
  PAD_7_HOME           : 94,
  PAD_8_UPARROW        : 95,
  PAD_9_PAGEUP         : 96,
  PAD_PLUS             : 97,
  PAD_4_LEFTARROW      : 98,
  PAD_5                : 99,
  PAD_6_RIGHTARROW     : 100,
  PAD_COMMA            : 101,
  PAD_1_END            : 102,
  PAD_2_DOWNARROW      : 103,
  PAD_3_PAGEDOWN       : 104,
  PAD_0_INSERT         : 105,
  PAD_DECIMAL_DELETE   : 106,
  PAD_ENTER            : 107,
  F13                  : 108,
  F14                  : 109,
  F15                  : 110,
  F16                  : 111,
  F17                  : 112,
  F18                  : 113,
  F19                  : 114,
  F20                  : 115,
  F21                  : 116,
  F22                  : 117,
  F23                  : 118,
  F24                  : 119,
  INTERNATIONAL1       : 120,
  INTERNATIONAL2       : 121,
  INTERNATIONAL3       : 122,
  INTERNATIONAL4       : 123,
  INTERNATIONAL5       : 124,
  INTERNATIONAL6       : 125,
  INTERNATIONAL7       : 126,
  INTERNATIONAL8       : 127,
  INTERNATIONAL9       : 128,
  LANG1                : 129,
  LANG2                : 130,
  LANG3                : 131,
  LANG4                : 132,
  LANG5                : 133,
  HELP                 : 134,
  MUTE                 : 135,
  VOLUMEUP             : 136,
  VOLUMEDOWN           : 137
};

// Provides: KEY_STATE_TAG
var KEY_STATE_TAG = {
  UP   : 0,
  DOWN : 1
};

// Provides: BUTTON_TAG
var BUTTON_TAG = {
  NONE       : 0,
  LEFT       : 1,
  MIDDLE     : 2,
  RIGHT      : 3,
  WHEEL_UP   : 4,
  WHEEL_DOWN : 5
};

// Provides: BUTTON_STATE_TAG
var BUTTON_STATE_TAG = {
  UP   : 0,
  DOWN : 1
};

// Provides: EVENT_TAG
var EVENT_TAG = {
  FRAME          : 0,
  CANVAS_FOCUSED : 1,
  CANVAS_RESIZED : 2,
  CANVAS_MOVED   : 3,
  CANVAS_CLOSED  : 4,
  KEY_ACTION     : 5,
  BUTTON_ACTION  : 6,
  MOUSE_MOVE     : 7
};

//Provides: STYLE_TAG
var STYLE_TAG = {
  COLOR    : 0,
  GRADIENT : 1,
  PATTERN  : 2
};

//Provides: REPEAT_TAG
var REPEAT_TAG = {
  NO_REPEAT : 0,
  REPEAT_X  : 1,
  REPEAT_Y  : 2,
  REPEAT_XY : 3
};

//Provides: SLANT_TAG
var SLANT_TAG = {
  ROMAN   : 0,
  ITALIC  : 1,
  OBLIQUE : 2
};

//Provides: LINE_JOIN_TAG
var LINE_JOIN_TAG = {
  ROUND : 0,
  MITER : 1,
  BEVEL : 2
}

//Provides: LINE_CAP_TAG
var LINE_CAP_TAG = {
  BUTT   : 0,
  SQUARE : 1,
  ROUND  : 2
};

//Provides : COMPOP_TAG
var COMPOP_TAG = {
  SOURCE_OVER      : 0,
  SOURCE_IN        : 1,
  SOURCE_OUT       : 2,
  SOURCE_ATOP      : 3,
  DESTINATION_OVER : 4,
  DESTINATION_IN   : 5,
  DESTINATION_OUT  : 6,
  DESTINATION_ATOP : 7,
  LIGHTER          : 8,
  COPY             : 9,
  XOR              : 10,
  MULTIPLY         : 11,
  SCREEN           : 12,
  OVERLAY          : 13,
  DARKEN           : 14,
  LIGHTEN          : 15,
  COLOR_DODGE      : 16,
  COLOR_BURN       : 17,
  HARD_LIGHT       : 18,
  SOFT_LIGHT       : 19,
  DIFFERENCE       : 20,
  EXCLUSION        : 21,
  HUE              : 22,
  SATURATION       : 23,
  COLOR            : 24,
  LUMINOSITY       : 25
}

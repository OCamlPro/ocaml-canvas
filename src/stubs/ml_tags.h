/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __ML_TAGS_H
#define __ML_TAGS_H

typedef enum slant_tag_t {
  TAG_ROMAN   = 0,
  TAG_ITALIC  = 1,
  TAG_OBLIQUE = 2
} slant_tag_t;

typedef enum focus_tag_t {
  TAG_FOCUS_OUT = 0,
  TAG_FOCUS_IN  = 1
} focus_tag_t;

typedef enum key_tag_t {
  TAG_KEY_ESCAPE               = 0,
  TAG_KEY_F1                   = 1,
  TAG_KEY_F2                   = 2,
  TAG_KEY_F3                   = 3,
  TAG_KEY_F4                   = 4,
  TAG_KEY_F5                   = 5,
  TAG_KEY_F6                   = 6,
  TAG_KEY_F7                   = 7,
  TAG_KEY_F8                   = 8,
  TAG_KEY_F9                   = 9,
  TAG_KEY_F10                  = 10,
  TAG_KEY_F11                  = 11,
  TAG_KEY_F12                  = 12,
  TAG_KEY_PRINTSCREEN          = 13,
  TAG_KEY_SCROLLLOCK           = 14,
  TAG_KEY_PAUSE                = 15,
  TAG_KEY_GRAVE_TILDE          = 16,
  TAG_KEY_1_EXCLAMATION        = 17,
  TAG_KEY_2_AT                 = 18,
  TAG_KEY_3_NUMBER             = 19,
  TAG_KEY_4_DOLLAR             = 20,
  TAG_KEY_5_PERCENT            = 21,
  TAG_KEY_6_CARET              = 22,
  TAG_KEY_7_AMPERSAND          = 23,
  TAG_KEY_8_ASTERISK           = 24,
  TAG_KEY_9_LPARENTHESIS       = 25,
  TAG_KEY_0_RPARENTHESIS       = 26,
  TAG_KEY_MINUS_UNDERSCORE     = 27,
  TAG_KEY_EQUAL_PLUS           = 28,
  TAG_KEY_BACKSPACE            = 29,
  TAG_KEY_TAB                  = 30,
  TAG_KEY_Q                    = 31,
  TAG_KEY_W                    = 32,
  TAG_KEY_E                    = 33,
  TAG_KEY_R                    = 34,
  TAG_KEY_T                    = 35,
  TAG_KEY_Y                    = 36,
  TAG_KEY_U                    = 37,
  TAG_KEY_I                    = 38,
  TAG_KEY_O                    = 39,
  TAG_KEY_P                    = 40,
  TAG_KEY_LBRACKET_CURLY       = 41,
  TAG_KEY_RBRACKET_CURLY       = 42,
  TAG_KEY_BACKSLASH_PIPE       = 43,
  TAG_KEY_CAPSLOCK             = 44,
  TAG_KEY_A                    = 45,
  TAG_KEY_S                    = 46,
  TAG_KEY_D                    = 47,
  TAG_KEY_F                    = 48,
  TAG_KEY_G                    = 49,
  TAG_KEY_H                    = 50,
  TAG_KEY_J                    = 51,
  TAG_KEY_K                    = 52,
  TAG_KEY_L                    = 53,
  TAG_KEY_SEMICOLON_COLON      = 54,
  TAG_KEY_QUOTE_DOUBLEQUOTE    = 55,
  TAG_KEY_NONUS_NUMBER_TILDE   = 56,
  TAG_KEY_RETURN               = 57,
  TAG_KEY_LSHIFT               = 58,
  TAG_KEY_NONUS_BACKSLASH_PIPE = 59,
  TAG_KEY_Z                    = 60,
  TAG_KEY_X                    = 61,
  TAG_KEY_C                    = 62,
  TAG_KEY_V                    = 63,
  TAG_KEY_B                    = 64,
  TAG_KEY_N                    = 65,
  TAG_KEY_M                    = 66,
  TAG_KEY_COMMA_LESS           = 67,
  TAG_KEY_PERIOD_GREATER       = 68,
  TAG_KEY_SLASH_QUESTION       = 69,
  TAG_KEY_RSHIFT               = 70,
  TAG_KEY_LCONTROL             = 71,
  TAG_KEY_LMETA                = 72,
  TAG_KEY_LALT                 = 73,
  TAG_KEY_SPACEBAR             = 74,
  TAG_KEY_RALT                 = 75,
  TAG_KEY_RMETA                = 76,
  TAG_KEY_MENU                 = 77,
  TAG_KEY_RCONTROL             = 78,
  TAG_KEY_INSERT               = 79,
  TAG_KEY_HOME                 = 80,
  TAG_KEY_PAGEUP               = 81,
  TAG_KEY_DELETEFORWARD        = 82,
  TAG_KEY_END                  = 83,
  TAG_KEY_PAGEDOWN             = 84,
  TAG_KEY_UPARROW              = 85,
  TAG_KEY_LEFTARROW            = 86,
  TAG_KEY_DOWNARROW            = 87,
  TAG_KEY_RIGHTARROW           = 88,
  TAG_KEY_PAD_NUMLOCK_CLEAR    = 89,
  TAG_KEY_PAD_EQUALS           = 90,
  TAG_KEY_PAD_DIVIDE           = 91,
  TAG_KEY_PAD_MULTIPLY         = 92,
  TAG_KEY_PAD_MINUS            = 93,
  TAG_KEY_PAD_7_HOME           = 94,
  TAG_KEY_PAD_8_UPARROW        = 95,
  TAG_KEY_PAD_9_PAGEUP         = 96,
  TAG_KEY_PAD_PLUS             = 97,
  TAG_KEY_PAD_4_LEFTARROW      = 98,
  TAG_KEY_PAD_5                = 99,
  TAG_KEY_PAD_6_RIGHTARROW     = 100,
  TAG_KEY_PAD_COMMA            = 101,
  TAG_KEY_PAD_1_END            = 102,
  TAG_KEY_PAD_2_DOWNARROW      = 103,
  TAG_KEY_PAD_3_PAGEDOWN       = 104,
  TAG_KEY_PAD_0_INSERT         = 105,
  TAG_KEY_PAD_DECIMAL_DELETE   = 106,
  TAG_KEY_PAD_ENTER            = 107,
  TAG_KEY_F13                  = 108,
  TAG_KEY_F14                  = 109,
  TAG_KEY_F15                  = 110,
  TAG_KEY_F16                  = 111,
  TAG_KEY_F17                  = 112,
  TAG_KEY_F18                  = 113,
  TAG_KEY_F19                  = 114,
  TAG_KEY_F20                  = 115,
  TAG_KEY_F21                  = 116,
  TAG_KEY_F22                  = 117,
  TAG_KEY_F23                  = 118,
  TAG_KEY_F24                  = 119,
  TAG_KEY_INTERNATIONAL1       = 120,
  TAG_KEY_INTERNATIONAL2       = 121,
  TAG_KEY_INTERNATIONAL3       = 122,
  TAG_KEY_INTERNATIONAL4       = 123,
  TAG_KEY_INTERNATIONAL5       = 124,
  TAG_KEY_INTERNATIONAL6       = 125,
  TAG_KEY_INTERNATIONAL7       = 126,
  TAG_KEY_INTERNATIONAL8       = 127,
  TAG_KEY_INTERNATIONAL9       = 128,
  TAG_KEY_LANG1                = 129,
  TAG_KEY_LANG2                = 130,
  TAG_KEY_LANG3                = 131,
  TAG_KEY_LANG4                = 132,
  TAG_KEY_LANG5                = 133,
  TAG_KEY_HELP                 = 134,
  TAG_KEY_MUTE                 = 135,
  TAG_KEY_VOLUMEUP             = 136,
  TAG_KEY_VOLUMEDOWN           = 137
} key_tag_t;

typedef enum key_state_tag_t {
  TAG_KEY_UP   = 0,
  TAG_KEY_DOWN = 1
} key_state_tag_t;

typedef enum button_tag_t {
  TAG_BUTTON_NONE       = 0,
  TAG_BUTTON_LEFT       = 1,
  TAG_BUTTON_MIDDLE     = 2,
  TAG_BUTTON_RIGHT      = 3,
  TAG_BUTTON_WHEEL_UP   = 4,
  TAG_BUTTON_WHEEL_DOWN = 5
} button_tag_t;

typedef enum button_state_tag_t {
  TAG_BUTTON_UP   = 0,
  TAG_BUTTON_DOWN = 1
} button_state_tag_t;

typedef enum event_tag_t {
  TAG_FRAME          = 0,
  TAG_CANVAS_FOCUSED = 1,
  TAG_CANVAS_RESIZED = 2,
  TAG_CANVAS_MOVED   = 3,
  TAG_CANVAS_CLOSED  = 4,
  TAG_KEY_ACTION     = 5,
  TAG_BUTTON_ACTION  = 6,
  TAG_MOUSE_MOVE     = 7
} event_tag_t;

typedef enum style_tag_t {
  TAG_COLOR    = 0,
  TAG_GRADIENT = 1
} style_tag_t;

typedef enum line_join_tag_t {
  TAG_ROUND = 0,
  TAG_MITER = 1,
  TAG_BEVEL = 2
} line_join_tag_t;

typedef enum line_cap_tag_t {
  TAG_CAP_BUTT   = 0,
  TAG_CAP_SQUARE = 1,
  TAG_CAP_ROUND  = 2
} line_cap_tag_t;

typedef enum backend_tag_t {
  TAG_CANVAS  = 0,
  TAG_GDI     = 1,
  TAG_QUARTZ  = 2,
  TAG_X11     = 3,
  TAG_WAYLAND = 4
} backend_tag_t;

#endif /* __ML_TAGS_H */

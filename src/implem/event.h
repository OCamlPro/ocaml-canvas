/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __EVENT_H
#define __EVENT_H

#include <stdint.h>
#include <stdbool.h>

#include "present_data.h"

#ifdef HAS_GDI
#undef MOD_SHIFT
#undef MOD_ALT
#endif

typedef enum {
  EVENT_NULL      = 0, // internal use
  EVENT_FRAME     = 1,
  EVENT_FOCUS     = 2,
  EVENT_RESIZE    = 3,
  EVENT_MOVE      = 4,
  EVENT_CLOSE     = 5,
  EVENT_KEY       = 6,
  EVENT_BUTTON    = 7,
  EVENT_CURSOR    = 8,

  EVENT_PRESENT = 64 // internal event
} event_type_t;

typedef enum {
  FOCUS_OUT = 0,
  FOCUS_IN  = 1
} focus_in_out_t;

/* Platform independent keycodes, identical to USB Keyboard standard */
/* Assuming an ANSI keyboard layout (with ISO/JIS extensions) */
/* https://www.usb.org/sites/default/files/documents/hut1_12v2.pdf */
/* https://www.w3.org/TR/uievents-code */
/* https://developer.apple.com/documentation/appkit/1535851-function-key_unicodes?language=objc */
typedef enum {
  KEY_UNDEFINED = 0x03, /* to handle translation errors */

  /* Function */
  KEY_ESCAPE = 0x29,
  KEY_F1 = 0x3A,
  KEY_F2 = 0x3B,
  KEY_F3 = 0x3C,
  KEY_F4 = 0x3D,
  KEY_F5 = 0x3E,
  KEY_F6 = 0x3F,
  KEY_F7 = 0x40,
  KEY_F8 = 0x41,
  KEY_F9 = 0x42,
  KEY_F10 = 0x43,
  KEY_F11 = 0x44,
  KEY_F12 = 0x45,
  KEY_PRINTSCREEN = 0x46,
  KEY_SCROLLLOCK = 0x47,
  KEY_PAUSE = 0x48,

  /* Alphanumeric, first row */
  KEY_GRAVE_TILDE = 0x35, /* possibly Zenkaku/Hankaku key on Japanese KB */
  KEY_1_EXCLAMATION = 0x1E,
  KEY_2_AT = 0x1F,
  KEY_3_NUMBER = 0x20,
  KEY_4_DOLLAR = 0x21,
  KEY_5_PERCENT = 0x22,
  KEY_6_CARET = 0x23,
  KEY_7_AMPERSAND = 0x24,
  KEY_8_ASTERISK = 0x25,
  KEY_9_LPARENTHESIS = 0x26,
  KEY_0_RPARENTHESIS = 0x27,
  KEY_MINUS_UNDERSCORE = 0x2D,
  KEY_EQUAL_PLUS = 0x2E,
  KEY_BACKSPACE = 0x2A, /* also called Delete */

  /* Alphanumeric, second row */
  KEY_TAB = 0x2B,
  KEY_Q = 0x14,
  KEY_W = 0x1A,
  KEY_E = 0x08,
  KEY_R = 0x15,
  KEY_T = 0x17,
  KEY_Y = 0x1C,
  KEY_U = 0x18,
  KEY_I = 0x0C,
  KEY_O = 0x12,
  KEY_P = 0x13,
  KEY_LBRACKET_CURLY = 0x2F,
  KEY_RBRACKET_CURLY = 0x30,
  KEY_BACKSLASH_PIPE = 0x31, /* replaced by KEY_NONUS_NUMBER_TILDE on ISO KB */

  /* Alphanumeric, third row */
  KEY_CAPSLOCK = 0x39, /* also Eisu (英数) on JIS keyboards */
  KEY_A = 0x04,
  KEY_S = 0x16,
  KEY_D = 0x07,
  KEY_F = 0x09,
  KEY_G = 0x0A,
  KEY_H = 0x0B,
  KEY_J = 0x0D,
  KEY_K = 0x0E,
  KEY_L = 0x0F,
  KEY_SEMICOLON_COLON = 0x33,
  KEY_QUOTE_DOUBLEQUOTE = 0x34,
  KEY_NONUS_NUMBER_TILDE = 0x32, /* extra key left of Return on ISO KB */
  KEY_RETURN = 0x28, /* also called Enter */

  /* Alphanumeric, fourth row */
  KEY_LSHIFT = 0xE1,
  KEY_NONUS_BACKSLASH_PIPE = 0x64, /* extra key right of LShift on ISO KB */
  KEY_Z = 0x1D,
  KEY_X = 0x1B,
  KEY_C = 0x06,
  KEY_V = 0x19,
  KEY_B = 0x05,
  KEY_N = 0x11,
  KEY_M = 0x10,
  KEY_COMMA_LESS = 0x36,
  KEY_PERIOD_GREATER = 0x37,
  KEY_SLASH_QUESTION = 0x38,
  KEY_RSHIFT = 0xE5,

  /* Alphanumeric, fifth row */
  KEY_LCONTROL = 0xE0,
  KEY_LMETA = 0xE3, /* Left Windows, Left Command */
  KEY_LALT = 0xE2,
  KEY_SPACEBAR = 0x2C,
  KEY_RALT = 0xE6,
  KEY_RMETA = 0xE7, /* Right Windows, Right Command */
  KEY_MENU = 0x76,
  KEY_RCONTROL = 0xE4,

  /* Control pad */
  KEY_INSERT = 0x49, /* replaced by a Fn key on Mac (different code) */
  KEY_HOME = 0x4A,
  KEY_PAGEUP = 0x4B,
  KEY_DELETEFORWARD = 0x4C,
  KEY_END = 0x4D,
  KEY_PAGEDOWN = 0x4E,

  /* Arrow pad */
  KEY_UPARROW = 0x52,
  KEY_LEFTARROW = 0x50,
  KEY_DOWNARROW = 0x51,
  KEY_RIGHTARROW = 0x4F,

  /* Numeric pad */
  KEY_PAD_NUMLOCK_CLEAR = 0x53, /* no NumLock on Mac keyboards */
  KEY_PAD_EQUALS = 0x67, /* on Mac keyboards */
  KEY_PAD_DIVIDE = 0x54,
  KEY_PAD_MULTIPLY = 0x55,
  KEY_PAD_MINUS = 0x56,
  KEY_PAD_7_HOME = 0x5F,
  KEY_PAD_8_UPARROW = 0x60,
  KEY_PAD_9_PAGEUP = 0x61,
  KEY_PAD_PLUS = 0x57,
  KEY_PAD_4_LEFTARROW = 0x5C,
  KEY_PAD_5 = 0x5D,
  KEY_PAD_6_RIGHTARROW = 0x5E,
  KEY_PAD_COMMA = 0x85, /* specific to Brazilian keyboards */
  KEY_PAD_1_END = 0x59,
  KEY_PAD_2_DOWNARROW = 0x5A,
  KEY_PAD_3_PAGEDOWN = 0x5B,
  KEY_PAD_0_INSERT = 0x62,
  KEY_PAD_DECIMAL_DELETE = 0x63,
  KEY_PAD_ENTER = 0x58,
  // AS/400 keyboards also have an equals sign at 0x86

  /* Extra function keys */
  KEY_F13 = 0x68,
  KEY_F14 = 0x69,
  KEY_F15 = 0x6A,
  KEY_F16 = 0x6B,
  KEY_F17 = 0x6C,
  KEY_F18 = 0x6D,
  KEY_F19 = 0x6E,
  KEY_F20 = 0x6F,
  KEY_F21 = 0x70,
  KEY_F22 = 0x71,
  KEY_F23 = 0x72,
  KEY_F24 = 0x73,

  /* International & LANG keys */
  KEY_INTERNATIONAL1 = 0x87, /* _ and \ key left of RShift on JIS KB */
                             /* also Brazilian / and ? key */
  KEY_INTERNATIONAL2 = 0x88, /* Katakana/Hiragana key right of Space on JIS KB */
  KEY_INTERNATIONAL3 = 0x89, /* | and Yen key left of Backspace on JIS KB */
  KEY_INTERNATIONAL4 = 0x8A, /* Henkan key right of Space on JIS KB + 前候補 */
  KEY_INTERNATIONAL5 = 0x8B, /* Muhenkan key left of Space on JIS KB */
  KEY_INTERNATIONAL6 = 0x8C, /* Kanma (comma), right of KP0 on JIS KB */
  KEY_INTERNATIONAL7 = 0x8D, /* Double-Byte/Single-Byte toggle key */
  KEY_INTERNATIONAL8 = 0x8E, /* Undefined */
  KEY_INTERNATIONAL9 = 0x8F, /* Undefined */
  KEY_LANG1 = 0x90, /* Hangul/English toggle key (Korean) */
  KEY_LANG2 = 0x91, /* Hanja conversion key (Korean) */
  KEY_LANG3 = 0x92, /* Katakana key (Japanese) */
  KEY_LANG4 = 0x93, /* Hiragana key (Japanese) */
  KEY_LANG5 = 0x94, /* Zenkaku/Hankaku key (Japanese) */

  /* Extensions (0x74-0x86), mostly obsolete */
  KEY_HELP = 0x75,
  KEY_MUTE = 0x7F,
  KEY_VOLUMEUP = 0x80,
  KEY_VOLUMEDOWN = 0x81

  /* Extensions (0x99-0xDD), mostly obsolete */

// 65 Keyboard Application (10)
// 66 Keyboard Power (9)
// mac : eject button between f12 and f13 is not a key but an OSC
// function key on bottom left ?

// ANSI TKL : 87
// ISO TKL : 88
// ANSI + KP17 : 104
// ANSI + KP18 : 105
// ISO + KP17 : 105
// ISO = KP18 : 106
// ANSI 75% : 82 keys
// ANSI 60% : 61 keys
} key_code_t;

typedef enum {
  KEY_UP   = 0,
  KEY_DOWN = 1
} key_state_t;

typedef enum {
  MOD_NONE     = 0,
  MOD_SHIFT    = 1,
  MOD_ALT      = 2, /* aka Option */
  MOD_CTRL     = 4,
  MOD_META     = 8, /* aka Command */
  MOD_CAPSLOCK = 16,
  MOD_NUMLOCK  = 32
} key_modifier_t;

typedef enum {
  BUTTON_NONE       = 0,
  BUTTON_LEFT       = 1,
  BUTTON_MIDDLE     = 2,
  BUTTON_RIGHT      = 3,
  BUTTON_WHEEL_UP   = 4,
  BUTTON_WHEEL_DOWN = 5
} button_t;

typedef enum {
  BUTTON_UP   = 0,
  BUTTON_DOWN = 1
} button_state_t;

/*
X11 buttons
1 = left button
2 = middle button (pressing the scroll wheel)
3 = right button
4 = turn scroll wheel up
5 = turn scroll wheel down
6 = push scroll wheel left
7 = push scroll wheel right
8 = 4th button (aka browser backward button)
9 = 5th button (aka browser forward button)
*/

typedef struct {
  void *dummy;
} event_null_t;

typedef struct {
  void *dummy;
} event_frame_t;

typedef struct {
  focus_in_out_t inout;
} event_focus_t;

typedef struct {
  uint32_t width;
  uint32_t height;
} event_resize_t;

typedef struct {
  int32_t x;
  int32_t y;
} event_move_t;

typedef struct {
  key_code_t code; // physical key (abstract between the different platforms)
  uint32_t char_; // UTF-32 character
  bool dead; // whether it is a dead key
  key_modifier_t modifiers; // current state of modifiers
  key_state_t state; // pressed or released ?
} event_key_t;

typedef struct {
  uint32_t x; // int32 ?
  uint32_t y; // int32 ?
  button_t button;
  button_state_t state;
} event_button_t;

typedef struct {
  uint32_t x; // int32 ?
  uint32_t y; // int32 ?
} event_cursor_t;

typedef struct {
  present_data_t data;
} event_present_t;

typedef union {
  event_null_t null;
  event_frame_t frame;
  event_focus_t focus;
  event_resize_t resize;
  event_move_t move;
  event_key_t key;
  event_button_t button;
  event_cursor_t cursor;
  event_present_t present;
} event_desc_t;

typedef struct {
  event_type_t type;
  event_desc_t desc;
  uint64_t time;
  void *target;
} event_t;

typedef struct event_listener_t event_listener_t;

typedef bool event_processor_t(event_t *e, event_listener_t *l);

struct event_listener_t {
  event_processor_t *process_event;
  event_listener_t *next_listener;
};

bool
event_notify(
  event_listener_t *event_listener,
  event_t *event);

#endif /* __EVENT_H */

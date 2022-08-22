/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifdef HAS_WAYLAND

#include "wl_keyboard.h"

key_code_t
wl_translate_sym_to_keycode(
  uint8_t sym)
{
  // TODO: Complete the list / Test on other keyboards
  static const key_code_t map[256] = {

    [  1] = KEY_ESCAPE,

    [ 59] = KEY_F1,
    [ 60] = KEY_F2,
    [ 61] = KEY_F3,
    [ 62] = KEY_F4,
    [ 63] = KEY_F5,
    [ 64] = KEY_F6,
    [ 65] = KEY_F7,
    [ 66] = KEY_F8,
    [ 67] = KEY_F9,
    [ 68] = KEY_F10,
    [ 87] = KEY_F11,
    [ 88] = KEY_F12,

    [ 99] = KEY_PRINTSCREEN,
    [119] = KEY_PAUSE,
    [ 41] = KEY_GRAVE_TILDE,

    [  2] = KEY_1_EXCLAMATION,
    [  3] = KEY_2_AT,
    [  4] = KEY_3_NUMBER,
    [  5] = KEY_4_DOLLAR,
    [  6] = KEY_5_PERCENT,
    [  7] = KEY_6_CARET,
    [  8] = KEY_7_AMPERSAND,
    [  9] = KEY_8_ASTERISK,
    [ 10] = KEY_9_LPARENTHESIS,
    [ 11] = KEY_0_RPARENTHESIS,
    [ 12] = KEY_MINUS_UNDERSCORE,
    [ 13] = KEY_EQUAL_PLUS,
    [ 14] = KEY_BACKSPACE,

    [ 15] = KEY_TAB,
    [ 16] = KEY_Q,
    [ 17] = KEY_W,
    [ 18] = KEY_E,
    [ 19] = KEY_R,
    [ 20] = KEY_T,
    [ 21] = KEY_Y,
    [ 22] = KEY_U,
    [ 23] = KEY_I,
    [ 24] = KEY_O,
    [ 25] = KEY_P,
    [ 26] = KEY_LBRACKET_CURLY,
    [ 27] = KEY_RBRACKET_CURLY,

    [ 43] = KEY_BACKSLASH_PIPE,

    [ 58] = KEY_CAPSLOCK,

    [ 30] = KEY_A,
    [ 31] = KEY_S,
    [ 32] = KEY_D,
    [ 33] = KEY_F,
    [ 34] = KEY_G,
    [ 35] = KEY_H,
    [ 36] = KEY_J,
    [ 37] = KEY_K,
    [ 38] = KEY_L,
    [ 39] = KEY_SEMICOLON_COLON,
    [ 40] = KEY_QUOTE_DOUBLEQUOTE,

    [ 28] = KEY_RETURN,
    [ 42] = KEY_LSHIFT,

    [ 44] = KEY_Z,
    [ 45] = KEY_X,
    [ 46] = KEY_C,
    [ 47] = KEY_V,
    [ 48] = KEY_B,
    [ 49] = KEY_N,
    [ 50] = KEY_M,
    [ 51] = KEY_COMMA_LESS,
    [ 52] = KEY_PERIOD_GREATER,
    [ 53] = KEY_SLASH_QUESTION,

    [ 54] = KEY_RSHIFT,
    [ 29] = KEY_LCONTROL,
    [ 56] = KEY_LALT,
    [ 57] = KEY_SPACEBAR,
    [100] = KEY_RALT,
    [127] = KEY_MENU,
    [ 97] = KEY_RCONTROL,
    [110] = KEY_INSERT,
    [102] = KEY_HOME,
    [104] = KEY_PAGEUP,
    [111] = KEY_DELETEFORWARD,
    [107] = KEY_END,
    [109] = KEY_PAGEDOWN,
    [103] = KEY_UPARROW,
    [105] = KEY_LEFTARROW,
    [108] = KEY_DOWNARROW,
    [106] = KEY_RIGHTARROW
  };

  return map[sym];
}

#endif /* HAS_WAYLAND */

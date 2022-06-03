/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifdef HAS_GDI

#include <stdint.h>
#include <stdbool.h>

#include <windows.h>

#include "../event.h"

key_code_t
gdi_keyboard_translate_scancode(
  uint16_t scancode,
  bool extended)
{
  static const key_code_t map[128] = {

    [0x00] = -1,

    [0x01] = KEY_ESCAPE,

    [0x02] = KEY_1_EXCLAMATION,
    [0x03] = KEY_2_AT,
    [0x04] = KEY_3_NUMBER,
    [0x05] = KEY_4_DOLLAR,
    [0x06] = KEY_5_PERCENT,
    [0x07] = KEY_6_CARET,
    [0x08] = KEY_7_AMPERSAND,
    [0x09] = KEY_8_ASTERISK,
    [0x0A] = KEY_9_LPARENTHESIS,
    [0x0B] = KEY_0_RPARENTHESIS,
    [0x0C] = KEY_MINUS_UNDERSCORE,
    [0x0D] = KEY_EQUAL_PLUS,
    [0x0E] = KEY_BACKSPACE,

    [0x0F] = KEY_TAB,
    [0x10] = KEY_Q,
    [0x11] = KEY_W,
    [0x12] = KEY_E,
    [0x13] = KEY_R,
    [0x14] = KEY_T,
    [0x15] = KEY_Y,
    [0x16] = KEY_U,
    [0x17] = KEY_I,
    [0x18] = KEY_O,
    [0x19] = KEY_P,
    [0x1A] = KEY_LBRACKET_CURLY,
    [0x1B] = KEY_RBRACKET_CURLY,
    [0x1C] = KEY_RETURN, // KEY_PAD_ENTER

    [0x1D] = KEY_LCONTROL, // KEY_RCONTROL
    [0x1E] = KEY_A,
    [0x1F] = KEY_S,
    [0x20] = KEY_D,
    [0x21] = KEY_F,
    [0x22] = KEY_G,
    [0x23] = KEY_H,
    [0x24] = KEY_J,
    [0x25] = KEY_K,
    [0x26] = KEY_L,
    [0x27] = KEY_SEMICOLON_COLON,
    [0x28] = KEY_QUOTE_DOUBLEQUOTE,
    [0x29] = KEY_GRAVE_TILDE,

    [0x2A] = KEY_LSHIFT,
    [0x2B] = KEY_BACKSLASH_PIPE, /* or KEY_NONUS_NUMBER_TILDE */
    [0x2C] = KEY_Z,
    [0x2D] = KEY_X,
    [0x2E] = KEY_C,
    [0x2F] = KEY_V,
    [0x30] = KEY_B,
    [0x31] = KEY_N,
    [0x32] = KEY_M,
    [0x33] = KEY_COMMA_LESS,
    [0x34] = KEY_PERIOD_GREATER,
    [0x35] = KEY_SLASH_QUESTION, // KEY_PAD_DIVIDE
    [0x36] = KEY_RSHIFT,

    [0x37] = KEY_PAD_MULTIPLY, // KEY_PRINTSCREEN

    [0x38] = KEY_LALT, // KEY_RALT
    [0x39] = KEY_SPACEBAR,

    [0x3A] = KEY_CAPSLOCK,

    [0x3B] = KEY_F1,
    [0x3C] = KEY_F2,
    [0x3D] = KEY_F3,
    [0x3E] = KEY_F4,
    [0x3F] = KEY_F5,
    [0x40] = KEY_F6,
    [0x41] = KEY_F7,
    [0x42] = KEY_F8,
    [0x43] = KEY_F9,
    [0x44] = KEY_F10,

    [0x45] = KEY_PAUSE, // KEY_PAD_NUMLOCK_CLEAR,
    [0x46] = KEY_SCROLLLOCK,

    [0x47] = KEY_PAD_7_HOME, // KEY_HOME
    [0x48] = KEY_PAD_8_UPARROW, // KEY_UPARROW
    [0x49] = KEY_PAD_9_PAGEUP, // KEY_PAGEUP
    [0x4A] = KEY_PAD_MINUS,
    [0x4B] = KEY_PAD_4_LEFTARROW, // KEY_LEFTARROW
    [0x4C] = KEY_PAD_5,
    [0x4D] = KEY_PAD_6_RIGHTARROW, // KEY_RIGHTARROW
    [0x4E] = KEY_PAD_PLUS,
    [0x4F] = KEY_PAD_1_END, // KEY_END
    [0x50] = KEY_PAD_2_DOWNARROW, // KEY_DOWNARROW
    [0x51] = KEY_PAD_3_PAGEDOWN, // KEY_PAGEDOWN
    [0x52] = KEY_PAD_0_INSERT, // KEY_INSERT
    [0x53] = KEY_PAD_DECIMAL_DELETE, // KEY_DELETEFORWARD

    [0x54] = -1,
    [0x55] = -1,

    [0x56] = KEY_NONUS_BACKSLASH_PIPE,

    [0x57] = KEY_F11,
    [0x58] = KEY_F12,
    [0x59] = KEY_PAUSE,

    [0x5A] = -1,

    [0x5B] = -1, // KEY_LMETA
    [0x5C] = -1, // KEY_RMETA
    [0x5D] = -1, // KEY_MENU

    [0x5E] = -1,
    [0x5F] = -1,

    [0x60] = -1,
    [0x61] = -1,
    [0x62] = -1,
    [0x63] = -1,

    [0x64] = KEY_F13,
    [0x65] = KEY_F14,
    [0x66] = KEY_F15,
    [0x67] = KEY_F16,
    [0x68] = KEY_F17,
    [0x69] = KEY_F18,
    [0x6A] = KEY_F19,

    [0x6B] = -1,
    [0x6C] = -1,
    [0x6D] = -1,
    [0x6E] = -1,
    [0x6F] = -1,

    [0x70] = KEY_INTERNATIONAL2, /* Katakana/Hiragana key right of Space on JIS KB */

    [0x71] = -1,
    [0x72] = -1,

    [0x73] = KEY_INTERNATIONAL1, /* _ and \ key left of RShift on JIS KB */

    [0x74] = -1,
    [0x75] = -1,
    [0x76] = -1,
    [0x77] = -1,
    [0x78] = -1,

    [0x79] = KEY_INTERNATIONAL4, /* Henkan key right of Space on JIS KB */

    [0x7A] = -1,

    [0x7B] = KEY_INTERNATIONAL5, /* Muhenkan key left of Space on JIS KB */

    [0x7C] = -1,

    [0x7D] = KEY_INTERNATIONAL3, /* | and Yen key left of Backspace on JIS KB */

    [0x7E] = -1,
    [0x7F] = -1,

/*
    [kVK_ANSI_KeypadEquals] = KEY_PAD_EQUALS,
    [kVK_VolumeUp] = KEY_VOLUMEUP,
    [kVK_VolumeDown] = KEY_VOLUMEDOWN,
    [kVK_Mute] = KEY_MUTE,
    [kVK_Help] = KEY_HELP,
    [kVK_JIS_KeypadComma] = KEY_INTERNATIONAL6,
    [kVK_JIS_Eisu] = KEY_LANG2, // or KEY_INTERNATIONAL2
    [kVK_JIS_Kana] = KEY_LANG1, // or KEY_INTERNATIONAL2
*/
  };

  if (extended) {
    switch (scancode) {
      case 0x1C: return KEY_PAD_ENTER;
      case 0x1D: return KEY_RCONTROL;
      case 0x35: return KEY_PAD_DIVIDE;
      case 0x37: return KEY_PRINTSCREEN;
      case 0x38: return KEY_RALT; // Although it always comes with Ctrl
      case 0x45: return KEY_PAD_NUMLOCK_CLEAR; // Only fired before other keys
      case 0x47: return KEY_HOME;
      case 0x48: return KEY_UPARROW;
      case 0x49: return KEY_PAGEUP;
      case 0x4B: return KEY_LEFTARROW;
      case 0x4D: return KEY_RIGHTARROW;
      case 0x4F: return KEY_END;
      case 0x50: return KEY_DOWNARROW;
      case 0x51: return KEY_PAGEDOWN;
      case 0x52: return KEY_INSERT;
      case 0x53: return KEY_DELETEFORWARD;
      case 0x5B: return KEY_LMETA;
      case 0x5C: return KEY_RMETA;
      case 0x5D: return KEY_MENU;
      default: return map[scancode]; // assert(!"Unknown extended key");
    }
  } else {
    return map[scancode];
  }
}

int32_t
gdi_keyboard_scancode_to_unicode(
  uint32_t virtkey,
  uint16_t scancode,
  bool *out_dead)
{
  WCHAR chars[4];
  int nb_chars = 0;
  BYTE state[256] = { 0 };
  GetKeyboardState(state);
  nb_chars = ToUnicode(virtkey, scancode, state, chars, 4, 0);
  if (out_dead != NULL) {
    *out_dead = (nb_chars < 0 || nb_chars > 1);
  }
  return chars[0];
}

#else

const int gdi_keyboard = 0;

#endif /* HAS_GDI */

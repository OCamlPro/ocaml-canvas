/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifdef HAS_QUARTZ

#include <stdint.h>
#include <stdbool.h>

#include <Cocoa/Cocoa.h>
#include <Carbon/Carbon.h>

#include "../event.h"
#include "qtz_backend_internal.h"

key_code_t
qtz_keyboard_translate_keycode(
  uint16_t keycode)
{
  /* /System/Library/Frameworks/Carbon.framework/Versions/A
       /Frameworks/HIToolbox.framework/Versions/A/Headers/Events.h */
  static const key_code_t map[128] = {

    [kVK_ANSI_1] = KEY_1_EXCLAMATION,
    [kVK_ANSI_2] = KEY_2_AT,
    [kVK_ANSI_3] = KEY_3_NUMBER,
    [kVK_ANSI_4] = KEY_4_DOLLAR,
    [kVK_ANSI_5] = KEY_5_PERCENT,
    [kVK_ANSI_6] = KEY_6_CARET,
    [kVK_ANSI_7] = KEY_7_AMPERSAND,
    [kVK_ANSI_8] = KEY_8_ASTERISK,
    [kVK_ANSI_9] = KEY_9_LPARENTHESIS,
    [kVK_ANSI_0] = KEY_0_RPARENTHESIS,

    [kVK_ANSI_A] = KEY_A,
    [kVK_ANSI_B] = KEY_B,
    [kVK_ANSI_C] = KEY_C,
    [kVK_ANSI_D] = KEY_D,
    [kVK_ANSI_E] = KEY_E,
    [kVK_ANSI_F] = KEY_F,
    [kVK_ANSI_G] = KEY_G,
    [kVK_ANSI_H] = KEY_H,
    [kVK_ANSI_I] = KEY_I,
    [kVK_ANSI_J] = KEY_J,
    [kVK_ANSI_K] = KEY_K,
    [kVK_ANSI_L] = KEY_L,
    [kVK_ANSI_M] = KEY_M,
    [kVK_ANSI_N] = KEY_N,
    [kVK_ANSI_O] = KEY_O,
    [kVK_ANSI_P] = KEY_P,
    [kVK_ANSI_Q] = KEY_Q,
    [kVK_ANSI_R] = KEY_R,
    [kVK_ANSI_S] = KEY_S,
    [kVK_ANSI_T] = KEY_T,
    [kVK_ANSI_U] = KEY_U,
    [kVK_ANSI_V] = KEY_V,
    [kVK_ANSI_W] = KEY_W,
    [kVK_ANSI_X] = KEY_X,
    [kVK_ANSI_Y] = KEY_Y,
    [kVK_ANSI_Z] = KEY_Z,

    [kVK_ANSI_Equal] = KEY_EQUAL_PLUS,
    [kVK_ANSI_Minus] = KEY_MINUS_UNDERSCORE,
    [kVK_ANSI_RightBracket] = KEY_RBRACKET_CURLY,
    [kVK_ANSI_LeftBracket] = KEY_LBRACKET_CURLY,
    [kVK_ANSI_Quote] = KEY_QUOTE_DOUBLEQUOTE,
    [kVK_ANSI_Semicolon] = KEY_SEMICOLON_COLON,
    [kVK_ANSI_Backslash] = KEY_BACKSLASH_PIPE, /* or KEY_NONUS_NUMBER_TILDE */
    [kVK_ANSI_Comma] = KEY_COMMA_LESS,
    [kVK_ANSI_Slash] = KEY_SLASH_QUESTION,
    [kVK_ANSI_Period] = KEY_PERIOD_GREATER,
    [kVK_ANSI_Grave] = KEY_NONUS_BACKSLASH_PIPE,

    [kVK_ANSI_KeypadDecimal] = KEY_PAD_DECIMAL_DELETE,
    [kVK_ANSI_KeypadMultiply] = KEY_PAD_MULTIPLY,
    [kVK_ANSI_KeypadPlus] = KEY_PAD_PLUS,
    [kVK_ANSI_KeypadClear] = KEY_PAD_NUMLOCK_CLEAR,
    [kVK_ANSI_KeypadDivide] = KEY_PAD_DIVIDE,
    [kVK_ANSI_KeypadEnter] = KEY_PAD_ENTER,
    [kVK_ANSI_KeypadMinus] = KEY_PAD_MINUS,
    [kVK_ANSI_KeypadEquals] = KEY_PAD_EQUALS,
    [kVK_ANSI_Keypad0] = KEY_PAD_0_INSERT,
    [kVK_ANSI_Keypad1] = KEY_PAD_1_END,
    [kVK_ANSI_Keypad2] = KEY_PAD_2_DOWNARROW,
    [kVK_ANSI_Keypad3] = KEY_PAD_3_PAGEDOWN,
    [kVK_ANSI_Keypad4] = KEY_PAD_4_LEFTARROW,
    [kVK_ANSI_Keypad5] = KEY_PAD_5,
    [kVK_ANSI_Keypad6] = KEY_PAD_6_RIGHTARROW,
    [kVK_ANSI_Keypad7] = KEY_PAD_7_HOME,
    [kVK_ANSI_Keypad8] = KEY_PAD_8_UPARROW,
    [kVK_ANSI_Keypad9] = KEY_PAD_9_PAGEUP,

    [kVK_F1] = KEY_F1,
    [kVK_F2] = KEY_F2,
    [kVK_F3] = KEY_F3,
    [kVK_F4] = KEY_F4,
    [kVK_F5] = KEY_F5,
    [kVK_F6] = KEY_F6,
    [kVK_F7] = KEY_F7,
    [kVK_F8] = KEY_F8,
    [kVK_F9] = KEY_F9,
    [kVK_F10] = KEY_F10,
    [kVK_F11] = KEY_F11,
    [kVK_F12] = KEY_F12,
    [kVK_F13] = KEY_F13,
    [kVK_F14] = KEY_F14,
    [kVK_F15] = KEY_F15,
    [kVK_F16] = KEY_F16,
    [kVK_F17] = KEY_F17,
    [kVK_F18] = KEY_F18,

    [kVK_Return] = KEY_RETURN,
    [kVK_Tab] = KEY_TAB,
    [kVK_Space] = KEY_SPACEBAR,
    [kVK_Delete] = KEY_BACKSPACE,
    [kVK_Escape] = KEY_ESCAPE,
    [kVK_Command] = KEY_LMETA,
    [kVK_Shift] = KEY_LSHIFT,
    [kVK_CapsLock] = KEY_CAPSLOCK,
    [kVK_Option] = KEY_LALT,
    [kVK_Control] = KEY_LCONTROL,
    [kVK_RightCommand] = KEY_RMETA,
    [kVK_RightShift] = KEY_RSHIFT,
    [kVK_RightOption] = KEY_RALT,
    [kVK_RightControl] = KEY_RCONTROL,
    [kVK_Function] = -1, /* don't know */
    [kVK_VolumeUp] = KEY_VOLUMEUP,
    [kVK_VolumeDown] = KEY_VOLUMEDOWN,
    [kVK_Mute] = KEY_MUTE,
    [kVK_Help] = KEY_HELP,
    [kVK_Home] = KEY_HOME,
    [kVK_PageUp] = KEY_PAGEUP,
    [kVK_ForwardDelete] = KEY_DELETEFORWARD,
    [kVK_End] = KEY_END,
    [kVK_PageDown] = KEY_PAGEDOWN,
    [kVK_LeftArrow] = KEY_LEFTARROW,
    [kVK_RightArrow] = KEY_RIGHTARROW,
    [kVK_DownArrow] = KEY_DOWNARROW,
    [kVK_UpArrow] = KEY_UPARROW,

    [kVK_ISO_Section] = KEY_GRAVE_TILDE,

    [kVK_JIS_Yen] = KEY_INTERNATIONAL3,
    [kVK_JIS_Underscore] = KEY_INTERNATIONAL1,
    [kVK_JIS_KeypadComma] = KEY_INTERNATIONAL6,
    [kVK_JIS_Eisu] = KEY_LANG2, /* or KEY_INTERNATIONAL2 */
    [kVK_JIS_Kana] = KEY_LANG1, /* or KEY_INTERNATIONAL2 */

    /* Undocumened (missing from Events.h) */
    [0x34] = -1,
    [0x42] = -1,
    [0x44] = -1,
    [0x46] = -1,
    [0x4D] = -1,
    [0x6C] = -1,
    [0x6E] = KEY_MENU,
    [0x70] = -1,
    [0x7F] = -1,

    /* Unmapped keys */
    //KEY_INSERT
    //KEY_PRINTSCREEN
    //KEY_SCROLLLOCK - test with F-Mode + Inser (works on Windows)
    //KEY_PAUSE

    // With a PC keyboard:
    // F5 = Cmd + r
    // Home = Cmd + LeftArrow
    // End = Cmd + RightArrow

  };

  return map[keycode];
}

int32_t
qtz_keyboard_keycode_to_unicode(
  uint16_t keycode,
  NSEventModifierFlags modifiers,
  bool *out_dead)
{
  assert(qtz_back != NULL);

  UniChar chars[4]; /* = uint16_t */
  UniCharCount nb_chars = 0; /* = uint32_t */
  UInt32 state = 0;
  OSStatus status =
    UCKeyTranslate(
      qtz_back->layout, keycode, kUCKeyActionDisplay,
      (modifiers & (NSEventModifierFlagShift |
                    NSEventModifierFlagOption)) >> 16,
      LMGetKbdType(), 0, &state, 4, &nb_chars, chars);
  if (out_dead != NULL) {
    *out_dead = (state != 0);
  }
  if (status == noErr && nb_chars == 1) {
    return chars[0];
  } else {
    return 0;
  }
}

#else

const int qtz_keyboard = 0;

#endif /* HAS_QUARTZ */

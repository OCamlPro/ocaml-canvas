/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include "caml/version.h"
#include "caml/mlvalues.h"
#include "caml/bigarray.h"
#include "caml/memory.h"
#include "caml/alloc.h"
#include "caml/custom.h"
#include "caml/fail.h"
#include "caml/callback.h"

#include "../implem/canvas.h"
#include "../implem/window.h"
#include "../implem/event.h"
#include "../implem/image_data.h"
#include "../implem/color.h"
#include "../implem/font_desc.h"

#include "ml_tags.h"
#include "ml_convert.h"

#if OCAML_VERSION < 41200

CAMLexport value caml_alloc_some(value v)
{
  CAMLparam1(v);
  value some = caml_alloc_small(1, 0);
  Field(some, 0) = v;
  CAMLreturn(some);
}

#endif

value
Val_int32_clip(
  int32_t i)
{
  CAMLparam0();
  CAMLlocal1(mlValue);
  if (sizeof(intnat) == 4) {
    if (i < INT32_MIN/2) {
      i = INT32_MIN/2;
    } else if (i > INT32_MAX/2) {
      i = INT32_MAX/2;
    }
  }
  mlValue = Val_long(i);
  CAMLreturn(mlValue);
}

int32_t
Int32_val_clip(
  value mlValue)
{
  CAMLparam1(mlValue);
  intnat i = Long_val(mlValue);
  if (i < INT32_MIN) {
    i = INT32_MIN;
  } else if (i > INT32_MAX) {
    i = INT32_MAX;
  }
  CAMLreturnT(int32_t, i);
}

font_slant_t
Slant_val(
  value mlSlant)
{
  CAMLparam1(mlSlant);
  static const font_slant_t map[3] = {
    [TAG_ROMAN]   = SLANT_ROMAN,
    [TAG_ITALIC]  = SLANT_ITALIC,
    [TAG_OBLIQUE] = SLANT_OBLIQUE
  };
  CAMLreturnT(font_slant_t, map[Int_val(mlSlant)]);
}

value
Val_focus_in_out(
  focus_in_out_t s)
{
  CAMLparam0();
  static intnat map[6] = {
    [FOCUS_OUT] = TAG_FOCUS_OUT,
    [FOCUS_IN]  = TAG_FOCUS_IN,
  };
  CAMLreturn(Val_int(map[s]));
}

value
Val_key_code(
  key_code_t key_code)
{
  CAMLparam0();
  static const intnat map[255] = {
    [KEY_ESCAPE]               = TAG_KEY_ESCAPE,
    [KEY_F1]                   = TAG_KEY_F1,
    [KEY_F2]                   = TAG_KEY_F2,
    [KEY_F3]                   = TAG_KEY_F3,
    [KEY_F4]                   = TAG_KEY_F4,
    [KEY_F5]                   = TAG_KEY_F5,
    [KEY_F6]                   = TAG_KEY_F6,
    [KEY_F7]                   = TAG_KEY_F7,
    [KEY_F8]                   = TAG_KEY_F8,
    [KEY_F9]                   = TAG_KEY_F9,
    [KEY_F10]                  = TAG_KEY_F10,
    [KEY_F11]                  = TAG_KEY_F11,
    [KEY_F12]                  = TAG_KEY_F12,
    [KEY_PRINTSCREEN]          = TAG_KEY_PRINTSCREEN,
    [KEY_SCROLLLOCK]           = TAG_KEY_SCROLLLOCK,
    [KEY_PAUSE]                = TAG_KEY_PAUSE,
    [KEY_GRAVE_TILDE]          = TAG_KEY_GRAVE_TILDE,
    [KEY_1_EXCLAMATION]        = TAG_KEY_1_EXCLAMATION,
    [KEY_2_AT]                 = TAG_KEY_2_AT,
    [KEY_3_NUMBER]             = TAG_KEY_3_NUMBER,
    [KEY_4_DOLLAR]             = TAG_KEY_4_DOLLAR,
    [KEY_5_PERCENT]            = TAG_KEY_5_PERCENT,
    [KEY_6_CARET]              = TAG_KEY_6_CARET,
    [KEY_7_AMPERSAND]          = TAG_KEY_7_AMPERSAND,
    [KEY_8_ASTERISK]           = TAG_KEY_8_ASTERISK,
    [KEY_9_LPARENTHESIS]       = TAG_KEY_9_LPARENTHESIS,
    [KEY_0_RPARENTHESIS]       = TAG_KEY_0_RPARENTHESIS,
    [KEY_MINUS_UNDERSCORE]     = TAG_KEY_MINUS_UNDERSCORE,
    [KEY_EQUAL_PLUS]           = TAG_KEY_EQUAL_PLUS,
    [KEY_BACKSPACE]            = TAG_KEY_BACKSPACE,
    [KEY_TAB]                  = TAG_KEY_TAB,
    [KEY_Q]                    = TAG_KEY_Q,
    [KEY_W]                    = TAG_KEY_W,
    [KEY_E]                    = TAG_KEY_E,
    [KEY_R]                    = TAG_KEY_R,
    [KEY_T]                    = TAG_KEY_T,
    [KEY_Y]                    = TAG_KEY_Y,
    [KEY_U]                    = TAG_KEY_U,
    [KEY_I]                    = TAG_KEY_I,
    [KEY_O]                    = TAG_KEY_O,
    [KEY_P]                    = TAG_KEY_P,
    [KEY_LBRACKET_CURLY]       = TAG_KEY_LBRACKET_CURLY,
    [KEY_RBRACKET_CURLY]       = TAG_KEY_RBRACKET_CURLY,
    [KEY_BACKSLASH_PIPE]       = TAG_KEY_BACKSLASH_PIPE,
    [KEY_CAPSLOCK]             = TAG_KEY_CAPSLOCK,
    [KEY_A]                    = TAG_KEY_A,
    [KEY_S]                    = TAG_KEY_S,
    [KEY_D]                    = TAG_KEY_D,
    [KEY_F]                    = TAG_KEY_F,
    [KEY_G]                    = TAG_KEY_G,
    [KEY_H]                    = TAG_KEY_H,
    [KEY_J]                    = TAG_KEY_J,
    [KEY_K]                    = TAG_KEY_K,
    [KEY_L]                    = TAG_KEY_L,
    [KEY_SEMICOLON_COLON]      = TAG_KEY_SEMICOLON_COLON,
    [KEY_QUOTE_DOUBLEQUOTE]    = TAG_KEY_QUOTE_DOUBLEQUOTE,
    [KEY_NONUS_NUMBER_TILDE]   = TAG_KEY_NONUS_NUMBER_TILDE,
    [KEY_RETURN]               = TAG_KEY_RETURN,
    [KEY_LSHIFT]               = TAG_KEY_LSHIFT,
    [KEY_NONUS_BACKSLASH_PIPE] = TAG_KEY_NONUS_BACKSLASH_PIPE,
    [KEY_Z]                    = TAG_KEY_Z,
    [KEY_X]                    = TAG_KEY_X,
    [KEY_C]                    = TAG_KEY_C,
    [KEY_V]                    = TAG_KEY_V,
    [KEY_B]                    = TAG_KEY_B,
    [KEY_N]                    = TAG_KEY_N,
    [KEY_M]                    = TAG_KEY_M,
    [KEY_COMMA_LESS]           = TAG_KEY_COMMA_LESS,
    [KEY_PERIOD_GREATER]       = TAG_KEY_PERIOD_GREATER,
    [KEY_SLASH_QUESTION]       = TAG_KEY_SLASH_QUESTION,
    [KEY_RSHIFT]               = TAG_KEY_RSHIFT,
    [KEY_LCONTROL]             = TAG_KEY_LCONTROL,
    [KEY_LMETA]                = TAG_KEY_LMETA,
    [KEY_LALT]                 = TAG_KEY_LALT,
    [KEY_SPACEBAR]             = TAG_KEY_SPACEBAR,
    [KEY_RALT]                 = TAG_KEY_RALT,
    [KEY_RMETA]                = TAG_KEY_RMETA,
    [KEY_MENU]                 = TAG_KEY_MENU,
    [KEY_RCONTROL]             = TAG_KEY_RCONTROL,
    [KEY_INSERT]               = TAG_KEY_INSERT,
    [KEY_HOME]                 = TAG_KEY_HOME,
    [KEY_PAGEUP]               = TAG_KEY_PAGEUP,
    [KEY_DELETEFORWARD]        = TAG_KEY_DELETEFORWARD,
    [KEY_END]                  = TAG_KEY_END,
    [KEY_PAGEDOWN]             = TAG_KEY_PAGEDOWN,
    [KEY_UPARROW]              = TAG_KEY_UPARROW,
    [KEY_LEFTARROW]            = TAG_KEY_LEFTARROW,
    [KEY_DOWNARROW]            = TAG_KEY_DOWNARROW,
    [KEY_RIGHTARROW]           = TAG_KEY_RIGHTARROW,
    [KEY_PAD_NUMLOCK_CLEAR]    = TAG_KEY_PAD_NUMLOCK_CLEAR,
    [KEY_PAD_EQUALS]           = TAG_KEY_PAD_EQUALS,
    [KEY_PAD_DIVIDE]           = TAG_KEY_PAD_DIVIDE,
    [KEY_PAD_MULTIPLY]         = TAG_KEY_PAD_MULTIPLY,
    [KEY_PAD_MINUS]            = TAG_KEY_PAD_MINUS,
    [KEY_PAD_7_HOME]           = TAG_KEY_PAD_7_HOME,
    [KEY_PAD_8_UPARROW]        = TAG_KEY_PAD_8_UPARROW,
    [KEY_PAD_9_PAGEUP]         = TAG_KEY_PAD_9_PAGEUP,
    [KEY_PAD_PLUS]             = TAG_KEY_PAD_PLUS,
    [KEY_PAD_4_LEFTARROW]      = TAG_KEY_PAD_4_LEFTARROW,
    [KEY_PAD_5]                = TAG_KEY_PAD_5,
    [KEY_PAD_6_RIGHTARROW]     = TAG_KEY_PAD_6_RIGHTARROW,
    [KEY_PAD_COMMA]            = TAG_KEY_PAD_COMMA,
    [KEY_PAD_1_END]            = TAG_KEY_PAD_1_END,
    [KEY_PAD_2_DOWNARROW]      = TAG_KEY_PAD_2_DOWNARROW,
    [KEY_PAD_3_PAGEDOWN]       = TAG_KEY_PAD_3_PAGEDOWN,
    [KEY_PAD_0_INSERT]         = TAG_KEY_PAD_0_INSERT,
    [KEY_PAD_DECIMAL_DELETE]   = TAG_KEY_PAD_DECIMAL_DELETE,
    [KEY_PAD_ENTER]            = TAG_KEY_PAD_ENTER,
    [KEY_F13]                  = TAG_KEY_F13,
    [KEY_F14]                  = TAG_KEY_F14,
    [KEY_F15]                  = TAG_KEY_F15,
    [KEY_F16]                  = TAG_KEY_F16,
    [KEY_F17]                  = TAG_KEY_F17,
    [KEY_F18]                  = TAG_KEY_F18,
    [KEY_F19]                  = TAG_KEY_F19,
    [KEY_F20]                  = TAG_KEY_F20,
    [KEY_F21]                  = TAG_KEY_F21,
    [KEY_F22]                  = TAG_KEY_F22,
    [KEY_F23]                  = TAG_KEY_F23,
    [KEY_F24]                  = TAG_KEY_F24,
    [KEY_INTERNATIONAL1]       = TAG_KEY_INTERNATIONAL1,
    [KEY_INTERNATIONAL2]       = TAG_KEY_INTERNATIONAL2,
    [KEY_INTERNATIONAL3]       = TAG_KEY_INTERNATIONAL3,
    [KEY_INTERNATIONAL4]       = TAG_KEY_INTERNATIONAL4,
    [KEY_INTERNATIONAL5]       = TAG_KEY_INTERNATIONAL5,
    [KEY_INTERNATIONAL6]       = TAG_KEY_INTERNATIONAL6,
    [KEY_INTERNATIONAL7]       = TAG_KEY_INTERNATIONAL7,
    [KEY_INTERNATIONAL8]       = TAG_KEY_INTERNATIONAL8,
    [KEY_INTERNATIONAL9]       = TAG_KEY_INTERNATIONAL9,
    [KEY_LANG1]                = TAG_KEY_LANG1,
    [KEY_LANG2]                = TAG_KEY_LANG2,
    [KEY_LANG3]                = TAG_KEY_LANG3,
    [KEY_LANG4]                = TAG_KEY_LANG4,
    [KEY_LANG5]                = TAG_KEY_LANG5,
    [KEY_HELP]                 = TAG_KEY_HELP,
    [KEY_MUTE]                 = TAG_KEY_MUTE,
    [KEY_VOLUMEUP]             = TAG_KEY_VOLUMEUP,
    [KEY_VOLUMEDOWN]           = TAG_KEY_VOLUMEDOWN,
  };
  intnat tag = map[key_code];
  if ((tag == TAG_KEY_ESCAPE) && (key_code != KEY_ESCAPE)) {
    caml_invalid_argument("invalid key code provided");
  }
  CAMLreturn(Val_int(tag));
}

key_code_t
Key_code_val(
  value mlKeyCode)
{
  CAMLparam1(mlKeyCode);
  static const key_code_t map[138] = {
    [TAG_KEY_ESCAPE]               = KEY_ESCAPE,
    [TAG_KEY_F1]                   = KEY_F1,
    [TAG_KEY_F2]                   = KEY_F2,
    [TAG_KEY_F3]                   = KEY_F3,
    [TAG_KEY_F4]                   = KEY_F4,
    [TAG_KEY_F5]                   = KEY_F5,
    [TAG_KEY_F6]                   = KEY_F6,
    [TAG_KEY_F7]                   = KEY_F7,
    [TAG_KEY_F8]                   = KEY_F8,
    [TAG_KEY_F9]                   = KEY_F9,
    [TAG_KEY_F10]                  = KEY_F10,
    [TAG_KEY_F11]                  = KEY_F11,
    [TAG_KEY_F12]                  = KEY_F12,
    [TAG_KEY_PRINTSCREEN]          = KEY_PRINTSCREEN,
    [TAG_KEY_SCROLLLOCK]           = KEY_SCROLLLOCK,
    [TAG_KEY_PAUSE]                = KEY_PAUSE,
    [TAG_KEY_GRAVE_TILDE]          = KEY_GRAVE_TILDE,
    [TAG_KEY_1_EXCLAMATION]        = KEY_1_EXCLAMATION,
    [TAG_KEY_2_AT]                 = KEY_2_AT,
    [TAG_KEY_3_NUMBER]             = KEY_3_NUMBER,
    [TAG_KEY_4_DOLLAR]             = KEY_4_DOLLAR,
    [TAG_KEY_5_PERCENT]            = KEY_5_PERCENT,
    [TAG_KEY_6_CARET]              = KEY_6_CARET,
    [TAG_KEY_7_AMPERSAND]          = KEY_7_AMPERSAND,
    [TAG_KEY_8_ASTERISK]           = KEY_8_ASTERISK,
    [TAG_KEY_9_LPARENTHESIS]       = KEY_9_LPARENTHESIS,
    [TAG_KEY_0_RPARENTHESIS]       = KEY_0_RPARENTHESIS,
    [TAG_KEY_MINUS_UNDERSCORE]     = KEY_MINUS_UNDERSCORE,
    [TAG_KEY_EQUAL_PLUS]           = KEY_EQUAL_PLUS,
    [TAG_KEY_BACKSPACE]            = KEY_BACKSPACE,
    [TAG_KEY_TAB]                  = KEY_TAB,
    [TAG_KEY_Q]                    = KEY_Q,
    [TAG_KEY_W]                    = KEY_W,
    [TAG_KEY_E]                    = KEY_E,
    [TAG_KEY_R]                    = KEY_R,
    [TAG_KEY_T]                    = KEY_T,
    [TAG_KEY_Y]                    = KEY_Y,
    [TAG_KEY_U]                    = KEY_U,
    [TAG_KEY_I]                    = KEY_I,
    [TAG_KEY_O]                    = KEY_O,
    [TAG_KEY_P]                    = KEY_P,
    [TAG_KEY_LBRACKET_CURLY]       = KEY_LBRACKET_CURLY,
    [TAG_KEY_RBRACKET_CURLY]       = KEY_RBRACKET_CURLY,
    [TAG_KEY_BACKSLASH_PIPE]       = KEY_BACKSLASH_PIPE,
    [TAG_KEY_CAPSLOCK]             = KEY_CAPSLOCK,
    [TAG_KEY_A]                    = KEY_A,
    [TAG_KEY_S]                    = KEY_S,
    [TAG_KEY_D]                    = KEY_D,
    [TAG_KEY_F]                    = KEY_F,
    [TAG_KEY_G]                    = KEY_G,
    [TAG_KEY_H]                    = KEY_H,
    [TAG_KEY_J]                    = KEY_J,
    [TAG_KEY_K]                    = KEY_K,
    [TAG_KEY_L]                    = KEY_L,
    [TAG_KEY_SEMICOLON_COLON]      = KEY_SEMICOLON_COLON,
    [TAG_KEY_QUOTE_DOUBLEQUOTE]    = KEY_QUOTE_DOUBLEQUOTE,
    [TAG_KEY_NONUS_NUMBER_TILDE]   = KEY_NONUS_NUMBER_TILDE,
    [TAG_KEY_RETURN]               = KEY_RETURN,
    [TAG_KEY_LSHIFT]               = KEY_LSHIFT,
    [TAG_KEY_NONUS_BACKSLASH_PIPE] = KEY_NONUS_BACKSLASH_PIPE,
    [TAG_KEY_Z]                    = KEY_Z,
    [TAG_KEY_X]                    = KEY_X,
    [TAG_KEY_C]                    = KEY_C,
    [TAG_KEY_V]                    = KEY_V,
    [TAG_KEY_B]                    = KEY_B,
    [TAG_KEY_N]                    = KEY_N,
    [TAG_KEY_M]                    = KEY_M,
    [TAG_KEY_COMMA_LESS]           = KEY_COMMA_LESS,
    [TAG_KEY_PERIOD_GREATER]       = KEY_PERIOD_GREATER,
    [TAG_KEY_SLASH_QUESTION]       = KEY_SLASH_QUESTION,
    [TAG_KEY_RSHIFT]               = KEY_RSHIFT,
    [TAG_KEY_LCONTROL]             = KEY_LCONTROL,
    [TAG_KEY_LMETA]                = KEY_LMETA,
    [TAG_KEY_LALT]                 = KEY_LALT,
    [TAG_KEY_SPACEBAR]             = KEY_SPACEBAR,
    [TAG_KEY_RALT]                 = KEY_RALT,
    [TAG_KEY_RMETA]                = KEY_RMETA,
    [TAG_KEY_MENU]                 = KEY_MENU,
    [TAG_KEY_RCONTROL]             = KEY_RCONTROL,
    [TAG_KEY_INSERT]               = KEY_INSERT,
    [TAG_KEY_HOME]                 = KEY_HOME,
    [TAG_KEY_PAGEUP]               = KEY_PAGEUP,
    [TAG_KEY_DELETEFORWARD]        = KEY_DELETEFORWARD,
    [TAG_KEY_END]                  = KEY_END,
    [TAG_KEY_PAGEDOWN]             = KEY_PAGEDOWN,
    [TAG_KEY_UPARROW]              = KEY_UPARROW,
    [TAG_KEY_LEFTARROW]            = KEY_LEFTARROW,
    [TAG_KEY_DOWNARROW]            = KEY_DOWNARROW,
    [TAG_KEY_RIGHTARROW]           = KEY_RIGHTARROW,
    [TAG_KEY_PAD_NUMLOCK_CLEAR]    = KEY_PAD_NUMLOCK_CLEAR,
    [TAG_KEY_PAD_EQUALS]           = KEY_PAD_EQUALS,
    [TAG_KEY_PAD_DIVIDE]           = KEY_PAD_DIVIDE,
    [TAG_KEY_PAD_MULTIPLY]         = KEY_PAD_MULTIPLY,
    [TAG_KEY_PAD_MINUS]            = KEY_PAD_MINUS,
    [TAG_KEY_PAD_7_HOME]           = KEY_PAD_7_HOME,
    [TAG_KEY_PAD_8_UPARROW]        = KEY_PAD_8_UPARROW,
    [TAG_KEY_PAD_9_PAGEUP]         = KEY_PAD_9_PAGEUP,
    [TAG_KEY_PAD_PLUS]             = KEY_PAD_PLUS,
    [TAG_KEY_PAD_4_LEFTARROW]      = KEY_PAD_4_LEFTARROW,
    [TAG_KEY_PAD_5]                = KEY_PAD_5,
    [TAG_KEY_PAD_6_RIGHTARROW]     = KEY_PAD_6_RIGHTARROW,
    [TAG_KEY_PAD_COMMA]            = KEY_PAD_COMMA,
    [TAG_KEY_PAD_1_END]            = KEY_PAD_1_END,
    [TAG_KEY_PAD_2_DOWNARROW]      = KEY_PAD_2_DOWNARROW,
    [TAG_KEY_PAD_3_PAGEDOWN]       = KEY_PAD_3_PAGEDOWN,
    [TAG_KEY_PAD_0_INSERT]         = KEY_PAD_0_INSERT,
    [TAG_KEY_PAD_DECIMAL_DELETE]   = KEY_PAD_DECIMAL_DELETE,
    [TAG_KEY_PAD_ENTER]            = KEY_PAD_ENTER,
    [TAG_KEY_F13]                  = KEY_F13,
    [TAG_KEY_F14]                  = KEY_F14,
    [TAG_KEY_F15]                  = KEY_F15,
    [TAG_KEY_F16]                  = KEY_F16,
    [TAG_KEY_F17]                  = KEY_F17,
    [TAG_KEY_F18]                  = KEY_F18,
    [TAG_KEY_F19]                  = KEY_F19,
    [TAG_KEY_F20]                  = KEY_F20,
    [TAG_KEY_F21]                  = KEY_F21,
    [TAG_KEY_F22]                  = KEY_F22,
    [TAG_KEY_F23]                  = KEY_F23,
    [TAG_KEY_F24]                  = KEY_F24,
    [TAG_KEY_INTERNATIONAL1]       = KEY_INTERNATIONAL1,
    [TAG_KEY_INTERNATIONAL2]       = KEY_INTERNATIONAL2,
    [TAG_KEY_INTERNATIONAL3]       = KEY_INTERNATIONAL3,
    [TAG_KEY_INTERNATIONAL4]       = KEY_INTERNATIONAL4,
    [TAG_KEY_INTERNATIONAL5]       = KEY_INTERNATIONAL5,
    [TAG_KEY_INTERNATIONAL6]       = KEY_INTERNATIONAL6,
    [TAG_KEY_INTERNATIONAL7]       = KEY_INTERNATIONAL7,
    [TAG_KEY_INTERNATIONAL8]       = KEY_INTERNATIONAL8,
    [TAG_KEY_INTERNATIONAL9]       = KEY_INTERNATIONAL9,
    [TAG_KEY_LANG1]                = KEY_LANG1,
    [TAG_KEY_LANG2]                = KEY_LANG2,
    [TAG_KEY_LANG3]                = KEY_LANG3,
    [TAG_KEY_LANG4]                = KEY_LANG4,
    [TAG_KEY_LANG5]                = KEY_LANG5,
    [TAG_KEY_HELP]                 = KEY_HELP,
    [TAG_KEY_MUTE]                 = KEY_MUTE,
    [TAG_KEY_VOLUMEUP]             = KEY_VOLUMEUP,
    [TAG_KEY_VOLUMEDOWN]           = KEY_VOLUMEDOWN,
  };
  CAMLreturnT(key_code_t, map[Int_val(mlKeyCode)]);
}

value
Val_key_state(
  key_state_t s)
{
  CAMLparam0();
  static intnat map[2] = {
    [KEY_UP]   = TAG_KEY_UP,
    [KEY_DOWN] = TAG_KEY_DOWN,
  };
  CAMLreturn(Val_int(map[s]));
}

value
Val_key_flags(
  key_modifier_t m,
  bool dead)
{
  CAMLparam0();
  CAMLlocal1(mlFlags);
  mlFlags = caml_alloc_tuple(7);
  Store_field(mlFlags, 0, m & MOD_SHIFT ? Val_true : Val_false);
  Store_field(mlFlags, 1, m & MOD_ALT ? Val_true : Val_false);
  Store_field(mlFlags, 2, m & MOD_CTRL ? Val_true : Val_false);
  Store_field(mlFlags, 3, m & MOD_META ? Val_true : Val_false);
  Store_field(mlFlags, 4, m & MOD_CAPSLOCK ? Val_true : Val_false);
  Store_field(mlFlags, 5, m & MOD_NUMLOCK ? Val_true : Val_false);
  Store_field(mlFlags, 6, dead ? Val_true : Val_false);
  CAMLreturn(mlFlags);
}

value
Val_button(
  button_t b)
{
  CAMLparam0();
  static intnat map[6] = {
    [BUTTON_NONE]       = TAG_BUTTON_NONE,
    [BUTTON_LEFT]       = TAG_BUTTON_LEFT,
    [BUTTON_MIDDLE]     = TAG_BUTTON_MIDDLE,
    [BUTTON_RIGHT]      = TAG_BUTTON_RIGHT,
    [BUTTON_WHEEL_UP]   = TAG_BUTTON_WHEEL_UP,
    [BUTTON_WHEEL_DOWN] = TAG_BUTTON_WHEEL_DOWN,
  };
  CAMLreturn(Val_int(map[b]));
}

value
Val_button_state(
  button_state_t s)
{
  CAMLparam0();
  static intnat map[6] = {
    [BUTTON_UP]   = TAG_BUTTON_UP,
    [BUTTON_DOWN] = TAG_BUTTON_DOWN,
  };
  CAMLreturn(Val_int(map[s]));
}

value
Val_frame_event(
  event_t *event)
{
  CAMLparam0();
  CAMLlocal1(mlEventDesc);
  mlEventDesc = caml_alloc_tuple(2);
  Store_field(mlEventDesc, 0, Val_canvas((canvas_t *)(event->target)));
  Store_field(mlEventDesc, 1, caml_copy_int64(event->time));
  CAMLreturn(mlEventDesc);
}

value
Val_focus_event(
  event_t *event)
{
  CAMLparam0();
  CAMLlocal1(mlEventDesc);
  mlEventDesc = caml_alloc_tuple(3);
  Store_field(mlEventDesc, 0, Val_canvas((canvas_t *)(event->target)));
  Store_field(mlEventDesc, 1, caml_copy_int64(event->time));
  Store_field(mlEventDesc, 2, Val_focus_in_out(event->desc.focus.inout));
  CAMLreturn(mlEventDesc);
}

value
Val_resize_event(
  event_t *event)
{
  CAMLparam0();
  CAMLlocal2(mlEventDesc, mlSize);
  mlEventDesc = caml_alloc_tuple(3);
  mlSize = caml_alloc_tuple(2);
  Store_field(mlEventDesc, 0, Val_canvas((canvas_t *)(event->target)));
  Store_field(mlEventDesc, 1, caml_copy_int64(event->time));
  Store_field(mlSize, 0, Val_int32_clip(event->desc.resize.width));
  Store_field(mlSize, 1, Val_int32_clip(event->desc.resize.height));
  Store_field(mlEventDesc, 2, mlSize);
  CAMLreturn(mlEventDesc);
}

value
Val_move_event(
  event_t *event)
{
  CAMLparam0();
  CAMLlocal2(mlEventDesc, mlPos);
  mlEventDesc = caml_alloc_tuple(3);
  mlPos = caml_alloc_tuple(2);
  Store_field(mlEventDesc, 0, Val_canvas((canvas_t *)(event->target)));
  Store_field(mlEventDesc, 1, caml_copy_int64(event->time));
  Store_field(mlPos, 0, Val_int32_clip(event->desc.move.x));
  Store_field(mlPos, 1, Val_int32_clip(event->desc.move.y));
  Store_field(mlEventDesc, 2, mlPos);
  CAMLreturn(mlEventDesc);
}

value
Val_close_event(
  event_t *event)
{
  CAMLparam0();
  CAMLlocal1(mlEventDesc);
  mlEventDesc = caml_alloc_tuple(2);
  Store_field(mlEventDesc, 0, Val_canvas((canvas_t *)(event->target)));
  Store_field(mlEventDesc, 1, caml_copy_int64(event->time));
  CAMLreturn(mlEventDesc);
}

value
Val_key_event(
  event_t *event)
{
  CAMLparam0();
  CAMLlocal1(mlEventDesc);
  mlEventDesc = caml_alloc_tuple(6);
  Store_field(mlEventDesc, 0, Val_canvas((canvas_t *)(event->target)));
  Store_field(mlEventDesc, 1, caml_copy_int64(event->time));
  Store_field(mlEventDesc, 2, Val_key_code(event->desc.key.code));
  Store_field(mlEventDesc, 3, Val_int(event->desc.key.char_));
  Store_field(mlEventDesc, 4, Val_key_flags(event->desc.key.modifiers,
                                            event->desc.key.dead));
  Store_field(mlEventDesc, 5, Val_key_state(event->desc.key.state));
  CAMLreturn(mlEventDesc);
}

value
Val_button_event(
  event_t *event)
{
  CAMLparam0();
  CAMLlocal2(mlEventDesc, mlPos);
  mlEventDesc = caml_alloc_tuple(5);
  mlPos = caml_alloc_tuple(2);
  Store_field(mlEventDesc, 0, Val_canvas((canvas_t *)(event->target)));
  Store_field(mlEventDesc, 1, caml_copy_int64(event->time));
  Store_field(mlPos, 0, Val_int32_clip(event->desc.button.x));
  Store_field(mlPos, 1, Val_int32_clip(event->desc.button.y));
  Store_field(mlEventDesc, 2, mlPos);
  Store_field(mlEventDesc, 3, Val_button(event->desc.button.button));
  Store_field(mlEventDesc, 4, Val_button_state(event->desc.button.state));
  CAMLreturn(mlEventDesc);
}

value
Val_cursor_event(
  event_t *event)
{
  CAMLparam0();
  CAMLlocal2(mlEventDesc, mlPos);
  mlEventDesc = caml_alloc_tuple(3);
  mlPos = caml_alloc_tuple(2);
  Store_field(mlEventDesc, 0, Val_canvas((canvas_t *)(event->target)));
  Store_field(mlEventDesc, 1, caml_copy_int64(event->time));
  Store_field(mlPos, 0, Val_int32_clip(event->desc.cursor.x));
  Store_field(mlPos, 1, Val_int32_clip(event->desc.cursor.y));
  Store_field(mlEventDesc, 2, mlPos);
  CAMLreturn(mlEventDesc);
}

value
Val_event(
  event_t *event)
{
  CAMLparam0();
  CAMLlocal1(mlEvent);
  switch (event->type) {
    case EVENT_NULL:
      assert(!"Null event crossing C boundary");
      break;
    case EVENT_PRESENT:
      assert(!"Present event crossing C boundary");
      break;
    case EVENT_FRAME:
      mlEvent = caml_alloc(1, TAG_FRAME);
      Store_field(mlEvent, 0, Val_frame_event(event));
      break;
    case EVENT_FOCUS:
      mlEvent = caml_alloc(1, TAG_CANVAS_FOCUSED);
      Store_field(mlEvent, 0, Val_focus_event(event));
      break;
    case EVENT_RESIZE:
      mlEvent = caml_alloc(1, TAG_CANVAS_RESIZED);
      Store_field(mlEvent, 0, Val_resize_event(event));
      break;
    case EVENT_MOVE:
      mlEvent = caml_alloc(1, TAG_CANVAS_MOVED);
      Store_field(mlEvent, 0, Val_move_event(event));
      break;
    case EVENT_CLOSE:
      mlEvent = caml_alloc(1, TAG_CANVAS_CLOSED);
      Store_field(mlEvent, 0, Val_close_event(event));
      break;
    case EVENT_KEY:
      mlEvent = caml_alloc(1, TAG_KEY_ACTION);
      Store_field(mlEvent, 0, Val_key_event(event));
      break;
    case EVENT_BUTTON:
      mlEvent = caml_alloc(1, TAG_BUTTON_ACTION);
      Store_field(mlEvent, 0, Val_button_event(event));
      break;
    case EVENT_CURSOR:
      mlEvent = caml_alloc(1, TAG_MOUSE_MOVE);
      Store_field(mlEvent, 0, Val_cursor_event(event));
      break;
    default:
      assert(!"Unknown event");
      break;
  }
  CAMLreturn(mlEvent);
}

int ml_canvas_compare_raw(value mlCanvas1, value mlCanvas2);
intnat ml_canvas_hash_raw(value mlCanvas);

static struct custom_operations _ml_canvas_ops = {
  "com.ocamlpro.ocaml-canvas",
  custom_finalize_default,
  ml_canvas_compare_raw,
  ml_canvas_hash_raw,
  custom_serialize_default,
  custom_deserialize_default,
  custom_compare_ext_default,
#if OCAML_VERSION >= 40800
  custom_fixed_length_default
#endif
};

value
Val_canvas(
  canvas_t *canvas)
{
  CAMLparam0();
  CAMLlocal1(mlCanvas);
  value *mlCanvas_ptr = (value *)canvas_get_user_data(canvas);
  if (mlCanvas_ptr != NULL) {
    mlCanvas = *mlCanvas_ptr;
  } else {
    mlCanvas = caml_alloc_custom(&_ml_canvas_ops, sizeof(canvas_t *), 0, 1);
    *((canvas_t **)Data_custom_val(mlCanvas)) = canvas;
    mlCanvas_ptr = (value *)calloc(1, sizeof(value));
    *mlCanvas_ptr = mlCanvas;
    canvas_set_user_data(canvas, (void *)mlCanvas_ptr);
    caml_register_generational_global_root(mlCanvas_ptr);
  }
  CAMLreturn(mlCanvas);
}

canvas_t *
Canvas_val(
  value mlCanvas)
{
  CAMLparam1(mlCanvas);
  canvas_t *canvas = *((canvas_t **)Data_custom_val(mlCanvas));
  if (canvas == NULL) {
    caml_raise_constant(*caml_named_value("canvas_destroyed"));
  }
  CAMLreturnT(canvas_t *, canvas);
}

void
Nullify_val(
  value mlValue)
{
  CAMLparam1(mlValue);
  *((void **)Data_custom_val(mlValue)) = NULL;
  CAMLreturn0;
}

value
Val_image_data(
  image_data_t *image_data)
{
  CAMLparam0();
  CAMLlocal1(mlImageData);
  intnat dims[CAML_BA_MAX_NUM_DIMS] = { (intnat)image_data->height,
                                        (intnat)image_data->width, 4 };
  mlImageData =
    caml_ba_alloc(CAML_BA_UINT8 | CAML_BA_C_LAYOUT | CAML_BA_MANAGED,
                  3, (void *)image_data->data, dims);
  CAMLreturn(mlImageData);
}

image_data_t
Image_data_val(
  value mlImageData)
{
  CAMLparam1(mlImageData);

  if (Caml_ba_array_val(mlImageData)->num_dims != 3) {
    caml_invalid_argument("Image data must have exactly 3 dimensions");
  }

  intnat height = Caml_ba_array_val(mlImageData)->dim[0];
  intnat width = Caml_ba_array_val(mlImageData)->dim[1];
  intnat bpp = Caml_ba_array_val(mlImageData)->dim[2];

  if (bpp != 4) {
    caml_invalid_argument("Image data third dimension must be 4");
  }

  if ((height <= 0) || (width <= 0)) {
    caml_invalid_argument("Image data dimensions must be strictly positive");
  }

  if ((Caml_ba_array_val(mlImageData)->flags
       & CAML_BA_KIND_MASK) != CAML_BA_UINT8) {
    caml_invalid_argument("Image data kind must be uint8");
  }

  if ((Caml_ba_array_val(mlImageData)->flags
       & CAML_BA_LAYOUT_MASK) != CAML_BA_C_LAYOUT) {
    caml_invalid_argument("Image data layout must be C");
  }

  CAMLreturnT(image_data_t,
              image_data((int32_t)width, (int32_t)height,
                         (color_t_ *)Caml_ba_data_val(mlImageData)));
}

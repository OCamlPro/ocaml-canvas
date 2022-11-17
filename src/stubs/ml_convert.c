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

#include "../implem/color.h"
#include "../implem/gradient.h"
#include "../implem/pattern.h"
#include "../implem/draw_style.h"
#include "../implem/font_desc.h"
#include "../implem/transform.h"
#include "../implem/path2d.h"
#include "../implem/polygonize.h"
#include "../implem/color_composition.h"
#include "../implem/pixmap.h"
#include "../implem/event.h"
#include "../implem/window.h"
#include "../implem/canvas.h"

#include "ml_tags.h"
#include "ml_convert.h"

#if OCAML_VERSION < 41200

#define Val_none Val_int(0)
#define Some_val(v) Field(v, 0)
#define Tag_some 0
#define Is_none(v) ((v) == Val_none)
#define Is_some(v) Is_block(v)

CAMLexport value caml_alloc_some(value v)
{
  CAMLparam1(v);
  value some = caml_alloc_small(1, 0);
  Field(some, 0) = v;
  CAMLreturn(some);
}

#endif /* OCAML_VERSION < 41200 */

#if OCAML_VERSION < 40400

CAMLprim value caml_make_float_vect(value len);

#define caml_alloc_float_array(len) caml_make_float_vect(Val_long(len))

#endif /* OCAML_VERSION < 40400 */

#if OCAML_VERSION < 40800 || OCAML_VERSION >= 50000

CAMLextern value caml_ephe_create(value len);
CAMLextern value caml_ephe_set_key(value ar, value n, value el);
CAMLextern value caml_ephe_get_key(value ar, value n);

CAMLexport value caml_weak_array_create(mlsize_t len)
{
  return caml_ephe_create(Val_long(len));
}

CAMLexport void caml_weak_array_set(value eph, mlsize_t offset, value k)
{
  caml_ephe_set_key(eph, Val_long(offset), k);
}

CAMLexport int caml_weak_array_get(value eph, mlsize_t offset, value *key)
{
  value res = caml_ephe_get_key(eph, Val_long(offset));
  if (Is_none(res)) {
    return 0;
  } else {
    *key = Field(res, 0);
    return 1;
  }
}

#else

#include "caml/weak.h"

#endif /* OCAML_VERSION < 40800 || OCAML_VERSION >= 50000 */


value
Val_int31_clip(
  intnat i)
{
  CAMLparam0();
  if (i < INT32_MIN/2) {
    i = INT32_MIN/2;
  } else if (i > INT32_MAX/2) {
    i = INT32_MAX/2;
  }
  CAMLreturn(Val_long(i));
}

intnat
Int31_val_clip(
  value mlValue)
{
  CAMLparam1(mlValue);
  intnat i = Long_val(mlValue);
  if (i < INT32_MIN/2) {
    i = INT32_MIN/2;
  } else if (i > INT32_MAX/2) {
    i = INT32_MAX/2;
  }
  CAMLreturnT(intnat, i);
}

bool
Optional_bool_val(
  value mlOptBool,
  bool def)
{
  CAMLparam1(mlOptBool);
  bool res = def;
  if (Is_some(mlOptBool)) {
    res = Bool_val(Some_val(mlOptBool));
  }
  CAMLreturnT(bool, res);
}

const char *
Optional_string_val(
  value mlOptString,
  const char *def)
{
  CAMLparam1(mlOptString);
  const char *res = def;
  if (Is_some(mlOptString)) {
    res = String_val(Some_val(mlOptString));
  }
  CAMLreturnT(const char *, res);
}

value
Val_double_array(
  const double *array,
  size_t n)
{
  CAMLparam0();
  CAMLlocal1(mlArray);
  mlArray = caml_alloc_float_array(n);
  for (size_t i = 0; i < n; ++i) {
    Store_double_field(mlArray, i, array[i]);
  }
  CAMLreturn(mlArray);
}

double *
Double_array_val(
  value mlArray)
{
  CAMLparam1(mlArray);
  size_t n = Wosize_val(mlArray);
  if (n == 0) {
    return NULL;
  }
  double *array = (double *)calloc(n, sizeof(double));
  if (array == NULL) {
    return NULL;
  }
  for (size_t i = 0; i < n; ++i) {
    array[i] = Double_field(mlArray, i);
  }
  CAMLreturnT(double *, array);
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
  CAMLreturn(Val_long(map[s]));
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
  CAMLreturn(Val_long(tag));
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
  CAMLreturn(Val_long(map[s]));
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
  CAMLreturn(Val_long(map[b]));
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
  CAMLreturn(Val_long(map[s]));
}

value
Val_frame_cycle_event(
  event_t *event)
{
  CAMLparam0();
  CAMLlocal1(mlEventDesc);
  mlEventDesc = caml_alloc_tuple(1);
  Store_field(mlEventDesc, 0, caml_copy_int64(event->time));
  CAMLreturn(mlEventDesc);
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
  Store_field(mlSize, 0, Val_int31_clip(event->desc.resize.width));
  Store_field(mlSize, 1, Val_int31_clip(event->desc.resize.height));
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
  Store_field(mlPos, 0, Val_int31_clip(event->desc.move.x));
  Store_field(mlPos, 1, Val_int31_clip(event->desc.move.y));
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
  Store_field(mlEventDesc, 3, Val_long(event->desc.key.char_));
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
  Store_field(mlPos, 0, Val_int31_clip(event->desc.button.x));
  Store_field(mlPos, 1, Val_int31_clip(event->desc.button.y));
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
  Store_field(mlPos, 0, Val_int31_clip(event->desc.cursor.x));
  Store_field(mlPos, 1, Val_int31_clip(event->desc.cursor.y));
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
    case EVENT_FRAME_CYCLE:
      mlEvent = caml_alloc(1, TAG_FRAME_CYCLE);
      Store_field(mlEvent, 0, Val_frame_cycle_event(event));
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

value ml_canvas_hash(value mlCanvas);

static intnat
_ml_canvas_hash(
  value mlCanvas)
{
  CAMLparam1(mlCanvas);
  CAMLreturnT(intnat, Long_val(ml_canvas_hash(mlCanvas)));
}

static void
_ml_canvas_finalize(
  value mlCanvas)
{
  canvas_t *canvas = *((canvas_t **)Data_custom_val(mlCanvas));
  if (canvas != NULL) {
    canvas_release(canvas);
  }
}

static struct custom_operations _ml_canvas_ops = {
  "com.ocamlpro.ocaml-canvas",
  _ml_canvas_finalize,
  ml_canvas_compare_raw,
  _ml_canvas_hash,
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
  CAMLlocal2(mlCanvas, mlWeakPointer);

  value *mlWeakPointer_ptr = (value *)canvas_get_data(canvas);

  if (mlWeakPointer_ptr != NULL) {
    mlWeakPointer = *mlWeakPointer_ptr;
  } else {
    mlWeakPointer = caml_weak_array_create(1);
    mlWeakPointer_ptr = (value *)calloc(1, sizeof(value));
    *mlWeakPointer_ptr = mlWeakPointer;
    caml_register_generational_global_root(mlWeakPointer_ptr);
    canvas_set_data(canvas, (void *)mlWeakPointer_ptr);
  }

  if (caml_weak_array_get(mlWeakPointer, 0, &mlCanvas) == 0) {
    mlCanvas = caml_alloc_custom(&_ml_canvas_ops, sizeof(canvas_t *), 0, 1);
    *((canvas_t **)Data_custom_val(mlCanvas)) = canvas_retain(canvas);
    caml_weak_array_set(mlWeakPointer, 0, mlCanvas);
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
    caml_failwith("invalid canvas object");
  }
  CAMLreturnT(canvas_t *, canvas);
}

void
_ml_canvas_path2d_finalize(
  value mlPath2d)
{
  path2d_t *path2d = *((path2d_t **)Data_custom_val(mlPath2d));
  if (path2d != NULL) {
    path2d_release(path2d);
  }
}

int
_ml_canvas_path2d_compare(
  value mlPath2d1,
  value mlPath2d2)
{
  path2d_t *p1 = *((path2d_t **)Data_custom_val(mlPath2d1));
  if (p1 == NULL) {
    caml_failwith("invalid path2d object");
  }
  path2d_t *p2 = *((path2d_t **)Data_custom_val(mlPath2d2));
  if (p2 == NULL) {
    caml_failwith("invalid path2d object");
  }
  if (p1 < p2) {
    return -1;
  }
  else if (p1 > p2) {
    return 1;
  }
  else {
    return 0;
  }
}

static struct custom_operations _ml_path2d_ops = {
  "com.ocamlpro.ocaml-canvas.path2d",
  _ml_canvas_path2d_finalize,
  _ml_canvas_path2d_compare,
  custom_hash_default,
  custom_serialize_default,
  custom_deserialize_default,
  custom_compare_ext_default,
#if OCAML_VERSION >= 40800
  custom_fixed_length_default
#endif
};

value
Val_path2d(
  path2d_t *path2d)
{
  CAMLparam0();
  CAMLlocal2(mlPath2d, mlWeakPointer);

  value *mlWeakPointer_ptr = (value *)path2d_get_data(path2d);

  if (mlWeakPointer_ptr != NULL) {
    mlWeakPointer = *mlWeakPointer_ptr;
  } else {
    mlWeakPointer = caml_weak_array_create(1);
    mlWeakPointer_ptr = (value *)calloc(1, sizeof(value));
    *mlWeakPointer_ptr = mlWeakPointer;
    caml_register_generational_global_root(mlWeakPointer_ptr);
    path2d_set_data(path2d, (void *)mlWeakPointer_ptr);
  }

  if (caml_weak_array_get(mlWeakPointer, 0, &mlPath2d) == 0) {
    mlPath2d = caml_alloc_custom(&_ml_path2d_ops, sizeof(path2d_t *), 0, 1);
    *((path2d_t **)Data_custom_val(mlPath2d)) = path2d_retain(path2d);
    caml_weak_array_set(mlWeakPointer, 0, mlPath2d);
  }

  CAMLreturn(mlPath2d);
}

path2d_t *
Path2d_val(
  value mlPath2d)
{
  CAMLparam1(mlPath2d);
  path2d_t *path2d = *((path2d_t **)Data_custom_val(mlPath2d));
  if (path2d == NULL) {
    caml_failwith("invalid path2d object");
  }
  CAMLreturnT(path2d_t *, path2d);
}

static void
_ml_canvas_gradient_finalize(
  value mlGradient)
{
  gradient_t *gradient = *((gradient_t **)Data_custom_val(mlGradient));
  if (gradient != NULL) {
    gradient_release(gradient);
  }
}

int
_ml_canvas_gradient_compare(
  value mlGradient1,
  value mlGradient2)
{
  gradient_t *g1 = *((gradient_t **)Data_custom_val(mlGradient1));
  if (g1 == NULL) {
    caml_failwith("invalid gradient object");
  }
  gradient_t *g2 = *((gradient_t **)Data_custom_val(mlGradient2));
  if (g2 == NULL) {
    caml_failwith("invalid gradient object");
  }
  if (g1 < g2) {
    return -1;
  }
  else if (g1 > g2) {
    return 1;
  }
  else {
    return 0;
  }
}

static struct custom_operations _ml_gradient_ops = {
  "com.ocamlpro.ocaml-canvas.gradient",
  _ml_canvas_gradient_finalize,
  _ml_canvas_gradient_compare,
  custom_hash_default,
  custom_serialize_default,
  custom_deserialize_default,
  custom_compare_ext_default,
#if OCAML_VERSION >= 40800
  custom_fixed_length_default
#endif
};

value
Val_gradient(
  gradient_t *gradient)
{
  CAMLparam0();
  CAMLlocal2(mlGradient, mlWeakPointer);

  value *mlWeakPointer_ptr = (value *)gradient_get_data(gradient);

  if (mlWeakPointer_ptr != NULL) {
    mlWeakPointer = *mlWeakPointer_ptr;
  } else {
    mlWeakPointer = caml_weak_array_create(1);
    mlWeakPointer_ptr = (value *)calloc(1, sizeof(value));
    *mlWeakPointer_ptr = mlWeakPointer;
    caml_register_generational_global_root(mlWeakPointer_ptr);
    gradient_set_data(gradient, (void *)mlWeakPointer_ptr);
  }

  if (caml_weak_array_get(mlWeakPointer, 0, &mlGradient) == 0) {
    mlGradient =
      caml_alloc_custom(&_ml_gradient_ops, sizeof(gradient_t *), 0, 1);
    *((gradient_t **)Data_custom_val(mlGradient)) = gradient_retain(gradient);
    caml_weak_array_set(mlWeakPointer, 0, mlGradient);
  }

  CAMLreturn(mlGradient);
}

gradient_t *
Gradient_val(
  value mlGradient)
{
  CAMLparam1(mlGradient);
  gradient_t *gradient = *((gradient_t **)Data_custom_val(mlGradient));
  if (gradient == NULL) {
    caml_failwith("invalid gradient object");
  }
  CAMLreturnT(gradient_t *, gradient);
}

static void
_ml_canvas_pattern_finalize(
  value mlPattern)
{
  pattern_t *pattern = *((pattern_t **)Data_custom_val(mlPattern));
  if (pattern != NULL) {
    pattern_release(pattern);
  }
}

int
_ml_canvas_pattern_compare(
  value mlPattern1,
  value mlPattern2)
{
  pattern_t *p1 = *((pattern_t **)Data_custom_val(mlPattern1));
  if (p1 == NULL) {
    caml_failwith("invalid pattern object");
  }
  pattern_t *p2 = *((pattern_t **)Data_custom_val(mlPattern2));
  if (p2 == NULL) {
    caml_failwith("invalid pattern object");
  }
  if (p1 < p2) {
    return -1;
  }
  else if (p1 > p2) {
    return 1;
  }
  else {
    return 0;
  }
}

static struct custom_operations _ml_pattern_ops = {
  "com.ocamlpro.ocaml-canvas.pattern",
  _ml_canvas_pattern_finalize,
  _ml_canvas_pattern_compare,
  custom_hash_default,
  custom_serialize_default,
  custom_deserialize_default,
  custom_compare_ext_default,
#if OCAML_VERSION >= 40800
  custom_fixed_length_default
#endif
};

value
Val_pattern(
  pattern_t *pattern)
{
  CAMLparam0();
  CAMLlocal2(mlPattern, mlWeakPointer);

  value *mlWeakPointer_ptr = (value *)pattern_get_data(pattern);

  if (mlWeakPointer_ptr != NULL) {
    mlWeakPointer = *mlWeakPointer_ptr;
  } else {
    mlWeakPointer = caml_weak_array_create(1);
    mlWeakPointer_ptr = (value *)calloc(1, sizeof(value));
    *mlWeakPointer_ptr = mlWeakPointer;
    caml_register_generational_global_root(mlWeakPointer_ptr);
    pattern_set_data(pattern, (void *)mlWeakPointer_ptr);
  }

  if (caml_weak_array_get(mlWeakPointer, 0, &mlPattern) == 0) {
    mlPattern =
      caml_alloc_custom(&_ml_pattern_ops, sizeof(pattern_t *), 0, 1);
    *((pattern_t **)Data_custom_val(mlPattern)) = pattern_retain(pattern);
    caml_weak_array_set(mlWeakPointer, 0, mlPattern);
  }

  CAMLreturn(mlPattern);
}

pattern_t *
Pattern_val(
  value mlPattern)
{
  CAMLparam1(mlPattern);
  pattern_t *pattern = *((pattern_t **)Data_custom_val(mlPattern));
  if (pattern == NULL) {
    caml_failwith("invalid pattern object");
  }
  CAMLreturnT(pattern_t *, pattern);
}

value
Val_repeat(
  pattern_repeat_t repeat)
{
  CAMLparam0();
  static const repeat_tag_t map[4] = {
    [PATTERN_NO_REPEAT] = TAG_NO_REPEAT,
    [PATTERN_REPEAT_X]  = TAG_REPEAT_X,
    [PATTERN_REPEAT_Y]  = TAG_REPEAT_Y,
    [PATTERN_REPEAT_XY] = TAG_REPEAT_XY
  };
  CAMLreturn(Val_long(map[repeat]));
}

pattern_repeat_t
Repeat_val(
  value mlRepeat)
{
  CAMLparam1(mlRepeat);
  static const pattern_repeat_t map[4] = {
    [TAG_NO_REPEAT]  = PATTERN_NO_REPEAT,
    [TAG_REPEAT_X]   = PATTERN_REPEAT_X,
    [TAG_REPEAT_Y]   = PATTERN_REPEAT_Y,
    [TAG_REPEAT_XY]  = PATTERN_REPEAT_XY
  };
  CAMLreturnT(pattern_repeat_t, map[Int_val(mlRepeat)]);
}

value
Val_style(
  draw_style_t style)
{
  CAMLparam0();
  CAMLlocal1(mlStyle);
  switch (style.type) {
    case DRAW_STYLE_COLOR:
      mlStyle = caml_alloc(1, TAG_COLOR);
      Store_field(mlStyle, 0,
                  caml_copy_int32(color_to_int(style.content.color)));
      break;
    case DRAW_STYLE_GRADIENT:
      mlStyle = caml_alloc(1, TAG_GRADIENT);
      Store_field(mlStyle, 0,
                  Val_gradient(gradient_retain(style.content.gradient)));
      break;
    case DRAW_STYLE_PATTERN:
      mlStyle = caml_alloc(1, TAG_PATTERN);
      Store_field(mlStyle, 0,
                  Val_pattern(pattern_retain(style.content.pattern)));
    default:
      assert(!"Unknown draw style");
      break;
  }
  CAMLreturn(mlStyle);
}

draw_style_t
Style_val(
  value mlStyle)
{
  CAMLparam1(mlStyle);
  draw_style_t style = { 0 };
  switch (Tag_val(mlStyle)) {
    case TAG_COLOR:
      style.type = DRAW_STYLE_COLOR;
      style.content.color = color_of_int(Int32_val(Field(mlStyle, 0)));
      break;
    case TAG_GRADIENT:
      style.type = DRAW_STYLE_GRADIENT;
      style.content.gradient = gradient_retain(Gradient_val(Field(mlStyle, 0)));
      break;
    case TAG_PATTERN:
      style.type = DRAW_STYLE_PATTERN;
      style.content.pattern = pattern_retain(Pattern_val(Field(mlStyle, 0)));
      break;
    default:
      assert(!"Unknown draw style");
      break;
  }
  CAMLreturnT(draw_style_t, style);
}

transform_t
Transform_val(
  value mlTransform)
{
  CAMLparam1(mlTransform);
  transform_t transform = { 0 };
  transform_set(&transform,
                Double_field(mlTransform, 0),
                Double_field(mlTransform, 1),
                Double_field(mlTransform, 2),
                Double_field(mlTransform, 3),
                Double_field(mlTransform, 4),
                Double_field(mlTransform, 5));
  CAMLreturnT(transform_t, transform);
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
Val_join_type(
  join_type_t join)
{
  CAMLparam0();
  static intnat map[3] = {
    [JOIN_ROUND] = TAG_JOIN_ROUND,
    [JOIN_MITER] = TAG_JOIN_MITER,
    [JOIN_BEVEL] = TAG_JOIN_BEVEL
  };
  CAMLreturn(Val_long(map[join]));
}

join_type_t
Join_type_val(
  value mlLineJoin)
{
  CAMLparam1(mlLineJoin);
  static const join_type_t map[3] = {
    [TAG_JOIN_ROUND] = JOIN_ROUND,
    [TAG_JOIN_MITER] = JOIN_MITER,
    [TAG_JOIN_BEVEL] = JOIN_BEVEL
  };
  CAMLreturnT(join_type_t, map[Int_val(mlLineJoin)]);
}

value
Val_cap_type(
  cap_type_t cap)
{
  CAMLparam0();
  static intnat map[3] = {
    [CAP_BUTT]   = TAG_CAP_BUTT,
    [CAP_SQUARE] = TAG_CAP_SQUARE,
    [CAP_ROUND]  = TAG_CAP_ROUND
  };
  CAMLreturn(Val_long(map[cap]));
}

cap_type_t
Cap_type_val(
  value mlLineCap)
{
  CAMLparam1(mlLineCap);
  static const cap_type_t map[3] = {
    [TAG_CAP_BUTT]   = CAP_BUTT,
    [TAG_CAP_SQUARE] = CAP_SQUARE,
    [TAG_CAP_ROUND]  = CAP_ROUND
  };
  CAMLreturnT(cap_type_t, map[Int_val(mlLineCap)]);
}

value
Val_compop(
  composite_operation_t compop)
{
  CAMLparam0();
  static const intnat map[26] = {
    [SOURCE_OVER]      = TAG_OP_SOURCE_OVER,
    [SOURCE_IN]        = TAG_OP_SOURCE_IN,
    [SOURCE_OUT]       = TAG_OP_SOURCE_OUT,
    [SOURCE_ATOP]      = TAG_OP_SOURCE_ATOP,
    [DESTINATION_OVER] = TAG_OP_DESTINATION_OVER,
    [DESTINATION_IN]   = TAG_OP_DESTINATION_IN,
    [DESTINATION_OUT]  = TAG_OP_DESTINATION_OUT ,
    [DESTINATION_ATOP] = TAG_OP_DESTINATION_ATOP,
    [LIGHTER]          = TAG_OP_LIGHTER,
    [COPY]             = TAG_OP_COPY,
    [XOR]              = TAG_OP_XOR,
    [MULTIPLY]         = TAG_OP_MULTIPLY,
    [SCREEN]           = TAG_OP_SCREEN,
    [OVERLAY]          = TAG_OP_OVERLAY,
    [DARKEN]           = TAG_OP_DARKEN,
    [LIGHTEN]          = TAG_OP_LIGHTEN,
    [COLOR_DODGE]      = TAG_OP_COLOR_DODGE,
    [COLOR_BURN]       = TAG_OP_COLOR_BURN,
    [HARD_LIGHT]       = TAG_OP_HARD_LIGHT,
    [SOFT_LIGHT]       = TAG_OP_SOFT_LIGHT,
    [DIFFERENCE]       = TAG_OP_DIFFERENCE,
    [EXCLUSION]        = TAG_OP_EXCLUSION,
    [HUE]              = TAG_OP_HUE,
    [SATURATION]       = TAG_OP_SATURATION,
    [COLOR]            = TAG_OP_COLOR,
    [LUMINOSITY]       = TAG_OP_LUMINOSITY
  };
  CAMLreturn(Val_long(map[compop]));
}

composite_operation_t
Compop_val(
  value mlCompOp)
{
  CAMLparam1(mlCompOp);
  static const composite_operation_t map[26] = {
    [TAG_OP_SOURCE_OVER]      = SOURCE_OVER,
    [TAG_OP_SOURCE_IN]        = SOURCE_IN,
    [TAG_OP_SOURCE_OUT]       = SOURCE_OUT,
    [TAG_OP_SOURCE_ATOP]      = SOURCE_ATOP,
    [TAG_OP_DESTINATION_OVER] = DESTINATION_OVER,
    [TAG_OP_DESTINATION_IN]   = DESTINATION_IN,
    [TAG_OP_DESTINATION_OUT]  = DESTINATION_OUT ,
    [TAG_OP_DESTINATION_ATOP] = DESTINATION_ATOP,
    [TAG_OP_LIGHTER]          = LIGHTER,
    [TAG_OP_COPY]             = COPY,
    [TAG_OP_XOR]              = XOR,
    [TAG_OP_MULTIPLY]         = MULTIPLY,
    [TAG_OP_SCREEN]           = SCREEN,
    [TAG_OP_OVERLAY]          = OVERLAY,
    [TAG_OP_DARKEN]           = DARKEN,
    [TAG_OP_LIGHTEN]          = LIGHTEN,
    [TAG_OP_COLOR_DODGE]      = COLOR_DODGE,
    [TAG_OP_COLOR_BURN]       = COLOR_BURN,
    [TAG_OP_HARD_LIGHT]       = HARD_LIGHT,
    [TAG_OP_SOFT_LIGHT]       = SOFT_LIGHT,
    [TAG_OP_DIFFERENCE]       = DIFFERENCE,
    [TAG_OP_EXCLUSION]        = EXCLUSION,
    [TAG_OP_HUE]              = HUE,
    [TAG_OP_SATURATION]       = SATURATION,
    [TAG_OP_COLOR]            = COLOR,
    [TAG_OP_LUMINOSITY]       = LUMINOSITY
  };
  CAMLreturnT(composite_operation_t, map[Int_val(mlCompOp)]);
}

value
Val_pixmap(
  pixmap_t *pixmap)
{
  CAMLparam0();
  CAMLlocal1(mlImageData);
  intnat dims[CAML_BA_MAX_NUM_DIMS] = { (intnat)pixmap->height,
                                        (intnat)pixmap->width,
                                        COLOR_SIZE };
  mlImageData =
    caml_ba_alloc(CAML_BA_UINT8 | CAML_BA_C_LAYOUT | CAML_BA_MANAGED,
                  3, (void *)pixmap->data, dims);
  CAMLreturn(mlImageData);
}

pixmap_t
Pixmap_val(
  value mlPixmap)
{
  CAMLparam1(mlPixmap);

  if (Caml_ba_array_val(mlPixmap)->num_dims != 3) {
    caml_invalid_argument("Image data must have exactly 3 dimensions");
  }

  intnat height = Caml_ba_array_val(mlPixmap)->dim[0];
  intnat width = Caml_ba_array_val(mlPixmap)->dim[1];
  intnat bpp = Caml_ba_array_val(mlPixmap)->dim[2];

  if (bpp != COLOR_SIZE) {
    caml_invalid_argument("Image data third dimension must be 4");
  }

  if ((height <= 0) || (width <= 0)) {
    caml_invalid_argument("Image data dimensions must be strictly positive");
  }

  if ((Caml_ba_array_val(mlPixmap)->flags
       & CAML_BA_KIND_MASK) != CAML_BA_UINT8) {
    caml_invalid_argument("Image data kind must be uint8");
  }

  if ((Caml_ba_array_val(mlPixmap)->flags
       & CAML_BA_LAYOUT_MASK) != CAML_BA_C_LAYOUT) {
    caml_invalid_argument("Image data layout must be C");
  }

  CAMLreturnT(pixmap_t,
              pixmap((int32_t)width, (int32_t)height,
                     (color_t_ *)Caml_ba_data_val(mlPixmap)));
}

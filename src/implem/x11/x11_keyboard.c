/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifdef HAS_X11

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include <xcb/xcb.h>

#include "../event.h"
#include "x11_backend_internal.h"

static uint16_t
_x11_keyname_int(
  const char kn[4],
  uint8_t digits, /* up to 4 */
  uint16_t max) /* up to 9999 */
{
  uint16_t res = 0;
  for (uint8_t i = 4 - digits; i < 4; ++i) {
    if (!isdigit(kn[i])) return 0;
    res = res * 10 + (kn[i] - '0');
  }
  if (res > max) return 0; /* Always mapped to KEY_UNDEFINED */
  return res;
}

static bool
_x11_keyname_is(
  const char kn1[4],
  const char kn2[4])
{
  return (memcmp(kn1, kn2, 4) == 0);
}

static key_code_t
_x11_keyname_to_keycode(
  const char keyname[4])
{
  static const key_code_t fk_map[25] = {
    [ 0] = KEY_UNDEFINED,
    [ 1] = KEY_F1,
    [ 2] = KEY_F2,
    [ 3] = KEY_F3,
    [ 4] = KEY_F4,
    [ 5] = KEY_F5,
    [ 6] = KEY_F6,
    [ 7] = KEY_F7,
    [ 8] = KEY_F8,
    [ 9] = KEY_F9,
    [10] = KEY_F10,
    [11] = KEY_F11,
    [12] = KEY_F12,
    [13] = KEY_F13,
    [14] = KEY_F14,
    [15] = KEY_F15,
    [16] = KEY_F16,
    [17] = KEY_F17,
    [18] = KEY_F18,
    [19] = KEY_F19,
    [20] = KEY_F20,
    [21] = KEY_F21,
    [22] = KEY_F22,
    [23] = KEY_F23,
    [24] = KEY_F24,
  };

  static const key_code_t ae_map[14] = {
    [ 0] = KEY_UNDEFINED,
    [ 1] = KEY_1_EXCLAMATION,
    [ 2] = KEY_2_AT,
    [ 3] = KEY_3_NUMBER,
    [ 4] = KEY_4_DOLLAR,
    [ 5] = KEY_5_PERCENT,
    [ 6] = KEY_6_CARET,
    [ 7] = KEY_7_AMPERSAND,
    [ 8] = KEY_8_ASTERISK,
    [ 9] = KEY_9_LPARENTHESIS,
    [10] = KEY_0_RPARENTHESIS,
    [11] = KEY_MINUS_UNDERSCORE,
    [12] = KEY_EQUAL_PLUS,
    [13] = KEY_INTERNATIONAL3, /* | and Yen key left of Backspace on JIS KB */
  };

  static const key_code_t ad_map[14] = {
    [ 0] = KEY_UNDEFINED,
    [ 1] = KEY_Q,
    [ 2] = KEY_W,
    [ 3] = KEY_E,
    [ 4] = KEY_R,
    [ 5] = KEY_T,
    [ 6] = KEY_Y,
    [ 7] = KEY_U,
    [ 8] = KEY_I,
    [ 9] = KEY_O,
    [10] = KEY_P,
    [11] = KEY_LBRACKET_CURLY,
    [12] = KEY_RBRACKET_CURLY,
    [13] = KEY_UNDEFINED,
  };

  static const key_code_t ac_map[14] = {
    [ 0] = KEY_UNDEFINED,
    [ 1] = KEY_A,
    [ 2] = KEY_S,
    [ 3] = KEY_D,
    [ 4] = KEY_F,
    [ 5] = KEY_G,
    [ 6] = KEY_H,
    [ 7] = KEY_J,
    [ 8] = KEY_K,
    [ 9] = KEY_L,
    [10] = KEY_SEMICOLON_COLON,
    [11] = KEY_QUOTE_DOUBLEQUOTE,
    [12] = KEY_NONUS_NUMBER_TILDE, /* not seen on output */
    [13] = KEY_UNDEFINED,
  };

  static const key_code_t ab_map[14] = {
    [ 0] = KEY_UNDEFINED,
    [ 1] = KEY_Z,
    [ 2] = KEY_X,
    [ 3] = KEY_C,
    [ 4] = KEY_V,
    [ 5] = KEY_B,
    [ 6] = KEY_N,
    [ 7] = KEY_M,
    [ 8] = KEY_COMMA_LESS,
    [ 9] = KEY_PERIOD_GREATER,
    [10] = KEY_SLASH_QUESTION,
    [11] = KEY_INTERNATIONAL1, /* _ and \ key left of RShift on JIS KB */
    [12] = KEY_UNDEFINED,
    [13] = KEY_UNDEFINED,
  };

  static const key_code_t kp_map[10] = {
    [ 0] = KEY_PAD_0_INSERT,
    [ 1] = KEY_PAD_1_END,
    [ 2] = KEY_PAD_2_DOWNARROW,
    [ 3] = KEY_PAD_3_PAGEDOWN,
    [ 4] = KEY_PAD_4_LEFTARROW,
    [ 5] = KEY_PAD_5,
    [ 6] = KEY_PAD_6_RIGHTARROW,
    [ 7] = KEY_PAD_7_HOME,
    [ 8] = KEY_PAD_8_UPARROW,
    [ 9] = KEY_PAD_9_PAGEUP,
  };

  switch (keyname[0]) {
    case 'A':
      /* AB01-AB11 AC01-AC11 AD01-AD12 AE01-AE13 */
      switch (keyname[1]) {
        case 'B': return ab_map[_x11_keyname_int(keyname, 2, 13)];
        case 'C': return ac_map[_x11_keyname_int(keyname, 2, 13)];
        case 'D': return ad_map[_x11_keyname_int(keyname, 2, 13)];
        case 'E': return ae_map[_x11_keyname_int(keyname, 2, 13)];
      }
      /* **ALT *AGAI */
      return KEY_UNDEFINED;

    case 'B':
      /* BKSP BKSL */
      if (_x11_keyname_is(keyname, "BKSP")) return KEY_BACKSPACE;
      else if (_x11_keyname_is(keyname, "BKSL")) return KEY_BACKSLASH_PIPE;
      return KEY_UNDEFINED;                   /* or KEY_NONUS_NUMBER_TILDE */

    case 'C':
      /* CAPS COMP *COPY *CUT */
      if (_x11_keyname_is(keyname, "CAPS")) return KEY_CAPSLOCK;
      else if (_x11_keyname_is(keyname, "COMP")) return KEY_MENU;
      return KEY_UNDEFINED;

    case 'D':
      /* DOWN DELE */
      if (_x11_keyname_is(keyname, "DOWN")) return KEY_DOWNARROW;
      else if (_x11_keyname_is(keyname, "DELE")) return KEY_DELETEFORWARD;
      return KEY_UNDEFINED;

    case 'E':
      /* ESC END */
      if (_x11_keyname_is(keyname, "ESC\0")) return KEY_ESCAPE;
      else if (_x11_keyname_is(keyname, "END\0")) return KEY_END;
      return KEY_UNDEFINED;

    case 'F':
      /* FK01-24 *FRNT *FIND */
      if (keyname[1] == 'K') return fk_map[_x11_keyname_int(keyname, 2, 24)];
      return KEY_UNDEFINED;

    case 'H':
      /* HOME HIRA HENK HKTG HNGL HJCV +HZTG HELP **HYPR */
      if (_x11_keyname_is(keyname, "HOME")) return KEY_HOME;
      else if (_x11_keyname_is(keyname, "HIRA")) return KEY_LANG4;
      else if (_x11_keyname_is(keyname, "HENK")) return KEY_INTERNATIONAL4;
      else if (_x11_keyname_is(keyname, "HKTG")) return KEY_INTERNATIONAL2;
      else if (_x11_keyname_is(keyname, "HNGL")) return KEY_LANG1;
      else if (_x11_keyname_is(keyname, "HNCV")) return KEY_LANG2;
      else if (_x11_keyname_is(keyname, "HZTG")) return KEY_LANG5;
      else if (_x11_keyname_is(keyname, "HELP")) return KEY_HELP;
      return KEY_UNDEFINED;

    case 'I':
      /* INS I120 I126 I128-129 I147-I190 I208-255 */
      if (_x11_keyname_is(keyname, "INS\0")) return KEY_INSERT;
      else if (_x11_keyname_is(keyname, "I129")) return KEY_PAD_COMMA;
      return KEY_UNDEFINED;

    case 'J':
      /* JPCM */
      if (_x11_keyname_is(keyname, "JPCM")) return KEY_INTERNATIONAL6;
      return KEY_UNDEFINED;

    case 'K':
      /* KP0-9 KPDV KPMU KPSU KPAD KPEQ KPEN KPDL */
      if (keyname[1] == 'P') {
        if (isdigit(keyname[2])) return kp_map[keyname[2] - '0'];
        else if (_x11_keyname_is(keyname, "KPDV")) return KEY_PAD_DIVIDE;
        else if (_x11_keyname_is(keyname, "KPMU")) return KEY_PAD_MULTIPLY;
        else if (_x11_keyname_is(keyname, "KPSU")) return KEY_PAD_MINUS;
        else if (_x11_keyname_is(keyname, "KPAD")) return KEY_PAD_PLUS;
        else if (_x11_keyname_is(keyname, "KPEQ")) return KEY_PAD_EQUALS;
        else if (_x11_keyname_is(keyname, "KPEN")) return KEY_PAD_ENTER;
        else if (_x11_keyname_is(keyname,"KPDL")) return KEY_PAD_DECIMAL_DELETE;
        return KEY_UNDEFINED;
      }
      /* KATA */
      if (_x11_keyname_is(keyname, "KATA")) return KEY_LANG3;
      return KEY_UNDEFINED;

    case 'L':
      /* LSGT LFSH LCTL LALT LWIN **LVL3 LEFT *LNFD */
      if (_x11_keyname_is(keyname, "LSGT")) return KEY_NONUS_BACKSLASH_PIPE;
      else if (_x11_keyname_is(keyname, "LFSH")) return KEY_LSHIFT;
      else if (_x11_keyname_is(keyname, "LCTL")) return KEY_LCONTROL;
      else if (_x11_keyname_is(keyname, "LALT")) return KEY_LALT;
      else if (_x11_keyname_is(keyname, "LWIN")) return KEY_LMETA;
      else if (_x11_keyname_is(keyname, "LEFT")) return KEY_LEFTARROW;
      return KEY_UNDEFINED;

    case 'M':
      /* MUHE MUTE **MDSW **META */
      if (_x11_keyname_is(keyname, "MUHE")) return KEY_INTERNATIONAL5;
      else if (_x11_keyname_is(keyname, "MUTE")) return KEY_MUTE;
      return KEY_UNDEFINED;

    case 'N':
      /* NMLK */
      if (_x11_keyname_is(keyname, "NMLK")) return KEY_PAD_NUMLOCK_CLEAR;
      return KEY_UNDEFINED;

    case 'O':
      /* *OPEN */
      return KEY_UNDEFINED;

    case 'P':
      /* PGUP PGDN PRSC PAUS *PAST *PROP *POWR */
      if (_x11_keyname_is(keyname, "PGUP")) return KEY_PAGEUP;
      else if (_x11_keyname_is(keyname, "PGDN")) return KEY_PAGEDOWN;
      else if (_x11_keyname_is(keyname, "PRSC")) return KEY_PRINTSCREEN;
      else if (_x11_keyname_is(keyname, "PAUS")) return KEY_PAUSE;
      return KEY_UNDEFINED;

    case 'R':
      /* RTRN RTSH RCTL RALT RWIN RGHT */
      if (_x11_keyname_is(keyname, "RTRN")) return KEY_RETURN;
      else if (_x11_keyname_is(keyname, "RTSH")) return KEY_RSHIFT;
      else if (_x11_keyname_is(keyname, "RCTL")) return KEY_RCONTROL;
      else if (_x11_keyname_is(keyname, "RALT")) return KEY_RALT;
      else if (_x11_keyname_is(keyname, "RWIN")) return KEY_RMETA;
      else if (_x11_keyname_is(keyname, "RGHT")) return KEY_RIGHTARROW;
      return KEY_UNDEFINED;

    case 'S':
      /* SPCE SCLK **SUPR *STOP */
      if (_x11_keyname_is(keyname, "SPCE")) return KEY_SPACEBAR;
      else if (_x11_keyname_is(keyname, "SCLK")) return KEY_SCROLLLOCK;
      return KEY_UNDEFINED;

    case 'T':
      /* TAB TLDE */
      if (_x11_keyname_is(keyname, "TAB\0")) return KEY_TAB;
      else if (_x11_keyname_is(keyname, "TLDE")) return KEY_GRAVE_TILDE;
      return KEY_UNDEFINED;

    case 'U':
      /* UP *UNDO */
      if (_x11_keyname_is(keyname, "UP\0\0")) return KEY_UPARROW;
      return KEY_UNDEFINED;

    case 'V':
      /* VOL+ VOL- */
      if (_x11_keyname_is(keyname, "VOL+")) return KEY_VOLUMEUP;
      else if (_x11_keyname_is(keyname, "VOL-")) return KEY_VOLUMEDOWN;
      return KEY_UNDEFINED;
  }
  return KEY_UNDEFINED;
}

void
x11_keyboard_refresh(
  void)
{
  assert(x11_back != NULL);
  assert(x11_back->c != NULL);

  /* Request key map */
  xcb_xkb_get_map_cookie_t xkb_gm_cookie =
    xcb_xkb_get_map(x11_back->c, XCB_XKB_ID_USE_CORE_KBD,
      XCB_XKB_MAP_PART_KEY_SYMS | XCB_XKB_MAP_PART_KEY_TYPES, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

  /* Request key names */
  xcb_xkb_get_names_cookie_t xkb_gn_cookie =
    xcb_xkb_get_names(x11_back->c,
                      XCB_XKB_ID_USE_CORE_KBD,
                      XCB_XKB_NAME_DETAIL_KEY_NAMES);


  /* Process key map */

  xcb_xkb_get_map_reply_t *xkb_gm_rep =
    xcb_xkb_get_map_reply(x11_back->c, xkb_gm_cookie, NULL);

  if (x11_back->xkb_get_map_reply != NULL) {
    free(x11_back->xkb_get_map_reply);
  }
  x11_back->xkb_get_map_reply = xkb_gm_rep;

  xcb_xkb_get_map_map_t mm = { 0 };

  xcb_xkb_get_map_map_unpack(
    xcb_xkb_get_map_map(xkb_gm_rep),
    xkb_gm_rep->nTypes, xkb_gm_rep->nKeySyms,
    xkb_gm_rep->nKeyActions, xkb_gm_rep->totalActions,
    xkb_gm_rep->totalKeyBehaviors, xkb_gm_rep->virtualMods,
    xkb_gm_rep->totalKeyExplicit, xkb_gm_rep->totalModMapKeys,
    xkb_gm_rep->totalVModMapKeys, xkb_gm_rep->present, &mm);

  xcb_xkb_key_sym_map_iterator_t sym_map_iter =
    xcb_xkb_get_map_map_syms_rtrn_iterator(xkb_gm_rep, &mm);

  int nb_keys = xcb_xkb_get_map_map_syms_rtrn_length(xkb_gm_rep, &mm);

  for (int k = 0; k < nb_keys; ++k) {

    xcb_xkb_key_sym_map_t *sm = sym_map_iter.data;

    x11_keysyms_t *x11_ks = &x11_back->keysyms[k+8];

    x11_ks->keysyms = xcb_xkb_key_sym_map_syms(sm);
    x11_ks->grp_info = sm->groupInfo;
    x11_ks->grp_width = sm->width;
    x11_ks->grp_kt = sm->kt_index;

    //int nb_syms = xcb_xkb_key_sym_map_syms_length(sm);
    //or just sm->nSyms ; nb_syms = nb_grp * grp_width

    xcb_xkb_key_sym_map_next(&sym_map_iter);
  }

  xcb_xkb_key_type_iterator_t key_type_iter =
    xcb_xkb_get_map_map_types_rtrn_iterator(xkb_gm_rep, &mm);

  int nb_types = xcb_xkb_get_map_map_types_rtrn_length(xkb_gm_rep, &mm);

  for (int k = 0; k < nb_types; ++k) {

    xcb_xkb_key_type_t *ktm = key_type_iter.data;

    x11_keytypes_t *x11_kt = &x11_back->keytypes[k];

    x11_kt->keytypes = xcb_xkb_key_type_map(ktm);
    x11_kt->mods_mask = ktm->mods_mask;
    x11_kt->nb_entries = ktm->nMapEntries;

    xcb_xkb_key_type_next(&key_type_iter);
  }

  /* xkb_gm_rep intentionally not freed */

  /* Process key names */

  xcb_xkb_get_names_reply_t *xkb_gn_rep =
    xcb_xkb_get_names_reply(x11_back->c, xkb_gn_cookie, NULL);

  xcb_xkb_get_names_value_list_t nvl;

  xcb_xkb_get_names_value_list_unpack(
    xcb_xkb_get_names_value_list(xkb_gn_rep),
    xkb_gn_rep->nTypes, xkb_gn_rep->indicators,
    xkb_gn_rep->virtualMods, xkb_gn_rep->groupNames,
    xkb_gn_rep->nKeys, xkb_gn_rep->nKeyAliases,
    xkb_gn_rep->nRadioGroups, xkb_gn_rep->which, &nvl);

  nb_keys = xcb_xkb_get_names_value_list_key_names_length(xkb_gn_rep, &nvl);

  xcb_xkb_key_name_t *keynames = xcb_xkb_get_names_value_list_key_names(&nvl);

  int k = 0;
  for (k = 0; k < nb_keys; ++k) {
    x11_back->keycodes[k+8] = _x11_keyname_to_keycode(keynames[k].name);
  }
  for (; k < 256-8; ++k) {
    x11_back->keycodes[k+8] = KEY_UNDEFINED;
  }

  free(xkb_gn_rep);
}

key_code_t
x11_keyboard_translate_keycode(
  xcb_keycode_t keycode)
{
  assert(x11_back != NULL);

  return x11_back->keycodes[keycode];
}

#else

const int x11_keyboard = 0;

#endif /* HAS_X11 */

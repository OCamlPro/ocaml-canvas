/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __X11_KEYBOARD_H
#define __X11_KEYBOARD_H

#include <xcb/xcb.h>

#include "../event.h"

void
x11_keyboard_refresh(
  void);

key_code_t
x11_keyboard_translate_keycode(
  xcb_keycode_t keycode);

#endif /* __X11_KEYBOARD_H */

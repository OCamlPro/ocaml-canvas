/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __QTZ_KEYBOARD_H
#define __QTZ_KEYBOARD_H

#include <stdint.h>
#include <stdbool.h>

#include <Cocoa/Cocoa.h>
#include <Carbon/Carbon.h>

#include "../event.h"

key_code_t
qtz_keyboard_translate_keycode(
  uint16_t keycode);

int32_t
qtz_keyboard_keycode_to_unicode(
  uint16_t keycode,
  NSEventModifierFlags modifiers,
  bool *out_dead);

#endif /* __QTZ_KEYBOARD_H */

/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __GDI_KEYBOARD_H
#define __GDI_KEYBOARD_H

#include <stdint.h>
#include <stdbool.h>

#include "../event.h"

key_code_t
gdi_keyboard_translate_scancode(
  uint16_t scancode,
  bool extended);

int32_t
gdi_keyboard_scancode_to_unicode(
  uint32_t virtkey,
  uint16_t scancode,
  bool *out_dead);

#endif /* __GDI_KEYBOARD_H */

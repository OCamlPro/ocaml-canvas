/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __X11_KEYSYM_H
#define __X11_KEYSYM_H

#include <stdint.h>
#include <stdbool.h>

#include <xcb/xcb.h>

bool
x11_keysym_is_dead(
  uint32_t keysym);

uint32_t
x11_keysym_to_unicode(
  uint32_t keysym);

xcb_keysym_t
x11_keysym_of_event(
  xcb_key_press_event_t *e);

#endif /* __X11_KEYSYM_H */

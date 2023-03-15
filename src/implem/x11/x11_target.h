/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __X11_TARGET_H
#define __X11_TARGET_H

#include <xcb/xcb.h>

typedef struct x11_target_t {
  xcb_window_t wid;
} x11_target_t;

x11_target_t *
x11_target_create(
  xcb_window_t wid);

void
x11_target_destroy(
  x11_target_t *target);

#endif /* __X11_TARGET_H */

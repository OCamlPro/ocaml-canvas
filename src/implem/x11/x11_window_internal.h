/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __X11_WINDOW_INTERNAL_H
#define __X11_WINDOW_INTERNAL_H

#include <xcb/xcb.h>

#include "../window_internal.h"

typedef struct x11_window_t {

  /* Common to all windows */
  window_t base;

  /* Specific to X11 windows */
  xcb_window_t wid;
  xcb_gcontext_t cid;

} x11_window_t;

#endif /* __X11_WINDOW_INTERNAL_H */

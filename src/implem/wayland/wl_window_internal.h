/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __WL_WINDOW_INTERNAL_H
#define __WL_WINDOW_INTERNAL_H

#include <wayland-client.h>
#include "xdg-shell-client-protocol.h"

#include "../window_internal.h"

typedef struct wl_window_t {

  /* Common to all windows */
  window_t base;

  /* Specific to Quartz windows */
  struct wl_surface *wl_surface;
  struct xdg_surface *xdg_surface;
  struct xdg_toplevel *xdg_toplevel;
//  struct wl_buffer *wl_buffer;

} wl_window_t;

#endif /* __WL_WINDOW_INTERNAL_H */

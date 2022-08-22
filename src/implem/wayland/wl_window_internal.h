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
#ifdef HAS_XDG_DECORATION
#include "xdg-decor-protocol.h"
#endif /* HAS_XDG_DECORATION */

#include "wl_decoration.h"
#include "../window_internal.h"

#ifdef HAS_XDG_DECORATION
#define HAS_SERVER_DECORATION (wl_back->zxdg_decoration_manager_v1 != NULL)
#else
#define HAS_SERVER_DECORATION (false)
#endif /* HAS_XDG_DECORATION */

typedef struct wl_window_t {

  /* Common to all windows */
  window_t base;

  /* Specific to Wayland windows */
  struct wl_surface *wl_surface;
  struct xdg_surface *xdg_surface;
  struct xdg_toplevel *xdg_toplevel;
  struct wl_callback *wl_callback;
  const char *title;

  /* Client Side Decorations */
  wl_decoration_t *decoration;

  /* Server Side Decorations */
  struct zxdg_toplevel_decoration_v1 *server_decor;
  bool pending_resize;

} wl_window_t;

#endif /* __WL_WINDOW_INTERNAL_H */

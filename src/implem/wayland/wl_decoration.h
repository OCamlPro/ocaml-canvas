/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __WL_DECORATION_H
#define __WL_DECORATION_H

#include <stdint.h>
#include <stdbool.h>
#include <sys/mman.h>

#include <wayland-client.h>

#include "../color.h"

typedef struct wl_decoration_t {
  struct wl_surface *wl_surface;
  struct wl_subsurface *wl_subsurface;
  struct wl_buffer *background_buffer;
  
  struct wl_surface *wl_closebutton_surface;
  struct wl_subsurface *wl_closebutton_subsurface;
  struct wl_buffer *wl_closebutton_buffer;

} wl_decoration_t;

wl_decoration_t*
wl_decoration_create(
    struct wl_surface *parent,
    uint32_t width,
    const char* title
);

void
wl_decoration_destroy(
    wl_decoration_t *decoration
);

void
wl_decoration_present(
    wl_decoration_t* decoration
);

void 
wl_decoration_resize(
    wl_decoration_t* decoration,
    uint32_t width,
    const char* title
);



#endif /* __WL_DECORATION_H */
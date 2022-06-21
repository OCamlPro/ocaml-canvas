/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifdef HAS_WAYLAND

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <wayland-client.h>
#include <sys/mman.h>


#include "../config.h"
#include "../color.h"
#include "wl_backend.h"
#include "wl_backend_internal.h"
#include "wl_window_internal.h"
#include "wl_window.h"
#include "wl_memory.h"
#include "wl_target.h"
#include "wl_decoration.h"

typedef struct surface_impl_wl_t {
  impl_type_t type;
  struct wl_buffer *wl_buffer;
  struct wl_surface *wl_surface;
} surface_impl_wl_t;



static void
wl_buffer_release(void *data, struct wl_buffer *wl_buffer)
{
  //process resize requests here?
  struct wl_window_t *wl_window = data;
  event_t evt;
  if (wl_window->pending_resize)
  {
    //resize decorations
    wl_decoration_resize(wl_window->decoration,wl_window->base.width,wl_window->title);
    //resize surface
    evt.desc.resize.width = wl_window->base.width;
    evt.desc.resize.height = wl_window->base.height;
    evt.target = wl_window;
    evt.type = EVENT_RESIZE;
    event_notify(wl_back->listener,&evt);
    wl_window->pending_resize = false;
  }
}

static const struct wl_buffer_listener wl_buffer_listener = {
    .release = wl_buffer_release,
};


surface_impl_wl_t *
surface_create_wl_impl(
  wl_target_t *wl_target,
  int32_t width,
  int32_t height,
  color_t_ **data)
{
  assert(wl_target != NULL);
  assert(wl_target->wl_shm != NULL);
  assert(wl_target->wl_surface != NULL);
  assert(width > 0);
  assert(height  > 0);
  assert(data != NULL);
  assert(*data == NULL);

  surface_impl_wl_t *impl =
    (surface_impl_wl_t *)calloc(1, sizeof(surface_impl_wl_t));
  if (impl == NULL) {
    return NULL;
  }

  uint8_t *pool_data = NULL;
  struct wl_buffer *wl_buffer = wl_create_buffer(width,height,&pool_data);
  wl_buffer_add_listener(wl_buffer,&wl_buffer_listener,wl_surface_get_user_data(wl_target->wl_surface));

  impl->type = IMPL_WAYLAND;
  impl->wl_buffer = wl_buffer;
  impl->wl_surface = wl_target->wl_surface;
  *data = (color_t_ *)pool_data;

  return impl;
}

void
surface_destroy_wl_impl(
  surface_impl_wl_t *impl)
{
  assert(impl != NULL);
  assert(impl->type == IMPL_WAYLAND);

  //Surface is attached to the window. Its destruction is left for the window destroy function
  wl_buffer_destroy(impl->wl_buffer);

}


static void
_raw_surface_copy(
  color_t_ *s_data,
  int32_t s_width,
  int32_t s_height,
  color_t_ *d_data,
  int32_t d_width,
  int32_t d_height)
{
  assert(s_data != NULL);
  assert(s_width > 0);
  assert(s_height > 0);
  assert(d_data != NULL);
  assert(d_width > 0);
  assert(d_height > 0);
  
  uint32_t min_width = d_width < s_width ? d_width : s_width;
  uint32_t min_height = d_height < s_height ? d_height : s_height;
  for (size_t i = 0; i < d_height; i++) {
    for (size_t j = 0; j < d_width; j++) {
      if (i < min_height && j < min_width)
        d_data[i * d_width + j] = s_data[i * s_width + j];
      else
        d_data[i * d_width + j] = color_black;
    }
  }
  
}


bool
surface_resize_wl_impl(
  surface_impl_wl_t *impl,
  int32_t s_width,
  int32_t s_height,
  color_t_ **s_data,
  int32_t d_width,
  int32_t d_height,
  color_t_ **d_data)
{
  assert(impl != NULL);
  assert(s_width > 0);
  assert(s_height  > 0);
  assert(s_data != NULL);
  assert(*s_data != NULL);
  assert(d_width > 0);
  assert(d_height  > 0);
  assert(d_data != NULL);
  assert(*d_data == NULL);

  wl_surface_attach(impl->wl_surface,NULL,0,0);
  //Create new buffer
  uint8_t *pool_data = NULL;
  struct wl_buffer* newBuffer =  wl_create_buffer(d_width,d_height,&pool_data);
  *d_data = (color_t_ *)pool_data;
  //Copy data
  _raw_surface_copy(*s_data,s_width,s_height,*d_data,d_width,d_height);
  //Delete old data ?
  wl_buffer_destroy(impl->wl_buffer);
  munmap(*s_data,s_width * s_height * 4);
  wl_buffer_add_listener(newBuffer,&wl_buffer_listener,wl_surface_get_user_data(impl->wl_surface));
  impl->wl_buffer = newBuffer;
  return true;
}

void
surface_present_wl_impl(
  surface_impl_wl_t *impl,
  int32_t width,
  int32_t height,
  wl_present_data_t *present_data)
{
  assert(impl != NULL);
  assert(present_data != NULL);
  assert(width > 0);
  assert(height > 0);
  wl_surface_attach(impl->wl_surface, impl->wl_buffer, 0, 0);
  wl_surface_damage(impl->wl_surface,0,0,width,height);
  wl_surface_commit(impl->wl_surface);
}

#else

const int wl_surface = 0;

#endif /* HAS_WAYLAND */

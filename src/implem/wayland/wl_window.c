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
#include <string.h>
#include <assert.h>

#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#include <wayland-client.h>
#include "xdg-shell-client-protocol.h"
#include "xdg-decor-protocol.h"

#include "../util.h"
#include "wl_backend.h"
#include "wl_backend_internal.h"
#include "wl_target.h"
#include "wl_window_internal.h"
#include "wl_memory.h"



static void
_wl_window_update_position(
  wl_window_t *window)
{
  assert(window != NULL);
  //TODO
}

static void
_wl_xdg_surface_configure(
  void *data,
  struct xdg_surface *xdg_surface,
  uint32_t serial)
{
    wl_window_t *window = (wl_window_t *)data;
    xdg_surface_ack_configure(xdg_surface, serial);
}

static const struct xdg_surface_listener
_wl_xdg_surface_listener = {
  .configure = _wl_xdg_surface_configure,
};

static void
_wl_xdg_toplevel_configure_handler(
  void *data,
  struct xdg_toplevel *xdg_toplevel,
  int32_t width,
  int32_t height,
  struct wl_array *states)
{
  struct wl_window_t *wl_window = data;
  printf("top level configure: %dx%d\n", width, height);
  uint8_t* p;
  bool resizing = false;
  wl_array_for_each(p,states) 
  {
    if(*p == XDG_TOPLEVEL_STATE_RESIZING) 
      resizing = true;
    else if (*p == XDG_TOPLEVEL_STATE_ACTIVATED)
    {
      if (wl_back->activated_window && wl_back->activated_window != wl_window)
      {
        event_t evt;
        evt.desc.focus.inout = FOCUS_OUT;
        evt.time = _wl_get_time();
        evt.target = wl_window;
        evt.type = EVENT_FOCUS;
        event_notify(wl_back->listener,&evt);
      }
      event_t evt;
      evt.desc.focus.inout = FOCUS_IN;
      evt.time = _wl_get_time();
      evt.target = wl_window;
      evt.type = EVENT_FOCUS;
      event_notify(wl_back->listener,&evt);
      wl_back->activated_window = wl_window;
    }
  }
  bool movedEnough = abs(width - wl_window->base.width) > 10;
  movedEnough |= abs(height - wl_window->base.height) > 10;
  if (resizing && movedEnough)
  {
    wl_window->pending_resize = true;
    //This check solves some issues when using the Ubuntu compositor while having decorations activated
    if (wl_back->current_resize_edge & XDG_TOPLEVEL_RESIZE_EDGE_RIGHT || wl_back->current_resize_edge & XDG_TOPLEVEL_RESIZE_EDGE_LEFT || HAS_SERVER_DECORATION)
      wl_window->base.width = clip_i32_to_i16(width);
    if (wl_back->current_resize_edge & XDG_TOPLEVEL_RESIZE_EDGE_TOP || wl_back->current_resize_edge & XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM || HAS_SERVER_DECORATION)
      wl_window->base.height = clip_i32_to_i16(height);
  }
}

static void
_wl_xdg_toplevel_close_handler(
  void *data,
  struct xdg_toplevel *xdg_toplevel)
{
  struct wl_window_t *wl_window = data;
  event_t evt;
  evt.type = EVENT_CLOSE;
  evt.target = wl_window;
  event_notify(wl_back->listener,&evt);
}

const struct xdg_toplevel_listener
_wl_xdg_toplevel_listener =
{
  .configure = _wl_xdg_toplevel_configure_handler,
  .close = _wl_xdg_toplevel_close_handler,
};




wl_window_t *
wl_window_create(
  bool decorated,
  const char *title,
  int32_t x,
  int32_t y,
  int32_t width,
  int32_t height)
{
  wl_window_t *window = (wl_window_t *)calloc(1, sizeof(wl_window_t));
  if (window == NULL) {
    return NULL;
  }

  window->base.visible = false;
  window->base.decorated = decorated;
  window->base.x = clip_i32_to_i16(x);
  window->base.y = clip_i32_to_i16(y);
  window->base.width = clip_i32_to_i16(max(1, width));
  window->base.height = clip_i32_to_i16(max(1, height));

  window->title = title;
  window->pending_resize = false;

  window->wl_surface = wl_compositor_create_surface(wl_back->compositor);
  wl_surface_set_user_data(window->wl_surface,window);
  if (window->base.decorated && !HAS_SERVER_DECORATION)
  {
    window->decoration = wl_decoration_create(window->wl_surface,width,title);
    wl_surface_set_user_data(window->decoration->wl_surface,window);
    wl_surface_set_user_data(window->decoration->wl_closebutton_surface,window);
  }
  wl_backend_add_window(window);

  return window;
}

void
wl_window_destroy(
  wl_window_t *window)
{
  assert(window != NULL);
  wl_callback_destroy(window->wl_callback);
  xdg_toplevel_destroy(window->xdg_toplevel);
  xdg_surface_destroy(window->xdg_surface);
  if (window->server_decor)
    zxdg_toplevel_decoration_v1_destroy(window->server_decor);
  if (window->base.decorated && !HAS_SERVER_DECORATION)
    wl_decoration_destroy(window->decoration);
  wl_surface_destroy(window->wl_surface);
  free(window);
}

wl_target_t *
wl_window_get_target(
  wl_window_t *window)
{
  assert(window != NULL);
  assert(window->wl_surface != NULL);
  return wl_target_create(wl_back->shm, window->wl_surface);
}

void
wl_window_set_title(
  wl_window_t *window,
  const char *title)
{
  assert(window != NULL);
  assert(title != NULL);
  assert(window->xdg_toplevel != NULL);
  
  xdg_toplevel_set_title(window->xdg_toplevel,title);
}

void
wl_window_set_size(
  wl_window_t *window,
  int32_t width,
  int32_t height)
{
  assert(window != NULL);
  window->base.width = clip_i32_to_i16(width);
  window->base.height = clip_i32_to_i16(height);
}

//Not allowed to set on Wayland
void
wl_window_set_position(
  wl_window_t *window,
  int32_t x,
  int32_t y)
{
  assert(window != NULL);
  window->base.x = clip_i32_to_i16(x);
  window->base.y = clip_i32_to_i16(y);
  _wl_window_update_position(window);
}

void
wl_window_show(
  wl_window_t *window)
{
  assert(window != NULL);
  if (!window->xdg_surface) {
    window->xdg_surface =
      xdg_wm_base_get_xdg_surface(wl_back->xdg_wm_base, window->wl_surface);
    xdg_surface_add_listener(window->xdg_surface, &_wl_xdg_surface_listener, &window);
    window->xdg_toplevel = xdg_surface_get_toplevel(window->xdg_surface);
    xdg_toplevel_add_listener(window->xdg_toplevel,&_wl_xdg_toplevel_listener,window);
    xdg_toplevel_set_title(window->xdg_toplevel,window->title);
    if (window->base.decorated && HAS_SERVER_DECORATION)
    {
      window->server_decor = zxdg_decoration_manager_v1_get_toplevel_decoration(wl_back->zxdg_decoration_manager_v1,window->xdg_toplevel);
      zxdg_toplevel_decoration_v1_set_mode(window->server_decor,ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
    }
    //Need to attach any buffer here...
    uint8_t *dummy_data = NULL;
    struct wl_buffer *dummy = wl_create_buffer(1,1,&dummy_data);
    munmap(dummy_data,4);
    wl_surface_attach(window->wl_surface,dummy,0,0);
    wl_surface_commit(window->wl_surface);    
  }
  _wl_window_update_position(window);

}

void
wl_window_hide(
  wl_window_t *window)
{
  assert(window != NULL);
  //unmap xdg_toplevel and surface
  if (window->xdg_surface)
    xdg_surface_destroy(window->xdg_surface);
  if (window->xdg_toplevel)
    xdg_toplevel_destroy(window->xdg_toplevel);
  window->xdg_surface = NULL;
  window->xdg_toplevel = NULL;
  //Cannot recreate the XDG Surface from a surface with an attached buffer.
  wl_surface_attach(window->wl_surface,NULL,0,0);
  wl_surface_commit(window->wl_surface);
}

#else

const int wl_window = 0;

#endif /* HAS_WAYLAND */

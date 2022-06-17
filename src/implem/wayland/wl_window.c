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

#include "../util.h"
#include "wl_backend.h"
#include "wl_backend_internal.h"
#include "wl_target.h"
#include "wl_window_internal.h"


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
  printf("top level configure: %dx%d\n", width, height);
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

  window->wl_surface = wl_compositor_create_surface(wl_back->compositor);
  wl_surface_set_user_data(window->wl_surface,window);

  window->decoration = wl_decoration_create(window->wl_surface,width,title);
  wl_surface_set_user_data(window->decoration->wl_surface,window);
  wl_surface_set_user_data(window->decoration->wl_closebutton_surface,window);

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
  if (window->base.visible) {
    window->xdg_surface =
      xdg_wm_base_get_xdg_surface(wl_back->xdg_wm_base, window->wl_surface);
    xdg_surface_add_listener(window->xdg_surface, &_wl_xdg_surface_listener, &window);
    window->xdg_toplevel = xdg_surface_get_toplevel(window->xdg_surface);
    xdg_toplevel_add_listener(window->xdg_toplevel,&_wl_xdg_toplevel_listener,window);
    xdg_toplevel_set_title(window->xdg_toplevel,window->title);
    window->base.visible = true;
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
  window->base.visible = false;
  //Cannot recreate the XDG Surface from a surface with an attached buffer.
  wl_surface_attach(window->wl_surface,NULL,0,0);
  wl_surface_commit(window->wl_surface);
}

#else

const int wl_window = 0;

#endif /* HAS_WAYLAND */

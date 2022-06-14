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
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <wayland-client.h>
#include <wayland-cursor.h>
#include "xdg-shell-client-protocol.h"

#include "../hashtable.h"
#include "../event.h"
#include "wl_backend.h"
#include "wl_backend_internal.h"
#include "wl_window_internal.h"

wl_backend_t *wl_back = NULL;


static hash_t
_wl_wid_hash(
  const struct wl_surface *surf)
{
  return (hash_t)(uintptr_t)surf;
}

static bool
_wl_wid_equal(
  const struct wl_surface *surf1,
  const struct wl_surface *surf2)
{
  return (surf1) == (surf2);
}



static void
_wl_registry_global(
  void *data,
  struct wl_registry *registry,
  uint32_t id,
  const char *interface,
  uint32_t version)
{

  wl_backend_t *wl_back = (wl_backend_t *)data;

printf("Global: %s\n", interface);

  if (strcmp(interface, wl_compositor_interface.name) == 0) {
    wl_back->compositor = (struct wl_compositor *)
      wl_registry_bind(registry, id, &wl_compositor_interface, 4);
  } else if (strcmp(interface, wl_seat_interface.name) == 0) {
    wl_back->seat = (struct wl_seat *)
      wl_registry_bind(registry, id, &wl_seat_interface, 1);
  } else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
    wl_back->xdg_wm_base = (struct xdg_wm_base *)
     wl_registry_bind(registry, id, &xdg_wm_base_interface, 1);
  } else if (strcmp(interface, wl_shm_interface.name) == 0) {
    wl_back->shm = (struct wl_shm *)
      wl_registry_bind(registry, id, &wl_shm_interface, 1);
  }

}

static void
_wl_registry_global_remove(
  void *data,
  struct wl_registry *registry,
  uint32_t id)
{
}

static const struct wl_registry_listener
_wl_registry_listener =
{
  .global = _wl_registry_global,
  .global_remove = _wl_registry_global_remove,
};





static void
_wl_xdg_wm_base_ping(
  void *data,
  struct xdg_wm_base *xdg_wm_base,
  uint32_t serial)
{
  xdg_wm_base_pong(xdg_wm_base, serial);
}

static const struct xdg_wm_base_listener
_wl_xdg_wm_base_listener =
{
  .ping = _wl_xdg_wm_base_ping,
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
  printf("top level close\n");
}

const struct xdg_toplevel_listener
_wl_xdg_toplevel_listener =
{
  .configure = _wl_xdg_toplevel_configure_handler,
  .close = _wl_xdg_toplevel_close_handler,
};





static void
_wl_pointer_enter_handler(
  void *data,
  struct wl_pointer *pointer,
  uint32_t serial,
  struct wl_surface *surface,
  wl_fixed_t x,
  wl_fixed_t y)
{
  wl_backend_t *wl_back = (wl_backend_t *)data;
  wl_pointer_set_cursor(pointer, serial,
                        wl_back->cursor_surface,
                        wl_back->cursor_image->hotspot_x,
                        wl_back->cursor_image->hotspot_y);
  event_t evt;
  
}

static void
_wl_pointer_leave_handler(
  void *data,
  struct wl_pointer *pointer,
  uint32_t serial,
  struct wl_surface *surface)
{

}

static void
_wl_pointer_motion_handler(
  void *data,
  struct wl_pointer *pointer,
  uint32_t time,
  wl_fixed_t x,
  wl_fixed_t y)
{
  
}

static void
_wl_pointer_button_handler(
  void *data,
  struct wl_pointer *pointer,
  uint32_t serial,
  uint32_t time,
  uint32_t button,
  uint32_t state)
{
  
  printf("button: 0x%x state: %d\n", button, state);
}

static void
_wl_pointer_axis_handler(
  void *data,
  struct wl_pointer *pointer,
  uint32_t time,
  uint32_t axis,
  wl_fixed_t value)
{

}

const struct wl_pointer_listener
_wl_pointer_listener =
{
  .enter = _wl_pointer_enter_handler,
  .leave = _wl_pointer_leave_handler,
  .motion = _wl_pointer_motion_handler,
  .button = _wl_pointer_button_handler,
  .axis = _wl_pointer_axis_handler,
};





bool
wl_backend_init(
  void)
{
  assert(wl_back == NULL);

  /* Allocate the backend object */
  wl_back = (wl_backend_t *)calloc(1, sizeof(wl_backend_t));
  if (wl_back == NULL) {
    return false;
  }

  /* Map from WL windows IDs to window objects */

  wl_back->surf_to_win = ht_new((key_hash_fun_t *)_wl_wid_hash,
                                (key_equal_fun_t *)_wl_wid_equal,
                                32);
  if (wl_back->surf_to_win == NULL) {
    wl_backend_terminate();
    return false;
  }



  /* Connect to Wayland server */
  wl_back->display = wl_display_connect(NULL);
  if (wl_back->display == NULL) {
    wl_backend_terminate();
    return false;
  }

  /* Get registry */
  wl_back->registry = wl_display_get_registry(wl_back->display);
  wl_registry_add_listener(wl_back->registry, &_wl_registry_listener, wl_back);

  /* Force sync so registry listener are run (this calls dispatch until sync processed) */
  wl_display_roundtrip(wl_back->display); /* Block until requests processed by server */

  /* Add listener to XDG so the window remains responsive */
  xdg_wm_base_add_listener(
    wl_back->xdg_wm_base, &_wl_xdg_wm_base_listener, wl_back);

  /* Retrieve the pointer and add listener */
  wl_back->pointer = wl_seat_get_pointer(wl_back->seat);
  wl_pointer_add_listener(wl_back->pointer, &_wl_pointer_listener, wl_back);

  /* Force sync */
  wl_display_roundtrip(wl_back->display);

  /* Load cursor */
  wl_back->cursor_theme = wl_cursor_theme_load(NULL, 24, wl_back->shm);
  wl_back->cursor =
    wl_cursor_theme_get_cursor(wl_back->cursor_theme, "left_ptr");
  wl_back->cursor_image = wl_back->cursor->images[0];
  wl_back->cursor_buffer = wl_cursor_image_get_buffer(wl_back->cursor_image);
  wl_back->cursor_surface = wl_compositor_create_surface(wl_back->compositor);
  wl_surface_attach(wl_back->cursor_surface, wl_back->cursor_buffer, 0, 0);
  wl_surface_commit(wl_back->cursor_surface);

  wl_display_roundtrip(wl_back->display);

  printf("Cursor OK\n");

  return true;
}

void
wl_backend_terminate(
  void)
{
  if (wl_back == NULL) {
    return;
  }

  /* Destroy cursor stuff */
  wl_cursor_theme_destroy(wl_back->cursor_theme);
  wl_buffer_destroy(wl_back->cursor_buffer);
  wl_surface_destroy(wl_back->cursor_surface);

  /* Destroy globals */
  xdg_wm_base_destroy(wl_back->xdg_wm_base);
  wl_shm_destroy(wl_back->shm);
  wl_pointer_release(wl_back->pointer);
  wl_pointer_destroy(wl_back->pointer);
  wl_seat_release(wl_back->seat);
  wl_seat_destroy(wl_back->seat);
  wl_compositor_destroy(wl_back->compositor);
  wl_registry_destroy(wl_back->registry);
  wl_display_roundtrip(wl_back->display);
  wl_display_disconnect(wl_back->display);


  if (wl_back->surf_to_win != NULL) {
    ht_delete(wl_back->surf_to_win);
  }


  free(wl_back);

  wl_back = NULL;
}

//TODO : Clean this up if it's not needed
static const struct wl_callback_listener wl_callback_listener;
static void wl_callback_handle_frame(void* data, struct wl_callback* wl_callback, uint32_t time)
{
  struct wl_window_t *wl_window = data;
  wl_callback_destroy(wl_callback);
  wl_window->wl_callback = wl_surface_frame(wl_window->wl_surface);
  wl_callback_add_listener(wl_window->wl_callback,&wl_callback_listener,wl_window);
  //Trigger event
  event_t evt;
  evt.type = EVENT_FRAME;
  //TODO : More precise clock
  evt.time = time;
  if (wl_window->base.visible)
  {
    //Attaches buffer and commits surface currently attached to this document
    evt.target = data;
    if (event_notify(wl_back->listener,&evt))
    {
      evt.type = EVENT_PRESENT;
      event_notify(wl_back->listener,&evt);
    }
  }
  wl_surface_damage(wl_window->wl_surface,0,0,1280,720);
  wl_surface_commit(wl_window->wl_surface);
  


}

static const struct wl_callback_listener wl_callback_listener = 
{
  .done = wl_callback_handle_frame
};

void
wl_backend_add_window(
  wl_window_t *w)
{
  assert(w != NULL);
  ht_add(wl_back->surf_to_win,(void*)&(w->wl_surface), (void*)w);
  //create a callback for the window
  w->wl_callback = wl_surface_frame(w->wl_surface);
  wl_callback_add_listener(w->wl_callback,&wl_callback_listener,w);
  wl_callback_handle_frame(w,w->wl_callback,0);
  xdg_toplevel_add_listener(w->xdg_toplevel,&_wl_xdg_toplevel_listener,w);
}

void
wl_backend_remove_window(
  const wl_window_t *w)
{
  assert(w != NULL);
  ht_remove(wl_back->surf_to_win,(void*)&(w->wl_surface));
}

wl_window_t *
wl_backend_get_window(
  const struct wl_surface* wl_surface)
{
  assert(wl_surface != 0);

  return (wl_window_t *)ht_find(wl_back->surf_to_win, (void *)&wl_surface);
}

void
wl_backend_set_listener(
  event_listener_t *listener)
{
  if (listener != NULL) {
    assert(listener->process_event != NULL);
  }
  wl_back->listener = listener;
}

event_listener_t *
wl_backend_get_listener(
  void)
{
  return wl_back->listener;
}

void
wl_backend_run(
  void)
{

  wl_back->running = true;

  while (wl_display_dispatch(wl_back->display) >= 0) {
    //Events are handled separately. This loop is left empty on purpose.
  }

}

void
wl_backend_stop(
  void)
{
  assert(wl_back != NULL);

  wl_back->running = false;
}

#else

const int wl_backend = 0;

#endif /* HAS_WAYLAND */

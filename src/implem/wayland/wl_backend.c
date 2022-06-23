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
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <wayland-client.h>
#include <wayland-cursor.h>
#include "xdg-shell-client-protocol.h"
#include <xkbcommon/xkbcommon.h>



#include "../event.h"
#include "wl_backend.h"
#include "wl_backend_internal.h"
#include "wl_window_internal.h"
#include "wl_keyboard.h"


wl_backend_t *wl_back = NULL;



int64_t
_wl_get_time()
{
  struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts.tv_sec/1000000 + ts.tv_nsec/(1000);
}

void 
_wl_find_resize_edge()
{
    const wl_fixed_t margin = 10 * 256;
    uint32_t resizeEdge = XDG_TOPLEVEL_RESIZE_EDGE_NONE;
    if (wl_back->focus_window->base.decorated && wl_back->inside_decor_location == DECOR_REGION_BAR && wl_back->mouse_posy < margin)
      resizeEdge |= XDG_TOPLEVEL_RESIZE_EDGE_TOP;
    else if (!wl_back->focus_window->base.decorated && wl_back->mouse_posy < margin)
      resizeEdge |= XDG_TOPLEVEL_RESIZE_EDGE_TOP;
    else if (!wl_back->inside_decor_location && wl_back->mouse_posy > wl_back->focus_window->base.height*256 - margin)
      resizeEdge |= XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM;
    if (wl_back->mouse_posx < margin)
      resizeEdge |= XDG_TOPLEVEL_RESIZE_EDGE_LEFT;
    else if (wl_back->mouse_posx > wl_back->focus_window->base.width*256 - margin)
      resizeEdge |= XDG_TOPLEVEL_RESIZE_EDGE_RIGHT;
    wl_back->current_resize_edge = resizeEdge;
}

void 
_wl_change_cursor(
  const char* cursorName
)
{
  wl_back->cursor =
    wl_cursor_theme_get_cursor(wl_back->cursor_theme, cursorName);
  wl_back->cursor_image = wl_back->cursor->images[0];
  wl_back->cursor_buffer = wl_cursor_image_get_buffer(wl_back->cursor_image);
  wl_surface_attach(wl_back->cursor_surface, wl_back->cursor_buffer, 0, 0);
  wl_surface_damage(wl_back->cursor_surface,0,0,24,24);
  wl_surface_commit(wl_back->cursor_surface);
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
  } else if (strcmp(interface, wl_subcompositor_interface.name) == 0) {
    wl_back->subcompositor = (struct wl_subcompositor*)
      wl_registry_bind(registry, id, &wl_subcompositor_interface,version);
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
_wl_pointer_enter_handler(
  void *data,
  struct wl_pointer *pointer,
  uint32_t serial,
  struct wl_surface *surface,
  wl_fixed_t x,
  wl_fixed_t y)
{
  wl_backend_t *wl_back = (wl_backend_t *)data;
  wl_back->mouse_posx = x;
  wl_back->mouse_posy = y;
  wl_pointer_set_cursor(pointer, serial,
                        wl_back->cursor_surface,
                        wl_back->cursor_image->hotspot_x,
                        wl_back->cursor_image->hotspot_y);
  if (surface)
  {
    wl_back->focus_window = (wl_window_t*)wl_surface_get_user_data(surface);
    if (wl_back->focus_window->base.decorated)
    {
      if (surface == wl_back->focus_window->decoration->wl_surface)
        wl_back->inside_decor_location = DECOR_REGION_BAR;
      else if (surface == wl_back->focus_window->decoration->wl_closebutton_surface)
        wl_back->inside_decor_location = DECOR_REGION_CLOSE_BUTTON;
    }
    else
      wl_back->inside_decor_location = DECOR_REGION_OUTSIDE;
  }
  else
  {
    wl_back->focus_window = NULL;
    wl_back->inside_decor_location = DECOR_REGION_OUTSIDE;
  }
  event_t evt;
  
}

static void
_wl_pointer_leave_handler(
  void *data,
  struct wl_pointer *pointer,
  uint32_t serial,
  struct wl_surface *surface)
{
  wl_backend_t *wl_back = (wl_backend_t *)data;
  wl_back->inside_decor_location = DECOR_REGION_OUTSIDE;
}

static void
_wl_pointer_motion_handler(
  void *data,
  struct wl_pointer *pointer,
  uint32_t time,
  wl_fixed_t x,
  wl_fixed_t y)
{
  if (wl_back->focus_window && wl_back->focus_window->base.visible) {
    wl_backend_t *wl_back = (wl_backend_t *)data;
    wl_back->mouse_posx = x;
    wl_back->mouse_posy = y;
    event_t evt;
    evt.type = EVENT_CURSOR;
    evt.time = _wl_get_time();
    evt.target = wl_back->focus_window;
    evt.desc.cursor.x = x/256;
    evt.desc.cursor.y = y/256;
    event_notify(wl_back->listener, &evt);
    _wl_find_resize_edge();
    if (wl_back->current_resize_edge != wl_back->old_resize_edge)
    {
      switch (wl_back->current_resize_edge)
      {
      case XDG_TOPLEVEL_RESIZE_EDGE_NONE:
        _wl_change_cursor("left_ptr");
        break;
      case XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM:
        _wl_change_cursor("bottom_side");
        break;
      case XDG_TOPLEVEL_RESIZE_EDGE_TOP:
        _wl_change_cursor("top_side");
        break;
      case XDG_TOPLEVEL_RESIZE_EDGE_RIGHT:
        _wl_change_cursor("right_side");
        break;
      case XDG_TOPLEVEL_RESIZE_EDGE_LEFT:
        _wl_change_cursor("left_side");
        break;
      case XDG_TOPLEVEL_RESIZE_EDGE_TOP_LEFT:
        _wl_change_cursor("top_left_corner");
        break;
      case XDG_TOPLEVEL_RESIZE_EDGE_TOP_RIGHT:
        _wl_change_cursor("top_right_corner");
        break;
      case XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM_LEFT:
        _wl_change_cursor("bottom_left_corner");
        break;
      case XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM_RIGHT:
        _wl_change_cursor("bottom_right_corner");
        break;
      default:
        break;
      }
    }
    wl_back->old_resize_edge = wl_back->current_resize_edge;
    

  }
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
  //When the focus screen is hidden with a key down event for example, the key up event will still trigger. This check mitigates that.
  if (wl_back->focus_window && wl_back->focus_window->base.visible && !wl_back->inside_decor_location) {
    wl_backend_t *wl_back = (wl_backend_t *)data;
    event_t evt;
    evt.type = EVENT_BUTTON;
    evt.time = _wl_get_time();
    evt.target = wl_back->focus_window;
    switch (button)
    {
    case 272:
      evt.desc.button.button = BUTTON_LEFT;
      break;
    case 273:
      evt.desc.button.button = BUTTON_RIGHT;
      break;
    case 274:
      evt.desc.button.button = BUTTON_MIDDLE;
      break;
    default:
      break;
    }
    evt.desc.button.state = (state == WL_POINTER_BUTTON_STATE_PRESSED) ? BUTTON_DOWN : BUTTON_UP;
    evt.desc.button.x = wl_back->mouse_posx / 256;
    evt.desc.button.y = wl_back->mouse_posy / 256;
    event_notify(wl_back->listener, &evt);
    printf("button: 0x%x state: %d\n", button, state);
  }

  if (state == WL_POINTER_BUTTON_STATE_PRESSED && button == 272)
  {
    const wl_fixed_t margin = 5 * 256;
    uint32_t resizeEdge = XDG_TOPLEVEL_RESIZE_EDGE_NONE;
    _wl_find_resize_edge();
    if (wl_back->current_resize_edge != XDG_TOPLEVEL_RESIZE_EDGE_NONE)
    {
      xdg_toplevel_resize(wl_back->focus_window->xdg_toplevel,wl_back->seat,serial,wl_back->current_resize_edge);
    }
    else if (wl_back->inside_decor_location == DECOR_REGION_BAR)
    {
      xdg_toplevel_move(wl_back->focus_window->xdg_toplevel,wl_back->seat,serial);
    }
    else if (wl_back->inside_decor_location == DECOR_REGION_CLOSE_BUTTON)
    {
      event_t evt;
      evt.type = EVENT_CLOSE;
      evt.target = wl_back->focus_window;
      event_notify(wl_back->listener,&evt);
    }
  }
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



//Keyboard event handlers

static void
_wl_keyboard_enter_handler(
  void *data,
  struct wl_keyboard *keyboard,
  uint32_t serial,
  struct wl_surface *surface,
  struct wl_array *keys
)
{
  wl_backend_t *wl_back = (wl_backend_t *)data;
  if (surface)
  {
    if (wl_surface_get_user_data(surface) != wl_back->focus_window)
    {
      wl_back->focus_window = wl_surface_get_user_data(surface);
    }
  }
  else //happens for some reason when the window gets destroyed
    wl_back->focus_window = NULL;
}

static void
_wl_keyboard_leave_handler(
  void *data,
  struct wl_keyboard *keyboard,
  uint32_t serial,
  struct wl_surface *surface)
{
}

static void
_wl_keyboard_key_handler(
  void *data,
  struct wl_keyboard *keyboard,
  uint32_t serial,
  uint32_t time,
  uint32_t key,
  enum wl_keyboard_key_state state
)
{
  if (wl_back->focus_window && wl_back->focus_window->base.visible) {
    wl_backend_t *wl_back = (wl_backend_t *)data;
    uint32_t pressedKeyCharacter = xkb_state_key_get_utf32(wl_back->xkb_state, key+8);
    event_t evt;
    evt.target = wl_back->focus_window;
    evt.time = _wl_get_time();
    evt.type = EVENT_KEY;
    evt.desc.key.code = sym_to_keycode[key];
    evt.desc.key.char_ = pressedKeyCharacter;
    evt.desc.key.state = (state == WL_KEYBOARD_KEY_STATE_RELEASED) ? KEY_UP : KEY_DOWN;
    event_notify(wl_back->listener,&evt);
  }
}

static void
_wl_keyboard_keymap_handler(
  void *data,
  struct wl_keyboard *keyboard,
  uint32_t format,
  int32_t fd,
  uint32_t size
)
{
  assert(format == WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1);
  wl_backend_t *wl_back = (wl_backend_t *)data;
  
  char *map_shm = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
  assert(map_shm != MAP_FAILED);
  
  wl_back->xkb_keymap = xkb_keymap_new_from_string(wl_back->xkb_context,map_shm,XKB_KEYMAP_FORMAT_TEXT_V1,XKB_KEYMAP_COMPILE_NO_FLAGS);
  munmap(map_shm,size);

  close(fd);

  printf("Set up keymap \n");

  wl_back->xkb_state = xkb_state_new(wl_back->xkb_keymap);

}
static void
_wl_keyboard_modifiers_handler(
  void *data,
  struct wl_keyboard *wl_keyboard,
  uint32_t serial,
  uint32_t depressed,
  uint32_t latched,
  uint32_t locked,
  uint32_t group
)
{
    wl_backend_t *wl_back = (wl_backend_t *)data;
    xkb_state_update_mask(wl_back->xkb_state,
        depressed, latched, locked, 0, 0, group);
}
static void
_wl_keyboard_repeat_info_handler(
  void *data,
  struct wl_keyboard *wl_keyboard,
  int32_t rate,
  int32_t delay
)
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

const struct wl_keyboard_listener
_wl_keyboard_listener =
{
  .enter = _wl_keyboard_enter_handler,
  .key = _wl_keyboard_key_handler,
  .keymap = _wl_keyboard_keymap_handler,
  .leave = _wl_keyboard_leave_handler,
  .modifiers = _wl_keyboard_modifiers_handler,
  .repeat_info = _wl_keyboard_repeat_info_handler
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
  if (wl_back->pointer)
    wl_pointer_add_listener(wl_back->pointer, &_wl_pointer_listener, wl_back);
  wl_back->inside_decor_location = DECOR_REGION_OUTSIDE;
  /* Retrieve the keyboard, iinitiate xkb_context and add listener */
  wl_back->keyboard = wl_seat_get_keyboard(wl_back->seat);
  if (wl_back->keyboard) {
    wl_back->xkb_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    wl_keyboard_add_listener(wl_back->keyboard,&_wl_keyboard_listener,wl_back);
  }

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
  if (wl_back->pointer) {
    wl_pointer_release(wl_back->pointer);
    wl_pointer_destroy(wl_back->pointer);
  }
  if (wl_back->keyboard) {
    wl_keyboard_release(wl_back->keyboard);
    wl_keyboard_destroy(wl_back->keyboard);
    xkb_state_ref(wl_back->xkb_state);
    xkb_keymap_unref(wl_back->xkb_keymap);
    xkb_context_unref(wl_back->xkb_context);
  }
  wl_seat_release(wl_back->seat);
  wl_seat_destroy(wl_back->seat);
  wl_subcompositor_destroy(wl_back->subcompositor);
  wl_compositor_destroy(wl_back->compositor);
  wl_registry_destroy(wl_back->registry);
  wl_display_roundtrip(wl_back->display);
  wl_display_disconnect(wl_back->display);


  free(wl_back);

  wl_back = NULL;
}

static const struct wl_callback_listener wl_callback_listener;
static void wl_callback_handle_frame(
  void* data, 
  struct wl_callback* wl_callback,
  uint32_t time)
{
  struct wl_window_t *wl_window = data;
  if (wl_callback) {
    wl_callback_destroy(wl_callback);
    wl_window->wl_callback = wl_surface_frame(wl_window->wl_surface);
    wl_callback_add_listener(wl_window->wl_callback,&wl_callback_listener,wl_window);
  }
  event_t evt;
  if (wl_window->pending_resize)
  {
    //resize decorations
    if (wl_window->base.decorated)
      wl_decoration_resize(wl_window->decoration,wl_window->base.width,wl_window->title);
    //resize surface
    evt.desc.resize.width = wl_window->base.width;
    evt.desc.resize.height = wl_window->base.height;
    evt.target = wl_window;
    evt.type = EVENT_RESIZE;
    event_notify(wl_back->listener,&evt);
    wl_window->pending_resize = false;
  }
  evt.type = EVENT_FRAME;
  evt.time = _wl_get_time();
  if (wl_window->base.visible) {
    evt.target = (wl_window_t*)data;
    if (event_notify(wl_back->listener,&evt))
    {
      if (wl_window->base.decorated)
        wl_decoration_present(wl_window->decoration);
      evt.type = EVENT_PRESENT;
      event_notify(wl_back->listener,&evt);
    }
  }
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
  //create a callback for the window
  w->wl_callback = wl_surface_frame(w->wl_surface);
  wl_callback_add_listener(w->wl_callback,&wl_callback_listener,w);
  wl_callback_handle_frame(w,w->wl_callback,0);
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

  while (wl_display_dispatch(wl_back->display) >= 0 && wl_back->running) {
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

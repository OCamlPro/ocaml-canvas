/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __WL_BACKEND_INTERNAL_H
#define __WL_BACKEND_INTERNAL_H

#include <stdbool.h>

#include <wayland-client.h>
#include <wayland-cursor.h>
#include "xdg-shell-client-protocol.h"
#include <xkbcommon/xkbcommon.h>

#include "../hashtable.h"
#include "../event.h"

typedef struct wl_backend_t {


  bool running;
  event_listener_t *listener;

  /* Globals */
  struct wl_display *display;
  struct wl_registry *registry;
  struct wl_compositor *compositor;
  struct wl_subcompositor *subcompositor;
  struct wl_seat *seat;
  struct wl_pointer *pointer;
  struct wl_keyboard *keyboard;
  struct wl_shm *shm;
  struct xdg_wm_base *xdg_wm_base;

  /* Objects */
  struct wl_cursor_theme *cursor_theme; /* contains wl_cursors */
  struct wl_cursor *cursor; /* contains cursor_images */
  struct wl_cursor_image *cursor_image;
  struct wl_buffer *cursor_buffer;
  struct wl_surface *cursor_surface;
  wl_window_t *focus_window;

  /*Keyboard objects*/
  struct xkb_keymap *xkb_keymap;
  struct xkb_state *xkb_state;
  struct xkb_context *xkb_context;

  /*Mouse objects*/
  wl_fixed_t mouse_posx;
  wl_fixed_t mouse_posy;
  uint8_t inside_decor;
} wl_backend_t;

extern wl_backend_t *wl_back;

#endif /* __WL_BACKEND_INTERNAL_H */

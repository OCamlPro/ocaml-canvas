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


/*
// common to all windows
static void
_wl_window_set_wm_class(
  wl_window_t *window,
  const char *res_name,
  const char *res_class)
{
  assert(window != NULL);
  assert(window->wid != XCB_WINDOW_NONE);
  assert(res_name != NULL);
  assert(res_class != NULL);

  size_t class_len = strlen(res_name) + 1 + strlen(res_class) + 1;
  char *class_hint = (car *)calloc(class_len, sizeof(char));
  if (class_hint == NULL) {
    return;
  }

  strcpy(class_hint, res_name);
  strcpy(class_hint + strlen(res_name) + 1, res_class);

  // Window class - ISO-8859-1
  xcb_change_property(wl_back->c, XCB_PROP_MODE_REPLACE, window->wid,
                      XCB_ATOM_WM_CLASS, XCB_ATOM_STRING, 8,
                      class_len, class_hint);

  free(class_hint);
}
*/
static void
_wl_window_update_position(
  wl_window_t *window)
{
  assert(window != NULL);
/*
  assert(window->wid != XCB_WINDOW_NONE);
  xcb_configure_window(wl_back->c, window->wid,
                       XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y,
                       (int32_t[]){ window->x, window->y });
*/
}

static void
_wl_xdg_surface_configure(
  void *data,
  struct xdg_surface *xdg_surface,
  uint32_t serial)
{
    //wl_window_t *window = (wl_window_t *)data;
    xdg_surface_ack_configure(xdg_surface, serial);

    //struct wl_buffer *buffer = draw_frame(window);
    //wl_surface_attach(window->wl_surface, buffer, 0, 0);
    //wl_surface_commit(window->wl_surface);
}

static const struct xdg_surface_listener
_wl_xdg_surface_listener = {
  .configure = _wl_xdg_surface_configure,
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

  window->wl_surface = wl_compositor_create_surface(wl_back->compositor);
  window->xdg_surface =
    xdg_wm_base_get_xdg_surface(wl_back->xdg_wm_base, window->wl_surface);
  xdg_surface_add_listener(window->xdg_surface, &_wl_xdg_surface_listener, &window);
  window->xdg_toplevel = xdg_surface_get_toplevel(window->xdg_surface);
  if (title != NULL) {
    xdg_toplevel_set_title(window->xdg_toplevel, title);
  }

  wl_display_roundtrip(wl_back->display);

  xdg_surface_set_window_geometry(window->xdg_surface,
                                  window->base.x, window->base.y,
                                  window->base.width, window->base.height);

  wl_display_roundtrip(wl_back->display);

/*
  // Create the WL window
  window->wid = xcb_generate_id(wl_back->c);
  xcb_create_window(wl_back->c,
                    XCB_COPY_FROM_PARENT,          // depth
                    window->wid,                   // window id
                    wl_back->screen->root,        // parent window
                    window->x, window->y,          // position (ignored)
                    window->width, window->height, // size (ignored)
                    0,                             // border width (ignored)
                    XCB_WINDOW_CLASS_INPUT_OUTPUT,
                    wl_back->screen->root_visual, // display format
                    XCB_CW_BACK_PIXEL |
                    XCB_CW_BORDER_PIXEL |
                    XCB_CW_EVENT_MASK,
                    (uint32_t[]){
                        wl_back->screen->white_pixel,
                        wl_back->screen->black_pixel,
                     // XCB_EVENT_MASK_NO_EVENT |
                        XCB_EVENT_MASK_KEY_PRESS |
                        XCB_EVENT_MASK_KEY_RELEASE |
                        XCB_EVENT_MASK_BUTTON_PRESS |
                        XCB_EVENT_MASK_BUTTON_RELEASE |
                        XCB_EVENT_MASK_ENTER_WINDOW |
                        XCB_EVENT_MASK_LEAVE_WINDOW |
                        XCB_EVENT_MASK_POINTER_MOTION |
                     // XCB_EVENT_MASK_POINTER_MOTION_HINT |
                     // XCB_EVENT_MASK_BUTTON_1_MOTION |
                     // XCB_EVENT_MASK_BUTTON_2_MOTION |
                     // XCB_EVENT_MASK_BUTTON_3_MOTION |
                     // XCB_EVENT_MASK_BUTTON_4_MOTION |
                     // XCB_EVENT_MASK_BUTTON_5_MOTION |
                     // XCB_EVENT_MASK_BUTTON_MOTION |
                        XCB_EVENT_MASK_KEYMAP_STATE |
                        XCB_EVENT_MASK_EXPOSURE |
                        XCB_EVENT_MASK_VISIBILITY_CHANGE |
                        XCB_EVENT_MASK_STRUCTURE_NOTIFY |
                     // XCB_EVENT_MASK_RESIZE_REDIRECT | // Don't use
                        XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
                     // XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT |
                        XCB_EVENT_MASK_FOCUS_CHANGE |
                        XCB_EVENT_MASK_PROPERTY_CHANGE |
                        XCB_EVENT_MASK_COLOR_MAP_CHANGE |
                     // XCB_EVENT_MASK_OWNER_GRAB_BUTTON |
                          0 });

  // Create graphic context
  window->cid = xcb_generate_id(wl_back->c);
  xcb_create_gc(wl_back->c, window->cid, window->wid,
                XCB_GC_GRAPHICS_EXPOSURES, (uint32_t[]){ 1 });

  // Set the title
  const char *t = (title != NULL) ? title : "";
  _wl_window_set_wm_class(window, "OCaml-Canvas", "OCaml-Canvas");
  wl_window_set_title(window, t);

  // Set the various protocols to support
  xcb_change_property(wl_back->c, XCB_PROP_MODE_PREPEND, window->wid,
                      wl_back->WM_PROTOCOLS, XCB_ATOM_ATOM,
                      32, 4, (xcb_atom_t[]){
                        wl_back->WM_DELETE_WINDOW,
                        wl_back->WM_TAKE_FOCUS,
                        wl_back->_NET_WM_PING,
                        wl_back->_NET_WM_SYNC_REQUEST
                      });

  // Set the owner pid
  pid_t pid = getpid();
  xcb_change_property(wl_back->c, XCB_PROP_MODE_REPLACE, window->wid,
                      wl_back->_NET_WM_PID, XCB_ATOM_CARDINAL,
                      32, 1, &pid);

  // Motif hints to disable decorations
  if (window->decorated == false) {
    xcb_change_property(wl_back->c, XCB_PROP_MODE_REPLACE, window->wid,
                        wl_back->_MOTIF_WM_HINTS, wl_back->_MOTIF_WM_HINTS,
                        32, 5, (uint32_t[]){ 0x02, 0x00, 0x00, 0x00, 0x00 });
*//*
    xcb_change_property(wl_back->c, XCB_PROP_MODE_REPLACE, window->wid,
                        wl_back->_NET_WM_WINDOW_TYPE, XCB_ATOM_ATOM,
                        32, 1, (uint32_t[]){ wl_back->_NET_WM_WINDOW_TYPE_SPLASH });
*//*
  }

*//*
  xcb_change_property(wl_back->c, XCB_PROP_MODE_REPLACE, window->wid,
                      XCB_ATOM_WM_TRANSIENT_FOR, XCB_ATOM_WINDOW,
                      32, 1, &(wl_back->screen->root));
*//*

  // Add to managed winddows
  wl_backend_add_window(window);

*//*
  // Flush any pending request
  xcb_flush(wl_back->c);
*/
  return window;
}

void
wl_window_destroy(
  wl_window_t *window)
{
  assert(window != NULL);
/*
  assert(window->wid != XCB_WINDOW_NONE);
  wl_backend_remove_window(window);
  xcb_destroy_window(wl_back->c, window->wid);
*/
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
//  assert(window->wid != XCB_WINDOW_NONE);
  assert(title != NULL);
/*
  // Regular window name - ISO-8859-1
  xcb_change_property(wl_back->c, XCB_PROP_MODE_REPLACE, window->wid,
                      XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8,
                      strlen(title), title);

  // Regular window name - UTF8
  xcb_change_property(wl_back->c, XCB_PROP_MODE_REPLACE, window->wid,
                      wl_back->_NET_WM_NAME, wl_back->UTF8_STRING, 8,
                      strlen(title), title);

  // Iconified window name - ISO-8859-1
  xcb_change_property(wl_back->c, XCB_PROP_MODE_REPLACE, window->wid,
                      XCB_ATOM_WM_ICON_NAME, XCB_ATOM_STRING, 8,
                      strlen(title), title);

  // Iconified window name - UTF8
  xcb_change_property(wl_back->c, XCB_PROP_MODE_REPLACE, window->wid,
                      wl_back->_NET_WM_ICON_NAME, wl_back->UTF8_STRING, 8,
                      strlen(title), title);*/
}

void
wl_window_set_size(
  wl_window_t *window,
  int32_t width,
  int32_t height)
{
  assert(window != NULL);
//  assert(window->wid != XCB_WINDOW_NONE);
  window->base.width = clip_i32_to_i16(width);
  window->base.height = clip_i32_to_i16(height);/*
  xcb_configure_window(wl_back->c, window->wid,
                       XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT,
                       (uint32_t[]){ window->width, window->height });*/
}

void
wl_window_set_position(
  wl_window_t *window,
  int32_t x,
  int32_t y)
{
  assert(window != NULL);
//  assert(window->wid != XCB_WINDOW_NONE);
  window->base.x = clip_i32_to_i16(x);
  window->base.y = clip_i32_to_i16(y);
  _wl_window_update_position(window);
}

void
wl_window_show(
  wl_window_t *window)
{
  assert(window != NULL);
//  assert(window->wid != XCB_WINDOW_NONE);
//  xcb_map_window(wl_back->c, window->wid);
  _wl_window_update_position(window);
}

void
wl_window_hide(
  wl_window_t *window)
{
  assert(window != NULL);
//  assert(window->wid != XCB_WINDOW_NONE);
//  xcb_unmap_window(wl_back->c, window->wid);
}

#else

const int wl_window = 0;

#endif /* HAS_WAYLAND */

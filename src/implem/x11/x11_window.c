/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifdef HAS_X11

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include <sys/types.h>
#include <unistd.h>

#include <xcb/xcb.h>
#include <xcb/xcb_image.h>
#include <xcb/xcb_keysyms.h>
#include <xcb/shm.h>

#include "../util.h"
#include "x11_backend.h"
#include "x11_backend_internal.h"
#include "x11_target.h"
#include "x11_window_internal.h"

static void
_x11_window_set_wm_class(
  x11_window_t *window,
  const char *res_name,
  const char *res_class)
{
  assert(window != NULL);
  assert(window->wid != XCB_WINDOW_NONE);
  assert(res_name != NULL);
  assert(res_class != NULL);

  size_t class_len = strlen(res_name) + 1 + strlen(res_class) + 1;
  char *class_hint = (char *)calloc(class_len, sizeof(char));
  if (class_hint == NULL) {
    return;
  }

  strcpy(class_hint, res_name);
  strcpy(class_hint + strlen(res_name) + 1, res_class);

  /* Window class - ISO-8859-1 */
  xcb_change_property(x11_back->c, XCB_PROP_MODE_REPLACE, window->wid,
                      XCB_ATOM_WM_CLASS, XCB_ATOM_STRING, 8,
                      class_len, class_hint);

  free(class_hint);
}

static void
_x11_window_update_position(
  x11_window_t *window)
{
  assert(window != NULL);
  assert(window->wid != XCB_WINDOW_NONE);

  window->pending_configures++;

  xcb_configure_window(x11_back->c, window->wid,
                       XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y,
                       (int32_t[]){ window->base.x, window->base.y });
  xcb_flush(x11_back->c);
}

x11_window_t *
x11_window_create(
  bool decorated,
  bool resizeable,
  bool minimize,
  bool maximize,
  bool close,
  const char *title,
  int32_t x,
  int32_t y,
  int32_t width,
  int32_t height)
{
  x11_window_t *window = (x11_window_t *)calloc(1, sizeof(x11_window_t));
  if (window == NULL) {
    return NULL;
  }

  window->base.visible = false;
  window->base.decorated = decorated;
  window->base.resizeable = resizeable;
  window->base.x = clip_i32_to_i16(x);
  window->base.y = clip_i32_to_i16(y);
  window->base.width = clip_i32_to_i16(max(1, width));
  window->base.height = clip_i32_to_i16(max(1, height));

  /* Create the X11 window */
  window->wid = xcb_generate_id(x11_back->c);
  xcb_create_window(x11_back->c,
                    XCB_COPY_FROM_PARENT,           // depth
                    window->wid,                    // window id
                    x11_back->screen->root,         // parent window
                    window->base.x, window->base.y, // position (ignored)
                    window->base.width, window->base.height, // size (ignored)
                    0,                              // border width (ignored)
                    XCB_WINDOW_CLASS_INPUT_OUTPUT,
                    x11_back->screen->root_visual,  // display format
                    XCB_CW_BACK_PIXEL |
                    XCB_CW_BORDER_PIXEL |
                    XCB_CW_BIT_GRAVITY | // To avoid flicker
                    XCB_CW_WIN_GRAVITY |
                    XCB_CW_EVENT_MASK,
                    (uint32_t[]){
                        x11_back->screen->black_pixel,
                        x11_back->screen->black_pixel,
                        XCB_GRAVITY_NORTH_WEST,
                        XCB_GRAVITY_NORTH_WEST,
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

  /* Set the title */
  const char *t = (title != NULL) ? title : "";
  _x11_window_set_wm_class(window, "OCaml-Canvas", "OCaml-Canvas");
  x11_window_set_title(window, t);

  /* Set the various protocols to support */
  xcb_change_property(x11_back->c, XCB_PROP_MODE_PREPEND, window->wid,
                      x11_back->WM_PROTOCOLS, XCB_ATOM_ATOM,
                      32, 4, (xcb_atom_t[]){
                        x11_back->WM_DELETE_WINDOW,
                        x11_back->WM_TAKE_FOCUS,
                        x11_back->_NET_WM_PING,
                        x11_back->_NET_WM_SYNC_REQUEST
                      });

  /* Set the owner pid */
  pid_t pid = getpid();
  xcb_change_property(x11_back->c, XCB_PROP_MODE_REPLACE, window->wid,
                      x11_back->_NET_WM_PID, XCB_ATOM_CARDINAL,
                      32, 1, &pid);

  /* Motif hints (disable decoration/resize) */
  uint32_t functions = MWM_FUNC_ALL;
  uint32_t decorations = MWM_DECOR_ALL;
  if (!(resizeable && minimize && maximize && close)) {
    functions = MWM_FUNC_MOVE |
      (resizeable ? MWM_FUNC_RESIZE : 0) |
      (minimize ? MWM_FUNC_MINIMIZE : 0) |
      (maximize && resizeable ? MWM_FUNC_MAXIMIZE : 0) |
      (close ? MWM_FUNC_CLOSE : 0);
    decorations = MWM_DECOR_BORDER | MWM_DECOR_TITLE | MWM_DECOR_MENU |
      (resizeable ? MWM_DECOR_RESIZEH : 0) |
      (minimize ? MWM_DECOR_MINIMIZE : 0) |
      (maximize && resizeable ? MWM_DECOR_MAXIMIZE : 0) |
      (close ? 0 : 0);
  }
  if (decorated == false) {
    decorations = 0;
  }
  xcb_change_property(x11_back->c, XCB_PROP_MODE_REPLACE, window->wid,
                      x11_back->_MOTIF_WM_HINTS, x11_back->_MOTIF_WM_HINTS,
                      32, 5, (uint32_t[]){
                        MWM_HINTS_FUNCTIONS | MWM_HINTS_DECORATIONS,
                        functions, decorations, 0x00, 0x00 });

/*
  xcb_change_property(x11_back->c, XCB_PROP_MODE_REPLACE, window->wid,
                      x11_back->_NET_WM_WINDOW_TYPE, XCB_ATOM_ATOM,
                      32, 1, (uint32_t[]){
                        x11_back->_NET_WM_WINDOW_TYPE_SPLASH });
*/

  if (window->base.resizeable == false) {
    xcb_change_property(x11_back->c, XCB_PROP_MODE_REPLACE, window->wid,
                        XCB_ATOM_WM_NORMAL_HINTS, XCB_ATOM_WM_SIZE_HINTS,
                        32, 18, (uint32_t[]){
                          0x30, 0x00, 0x00, 0x00, 0x00,
                          window->base.width, window->base.height,
                          window->base.width, window->base.height,
                          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                          0x00, 0x00, 0x00, 0x00 });
  }

/*
  xcb_change_property(x11_back->c, XCB_PROP_MODE_REPLACE, window->wid,
                      XCB_ATOM_WM_TRANSIENT_FOR, XCB_ATOM_WINDOW,
                      32, 1, &(x11_back->screen->root));
*/

  /* Add to managed windows */
  x11_backend_add_window(window);

  /* Flush any pending request */
  xcb_flush(x11_back->c);

  return window;
}

void
x11_window_destroy(
  x11_window_t *window)
{
  assert(window != NULL);
  assert(window->wid != XCB_WINDOW_NONE);

  x11_backend_remove_window(window);
  xcb_destroy_window(x11_back->c, window->wid);
  xcb_flush(x11_back->c);
  free(window);
}

x11_target_t *
x11_window_get_target(
  x11_window_t *window)
{
  assert(window != NULL);
  assert(window->wid != XCB_WINDOW_NONE);

  return x11_target_create(window->wid);
}

void
x11_window_set_title(
  x11_window_t *window,
  const char *title)
{
  assert(window != NULL);
  assert(window->wid != XCB_WINDOW_NONE);
  assert(title != NULL);

  /* Regular window name - ISO-8859-1 */
  xcb_change_property(x11_back->c, XCB_PROP_MODE_REPLACE, window->wid,
                      XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8,
                      strlen(title), title);

  /* Regular window name - UTF8 */
  xcb_change_property(x11_back->c, XCB_PROP_MODE_REPLACE, window->wid,
                      x11_back->_NET_WM_NAME, x11_back->UTF8_STRING, 8,
                      strlen(title), title);

  /* Iconified window name - ISO-8859-1 */
  xcb_change_property(x11_back->c, XCB_PROP_MODE_REPLACE, window->wid,
                      XCB_ATOM_WM_ICON_NAME, XCB_ATOM_STRING, 8,
                      strlen(title), title);

  /* Iconified window name - UTF8 */
  xcb_change_property(x11_back->c, XCB_PROP_MODE_REPLACE, window->wid,
                      x11_back->_NET_WM_ICON_NAME, x11_back->UTF8_STRING, 8,
                      strlen(title), title);

  xcb_flush(x11_back->c);
}

void
x11_window_set_size(
  x11_window_t *window,
  int32_t width,
  int32_t height)
{
  assert(window != NULL);
  assert(window->wid != XCB_WINDOW_NONE);

  window->pending_configures++;

  window->base.width = clip_i32_to_i16(width);
  window->base.height = clip_i32_to_i16(height);

  xcb_configure_window(x11_back->c, window->wid,
                       XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT,
                       (uint32_t[]){ (uint32_t)window->base.width,
                                     (uint32_t)window->base.height });

  xcb_flush(x11_back->c);
}

void
x11_window_set_position(
  x11_window_t *window,
  int32_t x,
  int32_t y)
{
  assert(window != NULL);
  assert(window->wid != XCB_WINDOW_NONE);

  window->base.x = clip_i32_to_i16(x);
  window->base.y = clip_i32_to_i16(y);
  _x11_window_update_position(window);
}

void
x11_window_show(
  x11_window_t *window)
{
  assert(window != NULL);
  assert(window->wid != XCB_WINDOW_NONE);

  xcb_map_window(x11_back->c, window->wid);
  xcb_flush(x11_back->c);
  _x11_window_update_position(window);
}

void
x11_window_hide(
  x11_window_t *window)
{
  assert(window != NULL);
  assert(window->wid != XCB_WINDOW_NONE);

  xcb_unmap_window(x11_back->c, window->wid);
  xcb_flush(x11_back->c);
}

#else

const int x11_window = 0;

#endif /* HAS_X11 */

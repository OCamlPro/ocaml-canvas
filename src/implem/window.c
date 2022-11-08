/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "config.h"
#include "tuples.h"
#include "target.h"
#include "window_internal.h"

#ifdef HAS_GDI
#include "gdi/gdi_window.h"
#endif
#ifdef HAS_QUARTZ
#include "quartz/qtz_window.h"
#endif
#ifdef HAS_X11
#include "x11/x11_window.h"
#endif
#ifdef HAS_WAYLAND
#include "wayland/wl_window.h"
#endif

window_t *
window_create(
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
  window_t *w = NULL;

  switch_IMPL() {
    case_GDI(
      w = (window_t *)gdi_window_create(decorated, resizeable, minimize,
                                        maximize, close, title,
                                        x, y, width, height)
    );
    case_QUARTZ(
      w = (window_t *)qtz_window_create(decorated, resizeable, minimize,
                                        maximize, close, title,
                                        x, y, width, height)
    );
    case_X11(
      w = (window_t *)x11_window_create(decorated, resizeable, minimize,
                                        maximize, close, title,
                                        x, y, width, height)
    );
    case_WAYLAND(
      w = (window_t *)wl_window_create(decorated, resizeable, minimize,
                                       maximize, close, title,
                                       x, y, width, height)
    );
    default_fail();
  }

  return w;
}

void
window_destroy(
  window_t *window)
{
  switch_IMPL() {
    case_GDI(
      gdi_window_destroy((gdi_window_t *)window)
    );
    case_QUARTZ(
      qtz_window_destroy((qtz_window_t *)window)
    );
    case_X11(
      x11_window_destroy((x11_window_t *)window)
    );
    case_WAYLAND(
      wl_window_destroy((wl_window_t *)window)
    );
    default_fail();
  }
}

void *
window_get_data(
  window_t *window)
{
  assert(window != NULL);

  return window->data;
}

void
window_set_data(
  window_t *window,
  void *data)
{
  assert(window != NULL);

  window->data = data;
}

target_t *
window_get_target(
  window_t *window)
{
  target_t *t = NULL;

  switch_IMPL() {
    case_GDI(
      t = (target_t *)gdi_window_get_target((gdi_window_t *)window)
    );
    case_QUARTZ(
      t = (target_t *)qtz_window_get_target((qtz_window_t *)window)
    );
    case_X11(
      t = (target_t *)x11_window_get_target((x11_window_t *)window)
    );
    case_WAYLAND(
      t = (target_t *)wl_window_get_target((wl_window_t *)window)
    );
    default_fail();
  }

  return t;
}

void
window_set_title(
  window_t *window,
  const char *title)
{
  switch_IMPL() {
    case_GDI(
      gdi_window_set_title((gdi_window_t *)window, title)
    );
    case_QUARTZ(
      qtz_window_set_title((qtz_window_t *)window, title)
    );
    case_X11(
      x11_window_set_title((x11_window_t *)window, title)
    );
    case_WAYLAND(
      wl_window_set_title((wl_window_t *)window, title)
    );
    default_fail();
  }
}

void
window_set_size(
  window_t *window,
  int32_t width,
  int32_t height)
{
  switch_IMPL() {
    case_GDI(
      gdi_window_set_size((gdi_window_t *)window, width, height)
    );
    case_QUARTZ(
      qtz_window_set_size((qtz_window_t *)window, width, height)
    );
    case_X11(
      x11_window_set_size((x11_window_t *)window, width, height)
    );
    case_WAYLAND(
      wl_window_set_size((wl_window_t *)window, width, height)
    );
    default_fail();
  }
}

pair_t(int32_t)
window_get_position(
  window_t *window)
{
  assert(window != NULL);

  return pair(int32_t, window->x, window->y);
}

void
window_set_position(
  window_t *window,
  int32_t x,
  int32_t y)
{
  switch_IMPL() {
    case_GDI(
      gdi_window_set_position((gdi_window_t *)window, x, y)
    );
    case_QUARTZ(
      qtz_window_set_position((qtz_window_t *)window, x, y)
    );
    case_X11(
      x11_window_set_position((x11_window_t *)window, x, y)
    );
    case_WAYLAND(
      wl_window_set_position((wl_window_t *)window, x, y)
    );
    default_fail();
  }
}

void
window_show(
  window_t *window)
{
  assert(window != NULL);

  window->visible = true;

  switch_IMPL() {
    case_GDI(
      gdi_window_show((gdi_window_t *)window)
    );
    case_QUARTZ(
      qtz_window_show((qtz_window_t *)window)
    );
    case_X11(
      x11_window_show((x11_window_t *)window)
    );
    case_WAYLAND(
      wl_window_show((wl_window_t *)window)
    );
    default_fail();
  }
}

void
window_hide(
  window_t *window)
{
  assert(window != NULL);

  window->visible = false;

  switch_IMPL() {
    case_GDI(
      gdi_window_hide((gdi_window_t *)window)
    );
    case_QUARTZ(
      qtz_window_hide((qtz_window_t *)window)
    );
    case_X11(
      x11_window_hide((x11_window_t *)window)
    );
    case_WAYLAND(
      wl_window_hide((wl_window_t *)window)
    );
    default_fail();
  }
}

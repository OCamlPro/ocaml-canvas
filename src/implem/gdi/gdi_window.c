/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifdef HAS_GDI

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include <windows.h>
#undef min
#undef max

#include "../util.h"
#include "../unicode.h"
#include "gdi_backend.h"
#include "gdi_backend_internal.h"
#include "gdi_target.h"
#include "gdi_window_internal.h"

typedef struct window_size_pos_t {
  int32_t x;
  int32_t y;
  int32_t width;
  int32_t height;
} window_size_pos_t;

static window_size_pos_t
_gdi_window_size_pos(
  gdi_window_t *window)
{
  assert(window != NULL);

  RECT rect = (RECT){ .left = window->base.x, .top = window->base.y,
                      .right = window->base.x + window->base.width,
                      .bottom = window->base.y + window->base.height };
  AdjustWindowRect(&rect,
                   window->base.decorated ? WS_OVERLAPPEDWINDOW : WS_POPUP,
                   FALSE);
  return (window_size_pos_t){
    .x = rect.left,
    .y = rect.top,
    .width = rect.right - rect.left,
    .height = rect.bottom - rect.top
  };
}

static void
_gdi_window_update_position(
  gdi_window_t *window)
{
  assert(window != NULL);
  assert(window->hwnd != NULL);

  window_size_pos_t wsp = _gdi_window_size_pos(window);
  SetWindowPos(window->hwnd, NULL,
               wsp.x, wsp.y, wsp.width, wsp.height,
               SWP_NOSIZE | SWP_NOZORDER | SWP_NOREDRAW);
}

gdi_window_t *
gdi_window_create(
  bool decorated,
  const char *title,
  int32_t x,
  int32_t y,
  int32_t width,
  int32_t height)
{
  gdi_window_t *window = (gdi_window_t *)calloc(1, sizeof(gdi_window_t));
  if (window == NULL) {
    return NULL;
  }

  WCHAR *wtitle = (title == NULL) ? NULL : mbs_to_wcs(title);

  window->base.visible = false;
  window->base.decorated = decorated;
  window->base.x = clip_i32_to_i16(x);
  window->base.y = clip_i32_to_i16(y);
  window->base.width = clip_i32_to_i16(max(1, width));
  window->base.height = clip_i32_to_i16(max(1, height));

  window_size_pos_t wsp = _gdi_window_size_pos(window);

  /* Create the GDI window */
  window->hwnd = CreateWindowExW(0, //WS_EX_LAYERED,
                                 window->base.decorated ?
                                   gdi_back->class_framed :
                                   gdi_back->class_frameless,
                                 wtitle,
                                 window->base.decorated ?
                                   WS_OVERLAPPEDWINDOW :
                                   WS_POPUP,
                                 wsp.x, wsp.y,
                                 wsp.width, wsp.height,
                                 NULL, // Parent HWND
                                 NULL, // HMENU
                                 gdi_back->hinst,
                                 NULL /* LPVOID lpParam*/);

  if (wtitle != NULL) {
    free(wtitle);
  }

  if (window->hwnd == NULL) {
    free(window);
    return NULL;
  }

  /* Add to managed winddows */
  gdi_backend_add_window(window);

  return window;
}

void
gdi_window_destroy(
  gdi_window_t *window)
{
  assert(window != NULL);
  assert(window->hwnd != NULL);

  gdi_backend_remove_window(window);
  DestroyWindow(window->hwnd);
  free(window);
}

gdi_target_t *
gdi_window_get_target(
  gdi_window_t *window)
{
  assert(window != NULL);
  assert(window->hwnd != NULL);

  return gdi_target_create(window->hwnd);
}

void
gdi_window_set_title(
  gdi_window_t *window,
  const char *title)
{
  assert(window != NULL);
  assert(window->hwnd != NULL);
  assert(title != NULL);

  WCHAR *wtitle = mbs_to_wcs(title);
  if (wtitle != NULL) {
    SetWindowTextW(window->hwnd, wtitle);
    free(wtitle);
  }
}

void
gdi_window_set_size(
  gdi_window_t *window,
  int32_t width,
  int32_t height)
{
  assert(window != NULL);
  assert(window->hwnd != NULL);

  window->base.width = clip_i32_to_i16(width);
  window->base.height = clip_i32_to_i16(height);
  window_size_pos_t wsp = _gdi_window_size_pos(window);
  SetWindowPos(window->hwnd, NULL,
               wsp.x, wsp.y, wsp.width, wsp.height,
               SWP_NOMOVE | SWP_NOZORDER | SWP_NOREDRAW);
}

void
gdi_window_set_position(
  gdi_window_t *window,
  int32_t x,
  int32_t y)
{
  assert(window != NULL);
  assert(window->hwnd != NULL);

  window->base.x = clip_i32_to_i16(x);
  window->base.y = clip_i32_to_i16(y);
  _gdi_window_update_position(window);
}

void
gdi_window_show(
  gdi_window_t *window)
{
  assert(window != NULL);
  assert(window->hwnd != NULL);

  ShowWindow(window->hwnd, SW_SHOWNORMAL);
  _gdi_window_update_position(window);
}

void
gdi_window_hide(
  gdi_window_t *window)
{
  assert(window != NULL);
  assert(window->hwnd != NULL);

  ShowWindow(window->hwnd, SW_HIDE);
}

#else

const int gdi_window = 0;

#endif /* HAS_GDI */

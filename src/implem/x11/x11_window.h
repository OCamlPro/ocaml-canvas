/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __X11_WINDOW_H
#define __X11_WINDOW_H

#include "x11_target.h"

typedef struct x11_window_t x11_window_t;

x11_window_t *
x11_window_create(
  bool decorated,
  const char *title,
  int32_t x,
  int32_t y,
  int32_t width,
  int32_t height);

void
x11_window_destroy(
  x11_window_t *window);

x11_target_t *
x11_window_get_target(
  x11_window_t *window);

void
x11_window_set_title(
  x11_window_t *window,
  const char *title);

void
x11_window_set_size(
  x11_window_t *window,
  int32_t width,
  int32_t height);

void
x11_window_set_position(
  x11_window_t *window,
  int32_t x,
  int32_t y);

void
x11_window_show(
  x11_window_t *window);

void
x11_window_hide(
  x11_window_t *window);

#endif /* __X11_WINDOW_H */

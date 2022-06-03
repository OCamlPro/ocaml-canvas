/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __GDI_WINDOW_H
#define __GDI_WINDOW_H

#include "gdi_target.h"

typedef struct gdi_window_t gdi_window_t;

gdi_window_t *
gdi_window_create(
  bool decorated,
  const char *title,
  int32_t x,
  int32_t y,
  int32_t width,
  int32_t height);

void
gdi_window_destroy(
  gdi_window_t *window);

gdi_target_t *
gdi_window_get_target(
  gdi_window_t *window);

void
gdi_window_set_title(
  gdi_window_t *window,
  const char *title);

void
gdi_window_set_size(
  gdi_window_t *window,
  int32_t width,
  int32_t height);

void
gdi_window_set_position(
  gdi_window_t *window,
  int32_t x,
  int32_t y);

void
gdi_window_show(
  gdi_window_t *window);

void
gdi_window_hide(
  gdi_window_t *window);

#endif /* __GDI_WINDOW_H */

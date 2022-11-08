/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __QTZ_WINDOW_H
#define __QTZ_WINDOW_H

#include "qtz_target.h"

typedef struct qtz_window_t qtz_window_t;

qtz_window_t *
qtz_window_create(
  bool decorated,
  bool resizeable,
  bool minimize,
  bool maximize,
  bool close,
  const char *title,
  int32_t x,
  int32_t y,
  int32_t width,
  int32_t height);

void
qtz_window_destroy(
  qtz_window_t *window);

qtz_target_t *
qtz_window_get_target(
  qtz_window_t *window);

void
qtz_window_set_title(
  qtz_window_t *window,
  const char *title);

void
qtz_window_set_size(
  qtz_window_t *window,
  int32_t width,
  int32_t height);

void
qtz_window_set_position(
  qtz_window_t *window,
  int32_t x,
  int32_t y);

void
qtz_window_show(
  qtz_window_t *window);

void
qtz_window_hide(
  qtz_window_t *window);

#endif /* __QTZ_WINDOW_H */

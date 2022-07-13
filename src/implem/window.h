/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __WINDOW_H
#define __WINDOW_H

#include <stdbool.h>
#include <stdint.h>

#include "tuples.h"
#include "target.h"

typedef struct window_t window_t;

window_t *
window_create(
  bool decorated,
  const char *title,
  int32_t x,
  int32_t y,
  int32_t width,
  int32_t height);

void
window_destroy(
  window_t *window);

void *
window_get_data(
  window_t *window);

void
window_set_data(
  window_t *window,
  void *data);

target_t *
window_get_target(
  window_t *window);

void
window_set_title(
  window_t *window,
  const char *title);

void
window_set_size(
  window_t *window,
  int32_t width,
  int32_t height);

pair_t(int32_t)
window_get_position(
  window_t *window);

void
window_set_position(
  window_t *window,
  int32_t x,
  int32_t y);

void
window_show(
  window_t *window);

void
window_hide(
  window_t *window);

#endif /* __WINDOW_H */

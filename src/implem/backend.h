/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __BACKEND_H
#define __BACKEND_H

#include <stdint.h>
#include <stdbool.h>

#include "config.h"
#include "event.h"
#include "canvas.h"

bool
backend_init(
  impl_type_t impl_type);

void
backend_terminate(
  void);

void
backend_run(
  event_listener_t *event_listener);

void
backend_stop(
  void);

int32_t
backend_next_id(
  void);

void
backend_add_canvas(
  canvas_t *canvas);

void
backend_remove_canvas(
  canvas_t *canvas);

canvas_t *
backend_get_canvas(
  int32_t id);

#endif /* __BACKEND_H */

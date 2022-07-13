/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __CANVAS_INTERNAL_H
#define __CANVAS_INTERNAL_H

#include <stdint.h>

#include "list.h"
#include "window.h"
#include "surface.h"
#include "state.h"
#include "font.h"
#include "path.h"
#include "canvas.h"

typedef struct canvas_t {
  window_t *window;
  surface_t *surface;
  int32_t width; // update with window size
  int32_t height; // should be equal to window size
  state_t *state;
  font_t *font;
  list_t *state_stack;
  path_t *path;
  double first_x;
  double first_y;
  double last_x;
  double last_y;
  void *data; /* user data */
  int32_t id;
  canvas_type_t type;
} canvas_t;

#endif /* __CANVAS_INTERNAL_H */

/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __PATTERN_H
#define __PATTERN_H

#include "object.h"
#include "pixmap.h"
#include "transform.h"

typedef struct pattern_t pattern_t;

typedef enum pattern_repeat_t {
  PATTERN_NO_REPEAT = 0,
  PATTERN_REPEAT_X  = 1,
  PATTERN_REPEAT_Y  = 2,
  PATTERN_REPEAT_XY = 3
} pattern_repeat_t;

DECLARE_OBJECT_METHODS(pattern_t, pattern)

pattern_t *
pattern_create(
  const pixmap_t *image,
  pattern_repeat_t repeat);

color_t_
pattern_evaluate_pos(
  const pattern_t *pattern,
  double pos_x,
  double pos_y,
  const transform_t *inverse);

void
pattern_set_destroy_callback(
  void (*callback_function)(pattern_t *));

#endif /* __PATTERN_H */

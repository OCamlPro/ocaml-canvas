/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __GRADIENT_H
#define __GRADIENT_H

#include <stdbool.h>

#include "object.h"
#include "color.h"
#include "transform.h"

typedef struct gradient_t gradient_t;

DECLARE_OBJECT_METHODS(gradient_t, gradient)

gradient_t *
gradient_create_linear(
  double pos1_x,
  double pos1_y,
  double pos2_x,
  double pos2_y);

gradient_t *
gradient_create_radial(
  double center1_x,
  double center1_y,
  double rad1,
  double center2_x,
  double center2_y,
  double rad2);

gradient_t *
gradient_create_conic(
  double center_x,
  double center_y,
  double angle);

bool
gradient_add_color_stop(
  gradient_t *gradient,
  color_t_ color,
  double pos);

color_t_
gradient_evaluate_pos(
  const gradient_t *gradient,
  double pos_x,
  double pos_y,
  const transform_t *inverse);

void
gradient_set_destroy_callback(
  void (*callback_function)(gradient_t *));

#endif /* __GRADIENT_H */

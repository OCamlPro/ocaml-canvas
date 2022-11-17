/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __POLYGONIZE_H
#define __POLYGONIZE_H

#include <stdint.h>
#include <stdbool.h>

#include "point.h"
#include "rect.h"
#include "path.h"
#include "polygon.h"
#include "transform.h"

typedef enum join_type_t {
  JOIN_ROUND = 0,
  JOIN_MITER = 1,
  JOIN_BEVEL = 2,
} join_type_t;

typedef enum cap_type_t {
  CAP_BUTT = 0,
  CAP_SQUARE = 1,
  CAP_ROUND = 2
} cap_type_t;


void
quadratic_to_poly(
  point_t p1,
  point_t p2,
  point_t p3,
  polygon_t *p,
  int n);

void
bezier_to_poly(
  point_t p1,
  point_t p2,
  point_t p3,
  point_t p4,
  polygon_t *p,
  int n);

bool
polygonize(
  path_t *path,
  polygon_t *p,
  rect_t *bbox);

bool
polygonize_outline(
  path_t *path,
  double w,
  polygon_t *p,
  rect_t *bbox,
  join_type_t join_type,
  cap_type_t cap_type,
  double miter_limit,
  const transform_t *transform,
  bool only_linear,
  const double *dash,
  int32_t dash_array_size,
  double dash_offset);

void
polygon_offset(
  const polygon_t *p,
  polygon_t *np,
  double w,
  join_type_t join_type,
  cap_type_t cap_type,
  double miter_limit,
  const transform_t *transform,
  bool only_linear,
  const double *dash,
  int32_t dash_array_size,
  double dash_offset);

#endif /* __POLYGONIZE_H */

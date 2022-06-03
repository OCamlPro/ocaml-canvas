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

#include <stdbool.h>

#include "point.h"
#include "rect.h"
#include "path.h"
#include "polygon.h"

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
  rect_t *bbox);

void
polygon_offset(
  const polygon_t *p,
  polygon_t *np,
  double w);

#endif /* __POLYGONIZE_H */

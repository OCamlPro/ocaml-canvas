/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __ARC_H
#define __ARC_H

//#include <stdint.h>
//#include <stdbool.h>

#include "point.h"

int
arc_to_bezier(
  double *values,
  double x,
  double y,
  double rx,
  double ry,
  double a1,
  double a2,
  bool ccw);

int
arc_to_bezier_p(
  double *values,
  point_t c,
  double r,
  point_t p1,
  point_t p2);

int
arcto_to_bezier(
  double *values,
  double x0,
  double y0,
  double x1,
  double y1,
  double x2,
  double y2,
  double r);

#endif /* __ARC_H */

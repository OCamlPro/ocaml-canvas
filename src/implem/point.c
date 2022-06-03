/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#include <math.h>

#include "point.h"

double
point_angle(
  point_t p1,
  point_t p2,
  point_t p3)
{
  double d1_2 = point_dist_2(p1, p2); double d1 = sqrt(d1_2);
  double d2_2 = point_dist_2(p2, p3); double d2 = sqrt(d2_2);
  double d3_2 = point_dist_2(p3, p1);

  double c = (d1_2 + d2_2 - d3_2) / (2.0 * d1 * d2);
  if (c > 1.0) {
    c = 1.0;
  }

  return acos(c);
}

point_t
point_offset_ortho(
  point_t p1,
  point_t p2,
  double o)
{
  double r = o / point_dist(p1, p2);
  return point(p1.x + (p2.y - p1.y) * r, p1.y - (p2.x - p1.x) * r);
}

point_t
point_offset(
  point_t p1,
  point_t p2,
  point_t p3,
  double o)
{
  double a1 = atan2(p3.y - p2.y, p3.x - p2.x);
  double a2 = atan2(p1.y - p2.y, p1.x - p2.x);

  if (a2 < a1) { a2 += 2.0 * M_PI; }

  double a = (a1 + a2) * 0.5;

  return point(p2.x - o * cos(a), p2.y - o * sin(a));
}

int
point_position(
  point_t p1,
  point_t p2,
  point_t p)
{
  double d = (p2.y - p1.y) * (p.x - p1.x) - (p2.x - p1.x) * (p.y - p1.y);
  if (d < 0) return -1;
  else if (d > 0) return 1;
  else return 0;
}

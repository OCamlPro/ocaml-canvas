/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#include <stdbool.h>
#include <math.h> // Note: on Win32, add #define _USE_MATH_DEFINES for M_PI
#include <assert.h>

#include "util.h"
#include "point.h"

// Requires (abs(a2 - a1) < Pi)
// if a1 < a2 -> draws clockwise
// if a2 < a1 -> draws counter-clockwise
static void
_arc_to_bezier_internal(
  double *values,
  double x,
  double y,
  double rx,
  double ry,
  double a1,
  double a2)
{
  assert(values != NULL);
  assert(fabs(a2 - a1) < M_PI);

  double cos_a1 = cos(a1);
  double cos_a2 = cos(a2);
  double sin_a1 = sin(a1);
  double sin_a2 = sin(a2);
  double k = 4.0/3.0 * tan((a2 - a1) / 4.0);

  values[0] = x + rx * (cos_a1 - k * sin_a1);
  values[1] = y + ry * (sin_a1 + k * cos_a1);

  values[2] = x + rx * (cos_a2 + k * sin_a2);
  values[3] = y + ry * (sin_a2 - k * cos_a2);

  values[4] = x + rx * cos_a2;
  values[5] = y + ry * sin_a2;
}

int
arc_to_bezier(
  double *values,
  double x,
  double y,
  double rx,
  double ry,
  double a1,
  double a2,
  bool ccw)
{
  assert(values != NULL);

  double d = 0.0;

  if (!ccw) {
    if (a1 < a2) d = min(a2 - a1, 2.0 * M_PI);
    else if (a1 > a2) d = normalize_angle(a2 - a1);
    a1 = normalize_angle(a1);
    a2 = a1 + d;
  } else {
    if (a1 > a2) d = min(a1 - a2, 2.0 * M_PI);
    else if (a1 < a2) d = normalize_angle(a1 - a2);
    a2 = normalize_angle(a2);
    a1 = a2 + d;
  }

  values[0] = x + rx * cos(a1);
  values[1] = y + ry * sin(a1);

  if (d < M_PI_2) {
    _arc_to_bezier_internal(values + 2, x, y, rx, ry, a1, a2);
    return 1;

  } else if (d < M_PI) {
    _arc_to_bezier_internal(values + 2, x, y, rx, ry,
                            a1, a1 + (a2 - a1) * 0.5);
    _arc_to_bezier_internal(values + 8, x, y, rx, ry,
                            a1 + (a2 - a1) * 0.5, a2);
    return 2;

  } else {
    _arc_to_bezier_internal(values + 2, x, y, rx, ry,
                            a1, a1 + (a2 - a1) * 0.25);
    _arc_to_bezier_internal(values + 8, x, y, rx, ry,
                            a1 + (a2 - a1) * 0.25,
                            a1 + (a2 - a1) * 0.5);
    _arc_to_bezier_internal(values + 14, x, y, rx, ry,
                            a1 + (a2 - a1) * 0.5,
                            a1 + (a2 - a1) * 0.75);
    _arc_to_bezier_internal(values + 20, x, y, rx, ry,
                            a1 + (a2 - a1) * 0.75, a2);
    return 4;
  }
}

int
arc_to_bezier_p(
  double *values,
  point_t c,
  double r,
  point_t p1,
  point_t p2)
{
  assert(values != NULL);

  // Determine the start and end angles
  double a1 = point_angle(point(c.x + 1.0, c.y), c, p1);
  double a2 = point_angle(point(c.x + 1.0, c.y), c, p2);

  // Adjust angles depending on the position of their generating point p1 and p2
  if (p1.y < c.y) { a1 = 2.0 * M_PI - a1; }
  if (p2.y < c.y) { a2 = 2.0 * M_PI - a2; }

  // If the difference between the angles is above Pi,
  // then draw counter-clockwise
  double da = a2 - a1;
  if (da < 0) { da += 2.0 * M_PI; }
  bool ccw = da > M_PI;

  // Draw the arc
  return arc_to_bezier(values, c.x, c.y, r, r, a1, a2, ccw);
}

int
arcto_to_bezier(
  double *values,
  double x0,
  double y0,
  double x1,
  double y1,
  double x2,
  double y2,
  double r)
{
  assert(values != NULL);

  point_t p0 = { x0, y0 };
  point_t p1 = { x1, y1 };
  point_t p2 = { x2, y2 };

  // Angle between the three points
  double a = point_angle(p0, p1, p2);

  // If points are aligned, or if two (or more) points are identical,
  // then just output the middle point  and exit
  if ((a == 0.0) || point_equal(p0, p1) ||
      point_equal(p1, p2) || point_equal(p0, p2)) {
    values[0] = x1;
    values[1] = y1;
    return 0;
  }

  // Distance between the middle control point and the intermediate points
  double d = r / tan(a / 2.0);

  // Coordinates of the first intermediate point (between p0 and p1)
  double r1 = d / point_dist(p0, p1);
  point_t i1 = point(p1.x - (p1.x - p0.x) * r1, p1.y - (p1.y - p0.y) * r1);

  // Coordinates of the second intermediate point (between p1 and p2)
  double r2 = d / point_dist(p1, p2);
  point_t i2 = point(p1.x - (p1.x - p2.x) * r2, p1.y - (p1.y - p2.y) * r2);

  // Coordinate of the arc center
  point_t c;
  if (point_position(p0, p1, p2) < 0)
    c = point_average(point_offset_ortho(i1, p1, -r),
                      point_offset_ortho(i2, p1, r));
  else
    c = point_average(point_offset_ortho(i1, p1, r),
                      point_offset_ortho(i2, p1, -r));

  // Draw the arc
  return arc_to_bezier_p(values, c, r, i1, i2);
}

/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __POINT_H
#define __POINT_H

#include <math.h>

typedef struct point_t {
  double x;
  double y;
} point_t;

#define point(_x,_y) \
  ((point_t){ .x = (_x), .y = (_y) })

#define point_equal(p1,p2) \
  (((p1).x == (p2).x) && ((p1).y == (p2).y))
// add point_similar with epsilon

#define point_average(p1,p2) \
  ((point_t){ .x = ((p1).x + (p2).x) * 0.5, .y = ((p1).y + (p2).y) * 0.5 })

#define point_dist_2(p1,p2) \
  (pow((p2).x - (p1).x, 2.0) + pow((p2).y - (p1).y, 2.0))

#define point_dist(p1,p2) \
  (sqrt(point_dist_2((p1), (p2))))

// Compute the angle p1-p2-p3 between 0 and Pi
double
point_angle(
  point_t p1,
  point_t p2,
  point_t p3);

// Point p at a distance o from p1 to the right of p1->p2
// such that the angle p-p1-p2 is Pi/2
// If o < 0 then the point will be on the left
point_t
point_offset_ortho(
  point_t p1,
  point_t p2,
  double o);

// Point p at a distance o from p2 to the right of p1->p2->p3
// such that the angle p-p2-p3 is half the angle p1-p2-p3
// If o < 0 then the point will be on the left
point_t
point_offset(
  point_t p1,
  point_t p2,
  point_t p3,
  double o);

// Determine the position of point p with respect to p1->p2
// -1: point p is left of p1->p2
// +1: point p is right of p1->p2
//  0: point p is exactly on p1->2
int
point_position(
  point_t p1,
  point_t p2,
  point_t p);

#endif /* __POINT_H */

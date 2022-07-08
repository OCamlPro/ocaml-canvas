/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h> // Note: on Win32, add #define _USE_MATH_DEFINES for M_PI
#include <float.h>
#include <assert.h>

#include "util.h"
#include "point.h"
#include "rect.h"
#include "transform.h"
#include "path.h"
#include "path_internal.h"
#include "polygon.h"
#include "polygon_internal.h"
#include "polygonize.h"


void
quadratic_to_poly(
  point_t p1,
  point_t p2,
  point_t p3,
  polygon_t *p,
  int n)
{
  assert(p != NULL);
  assert(n >= 0);

  if (n <= 0) {
    if (p->nb_points == 0)
      polygon_add_point(p, p1);
    polygon_add_point(p, p2);
    polygon_add_point(p, p3);
  } else {
    point_t l1, l2, l3, r1, r2, r3;

    l1 = p1;

    l2 = point_average(p1, p2);
    r2 = point_average(p2, p3);

    l3 = r1 = point_average(l2, r2);

    r3 = p3;

    quadratic_to_poly(l1, l2, l3, p, n-1);
    quadratic_to_poly(r1, r2, r3, p, n-1);
  }
}

void
bezier_to_poly(
  point_t p1,
  point_t p2,
  point_t p3,
  point_t p4,
  polygon_t *p,
  int n)
{
  assert(p != NULL);
  assert(n >= 0);

  if (n <= 0) {
    if (p->nb_points == 0)
      polygon_add_point(p, p1);
    polygon_add_point(p, p2);
    polygon_add_point(p, p3);
    polygon_add_point(p, p4);
  } else {
    point_t l1, l2, l3, l4, r1, r2, r3, r4, h;

    l1 = p1;

    l2 = point_average(p1, p2);
    h = point_average(p2, p3);
    r3 = point_average(p3, p4);

    l3 = point_average(l2, h);
    r2 = point_average(h, r3);

    l4 = r1 = point_average(l3, r2);

    r4 = p4;

    bezier_to_poly(l1, l2, l3, l4, p, n-1);
    bezier_to_poly(r1, r2, r3, r4, p, n-1);
  }
}

// Assuming the points are on opposite major sides of an ellipse
// (image of a circle by the transform), fill the semi ellipse
static void
_arc_to_poly_transform(
  point_t p1,
  point_t p2,
  polygon_t *p,
  int precision,
  const transform_t *linear,
  const transform_t *inv_linear)
{
  assert(p != NULL);
  assert(linear != NULL);
  assert(inv_linear != NULL);

  point_t dp = point((p1.x - p2.x) / 2.0, (p1.y - p2.y) / 2.0);
  point_t center = point((p1.x + p2.x) / 2.0, (p1.y + p2.y) / 2.0);
  transform_apply(inv_linear, &dp);
  for (int i = 1; i < precision; i++) {
    double cs = cos(((double)(-i)) * M_PI / precision);
    double ss = sin(((double)(-i)) * M_PI / precision);
    point_t newPt = point((cs * dp.x + ss * dp.y), (-ss * dp.x + cs * dp.y));
    transform_apply(linear, &newPt);
    point_t output = point(newPt.x + center.x, newPt.y + center.y);
    polygon_add_point(p, output);
  }
}

// Same as previous, but now we don't assume the points are on opposite ends.
static void
_arc_to_poly_transform_center(
  point_t center,
  point_t p1,
  point_t p2,
  polygon_t *p,
  int precision,
  const transform_t *linear,
  const transform_t *inv_linear)
{
  assert(p != NULL);
  assert(linear != NULL);
  assert(inv_linear != NULL);

  point_t dp1 = point((p1.x - center.x), (p1.y - center.y));
  point_t dp2 = point((p2.x - center.x), (p2.y - center.y));
  transform_apply(inv_linear, &dp1);
  transform_apply(inv_linear, &dp2);

  double angle = atan2((dp2.y), (dp2.x)) - atan2((dp1.y), dp1.x);
  angle = angle - 2.0 * M_PI * floor(angle / (2.0 * M_PI));

  for (int i = 0; i <= precision; i++) {
    double cs = cos(((double)(-1)) * i * angle / precision);
    double ss = sin(((double)(-1)) * i * angle / precision);
    double x = dp1.x;
    double y = dp1.y;
    point_t new_pt = point((cs * x + ss * y), (-ss * x + cs * y));
    transform_apply(linear, &new_pt);
    point_t output = point(new_pt.x + center.x, new_pt.y + center.y);
    polygon_add_point(p, output);
  }
}

static void
_miter_to_poly(
  point_t p11,
  point_t p12,
  point_t p21,
  point_t p22,
  polygon_t *p)
{
  assert(p != NULL);

  // Compute intersection and add point
  double a11 = p12.x - p11.x;
  double a21 = p12.y - p11.y;
  double a12 = p21.x - p22.x;
  double a22 = p21.y - p22.y;
  double det = a11 * a22 - a21 * a12;
  double c1 = p21.x - p11.x;
  double c2 = p21.y - p11.y;
  double topdet = c1 * a22 - c2 * a12;
  double ans = topdet / det;
  point_t intersection =
    point(p11.x + ans * (p12.x - p11.x), p11.y + ans * (p12.y - p11.y));
  polygon_add_point(p, intersection);
}

static void
_line_offset(
  point_t p1,
  point_t p2,
  double o,
  point_t *out_p1,
  point_t *out_p2,
  const transform_t *lin,
  const transform_t *inv_lin)
{
  assert(out_p1 != NULL);
  assert(out_p2 != NULL);
  assert(lin != NULL);
  assert(inv_lin != NULL);

  *out_p1 = point_offset_ortho_transform(p1, p2, o, lin, inv_lin);
  *out_p2 = point_offset_ortho_transform(p2, p1, -o, lin, inv_lin);
}

void
polygon_offset(
  const polygon_t *p,
  polygon_t *np,
  double w,
  join_type_t join_type,
  cap_type_t cap_type,
  const transform_t *transform,
  bool only_linear)
{
  assert(p != NULL);
  assert(np != NULL);
  assert(transform != NULL);

  // Apply transformation to all points
  if (!only_linear) {
    for (int i = 0; i < p->nb_points; i++) {
      transform_apply(transform, &(p->points[i]));
    }
  }

  transform_t *lin = transform_extract_linear(transform);
  transform_t *inv_lin = transform_copy(lin);
  transform_inverse(inv_lin);

  point_t p1o, p2o, p1n, p2n;
  double o = w / 2.0;
  int iter = 5;

  for (int ip = 0; ip < p->nb_subpolys; ++ip) {

    int ifp = (ip == 0) ? 0 : p->subpolys[ip - 1] + 1;

    // Skip polygons with a single point
    if (ifp == p->subpolys[ip]) {
      continue;
    }

    // Draw the "left" part

    _line_offset(p->points[ifp], p->points[ifp + 1],
                 -o, &p1o, &p2o, lin, inv_lin);
    polygon_add_point(np, p1o);
    polygon_add_point(np, p2o);

    for (int i = ifp + 1; i < p->subpolys[ip]; ++i) {
      _line_offset(p->points[i], p->points[i + 1],
                   -o, &p1n, &p2n, lin, inv_lin);

      // Add join if turning "right"
      // TODO: don't bother if distance too small
      if (point_position(p->points[i-1], p->points[i + 1], p->points[i]) > 0) {
        switch (join_type) {
          case JOIN_ROUND:
            _arc_to_poly_transform_center(p->points[i], p2o, p1n, np,
                                          iter, lin, inv_lin);
            break;
          case JOIN_MITER:
            _miter_to_poly(p1o, p2o, p1n, p2n, np);
            break;
          case JOIN_BEVEL:
            break;
        }
      }

      polygon_add_point(np, p1n);
      polygon_add_point(np, p2n);

      p1o = p1n;
      p2o = p2n;
    }

    // If closed, add a join
    if (p->subpoly_closed[ip]) {
      // Add join if turning "right"
      // TODO: don't bother if distance too small
      // Assume end and start point are equal
      assert(point_equal(p->points[p->subpolys[ip]], p->points[ifp]));
      _line_offset(p->points[ifp], p->points[ifp + 1],
                   -o, &p1n, &p2n, lin, inv_lin);
      if (point_position(p->points[p->subpolys[ip] - 1],
                         p->points[ifp+1],
                         p->points[ifp]) > 0) {
        switch (join_type) {
          case JOIN_ROUND:
            _arc_to_poly_transform_center(p->points[ifp], p2o, p1n, np,
                                          iter, lin, inv_lin);
            break;
          case JOIN_MITER:
            _miter_to_poly(p1o, p2o, p1n, p2n, np);
            break;
          case JOIN_BEVEL:
            break;
        }
      } else {
        polygon_add_point(np, p1n);
      }

      polygon_end_subpoly(np, true);

    // If not closed, add a line cap
    } else {
      switch (cap_type) {
        case CAP_BUTT:
          break;
        case CAP_SQUARE: {
          double dist = point_dist(p->points[p->subpolys[ip] - 1],
                                   p->points[p->subpolys[ip]]);
          point_t new_p1 =
            point(p2o.x + (p2o.x - p1o.x) / dist * o,
                  p2o.y + (p2o.y - p1o.y) / dist * o);
          p1o = p->points[p->subpolys[ip] - 1];
          p2o = p->points[p->subpolys[ip]];
          _line_offset(p1o, p2o, o, &p1n, &p2n, lin, inv_lin);
          point_t new_p2 =
            point(p2n.x + ((p2n.x - p1n.x) / dist) * o,
                  p2n.y + ((p2n.y - p1n.y) / dist) * o);
          polygon_add_point(np, new_p1);
          polygon_add_point(np, new_p2);
          break;
        }
        case CAP_ROUND: {
          point_t new_p1 = point(p2o.x, p2o.y);
          p1o = p->points[p->subpolys[ip] - 1];
          p2o = p->points[p->subpolys[ip]];
          _line_offset(p1o, p2o, o, &p1n, &p2n, lin, inv_lin);
          _arc_to_poly_transform(new_p1, p2n, np, iter, lin, inv_lin);
          break;
        }
      }
    }

    // Draw the "right" part

    _line_offset(p->points[p->subpolys[ip]], p->points[p->subpolys[ip] - 1],
                 -o, &p1o, &p2o, lin, inv_lin);
    polygon_add_point(np, p1o);
    polygon_add_point(np, p2o);

    for (int i = p->subpolys[ip] - 1; i > ifp; --i) {
      _line_offset(p->points[i], p->points[i - 1],
                   -o, &p1n, &p2n, lin, inv_lin);

      // Add round join if turning "right"
      // TODO: don't bother if distance too small
      if (point_position(p->points[i+1], p->points[i - 1], p->points[i]) > 0) {
        switch(join_type) {
          case JOIN_ROUND:
            _arc_to_poly_transform_center(p->points[i], p2o, p1n, np,
                                          iter, lin, inv_lin);
            break;
          case JOIN_MITER:
            _miter_to_poly(p1o, p2o, p1n, p2n, np);
            break;
          case JOIN_BEVEL:
            break;
        }
      }
      polygon_add_point(np, p1n);
      polygon_add_point(np, p2n);

      p1o = p1n;
      p2o = p2n;
    }

    // If closed, add a join
    if (p->subpoly_closed[ip]) {
      // Add join if turning "right"
      // TODO: don't bother if distance too small
      // Assume end and start point are equal
      assert(point_equal(p->points[p->subpolys[ip]], p->points[ifp]));
      _line_offset(p->points[p->subpolys[ip]], p->points[p->subpolys[ip] - 1],
                   -o, &p1n, &p2n, lin, inv_lin);
      if (point_position(p->points[ifp+1],
                         p->points[p->subpolys[ip]-1],
                         p->points[ifp]) > 0) {
        switch (join_type) {
          case JOIN_ROUND:
            _arc_to_poly_transform_center(p->points[ifp], p2o, p1n, np,
                                          iter, lin, inv_lin);
            break;
          case JOIN_MITER:
            _miter_to_poly(p1o, p2o, p1n, p2n, np);
            break;
          case JOIN_BEVEL:
            break;
        }
      } else {
        polygon_add_point(np, p1n);
      }

    // If not closed, add a line cap
    } else {
      switch (cap_type) {
        case CAP_BUTT:
          break;
        case CAP_SQUARE: {
          double dist = point_dist(p1o,p2o);
          point_t new_p1 =
            point(p2o.x + (p2o.x - p1o.x) / dist * o,
                  p2o.y + (p2o.y - p1o.y) / dist * o);
          p1o = p->points[ifp + 1];
          p2o = p->points[ifp];
          _line_offset(p1o, p2o, o, &p1n, &p2n, lin, inv_lin);
          point_t new_p2 =
            point(p2n.x + (p2n.x - p1n.x) / dist * o,
                  p2n.y + (p2n.y - p1n.y) / dist * o);
          polygon_add_point(np,new_p1);
          polygon_add_point(np,new_p2);
          break;
        }
        case CAP_ROUND: {
          point_t new_p1 = point(p2o.x, p2o.y);
          p1o = p->points[ifp + 1];
          p2o = p->points[ifp];
          _line_offset(p1o, p2o, o, &p1n, &p2n, lin, inv_lin);
          _arc_to_poly_transform(new_p1, p2n, np, iter, lin, inv_lin);
          break;
        }
      }
    }

    polygon_end_subpoly(np, true);
  }

  transform_destroy(lin);
  transform_destroy(inv_lin);
}

bool
polygonize(
  path_t *path, // in
  polygon_t *p, // out
  rect_t *bbox) // out
{
  assert(path != NULL);
  assert(p != NULL);
  assert(bbox != NULL);

  int iter = 5; // TODO: Should be a parameter of polygonize (based on scale)

  path_iterator_t *i = path_get_iterator(path);
  if (i == NULL) {
    return false;
  }

  *bbox = rect(point(DBL_MAX, DBL_MAX), point(-DBL_MAX, -DBL_MAX));

  for (; path_iterator_at_end(i) == false; path_iterator_next(i)) {

    const point_t *points = path_iterator_get_points(i);

    point_t last =
      (p->nb_points == 0) ||
      ((p->nb_subpolys != 0) &&
       (p->subpolys[p->nb_subpolys - 1]) == (p->nb_points - 1)) ?
      points[0] : p->points[p->nb_points - 1];

    primitive_type_t prim = path_iterator_get_prim(i);

    for (size_t i = 0; i < prim_points[prim]; ++i) {
      rect_expand(bbox, points[i]);
    }

    switch (prim) {
      case PRIM_CLOSE:
        polygon_end_subpoly(p, true); // Note: automatically adds first point
        break;

      case PRIM_MOVE_TO:
        polygon_end_subpoly(p, false);
        polygon_add_point(p, points[0]);
        break;

      case PRIM_LINE_TO:
        polygon_add_point(p, points[0]);
        break;

      case PRIM_QUADR_TO:
        quadratic_to_poly(last, points[0], points[1], p, iter);
        break;

      case PRIM_BEZIER_TO:
        bezier_to_poly(last, points[0], points[1], points[2], p, iter);
        break;

      default:
        assert(!"Unknown primitive type");
    }

  }

  path_iterator_destroy(i);

  polygon_end_subpoly(p, false);

  return true;
}

bool
polygonize_outline(
  path_t *path,
  double w,
  polygon_t *p, // out
  rect_t *bbox,
  join_type_t join_type,
  cap_type_t cap_type,
  const transform_t *transform,
  bool only_linear)
{
  assert(path != NULL);
  assert(w > 0.0);
  assert(p != NULL);
  assert(bbox != NULL);
  assert(transform != NULL);

  // TODO: initial size according to number of primitive
  polygon_t *tp = polygon_create(1024, 16);
  if (tp == NULL) {
    return false;
  }

  bool res = polygonize(path, tp, bbox);
  if (res == false) {
    polygon_destroy(tp);
    return false;
  }

  polygon_offset(tp, p, w, join_type, cap_type, transform, only_linear);

  polygon_destroy(tp);

  if (!only_linear) {
    point_t pt1 = transform_apply_new(transform, &bbox->p1);
    point_t pt2 = transform_apply_new(transform, &bbox->p2);
    point_t bp3 = point(bbox->p2.x, bbox->p1.y);
    point_t bp4 = point(bbox->p1.x, bbox->p2.y);
    point_t pt3 = transform_apply_new(transform, &bp3);
    point_t pt4 = transform_apply_new(transform, &bp4);
    double xmin = min(pt1.x, min(pt2.x, min(pt3.x, pt4.x)));
    double ymin = min(pt1.y, min(pt2.y, min(pt3.y, pt4.y)));
    double xmax = max(pt1.x, max(pt2.x, max(pt3.x, pt4.x)));
    double ymax = max(pt1.y, max(pt2.y, max(pt3.y, pt4.y)));
    bbox->p1 = point(xmin, ymin);
    bbox->p2 = point(xmax, ymax);
  }

  double a, b, c, d;
  transform_extract_ft(transform, &a, &b, &c, &d);
  //TODO : Improve this (probably) not optimal coefficient
  double inf_norm = fabs(a) + fabs(b) + fabs(c) + fabs(d);
  bbox->p1.x -= inf_norm * w; bbox->p1.y -= inf_norm * w;
  bbox->p2.x += inf_norm * w; bbox->p2.y += inf_norm * w;

  return true;
}




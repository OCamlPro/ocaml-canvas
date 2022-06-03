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

#include "arc.h"

static void
_arc_to_poly_p(
  point_t c,
  double r,
  point_t p1,
  point_t p2,
  polygon_t *p,
  int n)
{
  assert(p != NULL);
  assert(n >= 0);

  double values[26];
  int nb_bezier = arc_to_bezier_p(values, c, r, p1, p2);

  n = max(1, n + 1 - nb_bezier);

  for (int i = 2; i < 2 + nb_bezier * 6; i += 6) {
    bezier_to_poly(point(values[i-2], values[i-1]),
                   point(values[i+0], values[i+1]),
                   point(values[i+2], values[i+3]),
                   point(values[i+4], values[i+5]), p, n);
  }
}

static void
_line_offset(
  point_t p1,
  point_t p2,
  double o,
  point_t *out_p1,
  point_t *out_p2)
{
  assert(out_p1 != NULL);
  assert(out_p2 != NULL);

  *out_p1 = point_offset_ortho(p1, p2, o);
  *out_p2 = point_offset_ortho(p2, p1, -o);
}

void
polygon_offset(
  const polygon_t *p,
  polygon_t *np,
  double w)
{
  assert(p != NULL);
  assert(np != NULL);

  int iter = 2; // TODO: Should be a parameter of polygonize (based on scale)

  point_t p1o, p2o, p1n, p2n;
  double o = w / 2.0;

  for (int ip = 0; ip < p->nb_subpolys; ++ip) {

    int ifp = (ip == 0) ? 0 : p->subpolys[ip - 1] + 1;

    // Skip polygons with a single point
    if (ifp == p->subpolys[ip]) {
      continue;
    }

    // Draw the "left" part

    _line_offset(p->points[ifp], p->points[ifp+1], -o, &p1o, &p2o);
    polygon_add_point(np, p1o);
    polygon_add_point(np, p2o);

    for (int i = ifp + 1; i < p->subpolys[ip]; ++i) {
      _line_offset(p->points[i], p->points[i+1], -o, &p1n, &p2n);

      // Add round join if turning "right"
      // TODO: don't bother if distance too small
      if (point_position(p->points[i-1], p->points[i+1], p->points[i]) < 0)
        _arc_to_poly_p(p->points[i], o, p2o, p1n, np, iter);

      polygon_add_point(np, p1n);
      polygon_add_point(np, p2n);

      p1o = p1n;
      p2o = p2n;
    }

    // If closed, add a join (round for now)
    if (p->subpoly_closed[ip]) {
      // Add round join if turning "right"
      // TODO: don't bother if distance too small
      // Assume end and start point are equal
      assert(point_equal(p->points[p->subpolys[ip]], p->points[ifp]));
      _line_offset(p->points[ifp], p->points[ifp + 1], -o, &p1n, &p2n);
      if (point_position(p->points[p->subpolys[ip]-1],
                         p->points[ifp+1],
                         p->points[ifp]) < 0) {
        _arc_to_poly_p(p->points[ifp], o, p2o, p1n, np, iter);
      } else {
        polygon_add_point(np, p1n);
      }

      polygon_end_subpoly(np, true);

    // If not closed, add a line butt
    } else {
      // TODO
    }

    // Draw the "right" part

    _line_offset(p->points[p->subpolys[ip]],
                 p->points[p->subpolys[ip] - 1], -o, &p1o, &p2o);
    polygon_add_point(np, p1o);
    polygon_add_point(np, p2o);

    for (int i = p->subpolys[ip] - 1; i > ifp; --i) {
      _line_offset(p->points[i], p->points[i-1], -o, &p1n, &p2n);

      // Add round join if turning "right"
      // TODO: don't bother if distance too small
      if (point_position(p->points[i+1], p->points[i-1], p->points[i]) < 0)
        _arc_to_poly_p(p->points[i], o, p2o, p1n, np, iter);

      polygon_add_point(np, p1n);
      polygon_add_point(np, p2n);

      p1o = p1n;
      p2o = p2n;
    }

    // If closed, add a join (round for now)
    if (p->subpoly_closed[ip]) {
      // Add round join if turning "right"
      // TODO: don't bother if distance too small
      // Assume end and start point are equal
      assert(point_equal(p->points[p->subpolys[ip]], p->points[ifp]));
      _line_offset(p->points[p->subpolys[ip]],
                   p->points[p->subpolys[ip]-1], -o, &p1n, &p2n);
      if (point_position(p->points[ifp+1],
                         p->points[p->subpolys[ip]-1],
                         p->points[ifp]) < 0) {
        _arc_to_poly_p(p->points[ifp], o, p2o, p1n, np, iter);
      } else {
        polygon_add_point(np, p1n);
      }

    // If not closed, add a line butt
    } else {
      // TODO
    }

    polygon_end_subpoly(np, true);
  }
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
  rect_t *bbox) // out
{
  assert(path != NULL);
  assert(w > 0.0);
  assert(p != NULL);
  assert(bbox != NULL);

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

  polygon_offset(tp, p, w);

  polygon_destroy(tp);

  bbox->p1.x -= w / 2.0; bbox->p1.y -= w / 2.0;
  bbox->p2.x += w / 2.0; bbox->p2.y += w / 2.0;

  return true;
}




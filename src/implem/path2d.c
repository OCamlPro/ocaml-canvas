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
#include <assert.h>

#include "path.h"
#include "transform.h"
#include "arc.h"
#include "path2d.h"
#include "path2d_internal.h"

IMPLEMENT_OBJECT_METHODS(path2d_t, path2d, _path2d_destroy)

path2d_t *
path2d_create(
  void)
{
  path2d_t *path2d = path2d_alloc();
  if (path2d == NULL) {
    return NULL;
  }

  path2d->path = path_create(12, 18);
  if (path2d->path == NULL) {
    path_destroy(path2d->path);
    return NULL;
  }

  return path2d;
}

void
path2d_reset(
  path2d_t *path2d)
{
  assert(path2d != NULL);
  assert(path2d->path != NULL);

  path_reset(path2d->path);
}

static void
_path2d_update_first_last(
  path2d_t *path2d,
  double ftx,
  double fty,
  double fx,
  double fy,
  double lx,
  double ly,
  bool is_move)
{
  assert(path2d != NULL);

  if ((is_move == true) || (path_empty(path2d->path) == true)) {
    path2d->first_tx = ftx;
    path2d->first_ty = fty;
    path2d->first_x = fx;
    path2d->first_y = fy;
  }

  path2d->last_x = lx;
  path2d->last_y = ly;
}

bool
path2d_close(
  path2d_t *path2d)
{
  assert(path2d != NULL);
  assert(path2d->path != NULL);

  // Add a move to the first point before the close
  // This makes handling of primitives after close easier
  // TODO: this seems clumsy : if a transform is set before the close,
  // then we're going to move to some random point...
  // But arc_to needs the last untransformed point, which happens
  // to be the first point in the subpath when we close it
  if (path_add_close_path(path2d->path)) {
    path2d->last_x = path2d->first_x;
    path2d->last_y = path2d->first_y;
    return path_add_move_to(path2d->path, path2d->first_tx, path2d->first_ty);
  }

  return false;
}

bool
path2d_move_to(
  path2d_t *path2d,
  double x,
  double y,
  const transform_t *t)
{
  assert(path2d != NULL);
  assert(path2d->path != NULL);

  point_t p = point(x, y);
  if (t != NULL) {
    transform_apply(t, &p);
  }

  _path2d_update_first_last(path2d, p.x, p.y, x, y, x, y, true);

  return path_add_move_to(path2d->path, p.x, p.y);
}

bool
path2d_line_to(
  path2d_t *path2d,
  double x,
  double y,
  const transform_t *t)
{
  assert(path2d != NULL);
  assert(path2d->path != NULL);

  point_t p = point(x, y);
  if (t != NULL) {
    transform_apply(t, &p);
  }

  _path2d_update_first_last(path2d, p.x, p.y, x, y, x, y, false);

  return path_add_line_to(path2d->path, p.x, p.y);
}

bool
path2d_quadratic_curve_to(
  path2d_t *path2d,
  double cpx,
  double cpy,
  double x,
  double y,
  const transform_t *t)
{
  assert(path2d != NULL);
  assert(path2d->path != NULL);

  point_t cp = point(cpx, cpy);
  point_t p = point(x, y);

  if (t != NULL) {
    transform_apply(t, &cp);
    transform_apply(t, &p);
  }

  _path2d_update_first_last(path2d, cp.x, cp.y, cpx, cpy, x, y, false);

  return path_add_quadratic_curve_to(path2d->path, cp.x, cp.y, p.x, p.y);
}

bool
path2d_bezier_curve_to(
  path2d_t *path2d,
  double cp1x,
  double cp1y,
  double cp2x,
  double cp2y,
  double x,
  double y,
  const transform_t *t)
{
  assert(path2d != NULL);
  assert(path2d->path != NULL);

  point_t cp1 = point(cp1x, cp1y);
  point_t cp2 = point(cp2x, cp2y);
  point_t p = point(x, y);

  if (t != NULL) {
    transform_apply(t, &cp1);
    transform_apply(t, &cp2);
    transform_apply(t, &p);
  }

  _path2d_update_first_last(path2d, cp1.x, cp1.y, cp1x, cp1y, x, y, false);

  return path_add_bezier_curve_to(path2d->path, cp1.x, cp1.y,
                                  cp2.x, cp2.y, p.x, p.y);
}

void
path2d_rect(
  path2d_t *path2d,
  double x,
  double y,
  double width,
  double height,
  const transform_t *t)
{
  assert(path2d != NULL);
  assert(path2d->path != NULL);

  path2d_move_to(path2d, x, y, t);
  path2d_line_to(path2d, x + width, y, t);
  path2d_line_to(path2d, x + width, y + height, t);
  path2d_line_to(path2d, x, y + height, t);
  path2d_close(path2d);
}

static void
_path2d_bezier_list(
  path2d_t *path2d,
  const double *values,
  int nb_bezier,
  const transform_t *t)
{
  assert(path2d != NULL);
  assert(path2d->path != NULL);
  assert(values != NULL);
  assert(nb_bezier >= 0);

  path2d_line_to(path2d, values[0], values[1], t);

  for (int i = 2; i < 2 + nb_bezier * 6; i += 6) {
    path2d_bezier_curve_to(path2d,
                           values[i + 0], values[i + 1],
                           values[i + 2], values[i + 3],
                           values[i + 4], values[i + 5], t);
  }
}

void
path2d_arc(
  path2d_t *path2d,
  double x,
  double y,
  double r,
  double di,
  double df,
  bool ccw,
  const transform_t *t)
{
  assert(path2d != NULL);
  assert(path2d->path != NULL);

  double values[26];
  int nb_bezier = arc_to_bezier(values, x, y, r, r, di, df, ccw);
  _path2d_bezier_list(path2d, values, nb_bezier, t);
}

void
path2d_arc_to(
  path2d_t *path2d,
  double x1,
  double y1,
  double x2,
  double y2,
  double r,
  const transform_t *t)
{
  assert(path2d != NULL);
  assert(path2d->path != NULL);

  // When the path is completely empty, we just move to midpoint
  // Note that even when the path contains a single move,
  // we consider it is not empty and thus skip this case
  // Note: we could just call save_first and save_last
  if (path_empty(path2d->path)) {
    path2d_move_to(path2d, x1, y1, t);
    return;
  }

  double values[26];
  int nb_bezier =
    arcto_to_bezier(values, path2d->last_x, path2d->last_y, x1, y1, x2, y2, r);
  _path2d_bezier_list(path2d, values, nb_bezier, t);
}

static void
_rotate_list(int nb_points, double *values, double r)
{
  assert(values != NULL);
  assert(nb_points > 0);

  double cos_r = cos(r);
  double sin_r = sin(r);

  for (int i = 0; i < nb_points * 2; i += 2) {
    double tx = values[i] * cos_r + values[i+1] * sin_r;
    double ty = values[i+1] * cos_r - values[i] * sin_r;
    values[i] = tx;
    values[i+1] = ty;
  }
}

void
path2d_ellipse(
  path2d_t *path2d,
  double x,
  double y,
  double rx,
  double ry,
  double r,
  double di,
  double df,
  bool ccw,
  const transform_t *t)
{
  assert(path2d != NULL);
  assert(path2d->path != NULL);

  double values[26];
  int nb_bezier = arc_to_bezier(values, x, y, rx, ry, di, df, ccw);
  _rotate_list(1 + nb_bezier * 3, values, -r);
  _path2d_bezier_list(path2d, values, nb_bezier, t);
}

bool
path2d_add(
  path2d_t *dpath2d,
  const path2d_t *spath2d,
  const transform_t *t)
{
  assert(dpath2d != NULL);
  assert(dpath2d->path != NULL);
  assert(spath2d != NULL);
  assert(spath2d->path != NULL);

  int num_prims = path_get_nb_prims(spath2d->path);
  path_iterator_t *i = path_get_iterator((path_t *)spath2d->path);
  if (i == NULL) {
    return NULL;
  }

  for (int prim_seen = 0;
       (path_iterator_at_end(i) == false) && (prim_seen < num_prims);
       path_iterator_next(i)) {

    const point_t *points = path_iterator_get_points(i);
    primitive_type_t prim = path_iterator_get_prim(i);

    switch (prim) {
      case PRIM_CLOSE:
        if (!path2d_close(dpath2d)) {
          return false;
        }
        break;

      case PRIM_MOVE_TO:
        if (!path2d_move_to(dpath2d, points[0].x, points[0].y, t)) {
          return false;
        }
        break;

      case PRIM_LINE_TO:
        if (!path2d_line_to(dpath2d, points[0].x, points[0].y, t)) {
          return false;
        }
        break;

      case PRIM_QUADR_TO:
        if (!path2d_quadratic_curve_to(dpath2d,
                                       points[0].x, points[0].y,
                                       points[1].x, points[1].y, t)) {
          return false;
        }
        break;

      case PRIM_BEZIER_TO:
        if (!path2d_bezier_curve_to(dpath2d,
                                    points[0].x, points[0].y,
                                    points[1].x, points[1].y,
                                    points[2].x, points[2].y, t)) {
          return false;
        }
        break;
    }
    prim_seen++;
  }

  path_iterator_destroy(i);

  return true;
}

path_t *
path2d_get_path(
  path2d_t *path2d)
{
  assert(path2d != NULL);
  assert(path2d->path != NULL);

  return path2d->path;
}

static void (*_path2d_destroy_callback)(path2d_t *) = NULL;

void
path2d_set_destroy_callback(
  void (*callback)(path2d_t *))
{
  _path2d_destroy_callback = callback;
}

static void
_path2d_destroy(
  path2d_t *path2d)
{
  assert(path2d != NULL);
  assert(path2d->path != NULL);

  if (_path2d_destroy_callback != NULL) {
    _path2d_destroy_callback(path2d);
  }
  if (path2d->path != NULL) {
    path_destroy(path2d->path);
  }

  free(path2d);
}

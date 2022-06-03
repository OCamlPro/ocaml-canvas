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
#include <assert.h>

#include "util.h"
#include "path.h"
#include "path_internal.h"

const int32_t prim_points[NB_PRIMS] = {
  [PRIM_CLOSE]     = 0,
  [PRIM_MOVE_TO]   = 1,
  [PRIM_LINE_TO]   = 1,
  [PRIM_QUADR_TO]  = 2,
  [PRIM_BEZIER_TO] = 3,
};

path_t *
path_create(
  int32_t max_prims,
  int32_t max_points)
{
  assert(max_prims > 0);
  assert(max_points > 0);

  path_t *path = (path_t *)calloc(1, sizeof(path_t));
  if (path == NULL) {
    return NULL;
  }

  path->prims = (primitive_type_t *)calloc(max_prims, sizeof(primitive_type_t));
  if (path->prims == NULL) {
    free(path);
    return NULL;
  }

  path->points = (point_t *)calloc(max_points, sizeof(point_t));
  if (path->points == NULL) {
    free(path->prims);
    free(path);
    return NULL;
  }

  path->nb_prims = 0;
  path->max_prims = max_prims;
  path->nb_points = 0;
  path->max_points = max_points;

  return path;
}

void
path_destroy(
  path_t *path)
{
  assert(path != NULL);
  assert(path->prims != NULL);
  assert(path->points != NULL);

  free(path->points);
  free(path->prims);
  free(path);
}

static bool
_path_ensures(
  path_t *path,
  int32_t nb_new_prims,
  int32_t nb_new_points)
{
  assert(path != NULL);
  assert(path->prims != NULL);
  assert(path->points != NULL);
  assert(nb_new_prims >= 0);
  assert(nb_new_points >= 0);

  if (path->nb_prims + nb_new_prims > path->max_prims) {

    int32_t max_prims = max(path->max_prims + path->max_prims / 2,
                            path->max_prims + nb_new_prims);

    primitive_type_t *prims =
      (primitive_type_t *)realloc(path->prims,
                                  max_prims * sizeof(primitive_type_t));
    if (prims == NULL) {
      return false;
    }

    path->prims = prims;
    path->max_prims = max_prims;

  }

  if (path->nb_points + nb_new_points > path->max_points) {

    int32_t max_points = max(path->max_points + path->max_points / 2,
                             path->max_points + nb_new_points);

    point_t *points =
      (point_t *)realloc(path->points, max_points * sizeof(point_t));
    if (points == NULL) {
      return false;
    }

    path->points = points;
    path->max_points = max_points;

  }

  return true;
}

void
path_reset(
  path_t *path)
{
  assert(path != NULL);
  assert(path->prims != NULL);
  assert(path->points != NULL);

  path->nb_prims = 0;
  path->nb_points = 0;
}

bool
path_empty(
  path_t *path)
{
  assert(path != NULL);
  assert(path->prims != NULL);
  assert(path->points != NULL);
  assert((path->nb_prims <= 0) ||
         (path->prims[0] != PRIM_CLOSE) ||
         (path->prims[path->nb_prims - 1] != PRIM_CLOSE));

  return (path->nb_prims <= 0);
}

bool
path_add_close_path(
  path_t *path)
{
  assert(path != NULL);
  assert(path->prims != NULL);
  assert(path->points != NULL);

  // Don't close if empty, or after a close or a move
  if ((path->nb_prims <= 0) ||
      (path->prims[path->nb_prims - 1] == PRIM_CLOSE) ||
      (path->prims[path->nb_prims - 1] == PRIM_MOVE_TO)) {
    return true;
  }

  if (_path_ensures(path, 1, prim_points[PRIM_CLOSE]) == false) {
    return false;
  }
  path->prims[path->nb_prims++] = PRIM_CLOSE;

  return true;
}

bool
path_add_move_to(
  path_t *path,
  double x,
  double y)
{
  assert(path != NULL);
  assert(path->prims != NULL);
  assert(path->points != NULL);

  // After a move, just update the point
  if ((path->nb_prims > 0) &&
      (path->prims[path->nb_prims - 1] == PRIM_MOVE_TO)) {
    assert(path->nb_points > 0);
    path->points[path->nb_points - 1] = point(x, y);
    return true;
  }

  if (_path_ensures(path, 1, prim_points[PRIM_MOVE_TO]) == false) {
    return false;
  }
  path->prims[path->nb_prims++] = PRIM_MOVE_TO;
  path->points[path->nb_points++] = point(x, y);
  return true;
}

bool
path_add_line_to(
  path_t *path,
  double x,
  double y)
{
  assert(path != NULL);
  assert(path->prims != NULL);
  assert(path->points != NULL);

  // If empty, change to a move
  if (path->nb_prims <= 0) {
    return path_add_move_to(path, x, y);
  }

  // If last point is identical, ignore
  assert(path->nb_points > 0);
  if (point_equal(path->points[path->nb_points - 1], point(x, y))) {
    return true;
  }

  if (_path_ensures(path, 1, prim_points[PRIM_LINE_TO]) == false) {
    return false;
  }
  path->prims[path->nb_prims++] = PRIM_LINE_TO;
  path->points[path->nb_points++] = point(x, y);

  return true;
}

bool
path_add_quadratic_curve_to(
  path_t *path,
  double cpx,
  double cpy,
  double x,
  double y)
{
  assert(path != NULL);
  assert(path->prims != NULL);
  assert(path->points != NULL);

  if (_path_ensures(path, 1, prim_points[PRIM_QUADR_TO]) == false) {
    return false;
  }
  path->prims[path->nb_prims++] = PRIM_QUADR_TO;
  path->points[path->nb_points++] = point(cpx, cpy);
  path->points[path->nb_points++] = point(x, y);
  return true;
}

bool
path_add_bezier_curve_to(
  path_t *path,
  double cp1x,
  double cp1y,
  double cp2x,
  double cp2y,
  double x,
  double y)
{
  assert(path != NULL);
  assert(path->prims != NULL);
  assert(path->points != NULL);

  if (_path_ensures(path, 1, prim_points[PRIM_BEZIER_TO]) == false) {
    return false;
  }
  path->prims[path->nb_prims++] = PRIM_BEZIER_TO;
  path->points[path->nb_points++] = point(cp1x, cp1y);
  path->points[path->nb_points++] = point(cp2x, cp2y);
  path->points[path->nb_points++] = point(x, y);
  return true;
}

path_iterator_t *
path_get_iterator(
  path_t *p)
{
  assert(p != NULL);
  assert(p->prims != NULL);
  assert(p->points != NULL);

  path_iterator_t *i = (path_iterator_t *)calloc(1, sizeof(path_iterator_t));
  if (i == NULL) {
    return NULL;
  }
  i->path = p;
  i->prims = p->prims;
  i->points = p->points;
  return i;
}

void
path_iterator_destroy(
  path_iterator_t *i)
{
  assert(i != NULL);

  free(i);
}

bool
path_iterator_at_end(
  const path_iterator_t *i)
{
  assert(i != NULL);

  return i->prims >= i->path->prims + i->path->nb_prims;
}

void
path_iterator_next(
  path_iterator_t *i)
{
  assert(i != NULL);
  assert(i->prims != NULL);
  assert(i->points != NULL);

  if (i->prims < i->path->prims + i->path->nb_prims) {
    i->points += prim_points[*i->prims];
    i->prims++;
  }
}

void
path_iterator_prev(
  path_iterator_t *i)
{
  assert(i != NULL);
  assert(i->prims != NULL);
  assert(i->points != NULL);

  if (i->prims > i->path->prims) {
    i->prims--;
    i->points -= prim_points[*i->prims];
  }
}

primitive_type_t
path_iterator_get_prim(
  const path_iterator_t *i)
{
  assert(i != NULL);
  assert(i->prims != NULL);

  return i->prims[0];
}

const point_t *
path_iterator_get_points(
  const path_iterator_t *i)
{
  assert(i != NULL);
  assert(i->points != NULL);

  return i->points;
}

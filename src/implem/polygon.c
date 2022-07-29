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
#include "point.h"
#include "polygon.h"
#include "polygon_internal.h"

polygon_t *
polygon_create(
  int32_t max_points,
  int32_t max_subpolys)
{
  assert(max_points > 0);
  assert(max_subpolys > 0);

  polygon_t *p = (polygon_t *)calloc(1, sizeof(polygon_t));
  if (p == NULL) {
    return NULL;
  }

  p->points = (point_t *)calloc(max_points, sizeof(point_t));
  if (p->points == NULL) {
    free(p);
    return NULL;
  }

  p->subpolys = (int32_t *)calloc(max_subpolys, sizeof(int32_t));
  if (p->subpolys == NULL) {
    free(p->points);
    free(p);
    return NULL;
  }

  p->subpoly_closed = (bool *)calloc(max_subpolys, sizeof(bool));
  if (p->subpolys == NULL) {
    free(p->subpolys);
    free(p->points);
    free(p);
    return NULL;
  }

  p->max_points = max_points;
  p->max_subpolys = max_subpolys;
  p->nb_points = 0;

  return p;
}

void
polygon_destroy(
  polygon_t *p)
{
  assert(p != NULL);
  assert(p->points != NULL);
  assert(p->subpolys != NULL);

  free(p->subpoly_closed);
  free(p->subpolys);
  free(p->points);
  free(p);
}

void
polygon_reset(
  polygon_t *p)
{
  assert(p != NULL);

  p->nb_points = 0;
  p->nb_subpolys = 0;
}

bool
polygon_expand(
  polygon_t *p)
{
  assert(p != NULL);
  assert(p->points != NULL);
  assert(p->subpolys != NULL);

  int32_t max_points = p->max_points * 2;

  point_t *points =
    (point_t *)realloc(p->points, max_points * sizeof(point_t));
  if (points == NULL) {
    return false;
  }

  p->points = points;
  p->max_points = max_points;

  return true;
}

bool
polygon_expand_subpoly(
  polygon_t *p)
{
  assert(p != NULL);
  assert(p->points != NULL);
  assert(p->subpolys != NULL);

  int32_t max_subpolys = p->max_subpolys * 2;

  int32_t *subpolys =
    (int32_t *)realloc(p->subpolys, max_subpolys * sizeof(int32_t));
  if (subpolys == NULL) {
    return false;
  }
  p->subpolys = subpolys;

  bool *subpoly_closed =
    (bool *)realloc(p->subpoly_closed, max_subpolys * sizeof(bool));
  if (subpoly_closed == NULL) {
    return false;
  }
  p->subpoly_closed = subpoly_closed;

  p->max_subpolys = max_subpolys;

  return true;
}

bool
polygon_add_point(
  polygon_t *p,
  point_t pt)
{
  assert(p != NULL);
  assert(p->points != NULL);
  assert(p->subpolys != NULL);

  int32_t nb_points_subpoly =
    (p->nb_subpolys <= 0) ? p->nb_points :
      p->nb_points - 1 - p->subpolys[p->nb_subpolys - 1];

  if ((nb_points_subpoly <= 0) ||
      (point_equal(p->points[p->nb_points - 1], pt) == false)) {

    if (p->nb_points >= p->max_points) {
      if (!polygon_expand(p)) {
        return false;
      }
    }

    p->points[p->nb_points++] = pt;
  }

  return true;
}

bool
polygon_end_subpoly(
  polygon_t *p,
  bool close)
{
  assert(p != NULL);
  assert(p->points != NULL);
  assert(p->subpolys != NULL);

  int32_t nb_points_subpoly =
    (p->nb_subpolys <= 0) ? p->nb_points :
      p->nb_points - 1 - p->subpolys[p->nb_subpolys - 1];

  if (nb_points_subpoly > 1) {

    if (p->nb_subpolys >= p->max_subpolys) {
      if (!polygon_expand_subpoly(p)) {
        return false;
      }
    }

    point_t first_point_subpoly =
      (p->nb_subpolys <= 0) ? p->points[0] :
      p->points[p->subpolys[p->nb_subpolys - 1] + 1];

    if ((close == true) &&
        (point_equal(p->points[p->nb_points - 1],
                     first_point_subpoly) == false)) {
      polygon_add_point(p, first_point_subpoly);
    }

    p->subpolys[p->nb_subpolys] = p->nb_points - 1;
    p->subpoly_closed[p->nb_subpolys] = close;
    p->nb_subpolys++;

  } else if (nb_points_subpoly == 1) {

    p->nb_points--;

  }

  return true;
}

polygon_t *
polygon_copy(
  const polygon_t *p)
{
  assert(p != NULL);
  assert(p->points != NULL);
  assert(p->subpolys != NULL);

  polygon_t *output =
    polygon_create(max(8, p->nb_points), max(2, p->nb_subpolys));
  for (int32_t i = 0; i < p->nb_subpolys; ++i) {
    int32_t j = (i == 0) ? 0 : p->subpolys[i - 1] + 1;
    for (; j <= p->subpolys[i]; ++j) {
      polygon_add_point(output, p->points[j]);
    }
    polygon_end_subpoly(output, p->subpoly_closed[i]);
  }

  return output;
}

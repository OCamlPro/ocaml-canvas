/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __PATH_H
#define __PATH_H

#include <stdint.h>
#include <stdbool.h>

#include "point.h"

typedef struct path_t path_t;
typedef struct path_iterator_t path_iterator_t;

typedef enum primitive_type_t {
  PRIM_CLOSE     = 0,
  PRIM_MOVE_TO   = 1,
  PRIM_LINE_TO   = 2,
  PRIM_QUADR_TO  = 3,
  PRIM_BEZIER_TO = 4
} primitive_type_t;

path_t *
path_create(
  int32_t max_prims,
  int32_t max_points);

void
path_destroy(
  path_t *path);

void
path_reset(
  path_t *path);

bool
path_empty(
  path_t *path);

bool
path_add_close_path(
  path_t *path);

bool
path_add_move_to(
  path_t *path,
  double x,
  double y);

bool
path_add_line_to(
  path_t *path,
  double x,
  double y);

bool
path_add_quadratic_curve_to(
  path_t *path,
  double cpx,
  double cpy,
  double x,
  double y);

bool
path_add_bezier_curve_to(
  path_t *path,
  double cp1x,
  double cp1y,
  double cp2x,
  double cp2y,
  double x,
  double y);

int32_t
path_get_nb_prims(
  const path_t *path);

path_iterator_t *
path_get_iterator(
  path_t *path);

void
path_iterator_destroy(
  path_iterator_t *i);

bool
path_iterator_at_end(
  const path_iterator_t *i);

void
path_iterator_next(
  path_iterator_t *i);

void
path_iterator_prev(
  path_iterator_t *i);

primitive_type_t
path_iterator_get_prim(
  const path_iterator_t *i);

const point_t *
path_iterator_get_points(
  const path_iterator_t *i);

#endif /* __PATH_H */

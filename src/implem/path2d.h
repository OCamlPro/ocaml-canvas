/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __PATH2D_H
#define __PATH2D_H

#include "object.h"
#include "path.h"
#include "transform.h"

typedef struct path2d_t path2d_t;

DECLARE_OBJECT_METHODS(path2d_t, path2d)

path2d_t *
path2d_create(
  void);

void
path2d_reset(
  path2d_t *path2d);

bool
path2d_close(
  path2d_t *path2d);

bool
path2d_move_to(
  path2d_t *path2d,
  double x,
  double y,
  const transform_t *t);

bool
path2d_line_to(
  path2d_t *path2d,
  double x,
  double y,
  const transform_t *t);

void
path2d_arc(
  path2d_t *path2d,
  double x,
  double y,
  double r,
  double di,
  double df,
  bool ccw,
  const transform_t *t);

void
path2d_arc_to(
  path2d_t *path2d,
  double x1,
  double y1,
  double x2,
  double y2,
  double r,
  const transform_t *t);

bool
path2d_quadratic_curve_to(
  path2d_t *path2d,
  double cpx,
  double cpy,
  double x,
  double y,
  const transform_t *t);

bool
path2d_bezier_curve_to(
  path2d_t *path2d,
  double cp1x,
  double cp1y,
  double cp2x,
  double cp2y,
  double x,
  double y,
  const transform_t *t);

void
path2d_rect(
  path2d_t *path2d,
  double x,
  double y,
  double width,
  double height,
  const transform_t *t);

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
  const transform_t *t);

bool
path2d_add(
  path2d_t *dpath2d,
  const path2d_t *spath2d,
  const transform_t *t);

path_t *
path2d_get_path(
  path2d_t *path2d);

void
path2d_set_destroy_callback(
  void (*callback)(path2d_t *));

#endif /* __PATH2D_H */

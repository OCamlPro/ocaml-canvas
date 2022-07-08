/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __TRANSFORM_H
#define __TRANSFORM_H

#include <stdbool.h>

#include "point.h"

/*
  A transformation matrix
  a, d => scaling/flipping
  b, c => shearing
  e, f => translation
  a, b, c, d => rotation
*/
typedef struct transform_t {
  double a; double b; // 0.0
  double c; double d; // 0.0
  double e; double f; // 1.0
} transform_t;

transform_t *
transform_create(
  void);

void
transform_destroy(
  transform_t *t);

void
transform_reset(
  transform_t *t);

transform_t *
transform_copy(
  const transform_t *t);

void
transform_set(
  transform_t *t,
  double a,
  double b,
  double c,
  double d,
  double e,
  double f);

void
transform_mul(
  transform_t *t,
  double a,
  double b,
  double c,
  double d,
  double e,
  double f);

void
transform_translate(
  transform_t *t,
  double x,
  double y);

void
transform_scale(
  transform_t *t,
  double x,
  double y);

void
transform_shear(
  transform_t *t,
  double x,
  double y);

void
transform_rotate(
  transform_t *t,
  double angle);

void
transform_inverse(
  transform_t *t);

void
transform_apply(
  const transform_t *t,
  point_t *p);

point_t
transform_apply_new(
  const transform_t *t,
  const point_t *p);

bool
transform_is_identity(
  const transform_t *t);

bool
transform_is_pure_translation(
  const transform_t *t);

void
transform_extract_ft(
  const transform_t *t,
  double *a,
  double *b,
  double *c,
  double *d);

void
transform_extract_translation(
  const transform_t *t,
  double *e,
  double *f);

void
transform_extract_scale(
  const transform_t *t,
  double *sx,
  double *sy);

transform_t *
transform_extract_linear(
  const transform_t *t);

point_t
point_offset_ortho_transform(
  point_t p1,
  point_t p2,
  double o,
  const transform_t *lin,
  const transform_t *inv_lin);


#endif /* __TRANSFORM_H */

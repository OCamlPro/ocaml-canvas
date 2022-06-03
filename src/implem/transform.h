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

#include "point.h"

typedef struct transform_t transform_t;

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
transform_extract_ft(
  const transform_t *t,
  double *a,
  double *b,
  double *c,
  double *d);

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

void
transform_extract_scale(
  const transform_t *t,
  double *sx,
  double *sy);

#endif /* __TRANSFORM_H */

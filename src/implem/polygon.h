/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __POLYGON_H
#define __POLYGON_H

#include <stdint.h>
#include <stdbool.h>

#include "point.h"

typedef struct polygon_t polygon_t;

polygon_t *
polygon_create(
  int32_t max_points,
  int32_t max_subpolys);

void
polygon_destroy(
  polygon_t *p);

void
polygon_reset(
  polygon_t *p);

bool
polygon_expand(
  polygon_t *p);

bool
polygon_expand_subpoly(
  polygon_t *p);

bool
polygon_add_point(
  polygon_t *p,
  point_t pt);

bool
polygon_end_subpoly(
  polygon_t *p,
  bool close);

#endif /* __POLYGON_H */

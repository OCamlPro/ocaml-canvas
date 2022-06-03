/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __POLYGON_INTERNAL_H
#define __POLYGON_INTERNAL_H

#include <stdint.h>
#include <stdbool.h>

#include "point.h"

typedef struct polygon_t {
  point_t *points;
  int32_t nb_points;
  int32_t max_points;
  int32_t *subpolys; // index of last point in subpoly
  bool *subpoly_closed; // indicate if subpoly is closed
  int32_t nb_subpolys;
  int32_t max_subpolys;
} polygon_t;

#endif /* __POLYGON_INTERNAL_H */

/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __PATH_INTERNAL_H
#define __PATH_INTERNAL_H

#include <stdint.h>

#include "point.h"

#define NB_PRIMS 5

typedef struct path_t {
  primitive_type_t *prims;
  point_t *points;
  int32_t nb_prims;
  int32_t max_prims;
  int32_t nb_points;
  int32_t max_points;
} path_t;

typedef struct path_iterator_t {
  path_t *path;
  primitive_type_t *prims;
  point_t *points;
} path_iterator_t;

extern const int32_t prim_points[NB_PRIMS];

#endif /* __PATH_INTERNAL_H */

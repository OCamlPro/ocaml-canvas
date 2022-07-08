/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __PATH2D_INTERNAL_H
#define __PATH2D_INTERNAL_H

#include "object.h"
#include "path.h"

typedef struct path2d_t {
  INHERITS_OBJECT;
  path_t *path;
  double first_tx; /* First transformed point */
  double first_ty; /* First transformed point */
  double first_x;  /* First untransformed point */
  double first_y;  /* First untransformed point */
  double last_x;   /* Last untransformed point */
  double last_y;   /* Last untransformed point */
} path2d_t;

#endif /* __PATH2D_INTERNAL_H */

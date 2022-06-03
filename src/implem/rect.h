/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __RECT_H
#define __RECT_H

#include "point.h"

typedef struct rect_t {
  point_t p1;
  point_t p2;
} rect_t;

#define rect(_p1,_p2) \
  ((rect_t){ .p1 = (_p1), .p2 = (_p2) })

void
rect_expand(
  rect_t *r,
  point_t p);

#endif /* __RECT_H */

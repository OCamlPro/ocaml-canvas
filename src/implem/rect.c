/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#include <stddef.h>
#include <assert.h>

#include "point.h"
#include "rect.h"

void
rect_expand(
  rect_t *r,
  point_t p)
{
  assert(r != NULL);

  if (p.x < r->p1.x) {
    r->p1.x = p.x;
  }
  if (p.x > r->p2.x) {
    r->p2.x = p.x;
  }
  if (p.y < r->p1.y) {
    r->p1.y = p.y;
  }
  if (p.y > r->p2.y) {
    r->p2.y = p.y;
  }
}

/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __QTZ_FONT_H
#define __QTZ_FONT_H

#include "../point.h"
#include "../rect.h"
#include "../polygon.h"
#include "../transform.h"
#include "../font_desc.h"

typedef struct qtz_font_t qtz_font_t;

qtz_font_t *
qtz_font_create(
  font_desc_t *fd);

void
qtz_font_destroy(
  qtz_font_t *f);

bool
qtz_font_char_as_poly(
  const qtz_font_t *f,
  const transform_t *t,
  uint32_t c,
  point_t *pen, // in/out
  polygon_t *p, // out
  rect_t *bbox); // out

#endif /* __QTZ_FONT_H */

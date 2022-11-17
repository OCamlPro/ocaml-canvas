/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __GDI_FONT_H
#define __GDI_FONT_H

#include <stdint.h>
#include <stdbool.h>

#include "../point.h"
#include "../rect.h"
#include "../polygon.h"
#include "../transform.h"
#include "../font_desc.h"

typedef struct gdi_font_t gdi_font_t;

gdi_font_t *
gdi_font_create(
  font_desc_t *fd);

void
gdi_font_destroy(
  gdi_font_t *f);

bool
gdi_font_char_as_poly(
  const gdi_font_t *f,
  const transform_t *t,
  uint32_t c,
  point_t *pen, // in/out
  polygon_t *p, // out
  rect_t *bbox); // out

#endif /* __GDI_FONT_H */

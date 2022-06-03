/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __FT_FONT_H
#define __FT_FONT_H

#include "../point.h"
#include "../rect.h"
#include "../polygon.h"
#include "../transform.h"
#include "../font_desc.h"

typedef struct ft_font_t ft_font_t;

ft_font_t *
ft_font_create(
  font_desc_t *fd);

void
ft_font_destroy(
  ft_font_t *f);

bool
ft_font_char_as_poly(
  const ft_font_t *f,
  const transform_t *t,
  uint32_t c,
  point_t *pen, // in/out
  polygon_t *p, // out
  rect_t *bbox); // out

#endif /* __FT_FONT_H */

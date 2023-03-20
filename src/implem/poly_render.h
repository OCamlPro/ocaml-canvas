/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __POLY_RENDER_H
#define __POLY_RENDER_H

#include <stdbool.h>

#include "rect.h"
#include "color.h"
#include "transform.h"
#include "draw_style.h"
#include "color_composition.h"
#include "state.h" // just shadow
#include "polygon.h"

void
poly_render_init(
  void);

void
poly_render(
  pixmap_t *pm,
  const polygon_t *p,
  const rect_t *bbox,
  draw_style_t draw_style,
  double global_alpha,
  const shadow_t *shadow,
  composite_operation_t compose_op,
  const pixmap_t *clip_region,
  bool non_zero,
  const transform_t *transform);

#endif /* __POLY_RENDER_H */

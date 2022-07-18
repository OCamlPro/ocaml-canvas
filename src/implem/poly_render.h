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

#include <stdint.h>
#include <stdbool.h>

#include "rect.h"
#include "color.h"
#include "transform.h"
#include "fill_style.h"
#include "color_composition.h"
#include "polygon.h"
#include "surface.h"

void
poly_render_init(
  void);

void
poly_render(
  surface_t *s,
  const polygon_t *p,
  const rect_t *bbox,
  fill_style_t fill_style,
  double global_alpha,
  composite_operation_t compose_op,
  bool non_zero,
  transform_t *transform);

#endif /* __POLY_RENDER_H */

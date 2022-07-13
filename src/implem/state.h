/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __STATE_H
#define __STATE_H

#include <stdint.h>

#include "color.h"
#include "transform.h"
#include "font_desc.h"
#include "draw_style.h"
#include "polygonize.h"
#include "color_composition.h"

typedef struct state_t {
  transform_t *transform;
  font_desc_t *font_desc; // font, textAlign, textBaseline, direction
  // outline region
  // dash array
  draw_style_t fill_style;
  draw_style_t stroke_style;
  composite_operation_t global_composite_operation;
  double global_alpha; // 0.0 - 1.0
  double line_width;
  join_type_t join_type; // lineJoin, miterLimit, lineDashOffset
  cap_type_t cap_type; // lineCap
  // shadowOffsetXn shadowOffsetY, shadowBlur, shadowColor
  // imageSmoothingEnabled
} state_t;

state_t *
state_create(
  void);

void
state_destroy(
  state_t *s);

void
state_reset(
  state_t *s);

state_t *
state_copy(
  const state_t *s);

#endif /* __STATE_H */

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

#include "list.h"
#include "color.h"
#include "transform.h"
#include "font_desc.h"
#include "draw_style.h"
#include "polygonize.h"
#include "surface.h"
#include "path2d.h"
#include "color_composition.h"
#include "draw_instr.h"

typedef struct state_t {
  transform_t *transform;
  font_desc_t *font_desc; // font, textAlign, textBaseline, direction
  list_t *clip_path;
  double *line_dash;
  int32_t line_dash_len;
  double line_dash_offset;
  double line_width;
  double global_alpha; // 0.0 - 1.0
  // outline region
  draw_style_t fill_style;
  draw_style_t stroke_style;
  color_t_ shadow_color;
  double shadow_offset_x;
  double shadow_offset_y;
  double shadow_blur;
  composite_operation_t global_composite_operation;
  join_type_t join_type; // lineJoin, miterLimit, lineDashOffset
  cap_type_t cap_type; // lineCap
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

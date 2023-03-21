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
#include "path2d.h"
#include "color_composition.h"

typedef struct shadow_t {
  double offset_x;
  double offset_y;
  double blur;
  color_t_ color;
} shadow_t;

typedef struct state_t {
  transform_t *transform;

  /* Polygonizer state */
  list_t *clip_path;
  font_desc_t *font_desc; // font, textAlign, textBaseline, direction
  join_type_t join_type; // lineJoin
  cap_type_t cap_type; // lineCap
  int32_t line_dash_len;
  double *line_dash;
  double line_dash_offset;
  double line_width;
  double miter_limit;

  /* Polygon renderer state */
  double global_alpha; // 0.0 - 1.0
  draw_style_t fill_style;
  draw_style_t stroke_style;
  shadow_t shadow;
  composite_operation_t global_composite_operation;

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

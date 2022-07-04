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
#include "fill_style.h"
#include "polygonize.h"

typedef struct state_t {
  transform_t *transform;
  font_desc_t *font_desc;
  // outline region
  // dash array
  fill_style_t fill_style;
  fill_style_t stroke_style;
  double global_alpha; // 0.0 - 1.0
  double line_width;
  join_type_t join_type;
  cap_type_t cap_type;
  // globalCompositeOperation (source-over, source-in, source-out...)
  // lineWidth, lineCap, lineJoin, miterLimit, lineDashOffset
  // shadowOffsetXn shadowOffsetY, shadowBlur, shadowColor
  // font, textAlign, textBaseline, direction
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

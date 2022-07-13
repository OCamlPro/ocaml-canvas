/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __DRAW_STYLE_H
#define __DRAW_STYLE_H

#include "color.h"
#include "gradient.h"
#include "pattern.h"

typedef enum draw_style_type_t {
  DRAW_STYLE_COLOR    = 0,
  DRAW_STYLE_GRADIENT = 1,
  DRAW_STYLE_PATTERN  = 2
} draw_style_type_t;

typedef union draw_style_content_t {
  color_t_ color;
  gradient_t *gradient;
  pattern_t *pattern;
} draw_style_content_t;

typedef struct draw_style_t {
  draw_style_type_t type;
  draw_style_content_t content;
} draw_style_t;

void
draw_style_destroy(
  draw_style_t *s);

draw_style_t
draw_style_copy(
  const draw_style_t *s);

#endif /*__DRAW_STYLE_H */

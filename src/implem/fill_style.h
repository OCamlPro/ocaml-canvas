/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __FILL_STYLE_H
#define __FILL_STYLE_H

#include "color.h"
#include "gradient.h"

typedef enum fill_type_t {
  FILL_TYPE_COLOR    = 0,
  FILL_TYPE_GRADIENT = 1
} fill_type_t;

typedef union fill_content_t {
  color_t_ color;
  gradient_t *gradient;
} fill_content_t;

typedef struct fill_style_t {
  fill_type_t fill_type;
  fill_content_t content;
} fill_style_t;

void
fill_style_destroy(
  fill_style_t *fs);

fill_style_t
fill_style_copy(
  const fill_style_t *fs);

#endif /*__FILL_STYLE_H */

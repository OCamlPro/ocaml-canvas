/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __HW_CONTEXT_H
#define __HW_CONTEXT_H

#include <stdint.h>
#include <stdbool.h>

#include "target.h"
#include "pixmap.h"

#include "rect.h"
#include "polygon.h"
#include "transform.h"
#include "draw_style.h"
#include "list.h"
#include "state.h" // for shadow_t

typedef struct hw_context_t hw_context_t;

hw_context_t *
hw_context_create(
  int32_t width,
  int32_t height);

// Creates a context from a pixmap
// The data pointer is transfered to the context
// (thus removed from the pixmap); if you kept
// a copy, do NOT free it !
hw_context_t *
hw_context_create_from_pixmap(
  pixmap_t *pixmap);

hw_context_t *
hw_context_create_onscreen(
  target_t *target,
  int32_t width,
  int32_t height);

void
hw_context_destroy(
  hw_context_t *c);

bool
hw_context_resize(
  hw_context_t *c,
  int32_t width,
  int32_t height);

void
hw_context_present(
  hw_context_t *c);

bool
hw_context_clip(
  hw_context_t *c,
  list_t *clip_path,
  const transform_t *transform);

void
hw_context_clear_clip(
  hw_context_t *c);

void
hw_context_render_polygon(
  hw_context_t *c,
  const polygon_t *p,
  const rect_t *bbox,
  draw_style_t draw_style,
  double global_alpha,
  const shadow_t *shadow,
  composite_operation_t compose_op,
  bool non_zero,
  const transform_t *transform);

void
hw_context_blit(
  hw_context_t *dc,
  int32_t dx,
  int32_t dy,
  const hw_context_t *sc,
  int32_t sx,
  int32_t sy,
  int32_t width,
  int32_t height,
  double global_alpha,
  const shadow_t *shadow,
  composite_operation_t compose_op,
  const transform_t *transform);

color_t_
hw_context_get_pixel(
  const hw_context_t *c,
  int32_t x,
  int32_t y);

void
hw_context_put_pixel(
  hw_context_t *c,
  int32_t x,
  int32_t y,
  color_t_ color);

pixmap_t
hw_context_get_pixmap(
  const hw_context_t *c,
  int32_t sx,
  int32_t sy,
  int32_t width,
  int32_t height);

void
hw_context_put_pixmap(
  hw_context_t *c,
  int32_t dx,
  int32_t dy,
  const pixmap_t *sp,
  int32_t sx,
  int32_t sy,
  int32_t width,
  int32_t height);

bool
hw_context_export_png(
  const hw_context_t *c,
  const char *filename); // as UTF-8

bool
hw_context_import_png(
  hw_context_t *c,
  int32_t x,
  int32_t y,
  const char *filename); // as UTF-8

#endif /* __HW_CONTEXT_H */

/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include "config.h"
#include "target.h"
#include "pixmap.h"
#include "color.h"

#include "rect.h"
#include "polygon.h"
#include "transform.h"
#include "draw_style.h"
#include "list.h"
#include "state.h" // for shadow_t

#ifdef HAS_ACCEL
#include "hw_context.h"
#endif
#include "sw_context.h"
#include "context_internal.h"

context_t *
context_create(
  int32_t width,
  int32_t height)
{
  assert(width > 0);
  assert(height > 0);

  switch_ACCEL() {
    case_HW(return (context_t *)hw_context_create(width, height));
    case_SW(return (context_t *)sw_context_create(width, height));
  }
}

context_t *
context_create_from_pixmap(
  pixmap_t *pixmap)
{
  assert(pixmap != NULL);
  assert(pixmap_valid(*pixmap) == true);

  switch_ACCEL() {
    case_HW(return (context_t *)hw_context_create_from_pixmap(pixmap));
    case_SW(return (context_t *)sw_context_create_from_pixmap(pixmap));
  }
}

context_t *
context_create_onscreen(
  target_t *target,
  int32_t width,
  int32_t height)
{
  assert(target != NULL);
  assert(width > 0);
  assert(height > 0);

  switch_ACCEL() {
    case_HW(
      return (context_t *)hw_context_create_onscreen(target, width, height));
    case_SW(
      return (context_t *)sw_context_create_onscreen(target, width, height));
  }
}

void
context_destroy(
  context_t *c)
{
  assert(c != NULL);
  assert(c->width > 0);
  assert(c->height > 0);

  switch_ACCEL() {
    case_HW(hw_context_destroy((hw_context_t *)c));
    case_SW(sw_context_destroy((sw_context_t *)c));
  }
}

bool
context_resize(
  context_t *c,
  int32_t width,
  int32_t height)
{
  assert(c != NULL);
  assert(c->width > 0);
  assert(c->height > 0);

  switch_ACCEL() {
    case_HW(return hw_context_resize((hw_context_t *)c, width, height));
    case_SW(return sw_context_resize((sw_context_t *)c, width, height));
  }
}

void
context_present(
  context_t *c)
{
  assert(c != NULL);
  assert(c->offscreen == false);
  assert(c->width > 0);
  assert(c->height > 0);

  switch_ACCEL() {
    case_HW(hw_context_present((hw_context_t *)c));
    case_SW(sw_context_present((sw_context_t *)c));
  }
}

bool
context_clip(
  context_t *c,
  list_t *clip_path,
  const transform_t *transform)
{
  assert(c != NULL);
  assert(clip_path != NULL);
  assert(transform != NULL);

  switch_ACCEL() {
    case_HW(return hw_context_clip((hw_context_t *)c, clip_path, transform));
    case_SW(return sw_context_clip((sw_context_t *)c, clip_path, transform));
  }
}

void
context_clear_clip(
  context_t *c)
{
  assert(c != NULL);

  switch_ACCEL() {
    case_HW(hw_context_clear_clip((hw_context_t *)c));
    case_SW(sw_context_clear_clip((sw_context_t *)c));
  }
}

void
context_render_polygon(
  context_t *c,
  const polygon_t *p,
  const rect_t *bbox,
  draw_style_t draw_style,
  double global_alpha,
  const shadow_t *shadow,
  composite_operation_t compose_op,
  bool non_zero,
  const transform_t *transform)
{
  assert(c != NULL);
  assert(p != NULL);
  assert(bbox != NULL);
  assert(shadow != NULL);
  assert(transform != NULL);

  switch_ACCEL() {
    case_HW(hw_context_render_polygon((hw_context_t *)c, p, bbox,
                                      draw_style, global_alpha, shadow,
                                      compose_op, non_zero, transform));
    case_SW(sw_context_render_polygon((sw_context_t *)c, p, bbox,
                                      draw_style, global_alpha, shadow,
                                      compose_op, non_zero, transform));
  }
}

void
context_blit(
  context_t *dc,
  int32_t dx,
  int32_t dy,
  const context_t *sc,
  int32_t sx,
  int32_t sy,
  int32_t width,
  int32_t height,
  double global_alpha,
  const shadow_t *shadow,
  composite_operation_t compose_op,
  const transform_t *transform)
{
  assert(dc != NULL);
  assert(sc != NULL);
  assert(shadow != NULL);
  assert(transform != NULL);

  switch_ACCEL() {
    case_HW(hw_context_blit((hw_context_t *)dc, dx, dy,
                            (hw_context_t *)sc, sx, sy, width, height,
                            global_alpha, shadow, compose_op, transform));
    case_SW(sw_context_blit((sw_context_t *)dc, dx, dy,
                            (sw_context_t *)sc, sx, sy, width, height,
                            global_alpha, shadow, compose_op, transform));
  }
}

color_t_
context_get_pixel(
  const context_t *c,
  int32_t x,
  int32_t y)
{
  assert(c != NULL);

  switch_ACCEL() {
    case_HW(return hw_context_get_pixel((hw_context_t *)c, x, y));
    case_SW(return sw_context_get_pixel((sw_context_t *)c, x, y));
  }
}

void
context_put_pixel(
  context_t *c,
  int32_t x,
  int32_t y,
  color_t_ color)
{
  assert(c != NULL);

  switch_ACCEL() {
    case_HW(hw_context_put_pixel((hw_context_t *)c, x, y, color));
    case_SW(sw_context_put_pixel((sw_context_t *)c, x, y, color));
  }
}

pixmap_t
context_get_pixmap(
  const context_t *c,
  int32_t sx,
  int32_t sy,
  int32_t width,
  int32_t height)
{
  assert(c != NULL);

  switch_ACCEL() {
    case_HW(return hw_context_get_pixmap((hw_context_t *)c,
                                         sx, sy, width, height));
    case_SW(return sw_context_get_pixmap((sw_context_t *)c,
                                         sx, sy, width, height));
  }
}

void
context_put_pixmap(
  context_t *c,
  int32_t dx,
  int32_t dy,
  const pixmap_t *sp,
  int32_t sx,
  int32_t sy,
  int32_t width,
  int32_t height)
{
  assert(c != NULL);
  assert(sp != NULL);
  assert(pixmap_valid(*sp) == true);

  switch_ACCEL() {
    case_HW(hw_context_put_pixmap((hw_context_t *)c, dx, dy,
                                  sp, sx, sy, width, height));
    case_SW(sw_context_put_pixmap((sw_context_t *)c, dx, dy,
                                  sp, sx, sy, width, height));
  }
}

bool
context_export_png(
  const context_t *c,
  const char *filename) // as UTF-8
{
  assert(c != NULL);
  assert(filename != NULL);

  switch_ACCEL() {
    case_HW(return hw_context_export_png((hw_context_t *)c, filename));
    case_SW(return sw_context_export_png((sw_context_t *)c, filename));
  }
}

bool
context_import_png(
  context_t *c,
  int32_t x,
  int32_t y,
  const char *filename) // as UTF-8
{
  assert(c != NULL);
  assert(filename != NULL);

  switch_ACCEL() {
    case_HW(return hw_context_import_png((hw_context_t *)c, x, y, filename));
    case_SW(return sw_context_import_png((sw_context_t *)c, x, y, filename));
  }
}

/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifdef HAS_ACCEL

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

#include "draw_instr.h"
#include "poly_render.h"
#include "impexp.h"

#ifdef HAS_GDI
#include "gdi/gdi_hw_context.h"
#endif
#ifdef HAS_QUARTZ
#include "quartz/qtz_hw_context.h"
#endif
#ifdef HAS_X11
#include "x11/x11_hw_context.h"
#endif
#ifdef HAS_WAYLAND
#include "wayland/wl_hw_context.h"
#endif

#include "context_internal.h"
#include "hw_context_internal.h"

hw_context_t *
hw_context_create(
  int32_t width,
  int32_t height)
{
  assert(width > 0);
  assert(height > 0);

  hw_context_t *c = (hw_context_t *)calloc(1, sizeof(hw_context_t));
  if (c == NULL) {
    return NULL;
  }

  c->base.offscreen = true;
  c->base.width = width;
  c->base.height = height;

  return c;
}

hw_context_t *
hw_context_create_from_pixmap(
  pixmap_t *pixmap)
{
  assert(pixmap != NULL);
  assert(pixmap_valid(*pixmap) == true);

  hw_context_t *c = (hw_context_t *)calloc(1, sizeof(hw_context_t));
  if (c == NULL) {
    return NULL;
  }

  c->base.offscreen = true;
  c->base.width = pixmap->width;
  c->base.height = pixmap->height;

  free(pixmap->data);
  pixmap->data = NULL;
  pixmap->width = 0;
  pixmap->height = 0;

  return c;
}

hw_context_t *
hw_context_create_onscreen(
  target_t *target,
  int32_t width,
  int32_t height)
{
  assert(target != NULL);
  assert(width > 0);
  assert(height > 0);

  hw_context_t *c = NULL;
  switch_IMPL() {
    case_GDI(
      c = (hw_context_t *)gdi_hw_context_create((gdi_target_t *)target,
                                                width, height));
    case_QUARTZ(
      c = (hw_context_t *)qtz_hw_context_create((qtz_target_t *)target,
                                                width, height));
    case_X11(
      c = (hw_context_t *)x11_hw_context_create((x11_target_t *)target,
                                                width, height));
    case_WAYLAND(
      c = (hw_context_t *)wl_hw_context_create((wl_target_t *)target,
                                               width, height));
    default_fail();
  }
  if (c == NULL) {
    return NULL;
  }

  c->base.offscreen = false;

  return c;
}

void
hw_context_destroy(
  hw_context_t *c)
{
  assert(c != NULL);
  assert(c->data != NULL);

  if (c->base.offscreen == true) {
    free(c);
  } else {
    switch_IMPL() {
      case_GDI(gdi_hw_context_destroy((gdi_hw_context_t *)c));
      case_QUARTZ(qtz_hw_context_destroy((qtz_hw_context_t *)c));
      case_X11(x11_hw_context_destroy((x11_hw_context_t *)c));
      case_WAYLAND(wl_hw_context_destroy((wl_hw_context_t *)c));
      default_fail();
    }
  }
}

bool
hw_context_resize(
  hw_context_t *c,
  int32_t width,
  int32_t height)
{
  assert(c != NULL);
  assert(c->base.width > 0);
  assert(c->base.height > 0);

  if ((width <= 0) || (height <= 0)) {
    return false;
  }

  if ((width == c->base.width) && (height == c->base.height)) {
    return true;
  }

// TODO: fill extra data with background color

  if (c->base.offscreen == true) {

    c->base.width = width;
    c->base.height = height;

    return true;

  } else {
    bool result = false;
    switch_IMPL() {
      case_GDI(result =
               gdi_hw_context_resize((gdi_hw_context_t *)c, width, height));
      case_QUARTZ(result =
                  qtz_hw_context_resize((qtz_hw_context_t *)c, width, height));
      case_X11(result =
               x11_hw_context_resize((x11_hw_context_t *)c, width, height));
      case_WAYLAND(result =
                   wl_hw_context_resize((wl_hw_context_t *)c, width, height));
      default_fail();
    }

    return result;
  }
}

void
hw_context_present(
  hw_context_t *c)
{
  assert(c != NULL);
  assert(c->base.offscreen == false);
  assert(c->base.width > 0);
  assert(c->base.height > 0);

  switch_IMPL() {
    case_GDI(gdi_hw_context_present((gdi_hw_context_t *)c));
    case_QUARTZ(qtz_hw_context_present((qtz_hw_context_t *)c));
    case_X11(x11_hw_context_present((x11_hw_context_t *)c));
    case_WAYLAND(wl_hw_context_present((wl_hw_context_t *)c));
    default_fail();
  }
}

static void
_hw_context_clip_fill_instr(
  hw_context_t *c,
  const path_fill_instr_t *instr,
  const transform_t *transform)
{
  assert(c != NULL);
  assert(instr != NULL);
  assert(instr->poly != NULL);
  assert(transform != NULL);

}

bool
hw_context_clip(
  hw_context_t *c,
  list_t *clip_path,
  const transform_t *transform)
{
  assert(c != NULL);
  assert(clip_path != NULL);
  assert(transform != NULL);

  list_iterator_t *it = list_get_iterator(clip_path);
  if (it == NULL) {
    return false;
  }

  path_fill_instr_t *instr = NULL;
  while ((instr = (path_fill_instr_t *)list_iterator_next(it)) != NULL) {
    _hw_context_clip_fill_instr(c, instr, transform);
  }

  list_free_iterator(it);

  return true;
}

void
hw_context_clear_clip(
  hw_context_t *c)
{
  assert(c != NULL);

}

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
  const transform_t *transform)
{
  assert(c != NULL);
  assert(p != NULL);
  assert(bbox != NULL);
  assert(shadow != NULL);
  assert(transform != NULL);

}

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
  const transform_t *transform)
{
  assert(dc != NULL);
  assert(sc != NULL);
  assert(shadow != NULL);
  assert(transform != NULL);

}

color_t_
hw_context_get_pixel(
  const hw_context_t *c,
  int32_t x,
  int32_t y)
{
  assert(c != NULL);

  color_t_ color = color_black;

  return color;
}

void
hw_context_put_pixel(
  hw_context_t *c,
  int32_t x,
  int32_t y,
  color_t_ color)
{
  assert(c != NULL);

}

pixmap_t
hw_context_get_pixmap(
  const hw_context_t *c,
  int32_t sx,
  int32_t sy,
  int32_t width,
  int32_t height)
{
  assert(c != NULL);

  pixmap_t dp = pixmap_null();

  return dp;
}

void
hw_context_put_pixmap(
  hw_context_t *c,
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

}

bool
hw_context_export_png(
  const hw_context_t *c,
  const char *filename) // as UTF-8
{
  assert(c != NULL);
  assert(filename != NULL);

  const pixmap_t pm = pixmap_null();

  return impexp_export_png(&pm, filename);
}

bool
hw_context_import_png(
  hw_context_t *c,
  int32_t x,
  int32_t y,
  const char *filename) // as UTF-8
{
  assert(c != NULL);
  assert(filename != NULL);

  pixmap_t pm = pixmap_null();

  return impexp_import_png(&pm, x, y, filename);
}

#else

const int hw_context = 0;

#endif /* HAS_ACCEL */

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
#include "context.h"
#include "context_internal.h"

#include "list.h"
#include "rect.h"
#include "polygon.h"
#include "state.h" // for shadow_t
#include "transform.h"
#include "draw_style.h"
#include "draw_instr.h"
#include "poly_render.h"
#include "impexp.h"

#ifdef HAS_GDI
#include "gdi/gdi_context.h"
#endif
#ifdef HAS_QUARTZ
#include "quartz/qtz_context.h"
#endif
#ifdef HAS_X11
#include "x11/x11_context.h"
#endif
#ifdef HAS_WAYLAND
#include "wayland/wl_context.h"
#endif

context_t *
context_create(
  int32_t width,
  int32_t height)
{
  assert(width > 0);
  assert(height > 0);

  color_t_ *data = (color_t_ *)calloc(width * height, sizeof(color_t_));
  if (data == NULL) {
    return NULL;
  }

  context_t *c = (context_t *)calloc(1, sizeof(context_t));
  if (c == NULL) {
    free(data);
    return NULL;
  }

  c->offscreen = true;
  c->data = data;
  c->width = width;
  c->height = height;

  c->clip_region = pixmap_null();

  return c;
}

context_t *
context_create_from_pixmap(
  pixmap_t *pixmap)
{
  assert(pixmap != NULL);
  assert(pixmap_valid(*pixmap) == true);

  context_t *c = (context_t *)calloc(1, sizeof(context_t));
  if (c == NULL) {
    return NULL;
  }

  c->offscreen = true;
  c->data = pixmap->data;
  c->width = pixmap->width;
  c->height = pixmap->height;

  c->clip_region = pixmap_null();

  pixmap->data = NULL;
  pixmap->width = 0;
  pixmap->height = 0;

  return c;
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

  context_t *c = NULL;
  switch_IMPL() {
    case_GDI(c = (context_t *)gdi_context_create((gdi_target_t *)target,
                                                 width, height));
    case_QUARTZ(c = (context_t *)qtz_context_create((qtz_target_t *)target,
                                                    width, height));
    case_X11(c = (context_t *)x11_context_create((x11_target_t *)target,
                                                 width, height));
    case_WAYLAND(c = (context_t *)wl_context_create((wl_target_t *)target,
                                                    width, height));
    default_fail();
  }
  if (c == NULL) {
    return NULL;
  }
  c->offscreen = false;

  c->clip_region = pixmap_null();

  return c;
}

void
context_destroy(
  context_t *c)
{
  assert(c != NULL);
  assert(c->data != NULL);

  if (pixmap_valid(c->clip_region) == true) {
    pixmap_destroy(c->clip_region);
  }

  if (c->offscreen == true) {
    free(c->data);
    free(c);
  } else {
    switch_IMPL() {
      case_GDI(gdi_context_destroy((gdi_context_t *)c));
      case_QUARTZ(qtz_context_destroy((qtz_context_t *)c));
      case_X11(x11_context_destroy((x11_context_t *)c));
      case_WAYLAND(wl_context_destroy((wl_context_t *)c));
      default_fail();
    }
  }
}

void
_context_copy_to_buffer(
  context_t *c,
  color_t_ *data,
  int32_t width,
  int32_t height)
{
  assert(c != NULL);
  assert(c->data != NULL);
  assert(c->width > 0);
  assert(c->height > 0);
  assert(data != NULL);
  assert(width > 0);
  assert(height > 0);

  uint32_t min_width = width < c->width ? width : c->width;
  uint32_t min_height = height < c->height ? height : c->height;
  for (size_t i = 0; i < min_height; ++i) {
    for (size_t j = 0; j < min_width; ++j) {
      data[i * width + j] = c->data[i * c->width + j];
    }
  }
}

bool
context_resize(
  context_t *c,
  int32_t width,
  int32_t height)
{
  assert(c != NULL);
  assert(c->data != NULL);
  assert(c->width > 0);
  assert(c->height > 0);

  if ((width <= 0) || (height <= 0)) {
    return false;
  }

  if ((width == c->width) && (height == c->height)) {
    return true;
  }

  if (pixmap_valid(c->clip_region)) {
    pixmap_destroy(c->clip_region);
  }

// TODO: fill extra data with background color

  if (c->offscreen == true) {

    color_t_ *data = (color_t_ *)calloc(width * height, sizeof(color_t_));
    if (data == NULL) {
      return false;
    }

    _context_copy_to_buffer(c, data, width, height);

    free(c->data);

    c->data = data;
    c->width = width;
    c->height = height;

    return true;

  } else {

    bool result = false;

    switch_IMPL() {
      case_GDI(result =
               gdi_context_resize((gdi_context_t *)c, width, height));
      case_QUARTZ(result =
                  qtz_context_resize((qtz_context_t *)c, width, height));
      case_X11(result =
               x11_context_resize((x11_context_t *)c, width, height));
      case_WAYLAND(result =
                   wl_context_resize((wl_context_t *)c, width, height));
      default_fail();
    }

    return result;
  }
}

void
context_present(
  context_t *c)
{
  assert(c != NULL);
  assert(c->offscreen == false);
  assert(c->data != NULL);
  assert(c->width > 0);
  assert(c->height > 0);

  switch_IMPL() {
    case_GDI(gdi_context_present((gdi_context_t *)c));
    case_QUARTZ(qtz_context_present((qtz_context_t *)c));
    case_X11(x11_context_present((x11_context_t *)c));
    case_WAYLAND(wl_context_present((wl_context_t *)c));
    default_fail();
  }
}

// Direct access to the context pixels
// Do NOT free the data pointer !
static pixmap_t
_context_get_raw_pixmap(
  context_t *c)
{
  assert(c != NULL);
  assert(c->data != NULL);
  assert(c->width > 0);
  assert(c->height > 0);

  return pixmap(c->width, c->height, c->data);
}



static void
_context_clip_fill_instr(
  context_t *c,
  const path_fill_instr_t *instr,
  const transform_t *transform)
{
  assert(c != NULL);
  assert(pixmap_valid(c->clip_region) == true);
  assert(instr != NULL);
  assert(instr->poly != NULL);
  assert(transform != NULL);

  draw_style_t white = (draw_style_t){ .type = DRAW_STYLE_COLOR,
                                       .content.color = color_white };
  shadow_t noshadow = (shadow_t){
    .offset_x = 0, .offset_y = 0, .blur = 0,
    .color = color_transparent_black };

// TODO: could store bounding box with polygon
  rect_t bbox = rect(point(0.0, 0.0),
                     point((double)c->width, (double)c->height));

  poly_render(&(c->clip_region), instr->poly, &bbox, white, 1.0, &noshadow,
              ONE_MINUS_SRC, NULL, instr->non_zero, transform);
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

  if ((pixmap_valid(c->clip_region) == true) &&
      ((c->clip_region.width != c->width) ||
       (c->clip_region.height != c->height))) {
    pixmap_destroy(c->clip_region);
  }

  if (pixmap_valid(c->clip_region) == false) {
    c->clip_region = pixmap(c->width, c->height, NULL);
    if (pixmap_valid(c->clip_region) == false) {
      return false;
    }
  }

  pixmap_clear(c->clip_region);

  list_iterator_t *it = list_get_iterator(clip_path);
  if (it == NULL) {
    return false;
  }

  path_fill_instr_t *instr = NULL;
  while ((instr = (path_fill_instr_t *)list_iterator_next(it)) != NULL) {
    _context_clip_fill_instr(c, instr, transform);
  }

  list_free_iterator(it);

  return true;
}

void
context_clear_clip(
  context_t *c)
{
  assert(c != NULL);

  if (pixmap_valid(c->clip_region) == true) {
    pixmap_destroy(c->clip_region);
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
  pixmap_t pm = pixmap(c->width, c->height, c->data);
  poly_render(&pm, p, bbox, draw_style, global_alpha, shadow, compose_op,
              &(c->clip_region), non_zero, transform);
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

  bool draw_shadows =
    (shadow->blur > 0.0 ||
     shadow->offset_x != 0.0 || shadow->offset_y != 0.0) &&
    compose_op != COPY && shadow->color.a != 0;

  const pixmap_t sp = _context_get_raw_pixmap((context_t *)sc);
  pixmap_t dp = _context_get_raw_pixmap(dc);

// TODO: global_alpha ?
  if ((transform_is_pure_translation(transform) == true) &&
      (draw_shadows == false)) {

    double tx = 0.0, ty = 0.0;
    transform_extract_translation(transform, &tx, &ty);

    int32_t lo_x = max(dx + (int32_t)tx, 0);
    int32_t hi_x = min(dx + (int32_t)tx + width, dc->width); // canvas wd
    int32_t lo_y = max(dy + (int32_t)ty, 0);
    int32_t hi_y = min(dy + (int32_t)ty + height, dc->height); //canvas ht

    for (int32_t i = lo_x; i < hi_x; i++) {
      for (int32_t j = lo_y; j < hi_y; j++) {

        int32_t uvx = i + sx - dx - (int32_t)tx;
        int32_t uvy = j + sy - dy - (int32_t)ty;
        if (uvx < 0 || uvx >= sc->width || // canvas wd
            uvy < 0 || uvy >= sc->height) { // canvas ht
          continue;
        }

        color_t_ fill_color = pixmap_at(sp, uvy, uvx);
        int draw_alpha = fill_color.a;
        if (pixmap_valid(dc->clip_region) == true) {
          draw_alpha *= 255 - pixmap_at(dc->clip_region, j, i).a;
          draw_alpha /= 255;
        }

        pixmap_at(dp, j, i) =
          comp_compose(fill_color, pixmap_at(dp, j, i),
                       draw_alpha, compose_op);
      }
    }

  } else {

    draw_style_t draw_style =
      (draw_style_t){ .type = DRAW_STYLE_PIXMAP, .content.pixmap = &sp };

    polygon_t *p = polygon_create(8, 1);
    if (p == NULL) {
      return;
    }

    point_t p1 = point((double)dx, (double)dy);
    point_t p2 = point((double)(dx + width), (double)dy);
    point_t p3 = point((double)(dx + width), (double)(dy + height));
    point_t p4 = point((double)dx, (double)(dy + height));

    transform_apply(transform, &p1);
    transform_apply(transform, &p2);
    transform_apply(transform, &p3);
    transform_apply(transform, &p4);

    polygon_add_point(p, p1);
    polygon_add_point(p, p2);
    polygon_add_point(p, p3);
    polygon_add_point(p, p4);
    polygon_end_subpoly(p, true);

    rect_t bbox = rect(point(min4(p1.x, p2.x, p3.x, p4.x),
                             min4(p1.y, p2.y, p3.y, p4.y)),
                       point(max4(p1.x, p2.x, p3.x, p4.x),
                             max4(p1.y, p2.y, p3.y, p4.y)));

    transform_t *temp_transform = transform_copy(transform);
    transform_translate(temp_transform, dx - sx, dy - sy);

    pixmap_t pm = _context_get_raw_pixmap(dc);
    poly_render(&pm, p, &bbox, draw_style, global_alpha, shadow, compose_op,
                &(dc->clip_region), false, temp_transform);

    transform_destroy(temp_transform);

    polygon_destroy(p);
  }
}



color_t_
context_get_pixel(
  const context_t *c,
  int32_t x,
  int32_t y)
{
  assert(c != NULL);

  color_t_ color = color_black;

  const pixmap_t pm = _context_get_raw_pixmap((context_t *)c);
  if (pixmap_valid(pm) == true) {
    if ((x >= 0) && (x < pm.width) && (y >= 0) && (y < pm.height)) {
      color = pixmap_at(pm, y, x);
    }
  }

  return color;
}

void
context_put_pixel(
  context_t *c,
  int32_t x,
  int32_t y,
  color_t_ color)
{
  assert(c != NULL);

  pixmap_t pm = _context_get_raw_pixmap((context_t *)c);
  if (pixmap_valid(pm) == true) {
    if ((x >= 0) && (x < pm.width) && (y >= 0) && (y < pm.height)) {
      pixmap_at(pm, y, x) = color;
    }
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
  assert(context != NULL);

  pixmap_t dp = pixmap_null();
  const pixmap_t sp = _context_get_raw_pixmap((context_t *)c);
  if (pixmap_valid(sp) == true) {
    dp = pixmap(width, height, NULL);
    if (pixmap_valid(dp) == true) {
      pixmap_blit(&dp, 0, 0, &sp, sx, sy, width, height);
    }
  }
  return dp;
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

  pixmap_t dp = _context_get_raw_pixmap(c);
  if (pixmap_valid(dp) == true) {
    pixmap_blit(&dp, dx, dy, sp, sx, sy, width, height);
  }
}

bool
context_export_png(
  const context_t *c,
  const char *filename) // as UTF-8
{
  assert(c != NULL);
  assert(filename != NULL);

  const pixmap_t pm = _context_get_raw_pixmap((context_t *)c);
  if (pixmap_valid(pm) == false) {
    return false;
  }
  return impexp_export_png(&pm, filename);
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

  pixmap_t pm = _context_get_raw_pixmap(c);
  if (pixmap_valid(pm) == false) {
    return false;
  }
  return impexp_import_png(&pm, x, y, filename);
}
